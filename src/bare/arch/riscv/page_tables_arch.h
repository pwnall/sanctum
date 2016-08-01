#if !defined(BARE_ARCH_RISCV_PAGE_TABLES_ARCH_H_INCLUDED)
#define BARE_ARCH_RISCV_PAGE_TABLES_ARCH_H_INCLUDED

#include "../../phys_ptr.h"

namespace sanctum {
namespace bare {

constexpr size_t page_shift() {
  return 12;
}

// NOTE: The constants below reflect RV39.

constexpr size_t page_table_levels() {
  return 3;
}
constexpr inline size_t page_table_shift(size_t level) {
  return 9;
}
constexpr inline size_t page_table_entry_shift(size_t level) {
  return 3;  // 8 bytes per page table entry
}

inline bool is_valid_page_table_entry(uintptr_t entry_addr, size_t level) {
  return *(phys_ptr<uintptr_t>{entry_addr}) & 1;
}
inline uintptr_t page_table_entry_target(uintptr_t entry_addr, size_t level) {
  uintptr_t target_mask = ~((1 << page_shift()) - 1);
  return *(phys_ptr<uintptr_t>{entry_addr}) & target_mask;
}
inline void write_page_table_entry(uintptr_t entry_addr, size_t level,
    uintptr_t target, uintptr_t acl) {
  uintptr_t acl_mask = (1 << page_shift()) - 1;
  acl &= acl_mask;  // Mask off non-ACL bits.
  acl |= 1;         // Force valid to true.
  *(phys_ptr<uintptr_t>{entry_addr}) = target | acl;
}

};  // namespace sanctum::bare
};  // namespace sanctum
#endif  // !definded(BARE_ARCH_RISCV_PAGE_TABLES_ARCH_H_INCLUDED)
