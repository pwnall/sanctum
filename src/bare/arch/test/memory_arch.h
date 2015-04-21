#if !defined(BARE_ARCH_TEST_MEMORY_ARCH_H_INCLUDED)
#define BARE_ARCH_TEST_MEMORY_ARCH_H_INCLUDED

#include <cassert>  // Cache metadata operations use assert for bound-checking.

namespace sanctum {
namespace testing {

extern size_t dram_size;
extern size_t cache_levels;
extern size_t min_cache_index_shift, max_cache_index_shift;
constexpr size_t max_cache_levels = 4;
extern bool is_shared_cache[];
extern size_t cache_line_size[], cache_set_count[];

};  // namespace sanctum::testing
};  // namespace sanctum

namespace sanctum {
namespace bare {

inline size_t read_dram_size() { return testing::dram_size; }
inline size_t read_cache_levels() { return testing::cache_levels; }
inline bool is_shared_cache(size_t cache_level) {
  assert(cache_level < testing::cache_levels);
  return testing::is_shared_cache[cache_level];
}
inline size_t read_cache_line_size(size_t cache_level) {
  assert(cache_level < testing::cache_levels);
  return testing::cache_line_size[cache_level];
}
inline size_t read_cache_set_count(size_t cache_level) {
  assert(cache_level < testing::cache_levels);
  return testing::cache_set_count[cache_level];
}
inline size_t read_min_cache_index_shift() {
  return testing::min_cache_index_shift;
}
inline size_t read_max_cache_index_shift() {
  return testing::max_cache_index_shift;
}
template<typename T> inline void bzero(phys_ptr<T> start, size_t bytes) {
  // NOTE: relying on compiler to optimize division to bitwise shift
  size_t words = bytes / sizeof(T);
  assert(uintptr_t(start + words) <= testing::phys_buffer_size);

  phys_ptr<T> end = start + words;
  for (; start != end; start += 1)
    *start = T{0};
}
template<typename T> inline void bcopy(phys_ptr<T> dest, phys_ptr<T> source,
    size_t bytes) {
  // NOTE: relying on compiler to optimize division to bitwise shift
  size_t words = bytes / sizeof(T);
  assert(uintptr_t(source + words) <= testing::phys_buffer_size);
  assert(uintptr_t(dest + words) <= testing::phys_buffer_size);
  // NOTE: checking that the buffers are non-overlapping, per API contract
  assert(uintptr_t(source + words) <= uintptr_t(dest) ||
      uintptr_t(dest + words) <= uintptr_t(source));

  phys_ptr<T> end = source + words;
  for (; source != end; source += 1, dest += 1)
    *dest = *source;
}

};  // namespace sanctum::bare
};  // namespace sanctum

#endif  // !defined(BARE_ARCH_TEST_MEMORY_ARCH_H_INCLUDED)
