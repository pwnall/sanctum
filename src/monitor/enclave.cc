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
using sanctum::bare::bzero;
using sanctum::bare::atomic_fetch_add;
using sanctum::bare::is_page_aligned;
using sanctum::bare::page_size;
using sanctum::bare::page_shift;
using sanctum::bare::page_table_levels;
using sanctum::bare::page_table_size;
using sanctum::bare::phys_ptr;
using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;
using sanctum::bare::write_page_table_entry;
using sanctum::internal::clamped_dram_region_for;
using sanctum::internal::clear_dram_region_lock;
using sanctum::internal::core_info_t;
using sanctum::internal::current_core_info;
using sanctum::internal::dram_region_for;
using sanctum::internal::dram_region_info_t;
using sanctum::internal::dram_region_start;
using sanctum::internal::enclave_info_t;
using sanctum::internal::enclave_monitor_area_pages;
using sanctum::internal::enclave_monitor_area_size;
using sanctum::internal::enclave_thread_slot;
using sanctum::internal::enclave_thread_slots;
using sanctum::internal::g_dram_region;
using sanctum::internal::g_dram_region_shift;
using sanctum::internal::is_dram_address;
using sanctum::internal::is_enclave_virtual_address;
using sanctum::internal::is_valid_range;
using sanctum::internal::is_valid_enclave_id;
using sanctum::internal::read_enclave_region_bitmap_bit;
using sanctum::internal::test_and_set_dram_region_lock;
using sanctum::internal::thread_private_info_t;
using sanctum::internal::thread_slot_t;
using sanctum::internal::walk_page_table_to_entry;

namespace sanctum {
namespace internal {  // sanctum::internal

phys_ptr<size_t> g_os_region_bitmap{0};

};  // namespace sanctum::internal
};  // namespace sanctum

namespace sanctum {
namespace api {  // sanctum::api
namespace os {  // sancum::api::os

enclave_id_t create_enclave(size_t dram_region, uintptr_t ev_base,
    uintptr_t ev_mask, size_t max_threads) {
  if (!is_valid_range(ev_base, ev_mask))
    return null_enclave_id;  // monitor_invalid_value
  if (ev_mask + 1 < page_size())
    return null_enclave_id;  // monitor_invalid_value

  // The enclave's monitor area must not cross DRAM regions.
  size_t monitor_area_size = enclave_monitor_area_size(max_threads);
  size_t monitor_area_pages = enclave_monitor_area_pages(max_threads);
  if (enclave_monitor_area_size(max_threads) >= (1 << g_dram_region_shift)) {
    return null_enclave_id;  // monitor_invalid_value
  }

  enclave_id_t enclave_id{dram_region_start(dram_region)};
  phys_ptr<enclave_info_t> enclave_info{enclave_id};
  phys_ptr<thread_slot_t> thread_slots{enclave_thread_slots(enclave_id)};

  if (test_and_set_dram_region_lock(dram_region)) {
    return monitor_concurrent_call;
  }

  phys_ptr<dram_region_info_t> region = &g_dram_region[dram_region];
  if (region->*(&dram_region_info_t::state) == dram_region_free) {
    region->*(&dram_region_info_t::state) = dram_region_owned;
    atomic_store(&(region->*(&dram_region_info_t::owner)), enclave_id);
    region->*(&dram_region_info_t::monitor_pages) = monitor_area_pages;
    region->*(&dram_region_info_t::pinned_pages) = monitor_area_pages;

    enclave_info->*(&enclave_info_t::max_threads) = max_threads;
    for (size_t i = 0; i < max_threads; ++i) {
      phys_ptr<thread_slot_t> slot = &thread_slots[i];
      slot->*(&thread_slot_t::thread_info) =
          phys_ptr<thread_private_info_t>::null();
      atomic_flag_clear(&(slot->*(&thread_slot_t::lock)));
    }

    enclave_info->*(&enclave_info_t::is_debug) = 0;

    enclave_info->*(&enclave_info_t::ev_base) = ev_base;
    enclave_info->*(&enclave_info_t::ev_mask) = ev_mask;
    enclave_info->*(&enclave_info_t::is_initialized) = 0;
    enclave_info->*(&enclave_info_t::loading_eptbr) = 0;
    enclave_info->*(&enclave_info_t::loading_last_addr) = 0;
    enclave_info->*(&enclave_info_t::monitor_area_top) = enclave_id +
      (monitor_area_pages << page_shift());

    // TODO: attestation
  } else {
    enclave_id = null_enclave_id;  // monitor_invalid_state
  }

  clear_dram_region_lock(dram_region);
  return enclave_id;
}

api_result_t load_enclave_page_table(enclave_id_t enclave_id,
    uintptr_t phys_addr, uintptr_t virtual_addr, size_t level, size_t acl) {

  if (!is_dram_address(phys_addr))
    return monitor_invalid_value;
  // NOTE: we need to check the level to avoid an infinite loop; we don't do
  //       any unnecessary checking on measured arguments
  if (level >= page_table_levels())
    return monitor_invalid_value;

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
  if (enclave_info->*(&enclave_info_t::is_initialized) != 0) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_state;
  }
  if (phys_addr <= enclave_info->*(&enclave_info_t::loading_last_addr)) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_value;
  }
  if (level != page_table_levels() - 1 &&
      !is_enclave_virtual_address(virtual_addr, enclave_id)) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_value;
  }

