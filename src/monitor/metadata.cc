#include "metadata.h"

#include "bare/page_tables.h"
#include "bare/phys_ptr.h"
#include "dram_regions_inl.h"
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

using sanctum::bare::page_shift;
using sanctum::bare::phys_ptr;
using sanctum::internal::clear_dram_region_lock;
using sanctum::internal::dram_region_info_t;
using sanctum::internal::dram_region_start;
using sanctum::internal::g_dram_region;
using sanctum::internal::g_metadata_region_pages;
using sanctum::internal::g_metadata_region_start;
using sanctum::internal::free_enclave_id;
using sanctum::internal::is_valid_dram_region;
using sanctum::internal::metadata_enclave_id;
using sanctum::internal::metadata_page_info_t;
using sanctum::internal::read_dram_region_owner;
using sanctum::internal::test_and_set_dram_region_lock;
using sanctum::internal::thread_info_pages;

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
  return thread_info_pages();
}

api_result_t create_metadata_region(size_t dram_region) {
  if (!is_valid_dram_region(dram_region))
    return monitor_invalid_value;
  if (test_and_set_dram_region_lock(dram_region))
    return monitor_concurrent_call;

  if (read_dram_region_owner(dram_region) == free_enclave_id) {
    phys_ptr<dram_region_info_t> region = &g_dram_region[dram_region];
    region->*(&dram_region_info_t::owner) = metadata_enclave_id;
    region->*(&dram_region_info_t::pinned_pages) = 0;
  }

  phys_ptr<metadata_page_info_t> metadata_map{dram_region_start(dram_region)};
  bzero(metadata_map, g_metadata_region_start << page_shift());

  clear_dram_region_lock(dram_region);
  return monitor_ok;
}

api_result_t create_thread(enclave_id_t enclave_id, thread_id_t thread_id) {
  if (!is_valid_dram_region(dram_region))
    return monitor_invalid_value;
  if (test_and_set_dram_region_lock(dram_region))
    return monitor_concurrent_call;
}



};  // namespace sanctum::api::os
};  // namespace sanctum::api
};  // namespace sanctum
