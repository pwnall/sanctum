#if !defined(BARE_ARCH_RISCV_PHYS_PTR_ARCH_H_INCLUDED)
#define BARE_ARCH_RISCV_PHYS_PTR_ARCH_H_INCLUDED

namespace sanctum {
namespace bare {

// On RISC-V, the monitor runs in machine mode, where all loads and stores use
// physical addresses.

template<> inline phys_ref<uintptr_t>::operator uintptr_t() const {
  return *(reinterpret_cast<uintptr_t*>(addr));
}
template<> inline phys_ref<uintptr_t>& phys_ref<uintptr_t>::
    operator =(const uintptr_t& value) {
  *(reinterpret_cast<uintptr_t*>(addr)) = value;
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
