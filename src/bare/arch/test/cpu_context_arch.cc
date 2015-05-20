#include "../../cpu_context.h"

#include "../../phys_ptr.h"

#include <cassert>  // Core configuration use assert for bound checking.

using namespace sanctum::bare;

namespace sanctum {
namespace testing {

size_t core_count = 0;
size_t current_core = 0;
size_t dram_region_bitmap_words = 0;

size_t core_tlb_flush_count[max_cores];
size_t core_cache_flush_count[max_cores];
size_t core_cache_index_shift[max_cores];
uintptr_t core_ptbr[max_cores];
uintptr_t core_eptbr[max_cores];
uintptr_t core_ev_base[max_cores];
uintptr_t core_ev_mask[max_cores];
uintptr_t core_par_base[max_cores];
uintptr_t core_epar_base[max_cores];
uintptr_t core_par_mask[max_cores];
uintptr_t core_epar_mask[max_cores];
uintptr_t core_par_pmask[max_cores];
uintptr_t core_epar_pmask[max_cores];
size_t core_drb_map[max_cores][max_dram_region_bitmap_words];
size_t core_edrb_map[max_cores][max_dram_region_bitmap_words];

void set_current_core(size_t core_id) {
  assert(core_id < core_count);
  current_core = core_id;
}

void set_core_count(size_t new_core_count) {
  assert(new_core_count > 0 && new_core_count <= max_cores);
  core_count = new_core_count;
  current_core = 0;
}

void set_dram_region_bitmap_words(size_t new_dram_region_bitmap_words) {
  assert(new_dram_region_bitmap_words <= max_dram_region_bitmap_words);
  dram_region_bitmap_words = new_dram_region_bitmap_words;
}

void snapshot_dram_region_bitmap(uintptr_t* bitmap_register,
    uintptr_t phys_addr) {
  phys_ptr<size_t> map_ptr{phys_addr};
  for (size_t i = 0; i < dram_region_bitmap_words; ++i, map_ptr += 1)
    bitmap_register[i] = *map_ptr;
}

};  // namespace sanctum::testing
};  // namespace sanctum


