#if !defined(BARE_ARCH_RISCV_PAGE_TABLES_ARCH_H_INCLUDED)
#define BARE_ARCH_RISCV_PAGE_TABLES_ARCH_H_INCLUDED

#include <cassert>  // Memory operations use assert for bound-checking.

namespace sanctum {

namespace bare {

// TODO: set the correct constants here
constexpr size_t page_shift() { return 12; }
constexpr size_t page_table_levels() { return 4; }
inline size_t page_table_shift(size_t level) { return 9; }
inline size_t page_table_pages(size_t level) { return 1; }
inline size_t page_table_entries(size_t level) {
  return 1 << page_table_shift(level);
}
inline size_t page_table_size(size_t level) {
  return page_table_pages(level) << page_shift();
}
inline size_t page_table_entry_size(size_t level) {
  return sizeof(void *);
}


inline void set_eptrr(uintptr_t value) {
  // TODO: asm intrinsic
}

// Sets the PTRR (page table root register).
//
// This can only be issued by the security monitor. An invalid DRAM address
// will lock up or reboot the machine.
inline void set_ptrr(uintptr_t value) {
  // TODO: asm intrinsic
}

};  // namespace sanctum::bare
};  // namespace sanctum
#endif  // !definded(BARE_ARCH_RISCV_PAGE_TABLES_ARCH_H_INCLUDED)
