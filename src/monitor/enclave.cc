#include "enclave.h"

#include "bare/memory.h"
#include "bare/page_tables.h"
#include "cpu_core_inl.h"
#include "dram_regions_inl.h"
#include "enclave_inl.h"

using sanctum::api::api_result_t;
using sanctum::api::enclave_id_t;
using sanctum::api::null_enclave_id;
using sanctum::api::monitor_ok;
using sanctum::api::monitor_concurrent_call;
using sanctum::api::monitor_invalid_state;
using sanctum::api::monitor_invalid_value;
using sanctum::api::enclave_id_t;
using sanctum::api::os::dram_region_free;
using sanctum::api::os::dram_region_owned;
using sanctum::api::thread_id_t;
using sanctum::bare::atomic_fetch_add;
using sanctum::bare::is_page_aligned;
using sanctum::bare::page_size;
using sanctum::bare::phys_ptr;
using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;
using sanctum::internal::bzero_dram_region;
using sanctum::internal::clamped_dram_region_for;
using sanctum::internal::clear_dram_region_lock;
using sanctum::internal::core_info_t;
using sanctum::internal::current_core_info;
using sanctum::internal::dram_region_for;
using sanctum::internal::dram_region_info_t;
using sanctum::internal::enclave_info_t;
using sanctum::internal::enclave_region_bitmap;
using sanctum::internal::enclave_thread_slot;
using sanctum::internal::free_enclave_id;
using sanctum::internal::g_dram_region;
using sanctum::internal::g_dram_region_count;
using sanctum::internal::is_dram_address;
using sanctum::internal::is_enclave_monitor_address;
using sanctum::internal::is_valid_enclave_id;
using sanctum::internal::read_dram_region_owner;
using sanctum::internal::read_enclave_region_bitmap_bit;
using sanctum::internal::test_and_set_dram_region_lock;
using sanctum::internal::thread_private_info_pages;
using sanctum::internal::thread_private_info_t;
using sanctum::internal::thread_slot_t;

namespace sanctum {
namespace internal {  // sanctum::internal

phys_ptr<size_t> g_os_region_bitmap{0};

};  // namespace sanctum::internal
};  // namespace sanctum

namespace sanctum {
namespace api {  // sanctum::api

size_t thread_info_pages() {
  return thread_private_info_pages();
}

};  // namespace sanctum::api
};  // namespace sanctum

namespace sanctum {
namespace api {  // sanctum::api
namespace os {  // sancum::api::os

api_result_t delete_enclave(enclave_id_t enclave_id) {
  size_t dram_region = clamped_dram_region_for(enclave_id);
  if (test_and_set_dram_region_lock(dram_region))
    return monitor_concurrent_call;

  // NOTE: null_enclave_id is accepted by is_valid_enclave_id, but does not
  //       have a useful meaning here
  if (enclave_id == null_enclave_id || !is_valid_enclave_id(enclave_id)) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_value;
  }

  phys_ptr<enclave_info_t> enclave_info{enclave_id};
  if (atomic_load(&(enclave_info->*(&enclave_info_t::running_threads))) != 0) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_state;
  }

  phys_ptr<size_t> region_bitmap = enclave_region_bitmap(enclave_id);
  size_t region_iterator = 0;
  for (; region_iterator < g_dram_region_count; ++region_iterator) {
    if (region_iterator == dram_region)
      continue;  // We've already locked the enclave's main DRAM region.
    if (!read_bitmap_bit(region_bitmap, region_iterator))
      continue;  // This region does not belong to the enclave.
    if (test_and_set_dram_region_lock(region_iterator))
      break;  // Failed to acquire lock on region.
  }
  if (region_iterator < g_dram_region_count) {
    // We failed to acquire a DRAM region lock. Unlock everything we touched.
    for (size_t i = 0; i < region_iterator; ++i) {
      if (i == dram_region)
        continue;  // We've already locked the enclave's main DRAM region.
      if (!read_bitmap_bit(region_bitmap, i))
        continue;  // This region does not belong to the enclave.
      clear_dram_region_lock(i);
    }
    clear_dram_region_lock(dram_region);
    return monitor_concurrent_call;
  }

  // NOTE: we know that no enclave thread is running, so we can free the
  //       enclave's DRAM regions directly, without going through the blocking
  //       state
  for (size_t i = 0; i < g_dram_region_count; ++i) {
    if (!read_bitmap_bit(region_bitmap, region_iterator))
      continue;  // This region does not belong to the enclave.

    phys_ptr<dram_region_info_t> region = &g_dram_region[i];
    atomic_store(&(region->*(&dram_region_info_t::owner)), free_enclave_id);

    // NOTE: The enclave's DRAM regions have monitor-reserved pages and pinned
    //       pages, due to threads. The rest of the system assumes that both
    //       monitor_pages and pinned_pages are zero for a DRAM-region once the
    //       region is blocked, which is a prerequisite to the DRAM region
    //       being freed. So, we must make sure that these numbers to zero for
    //       the DRAM regions that we free.
    region->*(&dram_region_info_t::monitor_pages) = 0;
    region->*(&dram_region_info_t::pinned_pages) = 0;

    bzero_dram_region(i);
  }

  for (size_t i = 0; i < g_dram_region_count; ++i) {
    if (i == dram_region)
      continue;  // We've already locked the enclave's main DRAM region.
    if (!read_bitmap_bit(region_bitmap, i))
      continue;  // This region does not belong to the enclave.
    clear_dram_region_lock(i);
  }
  clear_dram_region_lock(dram_region);
  return monitor_ok;
}

api_result_t run_enclave_thread(enclave_id_t enclave_id,
    thread_id_t thread_id) {
  size_t dram_region = clamped_dram_region_for(enclave_id);
  if (test_and_set_dram_region_lock(dram_region))
    return monitor_concurrent_call;

  // NOTE: null_enclave_id is accepted by is_valid_enclave_id, but does not
  //       have a useful meaning here
  if (enclave_id == null_enclave_id || !is_valid_enclave_id(enclave_id)) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_value;
  }

