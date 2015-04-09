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
inline size_t page_table_shift(size_t level);

// The size of a page table at a given level, in pages.
inline size_t page_table_pages(size_t level);

// The number of page table entries at a given level.
inline size_t page_table_entries(size_t level);

// The size of a page table at a given level, in bytes.
inline size_t page_table_size(size_t level);

// The size of a page table entry, at a given level, in bytes.
inline size_t page_table_entry_size(size_t level);


// Sets the EPTRR (enclave page table root register).
//
// This can only be issued by the security monitor. An invalid DRAM address
// will lock up or reboot the machine.
void set_eptrr(uintptr_t value);


// Sets the PTRR (page table root register).
//
// This can only be issued by the security monitor. An invalid DRAM address
// will lock up or reboot the machine.
void set_ptrr(uintptr_t value);

};  // namespace sanctum::bare
};  // namespace sanctum

// Per-architecture page table constants and operations.
#include "page_tables_arch.h"

namespace sanctum {
namespace bare {

// Page size in bytes.
constexpr size_t page_size() { return (1 << page_shift()); }

// Page size in bits.
constexpr size_t bits_in_a_page() { return page_size() * 8; }

};  // namespace sanctum::bare
};  // namespace sanctum
#endif  // !defined(BARE_PAGE_TABLES_H_INCLUDED)
