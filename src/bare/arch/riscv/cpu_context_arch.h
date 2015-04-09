#if !defined(BARE_ARCH_RISCV_CPU_CONTEXT_ARCH_H_INCLUDED)
#define BARE_ARCH_RISCV_CPU_CONTEXT_ARCH_H_INCLUDED

namespace sanctum {
namespace bare {

inline size_t total_cores() {
  // TODO: asm intrinsic
  return 0;
}
inline size_t current_core() {
  // TODO: asm intrinsic
  return 0;
}

struct enclave_exit_state_t {
  // TODO: RISC V register state
};

};  // namespace sanctum::bare
};  // namespace sanctum

#endif  // !defined(BARE_ARCH_RISCV_CPU_CONTEXT_ARCH_H_INCLUDED)
