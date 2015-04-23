#if !defined(BARE_ARCH_TEST_BIT_MASKING_ARCH_H_INCLUDED)
#define BARE_ARCH_TEST_BIT_MASKING_ARCH_H_INCLUDED

namespace sanctum {
namespace bare {

constexpr inline bool is_big_endian() {
  // NOTE: We're just assuming that tests are compiled and executed on a
  //       little-endian system. If that's not the case, the SHA-256 tests will
  //       fail. Until someone complains, we won't bother figuring out a way to
  //       determine this at compile time.
  return false;
}

};  // namespace sanctum::bare
};  // namespace sanctum
#endif  // !definded(BARE_ARCH_TEST_BIT_MASKING_ARCH_H_INCLUDED)
