#include "boot_init.h"

#include "bare/bit_masking.h"
#include "bare/memory.h"
#include "cpu_core.h"
#include "dram_regions.h"
#include "dram_regions_inl.h"
#include "enclave.h"
#include "metadata_inl.h"

using sanctum::api::null_enclave_id;
using sanctum::api::os::dram_region_owned;
using sanctum::bare::atomic_flag_clear;
using sanctum::bare::atomic_init;
using sanctum::bare::address_bits_for;
using sanctum::bare::ceil_power_of_two;
using sanctum::bare::is_shared_cache;
using sanctum::bare::page_shift;
using sanctum::bare::page_size;
using sanctum::bare::pages_needed_for;
using sanctum::bare::read_cache_levels;
using sanctum::bare::read_cache_line_size;
using sanctum::bare::read_cache_set_count;
using sanctum::bare::read_core_count;
using sanctum::bare::read_dram_size;
using sanctum::bare::read_min_cache_index_shift;
using sanctum::bare::read_max_cache_index_shift;
using sanctum::bare::set_cache_index_shift;
using sanctum::bare::set_drb_map;
using sanctum::bare::set_dmar_base;
using sanctum::bare::set_dmar_mask;
using sanctum::bare::set_par_base;
using sanctum::bare::set_par_mask;
using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;

namespace sanctum {
namespace internal {  // sanctum::internal

uintptr_t g_monitor_top;

void boot_panic() {
  // TODO: come up with a better way to signal panic
  while(true) { }
}

void boot_init_monitor_top() {
  // TODO: implement this by reading some linker symbol
  g_monitor_top = 0;
}

void boot_init_dram_regions() {
  constexpr size_t bits_in_size_t = sizeof(size_t) * 8;

  g_dram_size = read_dram_size();
  size_t dram_address_bits = address_bits_for(g_dram_size);

  size_t cache_levels = read_cache_levels();
  for (size_t i = 0; i < cache_levels; ++i) {
    if (is_shared_cache(i) && i != cache_levels - 1)
      boot_panic();  // Sanctum assumes that only the LLC is shared.
  }
  size_t llc = cache_levels - 1;

  size_t line_size = read_cache_line_size(llc);
  size_t line_bits = address_bits_for(line_size);
  if (line_size != (1 << line_bits))
    boot_panic();  // Sanctum assumes power-of-two cache line sizes.

  size_t set_count = read_cache_set_count(llc);
  size_t set_bits = address_bits_for(set_count);
  if (set_count != (1 << set_bits))
    boot_panic();  // Sanctum assumes power-of-two cache set counts.

  size_t cache_bits = set_bits + line_bits;
  if (cache_bits <= page_shift())
    boot_panic();  // Address translation doesn't touch any cache indexing bit.
  size_t region_bits = cache_bits - page_shift();

  size_t stripe_page_bits = dram_address_bits - cache_bits;
  size_t max_shift = read_max_cache_index_shift();
  if (stripe_page_bits > max_shift) {
    // DRAM regions will not be continuous.
    stripe_page_bits = max_shift;
  }
  size_t min_shift = read_min_cache_index_shift();
  if (stripe_page_bits < min_shift) {
    // DRAM can't use the entire cache and some regions are invalid.
    //
    // It's not worth dealing with this case, not taking advantage of the
    // entire cache is pretty insane from a performance standpoint.
    boot_panic();  // Sanctum assumes that DRAM can use the entire cache.
  }

  // TODO: figure out if this requires coordination between cores.
  set_cache_index_shift(stripe_page_bits);

  g_dram_region_shift = page_shift() + stripe_page_bits;
  g_dram_stripe_shift = g_dram_region_shift + region_bits;

  g_dram_stripe_size = 1 << g_dram_region_shift;
  g_dram_region_count = 1 << region_bits;

  g_dram_stripe_page_mask = ((1 << stripe_page_bits) - 1) << page_shift();
  g_dram_region_mask = (g_dram_region_count - 1) << g_dram_region_shift;
  g_dram_stripe_mask =
      (g_dram_size - 1) >> g_dram_stripe_shift << g_dram_stripe_shift;

  // NOTE: relying on the compiler to optimize division to bitwise shift
  g_dram_region_bitmap_words =
      (g_dram_region_count + bits_in_size_t - 1) / bits_in_size_t;
}

void boot_init_metadata() {
  g_metadata_region_pages = g_dram_size >>
      (g_dram_stripe_shift - g_dram_region_shift + page_shift());

  // NOTE: relying on the compiler to optimize multiplication to bitwise shift
  size_t metadata_map_size =
      g_metadata_region_pages * sizeof(metadata_page_info_t);
  g_metadata_region_start = pages_needed_for(metadata_map_size);
}

void boot_init_dynamic_arrays() {
  g_core_count = read_core_count();
  g_core = phys_ptr<core_info_t>{g_monitor_top};
  g_monitor_top = static_cast<uintptr_t>(g_core + g_core_count);

  g_dram_region = phys_ptr<dram_region_info_t>{g_monitor_top};
  g_monitor_top = static_cast<uintptr_t>(g_dram_region + g_dram_region_count);
  for (size_t i = 0; i < g_dram_region_count; ++i) {
    phys_ptr<dram_region_info_t> region{g_dram_region + i};
    atomic_flag_clear(&(region->*(&dram_region_info_t::lock)));
    region->*(&dram_region_info_t::owner) = null_enclave_id;
    region->*(&dram_region_info_t::previous_owner) = null_enclave_id;
    region->*(&dram_region_info_t::pinned_pages) = 0;
    region->*(&dram_region_info_t::blocked_at) = 0;
  }

  g_dram_regions = phys_ptr<dram_regions_info_t>{g_monitor_top};
  g_monitor_top = static_cast<uintptr_t>(g_dram_regions + 1);
  atomic_init(&(g_dram_regions->*(&dram_regions_info_t::block_clock)),
      static_cast<size_t>(0));

  g_os_region_bitmap = phys_ptr<size_t>{g_monitor_top};
  g_monitor_top = static_cast<uintptr_t>(
      g_os_region_bitmap + g_dram_region_bitmap_words);
  for (size_t i = 0; i < g_dram_region_count; ++i)
    set_bitmap_bit(g_os_region_bitmap, i, 1);
}

void boot_init_protection() {
  // The monitor's code and data will be covered by a range in the address
  // translation, so we must round it up to a power of two.
  g_monitor_top = ceil_power_of_two(g_monitor_top);
  set_par_base(static_cast<uintptr_t>(0));
  set_par_mask(~(static_cast<uintptr_t>(g_monitor_top - 1)));
  set_drb_map(uintptr_t(g_os_region_bitmap));

  if (g_monitor_top > g_dram_stripe_size)
    boot_panic();  // Sanctum assumes that the monitor fits into a DRAM stripe.

  // NOTE: we're allowing DMA transfers for 1 byte at the top of the monitor.
  g_dma_range_start = g_monitor_top;
  g_dma_range_end = g_dma_range_start + 1;
  set_dmar_base(g_dma_range_start);
  set_dmar_mask(~(static_cast<uintptr_t>(0)));
}

};  // namespace sanctum::internal
};  // namespace sanctum
