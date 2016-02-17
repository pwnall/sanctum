#include "enclave.h"

#include "bare/memory.h"
#include "bare/page_tables.h"
#include "dram_regions_inl.h"
#include "enclave_inl.h"
#include "measure_inl.h"

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
using sanctum::internal::enclave_thread_slot;
using sanctum::internal::enclave_thread_slots;
using sanctum::internal::extend_enclave_hash_with_page;
using sanctum::internal::extend_enclave_hash_with_page_table;
using sanctum::internal::extend_enclave_hash_with_thread;
using sanctum::internal::finalize_enclave_hash;
using sanctum::internal::free_enclave_id;
using sanctum::internal::g_dram_region;
using sanctum::internal::g_dram_stripe_size;
using sanctum::internal::init_enclave_hash;
using sanctum::internal::is_dram_address;
using sanctum::internal::is_enclave_metadata_address;
using sanctum::internal::is_enclave_virtual_address;
using sanctum::internal::is_valid_dram_region;
using sanctum::internal::is_valid_enclave_id;
using sanctum::internal::read_dram_region_owner;
using sanctum::internal::read_enclave_region_bitmap_bit;
using sanctum::internal::test_and_set_dram_region_lock;
using sanctum::internal::thread_public_info_t;
using sanctum::internal::thread_private_info_pages;
using sanctum::internal::thread_private_info_size;
using sanctum::internal::thread_info_t;
using sanctum::internal::thread_slot_t;
using sanctum::internal::walk_page_tables;
using sanctum::internal::walk_page_tables_to_entry;

namespace sanctum {
namespace api {  // sanctum::api
namespace os {  // sancum::api::os

api_result_t create_enclave(enclave_id_t enclave_id, uintptr_t ev_base,
    uintptr_t ev_mask, size_t mailbox_count, bool debug) {
  if (!is_valid_range(ev_base, ev_mask))
    return  monitor_invalid_value;
  if (ev_mask + 1 < page_size())
    return monitor_invalid_value;

  size_t dram_region = dram_region_for(enclave_id);
  if (!is_valid_dram_region(dram_region))
    return monitor_invalid_value;

  if (test_and_set_dram_region_lock(dram_region))
    return monitor_concurrent_call;

  phys_ptr<dram_region_info_t> region = &g_dram_region[dram_region];
  if (region->*(&dram_region_info_t::owner) != metadata_enclave_id) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_state;
  }

  // The enclave's metadata area must not cross DRAM regions.
  size_t metadata_pages = enclave_metadata_pages(max_threads);
  size_t first_page_map_slot = metadata_page_map_slot(enclave_id);
  if (first_page_map_slot + metadata_pages >= g_metadata_region_pages) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_state;
  }


  phys_ptr<metadata_page_info_t> page_map_info =
      metadata_page_map_info_for(enclave_id);
  bool found_free_pages = true;
  for (size_t i = 0; i < metadata_pages; i += 1) {

  }

  phys_ptr<enclave_info_t> enclave_info{enclave_id};
  phys_ptr<mailbox_t> mailboxes{enclave_mailboxes(enclave_id)};


    phys_ptr<dram_region_info_t> region = &g_dram_region[dram_region];
    region->*(&dram_region_info_t::owner) = enclave_id;
    region->*(&dram_region_info_t::pinned_pages) = metadata_pages;

    enclave_info->*(&enclave_info_t::max_threads) = max_threads;
    for (size_t i = 0; i < max_threads; ++i) {
      phys_ptr<thread_slot_t> slot = &thread_slots[i];
      slot->*(&thread_slot_t::thread_public_info) =
          phys_ptr<thread_info_t>::null();
      atomic_flag_clear(&(slot->*(&thread_slot_t::lock)));
    }

    enclave_info->*(&enclave_info_t::is_debug) = 0;

    enclave_info->*(&enclave_info_t::ev_base) = ev_base;
    enclave_info->*(&enclave_info_t::ev_mask) = ev_mask;
    enclave_info->*(&enclave_info_t::is_initialized) = 0;

    // NOTE: It is safe to use 0 as the initial values here because that
    //       physical address is in DRAM region 0, which can never be assigned
    //       to an enclave.
    enclave_info->*(&enclave_info_t::load_eptbr) = 0;
    enclave_info->*(&enclave_info_t::last_load_addr) = 0;

    atomic_init(&(enclave_info->*(&enclave_info_t::running_threads)),
        static_cast<size_t>(0));

    init_enclave_hash(enclave_info, ev_base, ev_mask, mailbox_count);
  } else {
    enclave_id = null_enclave_id;  // monitor_invalid_state
  }

  clear_dram_region_lock(dram_region);
  return monitor_ok;
}

api_result_t load_enclave_page_table(enclave_id_t enclave_id,
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

  clear_dram_region_lock(dram_region);
  return monitor_ok;
}

api_result_t load_enclave_page(enclave_id_t enclave_id, uintptr_t phys_addr,
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
  if (is_enclave_metadata_address(phys_addr, enclave_id)) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_value;
  }

  // NOTE: See load_enclave_page_table for the explanation why we don't need to
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

  clear_dram_region_lock(dram_region);
  return monitor_ok;
}

