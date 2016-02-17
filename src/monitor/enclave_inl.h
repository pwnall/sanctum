#if !defined(MONITOR_ENCLAVE_INL_H_INCLUDED)
#define MONITOR_ENCLAVE_INL_H_INCLUDED

#include "bare/base_types.h"
#include "bare/bit_masking.h"
#include "bare/page_tables.h"
#include "bare/phys_ptr.h"
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

// Checks if a given virtual address is a valid enclave virtual address.
//
// The caller should hold the lock of the enclave metadata's DRAM region.
inline bool is_enclave_virtual_address(uintptr_t virtual_addr,
    enclave_id_t enclave_id) {
  phys_ptr<enclave_info_t> enclave_info{enclave_id};
  return (virtual_addr & enclave_info->*(&enclave_info_t::ev_mask)) ==
      enclave_info->*(&enclave_info_t::ev_base);
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

};  // namespace sanctum::internal
};  // namespace sanctum
#endif  // !defined(MONITOR_ENCLAVE_INL_H_INCLUDED)
