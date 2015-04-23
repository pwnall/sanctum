#if !defined(BARE_ARCH_TEST_PHYS_PTR_ARCH_H_INCLUDED)
#define BARE_ARCH_TEST_PHYS_PTR_ARCH_H_INCLUDED

#include <cassert>  // Memory operations use assert for bound-checking.

namespace sanctum {
namespace testing {

// For testing purposes, physical memory operations occur in this array.
extern char* phys_buffer;
extern size_t phys_buffer_size;
// This should be called once during test setup.
void init_phys_buffer(size_t buffer_size);

};  // namespace sanctum::testing

namespace bare {

// NOTE: specializing physptr_t for uint32_t and uint64_t is guaranteed to
//       cover size_t and uintptr_t

template<> inline phys_ref<uintptr_t>::operator uintptr_t() const {
  // NOTE: the assert would be prohibitively expensive in real code, but this
  //       implementation is only used by unit tests
  assert(addr + sizeof(uintptr_t) <= testing::phys_buffer_size);
  return *(reinterpret_cast<uintptr_t *>(&testing::phys_buffer[addr]));
}
template<> inline phys_ref<uintptr_t>& phys_ref<uintptr_t>::
    operator =(const uintptr_t& value) {
  // NOTE: the assert would be prohibitively expensive in real code, but this
  //       implementation is only used by unit tests
  assert(addr + sizeof(uintptr_t) <= testing::phys_buffer_size);
  *(reinterpret_cast<uintptr_t *>(&testing::phys_buffer[addr])) = value;
  return *this;
}

template<> inline phys_ref<uint32_t>::operator uint32_t() const {
  // NOTE: the assert would be prohibitively expensive in real code, but this
  //       implementation is only used by unit tests
  assert(addr + sizeof(uint32_t) <= testing::phys_buffer_size);
  return *(reinterpret_cast<uint32_t *>(&testing::phys_buffer[addr]));
}
template<> inline phys_ref<uint32_t>& phys_ref<uint32_t>::
    operator =(const uint32_t& value) {
  // NOTE: the assert would be prohibitively expensive in real code, but this
  //       implementation is only used by unit tests
  assert(addr + sizeof(uint32_t) <= testing::phys_buffer_size);
  *(reinterpret_cast<uint32_t *>(&testing::phys_buffer[addr])) = value;
  return *this;
}

};  // namespace sanctum::bare
};  // namespace sanctum
#endif  // !definded(BARE_ARCH_TEST_PHYS_PTR_ARCH_H_INCLUDED)
