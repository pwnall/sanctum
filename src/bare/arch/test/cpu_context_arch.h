#if !defined(BARE_ARCH_TEST_CPU_CONTEXT_ARCH_H_INCLUDED)
#define BARE_ARCH_TEST_CPU_CONTEXT_ARCH_H_INCLUDED

namespace sanctum {
namespace testing {

extern size_t core_count;
extern size_t current_core;
constexpr size_t max_cores = 32;
extern size_t dram_region_bitmap_words;
constexpr size_t max_dram_region_bitmap_words = 8;

// For testing, we maintain a count of the number of times that TLBs and
// private caches were flushed. Tests reset the counters before exercising
// flushing-related functionality, then compare counters against expected
// values.
extern size_t core_tlb_flush_count[], core_cache_flush_count[];

// For testing, the cache index shift register is virtualized.
extern size_t core_cache_index_shift[];

// For testing, the page walker input registers are virtualized.
extern uintptr_t core_ptbr[], core_eptbr[], core_ev_base[], core_ev_mask[];
extern uintptr_t core_par_base[], core_epar_base[];
extern uintptr_t core_par_mask[], core_epar_mask[];
extern uintptr_t core_par_pmask[], core_epar_pmask[];
extern size_t core_drb_map[][max_dram_region_bitmap_words];
extern size_t core_edrb_map[][max_dram_region_bitmap_words];

// Sets the return value of current_core().
void set_current_core(size_t core_id);

// Sets the return value of read_core_count().
//
// This also resets the current core to 0.
void set_core_count(size_t new_core_count);

// Sets the size of DRAM region bitmap registers, in words.
//
// The size of the registers cannot be read by monitor code. It just decides
// how much memory is snapshotted into the virtualized per-core registers.
void set_dram_region_bitmap_words(size_t new_dram_region_bitmap_words);

// Copies (virtual) physical memory into a virtualized bitmap register.
void snapshot_dram_region_bitmap(uintptr_t* bitmap_register,
    uintptr_t phys_addr);

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
inline void set_eptbr(uintptr_t value) {
  testing::core_eptbr[current_core()] = value;
}
inline void set_ptbr(uintptr_t value) {
  testing::core_ptbr[current_core()] = value;
}
inline void set_epar_base(uintptr_t value) {
  testing::core_epar_base[current_core()] = value;
}
inline void set_par_base(uintptr_t value) {
  testing::core_par_base[current_core()] = value;
}
inline void set_epar_mask(uintptr_t value) {
  testing::core_epar_mask[current_core()] = value;
}
inline void set_par_mask(uintptr_t value) {
  testing::core_par_mask[current_core()] = value;
}
inline void set_epar_pmask(uintptr_t value) {
  testing::core_epar_pmask[current_core()] = value;
}
inline void set_par_pmask(uintptr_t value) {
  testing::core_par_pmask[current_core()] = value;
}
inline void set_ev_base(uintptr_t value) {
  testing::core_ev_base[current_core()] = value;
}
inline void set_ev_mask(uintptr_t value) {
  testing::core_ev_mask[current_core()] = value;
}
inline void set_drb_map(uintptr_t phys_addr) {
  testing::snapshot_dram_region_bitmap(
      testing::core_drb_map[current_core()], phys_addr);
}
inline void set_edrb_map(uintptr_t phys_addr) {
  testing::snapshot_dram_region_bitmap(
      testing::core_edrb_map[current_core()], phys_addr);
}

struct enclave_exit_state_t {
  size_t rip;
};

};  // namespace sanctum::bare
};  // namespace sanctum

#endif  // !defined(BARE_ARCH_TEST_CPU_CONTEXT_ARCH_H_INCLUDED)
