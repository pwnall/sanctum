#include "enclave.h"

#include "bare/memory.h"
#include "bare/page_tables.h"
#include "dram_regions_inl.h"
#include "enclave_inl.h"
#include "measure_inl.h"
#include "metadata_inl.h"

using sanctum::bare::atomic_fetch_add;
using sanctum::bare::bcopy;
using sanctum::bare::bzero;
using sanctum::bare::ceil_power_of_two;
using sanctum::bare::is_page_aligned;
using sanctum::bare::is_valid_page_table_entry;
using sanctum::bare::is_valid_range;
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
using sanctum::internal::dram_region_for;
using sanctum::internal::dram_region_info_t;
using sanctum::internal::dram_region_start;
using sanctum::internal::enclave_info_t;
using sanctum::internal::enclave_info_pages;
using sanctum::internal::enclave_info_size;
using sanctum::internal::enclave_region_bitmap;
using sanctum::internal::extend_enclave_hash_with_page;
using sanctum::internal::extend_enclave_hash_with_page_table;
using sanctum::internal::extend_enclave_hash_with_thread;
using sanctum::internal::finalize_enclave_hash;
using sanctum::internal::free_enclave_id;
using sanctum::internal::g_dram_region;
using sanctum::internal::g_dram_stripe_size;
using sanctum::internal::init_enclave_hash;
using sanctum::internal::is_dram_address;
using sanctum::internal::is_enclave_virtual_address;
using sanctum::internal::is_valid_dram_region;
using sanctum::internal::is_valid_enclave_id;
using sanctum::internal::read_dram_region_owner;
using sanctum::internal::read_enclave_region_bitmap_bit;
using sanctum::internal::test_and_set_dram_region_lock;
using sanctum::internal::thread_metadata_pages;
using sanctum::internal::thread_metadata_size;
using sanctum::internal::thread_info_t;
using sanctum::internal::walk_page_tables;
using sanctum::internal::walk_page_tables_to_entry;

namespace sanctum {
namespace api {  // sanctum::api
namespace os {  // sancum::api::os


api_result_t load_page_table(enclave_id_t enclave_id,
    uintptr_t phys_addr, uintptr_t virtual_addr, size_t level, size_t acl) {
  if (!is_dram_address(phys_addr))
    return monitor_invalid_value;
  if (!is_page_aligned(phys_addr))
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
  if (phys_addr <= enclave_info->*(&enclave_info_t::last_load_addr)) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_value;
  }
  if (level != page_table_levels() - 1 &&
      !is_enclave_virtual_address(virtual_addr, enclave_id)) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_value;
  }
  /*
  if (is_enclave_metadata_address(phys_addr, enclave_id)) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_value;
  }

  // NOTE: We don't need to lock the DRAM regions of the page tables, because
  //       an enclave cannot relinquish its DRAM regions until it is
  //       initialized and running. Therefore, once the DRAM region is assigned
  //       and its bit is set in the enclave's region bitmap, we know the DRAM
  //       region will stay with the enclave until initialization completes.
  phys_ptr<size_t> region_bitmap = enclave_region_bitmap(enclave_id);
  size_t table_size = page_table_size(level);
  size_t phys_end = phys_addr + table_size;
  for (size_t table_page_addr = phys_addr; table_page_addr < phys_end;
       table_page_addr += page_size()) {
    size_t table_dram_region = dram_region_for(table_page_addr);
    if (!read_bitmap_bit(region_bitmap, table_dram_region)) {
      clear_dram_region_lock(dram_region);
      return monitor_invalid_value;
    }
  }

  // Allocating a page table at level N means walking until level N + 1, and
  // then editing the level N + 1 table to point to our new table.
  size_t edit_level = level + 1;
  if (edit_level == page_table_levels()) {
    enclave_info->*(&enclave_info_t::load_eptbr) = phys_addr;
    // NOTE: we completely ignore virtual_addr here; we don't bother checking
    //       that it's zero because the call gets measured
  } else {
    uintptr_t ptb = enclave_info->*(&enclave_info_t::load_eptbr);
    uintptr_t entry_addr = walk_page_tables_to_entry(ptb, virtual_addr,
        edit_level);
    if (entry_addr == 0 || is_valid_page_table_entry(entry_addr, edit_level)) {
      clear_dram_region_lock(dram_region);
      return monitor_invalid_state;
    }
    write_page_table_entry(entry_addr, edit_level, phys_addr, acl);
  }

  // NOTE: last_load_addr points to the last allocated physical page, so
  //       we have to subtract a page from the page table's end address.
  enclave_info->*(&enclave_info_t::last_load_addr) = phys_end - page_size();
  bzero(phys_ptr<size_t>{phys_addr}, table_size);

  extend_enclave_hash_with_page_table(enclave_info, virtual_addr, level, acl);
  */

