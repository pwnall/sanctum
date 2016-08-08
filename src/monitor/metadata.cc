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
using sanctum::internal::enclave_info_pages;
using sanctum::internal::g_dram_region;
using sanctum::internal::g_metadata_region_pages;
using sanctum::internal::g_metadata_region_start;
using sanctum::internal::free_enclave_id;
using sanctum::internal::is_valid_dram_region;
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

api_result_t create_thread(enclave_id_t enclave_id, thread_id_t thread_id) {
  return monitor_ok;
}



};  // namespace sanctum::api::os
};  // namespace sanctum::api
};  // namespace sanctum
