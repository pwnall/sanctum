#include "sanctum-internal.h"

size_t dram_region_count() {
  return g_dram_region_count;
}
size_t dram_region_size() {
  return g_dram_region_size;
}
api_result_t lock_dram_region(size_t dram_region) {
  // NOTE: the first DRAM region stores the security monitor and must always be
  //       assigned to the OS
  // NOTE: safe to do this outside lock, region count never changes
  if (dram_region == 0 || dram_region >= g_dram_region_count) {
    return monitor_invalid_value;
  }

  if (atomic_flag_test_and_set(&g_region_info_lock)) {
    return monitor_concurrent_call;
  }

  api_result_t result;
  region_info_t& region_info = g_region_info[dram_region];
  if (region_info.state == dram_region_owned) {
    enclave_id_t owner = region_info.owner;
    if (owner == 0 || region_info.mapped_pages == 0) {
      if (owner == g_core_info[current_core()].enclave) {
        region_info.state = dram_region_locked;
        region_info.previous_owner = region_info.owner;
        region_info.owner = 0;
        region_info.locked_at = atomic_fetch_add(&g_lock_clock, 1);
        result = monitor_ok;
      } else {
        result = monitor_access_denied;
      }
    } else {
      result = monitor_invalid_state;
    }
  } else {
    result = monitor_invalid_state;
  }

  atomic_flag_clear(g_region_info_lock);
  return result;
}

// Sets a bit in a DRAM region's page allocation bitmap.
//
// The caller must hold the lock of the enclave that owns the DRAM region
// containing the given address.
//
// `addr` must be a physical address in a DRAM region assigned to an enclave,
// otherwise DRAM trashing will occur.
void set_dram_region_bitmap_bit(uintptr_t addr) {
  uintptr_t region_start =
      addr & static_cast<uintptr_t>(g_dram_region_size - 1);
  uintptr_t region_offset =
      addr & (~static_cast<uintptr_t>(g_dram_region_size - 1));
  size_t region_page = static_cast<size_t>(region_offset >> page_shift);

  // NOTE: trusting the compiler to optimize / and % into shifts
  uintptr_t word_addr = region_start + region_page / sizeof(size_t)
  size_t mask = static_cast<size_t>(1) << (region_page % sizeof(size_t));
  size_t word = load_phys_size(word_addr);
  store_phys_size(word_addr, word | mask);
}

// Clears a bit in a DRAM region's page allocation bitmap.
//
// The caller must hold the lock of the enclave that owns the DRAM region
// containing the given address.
//
// `addr` must be a physical address in a DRAM region assigned to an enclave,
// otherwise DRAM trashing will occur.
void clear_dram_region_bitmap_bit(uintptr_t addr) {
  uintptr_t region_start =
      addr & static_cast<uintptr_t>(g_dram_region_size - 1);
  uintptr_t region_offset =
      addr & (~static_cast<uintptr_t>(g_dram_region_size - 1));
  size_t region_page = static_cast<size_t>(region_offset >> page_shift);

  // NOTE: trusting the compiler to optimize / and % into shifts
  uintptr_t word_addr = region_start + region_page / sizeof(size_t)
  size_t mask = static_cast<size_t>(1) << (region_page % sizeof(size_t));
  size_t word = load_phys_size(word_addr);
  store_phys_size(word_addr, word & (~mask));
}

// Reads a bit in a DRAM region's page allocation bitmap.
//
// The caller must hold the lock of the enclave that owns the DRAM region
// containing the given address.
//
// `addr` should be a physical address in a DRAM region assigned to an enclave,
// otherwise invalid memory accesses will be issued.
bool test_dram_region_bitmap_bit(uintptr_t addr) {
  uintptr_t region_start =
      addr & static_cast<uintptr_t>(g_dram_region_size - 1);
  uintptr_t region_offset =
      addr & (~static_cast<uintptr_t>(g_dram_region_size - 1));
  size_t region_page = static_cast<size_t>(region_offset >> page_shift);

  // NOTE: trusting the compiler to optimize / and % into shifts
  uintptr_t word_addr = region_start + region_page / sizeof(size_t)
  size_t mask = static_cast<size_t>(1) << (region_page % sizeof(size_t));
  size_t word = load_phys_size(word_addr);
  return (word & mask) != 0;
}

