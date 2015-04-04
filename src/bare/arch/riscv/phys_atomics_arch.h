#if !defined(BARE_ARCH_RISCV_PHYS_ATOMICS_ARCH_H_INCLUDED)
#define BARE_ARCH_RISCV_PHYS_ATOMICS_ARCH_H_INCLUDED

namespace sanctum {
namespace bare {

// Lock-free boolean type.
struct atomic_flag {
public:
  // Initializes the flag to an unknown value.
  inline atomic_flag() noexcept = default;

  // Atomic flags cannot be copied.
  atomic_flag(const atomic_flag&) = delete;
  atomic_flag& operator=(const atomic_flag&) = delete;
  atomic_flag& operator=(const atomic_flag&) volatile = delete;

  inline bool test_and_set() volatile noexcept {
    // TODO: asm intrinsic
    return false;
  }
  inline bool test_and_set() noexcept {
    // TODO: asm intrinsic
    return false;
  }
  inline void clear() volatile noexcept {
    // TODO: asm intrinsic
  }
  inline void clear() noexcept {
    // TODO: asm intrinsic
  }

private:
  int flag;
};

template<> struct atomic<uintptr_t> {
  uintptr_t __value;
};

inline template<> void atomic_init(phys_ptr<atomic<uintptr_t>> object,
    uintptr_t value) noexcept {
  // TODO: asm intrinsic
}
inline template<> uintptr_t atomic_load(phys_ptr<atomic<uintptr_t>> object)
    noexcept {
  // TODO: asm intrinsic
  return 0;
}
inline template<> void atomic_store(phys_ptr<atomic<uintptr_t>> object,
    uintptr_t value) noexcept {
  // TODO: asm intrinsic
}
inline template<> uintptr_t atomic_fetch_add(
    phys_ptr<atomic<uintptr_t>> object, uintptr_t value) noexcept {
  // TODO: asm intrinsic
  return 0;
}


};  // namespace sanctum::bare
};  // namespace sanctum
#endif  // !definded(BARE_ARCH_RISCV_PHYS_ATOMICS_ARCH_H_INCLUDED)
