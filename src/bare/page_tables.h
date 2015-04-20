#if !defined(BARE_PAGE_TABLES_H_INCLUDED)
#define BARE_PAGE_TABLES_H_INCLUDED

#include "base_types.h"

namespace sanctum {
namespace bare {

// Number of bits in an address that don't undergo address translation.
constexpr size_t page_shift();

// Number of page table levels.
//
// We number levels from 0, which is assigned to the page table leaves.
//
// For example, in x86_64, the levels are 0 (PT), 1 (PD), 2 (PDPT), 3 (PML4),
// and page_table_levels is 4.
constexpr size_t page_table_levels();

// The number of address bits translated by a page table level.
//
// On most architectures, the number of bits is not level-dependent.
constexpr inline size_t page_table_shift(size_t level);

// Log2 of the size of a page table entry, at a given level, in bytes.
//
// On most architectures, each page table entry holds a pointer whose least
// significant bits are reused for access control flags. Therefore, this is
// generally log2(sizeof(uintptr_t)).
constexpr inline size_t page_table_entry_shift(size_t level);

// Sets the EPTBR (enclave page table base register).
//
// This can only be issued by the security monitor. An invalid DRAM address
// will lock up or reboot the machine.
void set_eptbr(uintptr_t value);

// Sets the PTBR (page table base register).
//
// This can only be issued by the security monitor. An invalid DRAM address
// will lock up or reboot the machine.
void set_ptbr(uintptr_t value);

// Reads the valid (a.k.a. present) bit in a page table entry.
//
// Page entries with the valid bit unset have no other valid fields.
bool is_valid_page_table_entry(uintptr_t entry_addr, size_t level);

// Reads the destination pointer in a page table entry.
//
// The pointer can be the physical address of the next level page table, or the
// physical address for a virtual address.
uintptr_t page_table_entry_target(uintptr_t entry_addr, size_t level);

// Writes a page table entry.
//
// `target` points to the next level page table, or has the physical address
// that comes from the translation. `acl` has platform-dependent access control
// flags, such as W (writable) and NX (not-executable).
//
// Before being combined with `target`, the `acl` value is masked against a
// value that only leaves in bits with known access control roles. For example,
// the valid / present bit will be masked off of the ACL.
void write_page_table_entry(uintptr_t entry_addr, size_t level,
    uintptr_t target, uintptr_t acl);

};  // namespace sanctum::bare
};  // namespace sanctum

// Per-architecture page table constants and operations.
#include "page_tables_arch.h"

namespace sanctum {
namespace bare {

// Page size in bytes.
constexpr size_t page_size() {
  return 1 << page_shift();
}

// The size of a page table entry, at a given level, in bytes.
constexpr inline size_t page_table_entry_size(size_t level) {
  return 1 << page_table_entry_shift(level);
}

// The number of page table entries at a given level.
constexpr inline size_t page_table_entries(size_t level) {
  return 1 << page_table_shift(level);
}

// The size of a page table at a given level, in bytes.
constexpr inline size_t page_table_size(size_t level) {
  return page_table_entries(level) * page_table_entry_size(level);
}

// The size of a page table at a given level, in pages.
constexpr inline size_t page_table_pages(size_t level) {
  return page_table_size(level) >> page_shift();
}

// Used to implement page_table_translated_bits.
constexpr inline size_t __page_table_translated_bits(size_t level, size_t sum) {
  return (level == page_table_levels()) ? sum :
      __page_table_translated_bits(level + 1, sum + page_table_shift(level));
}

// The total number of bits translated by the page table.
//
// This should be optimized to a constant by the compiler.
constexpr inline size_t page_table_translated_bits() {
  return __page_table_translated_bits(0, page_shift());
}

};  // namespace sanctum::bare
};  // namespace sanctum
#endif  // !defined(BARE_PAGE_TABLES_H_INCLUDED)
