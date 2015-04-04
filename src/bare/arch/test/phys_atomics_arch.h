#if !defined(BARE_ARCH_RISCV_PHYS_ATOMICS_ARCH_H_INCLUDED)
#define BARE_ARCH_RISCV_PHYS_ATOMICS_ARCH_H_INCLUDED

namespace sanctum {
namespace bare {

// NOTE: Test-mode flags aren't actually atomic, because of the interactions
//       with phys_ptr. Also, there's no threading in tests, so it shouldn't
//       matter.
struct atomic_flag {
  uintptr_t __flag;
};

inline bool atomic_flag_test_and_set(phys_ptr<atomic_flag> flag) noexcept {
  bool old_value = flag->*(&atomic_flag::__flag) != 0;
  flag->*(&atomic_flag::__flag) = 1;
  return old_value;
}
inline void atomic_flag_clear(phys_ptr<atomic_flag> flag) noexcept {
  flag->*(&atomic_flag::__flag) = 0;
}

// NOTE: Test-mode atomics aren't actually atomic, just like atomic_flag.
template<> struct atomic<uintptr_t> {
  uintptr_t __value;
};

template<> inline void atomic_init(phys_ptr<atomic<uintptr_t>> object,
    uintptr_t value) noexcept {
  object->*(&atomic<uintptr_t>::__value) = value;
}
template<> inline uintptr_t atomic_load(phys_ptr<atomic<uintptr_t>> object)
    noexcept {
  return object->*(&atomic<uintptr_t>::__value);
}
template<> inline void atomic_store(phys_ptr<atomic<uintptr_t>> object,
    uintptr_t value) noexcept {
  object->*(&atomic<uintptr_t>::__value) = value;
}
template<> inline uintptr_t atomic_fetch_add(
    phys_ptr<atomic<uintptr_t>> object, uintptr_t value) noexcept {
  uintptr_t old_value = object->*(&atomic<uintptr_t>::__value);
  object->*(&atomic<uintptr_t>::__value) += value;
  return old_value;
}

};  // namespace sanctum::bare
};  // namespace sanctum
#endif  // !definded(BARE_ARCH_RISCV_PHYS_ATOMICS_ARCH_H_INCLUDED)