// Resets a DRAM region's page allocation bitmap.
//
// The enclave that owns this DRAM region must have its pages enabled in this
// thread.
//
// `dram_region` must be a region assigned to an enclave, otherwise memory
// thrashing will occur.
void init_dram_region_bitmap(size_t dram_region) {
  uintptr_t start = dram_region_start(dram_region);
  uintptr_t end = start + g_dram_region_bitmap_pages * page_size;

  // This is essentially memset with physical addresses.
  for (uintptr_t addr = start; addr < end; addr += sizeof(size_t)) {
    store_phys_size(addr, 0);
  }

  // This loop performs 2 cached DRAM accesses per bit, whereas it would be
  // optimal to perform 2 accesses per size_t. Optimizing doesn't seem to be
  // worthwhile.
  for (size_t page_addr = start; page_addr < end; page_addr += page_size) {
      set_dram_region_bitmap_bit(page_addr);
  }
}

api_result_t assign_dram_region(size_t dram_region, enclave_id_t new_owner) {
  // NOTE: the first DRAM region will never be freed, so we don't need to
  //       special-case it here
  // NOTE: safe to do this outside lock, region count never changes
  if (dram_region >= g_dram_region_count) {
    return monitor_invalid_value;
  }

  if (atomic_flag_test_and_set(&g_region_info_lock)) {
    return monitor_concurrent_call;
  }

  api_result_t result;
  region_info_t& region_info = g_region_info[dram_region];
  if (region_info.state == dram_region_free) {
    if (check_enclave_id(new_owner)) {
      if (new_owner == 0 || enclave_test_and_set_lock(new_owner)) {
        region_info.state = dram_region_owned;
        region_info.owner = new_owner;
        // NOTE: mapped_pages only makes sense for enclave-owned regions, but
        //       avoiding an if reduces this function's complexity, at the cost
        //       of an extra memory operation in rare cases
        region_info.mapped_pages = g_dram_region_bitmap_pages;
        result = monitor_ok;
      } else {
        result = monitor_concurrent_call;
      }
    } else {
      result = monitor_invalid_value;
    }
  } else {
    result = monitor_invalid_state;
  }

  atomic_flag_clear(g_region_info_lock);
  if (result == monitor_ok && new_owner != 0) {
    // NOTE: must set the core's owner to the enclave, in order to activate its
    //       page tables
    core_info_t& core_info = g_core_info[current_core()];
    atomic_store(&core_info->enclave, new_owner);
    enclave_info_t* enclave_info = enable_enclave_pages(new_owner);
    init_dram_region_bitmap(dram_region, enclave_info);
    disable_enclave_pages();
    atomic_store(&core_info->enclave, 0);
    enclave_clear_lock(new_owner);
  }
  return result;
}
api_result_t free_dram_region(size_t dram_region) {
  // NOTE: the first DRAM region will never be locked, so we don't need to
  //       special-case it here
  // NOTE: safe to do this outside lock, region count never changes
  if (dram_region >= g_dram_region_count) {
    return monitor_invalid_value;
  }

  if (atomic_flag_test_and_set(&g_region_info_lock)) {
    return monitor_concurrent_call;
  }

  api_result_t result;
  region_info_t& region_info = g_region_info[dram_region];
  if (region_info.state == dram_region_locked) {
    bool can_free = true;
    // Mappings for OS-owned regions must be TLB-flushed from all cores.
    //
    // Mappings for enclave-owned regions must be TLB-flushed from cores that
    // execute enclave code. However, every enclave exit causes a TLB flush and
    // updates the core's clock.
    for (size_t i = 0; i < g_core_count; ++i) {
      // TODO: use atomic operations for the flush counters
      unsigned lock_clock = atomic_load(&g_lock_clock);
      if (atomic_load(g_core_info[i].flushed_at) < lock_clock) {
        can_free = false;
        break;
      }
    }
    if (can_free) {
      region_info.state = dram_region_free;
      result = monitor_ok;
    } else {
      result = monitor_invalid_state;
    }
  } else {
    result = monitor_invalid_state;
  }

  atomic_flag_clear(g_region_info_lock);
  return result;
}
api_result_t dram_region_flush() {
  core_tlb_flush();
  return monitor_ok;
}

