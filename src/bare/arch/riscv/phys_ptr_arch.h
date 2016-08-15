#if !defined(BARE_ARCH_RISCV_PHYS_PTR_ARCH_H_INCLUDED)
#define BARE_ARCH_RISCV_PHYS_PTR_ARCH_H_INCLUDED

namespace sanctum {
namespace bare {

// Specializing physptr_t for uint32_t and uint64_t is guaranteed to cover
// size_t and uintptr_t.
static_assert(sizeof(uintptr_t) == sizeof(uint32_t) ||
    sizeof(uintptr_t) == sizeof(uint64_t), "uintptr_t isn't 32-bit or 64-bit");
static_assert(sizeof(size_t) == sizeof(uint32_t) ||
    sizeof(size_t) == sizeof(uint64_t), "size_t isn't 32-bit or 64-bit");

// On RISC-V, the monitor runs in machine mode, where all loads and stores use
// physical addresses.

template<> inline phys_ref<uint64_t>::operator uint64_t() const {
  return *(reinterpret_cast<uint64_t*>(addr));
}
template<> inline phys_ref<uint64_t>& phys_ref<uint64_t>::
    operator =(const uint64_t& value) {
  *(reinterpret_cast<uint64_t*>(addr)) = value;
  return *this;
}

template<> inline phys_ref<uint32_t>::operator uint32_t() const {
  return *(reinterpret_cast<uint32_t*>(addr));
}
template<> inline phys_ref<uint32_t>& phys_ref<uint32_t>::
    operator =(const uint32_t& value) {
  *(reinterpret_cast<uint32_t*>(addr)) = value;
  return *this;
}

};  // namespace sanctum::bare
};  // namespace sanctum
#endif  // !definded(BARE_ARCH_RISCV_PHYS_PTR_ARCH_H_INCLUDED)
