#if !defined(BARE_ARCH_RISCV_CPU_CONTEXT_ARCH_H_INCLUDED)
#define BARE_ARCH_RISCV_CPU_CONTEXT_ARCH_H_INCLUDED

namespace sanctum {
namespace bare {

inline size_t read_core_count() {
  // TODO: asm intrinsic
  return 0;
}
inline size_t current_core() {
  // TODO: asm intrinsic
  return 0;
}
inline void flush_tlbs() {
  // TODO: asm intrinsic
}
inline void flush_private_caches() {
  // TODO: asm intrinsic
}
inline void set_cache_index_shift(size_t cache_index_shift) {
  // TODO: asm intrinsic
}
inline void set_eptbr(uintptr_t value) {
  // TODO: asm intrinsic
}
inline void set_ptbr(uintptr_t value) {
  // TODO: asm intrinsic
}
inline void set_epar_base(uintptr_t value) {
  // TODO: asm intrinsic
}
inline void set_par_base(uintptr_t value) {
  // TODO: asm intrinsic
}
inline void set_epar_mask(uintptr_t value) {
  // TODO: asm intrinsic
}
inline void set_par_mask(uintptr_t value) {
  // TODO: asm intrinsic
}
inline void set_epar_pmask(uintptr_t value) {
  // TODO: asm intrinsic
}
inline void set_par_pmask(uintptr_t value) {
  // TODO: asm intrinsic
}
inline void set_ev_base(uintptr_t value) {
  // TODO: asm intrinsic
}
inline void set_ev_mask(uintptr_t value) {
  // TODO: asm intrinsic
}
inline void set_drb_map(uintptr_t phys_addr) {
  // TODO: asm intrinsic
}
inline void set_edrb_map(uintptr_t phys_addr) {
  // TODO: asm intrinsic
}

struct exec_state_t {
  uintptr_t pc;       // x1
  uintptr_t stack;    // x14
  uintptr_t gprs[29]; // x2-13, x15-x31
};

};  // namespace sanctum::bare
};  // namespace sanctum
#endif  // !defined(BARE_ARCH_RISCV_CPU_CONTEXT_ARCH_H_INCLUDED)
