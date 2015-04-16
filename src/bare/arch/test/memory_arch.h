#if !defined(BARE_ARCH_TEST_MEMORY_ARCH_H_INCLUDED)
#define BARE_ARCH_TEST_MEMORY_ARCH_H_INCLUDED

#include <cassert>  // Cache metadata operations use assert for bound-checking.

namespace sanctum {
namespace testing {

extern size_t dram_size;
extern size_t cache_levels;
constexpr size_t max_cache_levels = 4;
extern bool is_shared_cache[];
extern size_t cache_line_bits[], cache_set_bits[];

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
inline size_t read_cache_line_bits(size_t cache_level) {
  assert(cache_level < testing::cache_levels);
  return testing::cache_line_bits[cache_level];
}
inline size_t read_cache_set_bits(size_t cache_level) {
  assert(cache_level < testing::cache_levels);
  return testing::cache_set_bits[cache_level];
}

template<typename T> inline void bzero(phys_ptr<T> start, size_t bytes) {
}
template<typename T> inline void bcopy(phys_ptr<T> dest, phys_ptr<T> source,
    size_t bytes) {

}

};  // namespace sanctum::bare
};  // namespace sanctum

#endif  // !defined(BARE_ARCH_TEST_MEMORY_ARCH_H_INCLUDED)
