#if !defined(BARE_ARCH_RISCV_PAGE_TABLES_ARCH_H_INCLUDED)
#define BARE_ARCH_RISCV_PAGE_TABLES_ARCH_H_INCLUDED

#include <cassert>  // Memory operations use assert for bound-checking.

namespace sanctum {

namespace bare {

// TODO: set the correct constants here
constexpr size_t page_shift() { return 13; }
constexpr size_t page_size() { 1 << page_shift(); }
constexpr size_t page_table_levels() { return 3; }
constexpr inline size_t page_table_shift(size_t level) { return 10; }
constexpr inline size_t page_table_entry_size(size_t level) {
  return sizeof(void *);
}

// The implementations below are pretty generic.
//
// Furthermore, on reasonable architectures, the compiler should optimize them
// into constants.

constexpr inline size_t page_table_entries(size_t level) {
  return 1 << page_table_shift(level);
}
constexpr inline size_t page_table_size(size_t level) {
  return page_table_entries(level) * page_table_entry_size(level);
}
constexpr inline size_t page_table_pages(size_t level) {
  return page_table_size(level) >> page_shift();
}
inline void set_eptbr(uintptr_t value) {
  // TODO: asm intrinsic
}
inline void set_ptbr(uintptr_t value) {
  // TODO: asm intrinsic
}

};  // namespace sanctum::bare
};  // namespace sanctum
#endif  // !definded(BARE_ARCH_RISCV_PAGE_TABLES_ARCH_H_INCLUDED)