  clear_dram_region_lock(dram_region);
  return monitor_ok;
}

api_result_t load_page(enclave_id_t enclave_id, uintptr_t phys_addr,
    uintptr_t virtual_addr, uintptr_t os_addr, uintptr_t acl) {
  if (!is_dram_address(phys_addr) || !is_dram_address(os_addr))
    return monitor_invalid_value;
  if (!is_page_aligned(phys_addr) || !is_page_aligned(os_addr))
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
  if (phys_addr <= enclave_info->*(&enclave_info_t::last_load_addr)) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_value;
  }
  if (!is_enclave_virtual_address(virtual_addr, enclave_id)) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_value;
  }
  /*
  if (is_enclave_metadata_address(phys_addr, enclave_id)) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_value;
  }

  // NOTE: See load_page_table for the explanation why we don't need to
  //       lock phys_addr's DRAM region.
  size_t page_dram_region = dram_region_for(phys_addr);
  if (!read_bitmap_bit(enclave_region_bitmap(enclave_id), page_dram_region)) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_value;
  }

  uintptr_t ptb = enclave_info->*(&enclave_info_t::load_eptbr);
  uintptr_t entry_addr = walk_page_tables_to_entry(ptb, virtual_addr, 0);
  if (entry_addr == 0 || is_valid_page_table_entry(entry_addr, 0)) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_state;
  }

  // NOTE: We're performing the OS DRAM region checks last to minimize the
  //       number of times we have two release two locks when bailing out due
  //       to errors.

  // NOTE: We don't need to check if os_dram_region is the same as dram_region.
  //       If that's the case, we'll simply fail to acquire the lock and return
  //       concurrent_call. This is acceptable. Ideally, we'd return
  //       invalid_value, but that'd increase code size.
  size_t os_dram_region = dram_region_for(os_addr);
  if (test_and_set_dram_region_lock(os_dram_region)) {
    clear_dram_region_lock(dram_region);
    return monitor_concurrent_call;
  }

  // NOTE: Even though we're reading the DRAM region ownership atomically, we
  //       still need to lock the region to make sure that it doesn't go away
  //       while we bcopy a page out of it.
  phys_ptr<dram_region_info_t> region = &g_dram_region[os_dram_region];
  if (read_dram_region_owner(dram_region) != null_enclave_id) {
    clear_dram_region_lock(os_dram_region);
    clear_dram_region_lock(dram_region);
    return monitor_access_denied;
  }

  enclave_info->*(&enclave_info_t::last_load_addr) = phys_addr;
  bcopy(phys_ptr<size_t>{phys_addr}, phys_ptr<size_t>{os_addr}, page_size());
  clear_dram_region_lock(os_dram_region);

  extend_enclave_hash_with_page(enclave_info, virtual_addr, acl, phys_addr);
  */
  clear_dram_region_lock(dram_region);
  return monitor_ok;
}

api_result_t init_enclave(enclave_id_t enclave_id) {
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

  finalize_enclave_hash(enclave_info);

  enclave_info->*(&enclave_info_t::is_initialized) = 1;
  clear_dram_region_lock(dram_region);
  return monitor_ok;
}

};  // namespace sanctum::api::os
};  // namespace sanctum::api
};  // namespace sanctum
