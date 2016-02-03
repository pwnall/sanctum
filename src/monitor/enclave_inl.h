#if !defined(MONITOR_ENCLAVE_INL_H_INCLUDED)
#define MONITOR_ENCLAVE_INL_H_INCLUDED

#include "dram_regions.h"
#include "enclave.h"

namespace sanctum {
namespace internal {

using sanctum::bare::is_valid_page_table_entry;
using sanctum::bare::page_size;
using sanctum::bare::page_shift;
using sanctum::bare::page_table_levels;
using sanctum::bare::page_table_shift;
using sanctum::bare::page_table_entry_shift;
using sanctum::bare::page_table_entry_target;
using sanctum::bare::page_table_translated_bits;
using sanctum::bare::pages_needed_for;
using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;

// Computes the physical address of an enclave's DRAM region bitmap.
//
// The DRAM region bitmap has 1 bit for every DRAM region in the system. The
// bits corresponding to DRAM regions allocated to the enclave are set to 1.
inline phys_ptr<size_t> enclave_region_bitmap(enclave_id_t enclave_id) {
  phys_ptr<enclave_info_t> enclave_info{enclave_id};
  return phys_ptr<size_t>{uintptr_t(enclave_info + 1)};
}
// Computes the physical address of an enclave's thread slots.
inline phys_ptr<thread_slot_t> enclave_thread_slots(enclave_id_t enclave_id) {
  phys_ptr<enclave_info_t> enclave_info{enclave_id};
  return phys_ptr<thread_slot_t>{uintptr_t(
      enclave_region_bitmap(enclave_id) + g_dram_region_bitmap_words)};
}
// Computes the physical address of a specific enclave thread slot.
inline phys_ptr<thread_slot_t> enclave_thread_slot(enclave_id_t enclave_id,
    thread_id_t thread_id) {
  return enclave_thread_slots(enclave_id) + thread_id;
}

// The amount of memory used by the security monitor for an enclave.
//
// The monitor data consists of an enclave_info_t, a DRAM region bitmap, and a
// thread_slot_t array. It is stored at the beginning of an enclave's main DRAM
// region, and cannot be modified by the enclave.
//
// This returns the precise amount of memory used by the monitor. However, all
// memory management happens at page granularity, so
// enclave_metadata_area_pages() is a better reflect of the amount of DRAM
// allocated to monitor pages.
inline size_t enclave_metadata_size(size_t max_threads) {
  return static_cast<size_t>(uintptr_t(enclave_thread_slots(0) + max_threads));
}

// The number of pages used by the security monitor for an enclave.
//
// See enclave_metadata_area_size() for an explanation of the security monitor's
// data. The monitor pages are at the beginning of the enclave's main DRAM
// region.
inline size_t enclave_metadata_pages(size_t max_threads) {
  return pages_needed_for(enclave_metadata_size(max_threads));
}

// Sets a bit in a DRAM region bitmap.
//
// The caller should hold the lock of the enclave's main DRAM region.
//
// A null enclave_id causes a bit to be set in the OS' DRAM region bitmap.
inline void set_enclave_region_bitmap_bit(enclave_id_t enclave_id,
    size_t dram_region, bool true_for_set) {
  if (enclave_id == null_enclave_id)
    set_bitmap_bit(g_os_region_bitmap, dram_region, true_for_set);
  else
    set_bitmap_bit(enclave_region_bitmap(enclave_id), dram_region, true_for_set);
}

// Reads a bit from a DRAM region bitmap.
//
// The caller should hold the lock of the enclave's main DRAM region.
//
// A null enclave_id causes a bit to be read from the OS' DRAM region bitmap.
inline bool read_enclave_region_bitmap_bit(enclave_id_t enclave_id,
    size_t dram_region) {
  if (enclave_id == null_enclave_id)
    return read_bitmap_bit(g_os_region_bitmap, dram_region);
  else
    return read_bitmap_bit(enclave_region_bitmap(enclave_id), dram_region);
}

// Checks if a given virtual address is a valid enclave virtual address.
//
// The caller should hold the lock of the enclave's main DRAM region.
inline bool is_enclave_virtual_address(uintptr_t virtual_addr,
    enclave_id_t enclave_id) {
  phys_ptr<enclave_info_t> enclave_info{enclave_id};
  return (virtual_addr & enclave_info->*(&enclave_info_t::ev_mask)) ==
      enclave_info->*(&enclave_info_t::ev_base);
}

// Checks if a physical address points to an enclave's monitor-reserved pages.
//
// The caller should hold the lock of the enclave's main DRAM region.
inline bool is_enclave_metadata_address(uintptr_t addr,
    enclave_id_t enclave_id) {
  phys_ptr<enclave_info_t> enclave_info{enclave_id};
  return addr >= enclave_id &&
      addr <= enclave_info->*(&enclave_info_t::metadata_top);
}

// The size of an enclave hardware thread's metadata, in bytes.
constexpr inline size_t thread_private_info_size() {
  return sizeof(thread_private_info_t);
}

// The number of pages taken by an enclave hardware thread's metadata.
//
// This should be optimized to a constant by the compiler, but isn't constexpr
// because of pointer arithmetic.
constexpr inline size_t thread_private_info_pages() {
  return (thread_private_info_size() + page_size() - 1) >> page_shift();
}

// Walks a page table, stops at the entry at a given level.
//
// The level is assumed to be valid (between 0 and page_table_levels() - 1).
// The ptb (page table base) and the page tables are all assumed to point to
// accessible memory. This assumption only holds before an enclave is
// initialized, when the monitor is in charge of its page tables.
//
// Returns 0 if the walk was interrupted due to a page table entry not being
// valid / present.
inline uintptr_t walk_page_tables_to_entry(uintptr_t ptb,
    uintptr_t virtual_addr, size_t level) {
  size_t addr_shift = page_table_translated_bits();
  size_t table_addr = ptb;

  // NOTE: We're handling the special case of an unset PTB (page table base)
  //       here because it's equivalent to the valid bit being unset on an
  //       entry in the page tables.
  if (ptb == 0)
    return 0;

  size_t walk_level = page_table_levels();
  while (true) {
    walk_level--;
    size_t level_addr_shift = page_table_shift(walk_level);
    addr_shift -= level_addr_shift;
    uintptr_t addr_mask = ((1 << level_addr_shift) - 1);
    uintptr_t entry_offset = (virtual_addr >> addr_shift) & addr_mask;

    uintptr_t entry_addr = table_addr +
        (entry_offset << page_table_entry_shift(walk_level));
    if (walk_level == level)
      return entry_addr;
    if (!is_valid_page_table_entry(entry_addr, walk_level))
      break;
    table_addr = page_table_entry_target(entry_addr, walk_level);
  }
  return 0;
}

// Performs a software virtual address translation.
//
// The level is assumed to be valid (between 0 and page_table_levels() - 1).
// The ptb (page table base) and the page tables are all assumed to point to
// accessible memory. This assumption only holds before an enclave is
// initialized, when the monitor is in charge of its page tables.
//
// Returns 0 if the walk was interrupted due to a page table entry not being
// valid / present.
inline uintptr_t walk_page_tables(uintptr_t ptb, uintptr_t virtual_addr) {
  uintptr_t entry_addr = walk_page_tables_to_entry(ptb, virtual_addr, 0);
  if (entry_addr == 0)
    return 0;
  if (!is_valid_page_table_entry(entry_addr, 0))
    return 0;
  return page_table_entry_target(entry_addr, 0);
}

// Number of pages used by the enclave attestation process.
inline size_t enclave_attestation_pages() {
  return 0;  // TODO: actual number when we have attestation code
}

};  // namespace sanctum::internal
};  // namespace sanctum
#endif  // !defined(MONITOR_ENCLAVE_INL_H_INCLUDED)
