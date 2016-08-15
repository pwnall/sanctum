#include "metadata.h"

#include "bare/page_tables.h"
#include "bare/phys_ptr.h"
#include "cpu_core_inl.h"
#include "dram_regions_inl.h"
#include "enclave_inl.h"
#include "measure_inl.h"
#include "metadata_inl.h"

namespace sanctum {
namespace internal {  // sanctum::internal

size_t g_metadata_region_pages;
size_t g_metadata_region_start;

};  // namespace sanctum::internal
};  // namespace sanctum

using sanctum::bare::is_page_aligned;
using sanctum::bare::is_valid_range;
using sanctum::bare::page_shift;
using sanctum::bare::page_size;
using sanctum::bare::phys_ptr;
using sanctum::internal::accept_metadata_pages;
using sanctum::internal::clear_dram_region_lock;
using sanctum::internal::current_enclave;
using sanctum::internal::dram_region_for;
using sanctum::internal::dram_region_info_t;
using sanctum::internal::dram_region_start;
using sanctum::internal::enclave_info_pages;
using sanctum::internal::enclave_info_t;
using sanctum::internal::enclave_metadata_page_type;
using sanctum::internal::g_dram_region;
using sanctum::internal::g_metadata_region_pages;
using sanctum::internal::g_metadata_region_start;
using sanctum::internal::free_enclave_id;
using sanctum::internal::is_dram_address;
using sanctum::internal::init_enclave_info;
using sanctum::internal::lock_enclave;
using sanctum::internal::lock_metadata_region_for;
using sanctum::internal::metadata_enclave_id;
using sanctum::internal::metadata_page_info_t;
using sanctum::internal::read_dram_region_owner;
using sanctum::internal::reserve_metadata_pages;
using sanctum::internal::test_and_set_dram_region_lock;
using sanctum::internal::thread_metadata_page_type;
using sanctum::internal::thread_info_t;
using sanctum::internal::unlock_enclave;

namespace sanctum {
namespace api {  // sanctum::api
namespace os {  // sancum::api::os

size_t metadata_region_pages() {
  return g_metadata_region_pages;
}

size_t metadata_region_start() {
  return g_metadata_region_start;
}

size_t enclave_metadata_pages(size_t mailbox_count) {
  return enclave_info_pages(mailbox_count);
}

size_t thread_metadata_pages() {
  return sanctum::internal::thread_metadata_pages();
}

api_result_t create_enclave(enclave_id_t enclave_id, uintptr_t ev_base,
    uintptr_t ev_mask, size_t mailbox_count, bool debug) {
  if (!is_valid_range(ev_base, ev_mask))
    return  monitor_invalid_value;
  if (ev_mask + 1 < page_size())
    return monitor_invalid_value;

  size_t dram_region;
  api_result_t result = lock_metadata_region_for(enclave_id, dram_region);
  if (result != monitor_ok)
    return result;

  phys_ptr<dram_region_info_t> region = &g_dram_region[dram_region];
  if (region->*(&dram_region_info_t::owner) != metadata_enclave_id) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_value;
  }

  result = reserve_metadata_pages(enclave_id,
      enclave_info_pages(mailbox_count), enclave_id,
      enclave_metadata_page_type);
  if (result != monitor_ok) {
    clear_dram_region_lock(dram_region);
    return result;
  }