  uintptr_t monitor_area_top =
      enclave_info->*(&enclave_info_t::monitor_area_top);
  if (phys_addr >= enclave_id && phys_addr <= monitor_area_top) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_value;
  }

  // NOTE: we don't need to lock the DRAM region of the physical address,
  //       because an enclave cannot relinquish its DRAM regions until it is
  //       initialized and running; therefore, once the DRAM region is assigned
  //       and its bit is set in the enclave's region bitmap, we know the DRAM
  //       region will stay with the enclave until initialization completes
  size_t page_dram_region = dram_region_for(phys_addr);
  if (!read_enclave_region_bitmap_bit(enclave_id, page_dram_region)) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_value;
  }

  size_t walk_level = page_table_levels() - 1;
  if (level == page_table_levels() - 1) {
    enclave_info->*(&enclave_info_t::loading_eptbr) = phys_addr;
    // NOTE: we completely ignore virtual_addr here; we don't bother checking
    //       that it's zero because the call gets measured
  } else {
    uintptr_t ptb = enclave_info->*(&enclave_info_t::loading_eptbr);
    uintptr_t entry_addr = walk_page_table_to_entry(ptb, virtual_addr, level);
    if (entry_addr == 0) {
      clear_dram_region_lock(dram_region);
      return monitor_invalid_state;
    }
    write_page_table_entry(entry_addr, level, phys_addr, acl);
  }

  enclave_info->*(&enclave_info_t::loading_last_addr) +=
      page_table_size(level);
  bzero(phys_ptr<size_t>{phys_addr}, page_table_size(level));
  return monitor_ok;
}

api_result_t delete_enclave(enclave_id_t enclave_id) {
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
  size_t enclave_addr_dram_region = clamped_dram_region_for(enclave_addr);
  size_t os_addr_dram_region = clamped_dram_region_for(os_addr);

  if (test_and_set_dram_region_lock(enclave_dram_region))
    return monitor_concurrent_call;

  if (test_and_set_dram_region_lock(os_addr_dram_region)) {
    clear_dram_region_lock(enclave_dram_region);
    return monitor_concurrent_call;
  }
  if (enclave_addr_dram_region != enclave_dram_region) {
    if (test_and_set_dram_region_lock(enclave_addr_dram_region)) {
      clear_dram_region_lock(os_addr_dram_region);
      clear_dram_region_lock(enclave_dram_region);
      return monitor_concurrent_call;
    }
  }

  api_result_t result = monitor_ok;
  phys_ptr<dram_region_info_t> region = &g_dram_region[enclave_dram_region];
  if (region->*(&dram_region_info_t::state) != dram_region_owned)
    result = monitor_invalid_value;
  if (atomic_load(&(region->*(&dram_region_info_t::owner))) != enclave_id)
    result = monitor_invalid_value;

  region = &g_dram_region[enclave_addr_dram_region];
  if (region->*(&dram_region_info_t::state) != dram_region_owned)
    result = monitor_invalid_value;
  if (atomic_load(&(region->*(&dram_region_info_t::owner))) != enclave_id)
    result = monitor_invalid_value;

  region = &g_dram_region[os_addr_dram_region];
  if (region->*(&dram_region_info_t::state) != dram_region_owned)
    result = monitor_invalid_value;
  if (atomic_load(&(region->*(&dram_region_info_t::owner))) != null_enclave_id)
    result = monitor_invalid_value;

  phys_ptr<enclave_info_t> enclave_info{enclave_id};
  if (enclave_info->*(&enclave_info_t::is_debug) == 0)
    result = monitor_invalid_state;

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

  // NOTE: we must not clear the same lock twice, because we might be removing
  //       another thread's claim on it
  if (enclave_addr_dram_region != enclave_dram_region)
    clear_dram_region_lock(enclave_addr_dram_region);
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
