#if !defined(BARE_ARCH_RISCV_BASE_TYPES_ARCH_H_INCLUDED)
#define BARE_ARCH_RISCV_BASE_TYPES_ARCH_H_INCLUDED

namespace sanctum {
namespace bare {

// NOTE: The sizes below reflect RV64.

using size_t      = unsigned long long;
using uintptr_t   = unsigned long long;
using uint64_t    = unsigned long long;
using uint32_t    = unsigned int;
using uint8_t     = unsigned char;

};  // namespace sanctum::bare
};  // namespace sanctum
#endif  // !definded(BARE_ARCH_RISCV_BASE_TYPES_ARCH_H_INCLUDED)