// Allocates the enclave's zone and locks the enclave.
//
// Returns the ID for the new enclave (which is locked), or 0 in case of
// failure.
enclave_id_t _alloc_enclave(size_t dram_region) {
  // NOTE: the first DRAM region will never be freed, so we don't need to
  //       special-case it here
  // NOTE: safe to do this outside lock, region count never changes
  if (dram_region >= g_dram_region_count) {
    return 0;  // monitor_invalid_value
  }

  if (atomic_flag_test_and_set(&g_region_info_lock)) {
    return 0;  // monitor_concurrent_call
  }

  region_info_t& region_info = g_region_info[dram_region];
  enclave_id_t enclave_id;
  if (region_info.state == dram_region_free) {
    uintptr_t region_start = dram_region_start(dram_region);
    enclave_id = region_start + (g_dram_region_bitmap_pages << page_shift);

    region_info.state = dram_region_owned;
    region_info.owner = enclave_id;
    region_info.mapped_pages = g_dram_region_bitmap_pages;

    uintptr_t lock_addr = enclave_lock_addr(enclave_id);
    store_phys_size(lock_addr, static_cast<size_t>(ATOMIC_FLAG_INIT));
    phys_atomic_flag_test_and_set(lock_addr);
  } else {
    enclave_id = 0;  // monitor_invalid_state
  }

  atomic_flag_clear(g_region_info_lock);
  return enclave_id;
}

