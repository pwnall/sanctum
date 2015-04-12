#if !defined(BARE_ARCH_TEST_PAGE_TABLES_ARCH_H_INCLUDED)
#define BARE_ARCH_TEST_PAGE_TABLES_ARCH_H_INCLUDED

#include "../../cpu_context.h"  // For current_core().

namespace sanctum {
namespace testing {

// For testing, the page table root registers are virtualized.
extern uintptr_t core_ptbr[], core_eptbr[];

};  // namespace sanctum::testing

namespace bare {

constexpr size_t page_shift() { return 12; }
constexpr size_t page_table_levels() { return 4; }
constexpr inline size_t page_table_shift(size_t level) { return 9; }
constexpr inline size_t page_table_entry_size(size_t level) {
  return sizeof(void *);
}

// The definitions below replicate the RISC V definitions.

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
  testing::core_eptbr[current_core()] = value;
}
inline void set_ptbr(uintptr_t value) {
  testing::core_ptbr[current_core()] = value;
}

};  // namespace sanctum::bare
};  // namespace sanctum
#endif  // !definded(BARE_ARCH_TEST_PAGE_TABLES_ARCH_H_INCLUDED)

