#if !defined(BARE_ARCH_RISCV_PHYS_PTR_ARCH_H_INCLUDED)
#define BARE_ARCH_RISCV_PHYS_PTR_ARCH_H_INCLUDED

namespace sanctum {
namespace bare {

template<> inline phys_ref<size_t>::operator size_t() const {
  // TODO: asm intrinsic
  return 0;
}
template<> inline phys_ref<size_t>& phys_ref<size_t>::
    operator =(const size_t& value) {
  // TODO: asm intrinsic
  return *this;
}

template<> inline phys_ref<uint32_t>::operator uint32_t() const {
  // TODO: asm intrinsic
  return 0;
}
template<> inline phys_ref<uint32_t>& phys_ref<uint32_t>::
    operator =(const uint32_t& value) {
  // TODO: asm intrinsic
  return *this;
}

};  // namespace sanctum::bare
};  // namespace sanctum
#endif  // !definded(BARE_ARCH_RISCV_PHYS_PTR_ARCH_H_INCLUDED)