  init_enclave_info(phys_ptr<enclave_info_t>{enclave_id}, ev_base, ev_mask,
      mailbox_count, debug);
  clear_dram_region_lock(dram_region);
  return monitor_ok;
}

api_result_t assign_thread(enclave_id_t enclave_id, thread_id_t thread_id) {
  api_result_t result = lock_enclave(enclave_id);
  if (result != monitor_ok)
    return result;

  size_t dram_region;
  result = lock_metadata_region_for(thread_id, dram_region);
  if (result != monitor_ok) {
    unlock_enclave(enclave_id);
    return result;
  }

  phys_ptr<enclave_info_t> enclave_info{enclave_id};
  if (enclave_info->*(&enclave_info_t::is_initialized)) {
    result = reserve_metadata_pages(enclave_id, thread_metadata_pages(),
        enclave_id, thread_metadata_page_type);
  } else {
    result = monitor_invalid_state;
  }

  if (result != monitor_ok) {
    clear_dram_region_lock(dram_region);
    unlock_enclave(enclave_id);
    return result;
  }

  enclave_info->*(&enclave_info_t::thread_count) += 1;

  clear_dram_region_lock(dram_region);
  unlock_enclave(enclave_id);
  return monitor_ok;
}

api_result_t load_thread(enclave_id_t enclave_id,
    thread_id_t thread_id, uintptr_t entry_pc, uintptr_t entry_stack,
    uintptr_t fault_pc, uintptr_t fault_stack) {
  api_result_t result = lock_enclave(enclave_id);
  if (result != monitor_ok)
    return result;

  size_t thread_dram_region;
  result = lock_metadata_region_for(thread_id, thread_dram_region);
  if (result != monitor_ok) {
    unlock_enclave(enclave_id);
    return result;
  }

  phys_ptr<enclave_info_t> enclave_info{enclave_id};
  if (enclave_info->*(&enclave_info_t::is_initialized) ||
      enclave_info->*(&enclave_info_t::load_eptbr) == 0) {
    result = monitor_invalid_state;
  } else {
    result = reserve_metadata_pages(enclave_id, thread_metadata_pages(),
        enclave_id, thread_metadata_page_type);
  }

  if (result != monitor_ok) {
    clear_dram_region_lock(thread_dram_region);
    unlock_enclave(enclave_id);
    return result;
  }

  enclave_info->*(&enclave_info_t::thread_count) += 1;

  phys_ptr<thread_info_t> thread_metadata{thread_id};
  atomic_flag_clear(&(thread_metadata->*(&thread_info_t::lock)));
  thread_metadata->*(&thread_info_t::entry_pc) = entry_pc;
  thread_metadata->*(&thread_info_t::entry_stack) = entry_stack;
  thread_metadata->*(&thread_info_t::fault_pc) = fault_pc;
  thread_metadata->*(&thread_info_t::fault_stack) = fault_stack;
  thread_metadata->*(&thread_info_t::eptbr) =
      enclave_info->*(&enclave_info_t::load_eptbr);

  extend_enclave_hash_with_thread(enclave_info, entry_pc, entry_stack,
      fault_pc, fault_stack);

  clear_dram_region_lock(thread_dram_region);
  unlock_enclave(enclave_id);
  return monitor_ok;
}

};  // namespace sanctum::api::os

namespace enclave {  // namespace sanctum::api::enclave

static_assert(sizeof(thread_init_info_t) <= page_size(),
    "accept_thread assumes that thread_init_info_t fits into one page");

api_result_t accept_thread(thread_id_t thread_id, uintptr_t thread_info_addr) {
  if (!is_dram_address(thread_info_addr) || !is_page_aligned(thread_info_addr))
    return monitor_invalid_value;

  enclave_id_t enclave_id = current_enclave();

  size_t thread_dram_region = dram_region_for(thread_info_addr);
  if (test_and_set_dram_region_lock(thread_info_addr))
    return monitor_concurrent_call;

  if (read_dram_region_owner(thread_dram_region) != enclave_id) {
    clear_dram_region_lock(thread_dram_region);
    return monitor_invalid_value;
  }

  // NOTE: This enclave_id is known to be correct, so we can use a faster path
  //       to lock the enclave's metadata region.
  size_t enclave_dram_region = dram_region_for(enclave_id);
  if (test_and_set_dram_region_lock(enclave_dram_region)) {
    clear_dram_region_lock(thread_dram_region);
    return monitor_concurrent_call;
  }

  api_result_t result = accept_metadata_pages(thread_id,
      sanctum::internal::thread_metadata_pages(), enclave_id,
      thread_metadata_page_type);
  if (result != monitor_ok) {
    clear_dram_region_lock(enclave_dram_region);
    clear_dram_region_lock(thread_dram_region);
    return result;
  }

  // NOTE: The enclave's thread_count is NOT incremented here, because this
  //       thread was already accounted for in assign_thread().

  phys_ptr<thread_info_t> thread_metadata{thread_id};
  phys_ptr<thread_init_info_t> thread_info{thread_info_addr};

  atomic_flag_clear(&(thread_metadata->*(&thread_info_t::lock)));

  uintptr_t entry_pc = *phys_ptr<uintptr_t>{
      uintptr_t{&(thread_info->*(&thread_init_info_t::entry_pc))}};
  uintptr_t entry_stack = *phys_ptr<uintptr_t>{
      uintptr_t{&(thread_info->*(&thread_init_info_t::entry_stack))}};
  uintptr_t fault_pc = *phys_ptr<uintptr_t>{
      uintptr_t{&(thread_info->*(&thread_init_info_t::fault_pc))}};
  uintptr_t fault_stack = *phys_ptr<uintptr_t>{
      uintptr_t{&(thread_info->*(&thread_init_info_t::fault_stack))}};
  uintptr_t eptbr = thread_info->*(&thread_init_info_t::eptbr);

  thread_metadata->*(&thread_info_t::entry_pc) = entry_pc;
  thread_metadata->*(&thread_info_t::entry_stack) = entry_stack;
  thread_metadata->*(&thread_info_t::fault_pc) = fault_pc;
  thread_metadata->*(&thread_info_t::fault_stack) = fault_stack;
  thread_metadata->*(&thread_info_t::eptbr) = eptbr;

  clear_dram_region_lock(enclave_dram_region);
  clear_dram_region_lock(thread_dram_region);
  return monitor_ok;
}

};  // namespace sanctum::api::enclave
};  // namespace sanctum::api
};  // namespace sanctum