api_result_t load_enclave_thread(enclave_id_t enclave_id,
    thread_id_t thread_id, uintptr_t virtual_addr) {
  if (!is_page_aligned(virtual_addr))
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
  if (thread_id >= enclave_info->*(&enclave_info_t::max_threads)) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_value;
  }

  // The address of the last byte of the private_thread_public_info_t structure.
  uintptr_t virtual_end = virtual_addr + thread_private_info_size();

  // NOTE: the enclave virtual address range is continunous, so we can get away
  //       with checking the start and end address for inclusion
  if (!is_enclave_virtual_address(virtual_addr, enclave_id) ||
      !is_enclave_virtual_address(virtual_end - 1, enclave_id)) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_value;
  }

  uintptr_t ptb = enclave_info->*(&enclave_info_t::load_eptbr);
  phys_ptr<size_t> region_bitmap = enclave_region_bitmap(enclave_id);
  uintptr_t phys_addr = walk_page_tables(ptb, virtual_addr);
  uintptr_t phys_end = phys_addr + thread_private_info_size();

  // NOTE: The thread_info_t occupies contiguous space in physical
  //       memory, so we only need to check the end for DRAM inclusion. The
  //       start is guaranteed to be in DRAM, because it comes from a page walk
  //       over trusted page tables.
  if (!is_dram_address(phys_end - 1)) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_value;
  }

  uintptr_t last_phys_addr = phys_addr;
  size_t thread_dram_region = dram_region_for(phys_addr);
  bool is_supported_mapping = true;
  for (uintptr_t page_addr = virtual_addr + page_size();
       page_addr < virtual_end; page_addr += page_size()) {
    uintptr_t next_phys_addr = walk_page_tables(ptb, page_addr);
    if (next_phys_addr != last_phys_addr + page_size()) {
      // Virtual pages don't map to contiguous physical pages.
      is_supported_mapping = false;
      break;
    }
    last_phys_addr = next_phys_addr;

    if (dram_region_for(next_phys_addr) != thread_dram_region) {
      // Virtual pages map to physical pages in different DRAM regions. We
      // don't support that, because the code would be quite complex. We'd have
      // to acquire locks for multiple DRAM regions, and release them carefully
      // if any acquisition fails. Furthermore, on most architectures,
      // thread_info_t takes up a single page, so the extra complexity
      // is not warranted.
      is_supported_mapping = false;
      break;
    }

    // NOTE: The physical page is guaranteed to belong to an enclave's DRAM
    //       region, because it was obtained from a page walk on the enclave's
    //       page tables, which are trusted before the enclave initializes.
    //       So, we don't need to check DRAM region ownership. We also don't
    //       need to ensure that the virtual addresses don't point to
    //       monitor-reserved pages.
  }
  if (!is_supported_mapping) {
    clear_dram_region_lock(dram_region);
    return monitor_unsupported;
  }

  // NOTE: We're performing the thread slot checks towards the end to minimize
  //       the number of times we have two release two/three locks when bailing
  //       out due to errors.

  phys_ptr<thread_slot_t> slot{enclave_thread_slot(enclave_id, thread_id)};
  if (atomic_flag_test_and_set(&(slot->*(&thread_slot_t::lock)))) {
    clear_dram_region_lock(dram_region);
    return monitor_concurrent_call;
  }

  phys_ptr<thread_info_t> old_thread =
      slot->*(&thread_slot_t::thread_public_info);
  if (old_thread != phys_ptr<thread_info_t>::null()) {
    atomic_flag_clear(&(slot->*(&thread_slot_t::lock)));
    clear_dram_region_lock(dram_region);
    return monitor_invalid_state;
  }

  // NOTE: We're locking the thread info's DRAM region last, to minimize the
  //       number of times we have to release three locks when bailing out due
  //       to errors.

  // NOTE: The thread metadata's DRAM region may be the same as the main
  //       enclave's DRAM region, so we have to code around that.
  if (thread_dram_region != dram_region &&
      test_and_set_dram_region_lock(thread_dram_region))  {
    atomic_flag_clear(&(slot->*(&thread_slot_t::lock)));
    clear_dram_region_lock(dram_region);
    return monitor_concurrent_call;
  }

  phys_ptr<dram_region_info_t> thread_region{
      dram_region_start(thread_dram_region)};
  thread_region->*(&dram_region_info_t::pinned_pages) +=
      thread_private_info_pages();

  phys_ptr<thread_info_t> private_thread{phys_addr};
  slot->*(&thread_slot_t::thread_public_info) = private_thread;

  // NOTE: We're writing physical address fields in the thread_public_info_t because
  //       we don't want them included in the enclave's measurement, so we
  //       can't have the OS set them in the load_enclave_page() data.
  phys_ptr<thread_public_info_t> thread{phys_addr};
  thread->*(&thread_public_info_t::eptbr) = ptb;

  extend_enclave_hash_with_thread(enclave_info, thread_id, virtual_addr);

  if (thread_dram_region != dram_region)
    clear_dram_region_lock(thread_dram_region);
  atomic_flag_clear(&(slot->*(&thread_slot_t::lock)));
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
