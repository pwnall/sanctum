#if !defined(BARE_ARCH_TEST_CPU_CONTEXT_ARCH_H_INCLUDED)
#define BARE_ARCH_TEST_CPU_CONTEXT_ARCH_H_INCLUDED

namespace sanctum {
namespace testing {

extern size_t core_count;
extern size_t current_core;
constexpr size_t max_cores = 32;

// For testing, we maintain a count of the number of times that TLBs and
// private caches were flushed. Tests reset the counters before exercising
// flushing-related functionality, then compare counters against expected
// values.
extern size_t core_tlb_flush_count[], core_cache_flush_count[];

// For testing, the cache index shift register is virtualized.
extern size_t core_cache_index_shift[];

// Sets the return value of current_core().
void set_current_core(size_t core_id);

// Sets the return value of read_core_count().
//
// This also resets the current core to 0.
void set_core_count(size_t new_core_count);

};  // namespace sanctum::testing
};  // namespace sanctum

namespace sanctum {
namespace bare {

inline size_t read_core_count() { return testing::core_count; }
inline size_t current_core() { return testing::current_core; }

inline void flush_tlbs() {
  testing::core_tlb_flush_count[current_core()] += 1;
}
inline void flush_private_caches() {
  testing::core_cache_flush_count[current_core()] += 1;
}
inline void set_cache_index_shift(size_t cache_index_shift) {
  testing::core_cache_index_shift[current_core()] = cache_index_shift;
}

struct enclave_exit_state_t {
  size_t rip;
};

};  // namespace sanctum::bare
};  // namespace sanctum

#endif  // !defined(BARE_ARCH_TEST_CPU_CONTEXT_ARCH_H_INCLUDED)