  phys_ptr<enclave_info_t> enclave_info{enclave_id};
  if (thread_id >= enclave_info->*(&enclave_info_t::max_threads)) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_value;
  }

  if (enclave_info->*(&enclave_info_t::is_initialized) == 0) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_state;
  }

  phys_ptr<thread_slot_t> slot{enclave_thread_slot(enclave_id, thread_id)};
  if (atomic_flag_test_and_set(&(slot->*(&thread_slot_t::lock)))) {
    clear_dram_region_lock(dram_region);
    return monitor_concurrent_call;
  }

  phys_ptr<thread_private_info_t> thread =
      slot->*(&thread_slot_t::thread_info);
  if (thread == phys_ptr<thread_private_info_t>::null()) {
    atomic_flag_clear(&(slot->*(&thread_slot_t::lock)));
    clear_dram_region_lock(dram_region);
    return monitor_invalid_state;
  }

  atomic_fetch_add(&(enclave_info->*(&enclave_info_t::running_threads)),
      static_cast<size_t>(1));
  clear_dram_region_lock(dram_region);

  phys_ptr<core_info_t> core{current_core_info()};
  core->*(&core_info_t::enclave_id) = enclave_id;
  core->*(&core_info_t::thread_id) = thread_id;
  core->*(&core_info_t::thread) = thread;
  set_eptbr(enclave_info->*(&enclave_info_t::loading_eptbr));


  // TODO: modify the CPU state to perform an enclave jump

  return monitor_ok;
}

api_result_t debug_enclave_copy_page(enclave_id_t enclave_id,
    uintptr_t enclave_addr, uintptr_t os_addr, bool read_from_enclave) {
  if (!is_page_aligned(enclave_addr) || !is_page_aligned(os_addr))
    return monitor_invalid_value;
  if (!is_dram_address(enclave_addr) || !is_dram_address(os_addr))
    return monitor_invalid_value;

  size_t enclave_dram_region = clamped_dram_region_for(enclave_id);
  size_t enclave_addr_dram_region = dram_region_for(enclave_addr);
  size_t os_addr_dram_region = dram_region_for(os_addr);

  if (test_and_set_dram_region_lock(enclave_dram_region))
    return monitor_concurrent_call;

  // NOTE: We don't need to check if os_dram_region is the same as
  //       enclave_dram_region. If that's the case, we'll simply fail to
  //       acquire the lock and return concurrent_call. This is acceptable.
  //       Ideally, we'd return invalid_value, but that'd increase code size.
  if (test_and_set_dram_region_lock(os_addr_dram_region)) {
    clear_dram_region_lock(enclave_dram_region);
    return monitor_concurrent_call;
  }

  api_result_t result = monitor_ok;
  if (read_dram_region_owner(enclave_dram_region) != enclave_id)
    result = monitor_invalid_value;
  if (read_dram_region_owner(os_addr_dram_region) != null_enclave_id)
    result = monitor_invalid_value;

  // NOTE: We don't need to lock enclave_addr's DRAM region because we have the
  //       enclave's DRAM region lock, so the DRAM region cannot be added to or
  //       removed (blocked) from the enclave while this call is happening.
  if (!read_enclave_region_bitmap_bit(enclave_id, enclave_addr_dram_region))
    result = monitor_invalid_value;

  phys_ptr<enclave_info_t> enclave_info{enclave_id};
  if (enclave_info->*(&enclave_info_t::is_debug) == 0)
    result = monitor_invalid_state;

  // We can't allow the OS to write monitor-reserved pages, even for debug
  // enclaves. The monitor-reserved pages contain physical addresses and the
  // enclave's DRAM region bitmap, which are trusted. Allowing the OS to write
  // them would enalbe attacks on other (possibly non-debug) enclaves.
  if (is_enclave_monitor_address(enclave_addr, enclave_id) &&
      read_from_enclave == false)
    result = monitor_access_denied;

  if (result == monitor_ok) {
    phys_ptr<size_t> enclave_ptr{enclave_addr};
    phys_ptr<size_t> enclave_end{enclave_addr + page_size()};
    phys_ptr<size_t> os_ptr{os_addr};
    if (read_from_enclave) {
      for (; enclave_ptr != enclave_end; enclave_ptr += 1, os_ptr += 1) {
        *os_ptr = *enclave_ptr;
      }
    } else {
      for (; enclave_ptr != enclave_end; enclave_ptr += 1, os_ptr += 1) {
        *enclave_ptr = *os_ptr;
      }
    }
  }

  clear_dram_region_lock(os_addr_dram_region);
  clear_dram_region_lock(enclave_dram_region);
  return result;
}

};  // namespace sanctum::api::os
};  // namespace sanctum::api
};  // namespace sanctum

namespace sanctum {
namespace api {  // sanctum::api
namespace enclave {  // sanctum::api::enclave

api_result_t create_enclave_thread(thread_id_t thread_id,
    uintptr_t phys_addr) {
  return monitor_ok;
}

api_result_t delete_enclave_thread(thread_id_t thread_id) {
  return monitor_ok;
}

api_result_t exit_enclave() {
  return monitor_ok;
}

};  // namespace sanctum::api::enclave
};  // namespace sanctum::api
};  // namespace sanctum