// Sets up page tables for the enclave's wired area.
//
// The wired page tables must always be in memory, as they map vital strutures
// such as enclave_info, the DRAM region bitmaps, and the enclave page tables.
api_result_t _wire_enclave(size_t dram_region, enclave_id_t enclave_id,
                           uintptr_t ev_base, uintptr_t ev_mask) {
  uintptr_t ev_size = ev_mask + 1;
  uintptr_t ev_end = ev_base + ev_size;

  // Virtual memory layout:
  //
  // ---> ev_base
  // (room for enclave's structures)
  // ---> wired_top
  // - level n (root) page table for the enclave's page tables
  //      ...
  // - level 2 page tables for the enclave's page tables
  // - level 1 (leaf) page tables for the enclave's page tables
  // - bitmaps for all DRAM regions owned by the enclave
  // - enclave_info
  // ---> ev_end (ev_base + ev_mask + 1)
  // *** bottom of the enclave's linear address space ***

  uintptr_t enclave_info_vaddr = ev_end - (enclave_info_pages << page_shift);
  uintptr_t dram_region_bitmaps_pages =
      g_dram_region_bitmap_pages * g_dram_region_count;
  uintptr_t dram_region_bitmaps = enclave_info_vaddr -
      (dram_region_bitmaps_pages << page_shift);

  uintptr_t page_tables[page_table_levels];
  uintptr_t wired_top = dram_region_bitmaps;
  uintptr_t page_tables_entries = ev_size >> page_shift;
  for (size_t level = 0; level < page_table_levels; ++level) {
    // Number of tables at this level.
    page_table_entries >>= page_table_shift(level);
    // ev_size is a power of two, this is the only case where shifting doesn't
    // equal ceiling division
    if (page_table_entries < 1) page_table_entries = 1;
    // Number of pages taken up by the tables at this level.
    uintptr_t table_pages = page_table_entries * page_table_size(level);
    wired_top -= table_pages;
    page_tables[level] = wired_top;
  }

  // Wire the bottom of the address space so we can switch to virtual
  // addresses.
  size_t wired_pages = (ev_end - wired_top) >> page_shift;

  uintptr_t wired_page_tables[page_table_levels];
  page_tables_entries = wired_pages;
  for (size_t level = 0; level < page_table_levels; ++level) {
    // wired_pages is not a power of two, ceiling division requires extra add
    page_table_entries = (page_table_entries + page_table_entries(level) - 1)
        >> page_table_shift(level);
    wired_page_tables[level] = page_table_entries;
  }

  // Physical memory layout:
  //
  // ---> dram_region_start
  // - DRAM region bitmap
  // - enclave_info
  // - level n (root) page table for the wired pages
  //     ...
  // - level 2 page tables for the wired pages
  // - level 1 (leaf) page tables for the wired pages
  // --->

  uintptr_t phys_start = dram_region_start(dram_region) +
      ((g_dram_region_bitmap_pages + enclave_info_pages) << page_shift);
  uintptr_t enclave_ptrr = phys_start;

  // Set up the non-leaves to point to the next level tables.
  for (size_t level = page_table_levels - 1; level > 0; --level) {
    uintptr_t next_phys_start = phys_start +
        page_tables[level] * page_table_size(level);
    uintptr_t next_next_phys_start = phys_start +
        page_tables[level - 1] * page_table_size(level - 1);

    // We take advantage of the fact that the wired pages are right at the
    // bottom of the virtual address space. This makes it easier to fill in
    // each level's entries from last to first.
    uintptr_t entry_size = page_table_entry_size(level);
    uintptr_t table_size = page_table_size(level - 1);
    uintptr_t next_table = next_next_phys_start;
    uintptr_t entry = next_phys_start;
    while (entry > phys_start) {
      entry -= entry_size;
      next_table -= table_size;
      set_wired_page_table_entry(entry, level, next_table);
    }

    phys_start = next_phys_start;
  }

  // We also fill in leaves from last to first.
  uintptr_t entry_size = page_table_entry_size(0);
  uintptr_t entry = phys_start + page_tables[0] * page_table_size(0);
  // The bottom pages hold the enclave_info.
  for (size_t i = 0, uintptr_t phys_addr = enclave_id; i < enclave_info_pages;
       ++i, phys_addr += page_size)
    entry -= entry_size;
    set_wired_page_table_entry(entry, 0, phys_addr);
  }
  // Skip the DRAM region bitmap pages.
  entry -= entry_size * dram_region_bitmaps_pages;
  // Map the wired pages.
  for (uintptr_t phys_addr = ev_end; phys_addr > wired_top; ) {
    phys_addr -= page_size;
    entry -= entry_size;
    set_wired_page_table_entry(entry, 0, phys_addr);
  }

  // The wired page tables are built, now we can access enclave_info and the
  // page tables using normal memory operations.
  set_eptr_register(enclave_ptrr);

  enclave_info_t* enclave_info =
      static_cast<enclave_info_t*>(enclave_info_vaddr);

  enclave_info->enclave_ptrr = enclave_ptrr;
  enclave_info->virtual_address = enclave_info_vaddr;
  enclave_info->dram_region_bitmaps = dram_region_bitmaps;
  for (size_t i = 0; i < page_table_levels; ++i) {
    enclave_info->page_tables[i] = page_tables[i];
  }
}

enclave_id_t make_enclave(size_t dram_region, uintptr_t ev_base,
                          uintptr_t ev_mask) {
  if ((ev_mask & (ev_mask + 1)) != 0)  // Check that ev_mask is a mask.
    return 0;  // monitor_invalid_value
  if ((ev_base & ev_mask) != 0)  // Check that ev_base is aligned to the mask.
    return 0;  // monitor_invalid_value
  if (ev_mask + 1 < dram_region_size)  // TODO: come up with a better min size.
    return 0;  // monitor_invalid_value

  enclave_id_t enclave_id = _alloc_enclave(dram_region);
  if (enclave_id == 0)
    return enclave_id;

  _wire_enclave(dram_region, enclave_id);

  // _wire_enclave() sets ECR3, so the enclave's data structures can be
  // initialized
  set_eptr_register(0);
  core_tlb_flush();

  return enclave_id;
}


api_result_t make_enclave_page_table(uintptr_t phys_addr, uintptr_t addr,
                                     int lvl) {
}
