#if !defined(BARE_ARCH_TEST_BASE_TYPES_ARCH_H_INCLUDED)
#define BARE_ARCH_TEST_BASE_TYPES_ARCH_H_INCLUDED

#include <cstddef>  // size_t
#include <cstdint>  // uintptr_t, uint8_t, uint32_t, uint64_t

namespace sanctum {
namespace bare {

using size_t      = std::size_t;
using uintptr_t   = std::uintptr_t;
using uint8_t     = std::uint8_t;
using uint32_t    = std::uint32_t;
using uint64_t    = std::uint64_t;

};  // namespace sanctum::bare
};  // namespace sanctum
#endif  // !definded(BARE_ARCH_TEST_BASE_TYPES_ARCH_H_INCLUDED)
