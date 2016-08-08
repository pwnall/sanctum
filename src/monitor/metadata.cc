#include "metadata.h"

#include "bare/page_tables.h"
#include "bare/phys_ptr.h"
#include "dram_regions_inl.h"
#include "measure_inl.h"
#include "metadata_inl.h"

namespace sanctum {
namespace internal {  // sanctum::internal

size_t g_metadata_region_pages;
size_t g_metadata_region_start;

};  // namespace sanctum::internal
};  // namespace sanctum

namespace sanctum {
namespace api {  // sanctum::api
namespace os {  // sancum::api::os

using sanctum::bare::is_valid_range;
using sanctum::bare::page_shift;
using sanctum::bare::page_size;
using sanctum::bare::phys_ptr;
using sanctum::internal::alloc_metadata_item;
using sanctum::internal::clear_dram_region_lock;
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
using sanctum::internal::is_valid_dram_region;
using sanctum::internal::lock_metadata_region_for;
using sanctum::internal::metadata_enclave_id;
using sanctum::internal::metadata_page_info_t;
using sanctum::internal::read_dram_region_owner;
using sanctum::internal::test_and_set_dram_region_lock;

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

  result = alloc_metadata_item(enclave_id, enclave_info_pages(mailbox_count),
      enclave_id, enclave_metadata_page_type);
  if (result != monitor_ok) {
    clear_dram_region_lock(dram_region);
    return result;
  }

  phys_ptr<enclave_info_t> enclave_info{enclave_id};
  atomic_flag_clear(&(enclave_info->*(&enclave_info_t::lock)));
  enclave_info->*(&enclave_info_t::mailbox_count) = mailbox_count;
  enclave_info->*(&enclave_info_t::is_initialized) = 0;
  enclave_info->*(&enclave_info_t::is_debug) = debug;
  enclave_info->*(&enclave_info_t::ev_base) = ev_base;
  enclave_info->*(&enclave_info_t::ev_mask) = ev_mask;
  enclave_info->*(&enclave_info_t::load_eptbr) = 0;
  enclave_info->*(&enclave_info_t::last_load_addr) = 0;
  atomic_init(&(enclave_info->*(&enclave_info_t::running_threads)),
      static_cast<size_t>(0));
  init_enclave_hash(enclave_info, ev_base, ev_mask, mailbox_count, debug);

  clear_dram_region_lock(dram_region);
  return monitor_ok;
}

api_result_t create_thread(enclave_id_t enclave_id, thread_id_t thread_id) {
  return monitor_ok;
}



};  // namespace sanctum::api::os
};  // namespace sanctum::api
};  // namespace sanctum
