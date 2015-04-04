#if !defined(BARE_ATOMIC_H_INCLUDED)
#define BARE_ATOMIC_H_INCLUDED

#include "phys_ptr.h"

namespace sanctum {
namespace bare {

// C++11 lock-free atomic flag.

struct atomic_flag;
bool atomic_flag_test_and_set(phys_ptr<atomic_flag> flag) noexcept;
void atomic_flag_clear(phys_ptr<atomic_flag> flag) noexcept;

// C++11 atomic integers.
//
// The only specializations implemented by the bare-metal library are
// atomic<uintptr_t> and atomic<size_t>.

template<typename T> struct atomic;

template<typename T>
    void atomic_init(phys_ptr<atomic<T>> object, T value) noexcept;
template<typename T> T atomic_load(phys_ptr<atomic<T>> object) noexcept;
template<typename T>
    void atomic_store(phys_ptr<atomic<T>> object, T value) noexcept;
template<typename T>
    T atomic_fetch_add(phys_ptr<atomic<T>> object, T value) noexcept;


};  // namespace sanctum::bare
};  // namespace sanctum

// Per-architecture implementations of atomic operations.
#include "phys_atomics_arch.h"

#endif  // !definded(BARE_ATOMIC_H_INCLUDED)
