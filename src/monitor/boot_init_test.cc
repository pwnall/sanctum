#include "boot_init.h"

#include "bare/cpu_context.h"
#include "bare/memory.h"
#include "bare/page_tables.h"
#include "cpu_core.h"
#include "dram_regions.h"
#include "enclave.h"
#include "metadata.h"

#include "gtest/gtest.h"

using sanctum::bare::current_core;
using sanctum::bare::page_size;
using sanctum::bare::page_shift;
using sanctum::bare::phys_ptr;
using sanctum::internal::boot_init_dram_regions;
using sanctum::internal::boot_init_dynamic_arrays;
using sanctum::internal::boot_init_metadata;
using sanctum::internal::boot_init_protection;
using sanctum::internal::core_info_t;
using sanctum::internal::dram_region_info_t;
using sanctum::internal::g_core;
using sanctum::internal::g_core_count;
using sanctum::internal::g_dma_range_end;
using sanctum::internal::g_dma_range_start;
using sanctum::internal::g_dram_region;
using sanctum::internal::g_dram_regions;
using sanctum::internal::g_dram_region_bitmap_words;
using sanctum::internal::g_dram_region_count;
using sanctum::internal::g_dram_region_mask;
using sanctum::internal::g_dram_region_shift;
using sanctum::internal::g_dram_size;
using sanctum::internal::g_dram_stripe_mask;
using sanctum::internal::g_dram_stripe_page_mask;
using sanctum::internal::g_dram_stripe_shift;
using sanctum::internal::g_metadata_region_pages;
using sanctum::internal::g_metadata_region_start;
using sanctum::internal::g_monitor_top;
using sanctum::internal::g_os_region_bitmap;

namespace {

// Sets up the test rig with the toy memory parameters from the Sanctum paper.
void set_up_paper_memory_model() {
  sanctum::testing::dram_size = 1 << 18;
  ASSERT_GE(sanctum::testing::phys_buffer_size, sanctum::testing::dram_size);
  sanctum::testing::cache_levels = 3;

  sanctum::testing::is_shared_cache[0] = false;
  sanctum::testing::is_shared_cache[1] = false;
  sanctum::testing::is_shared_cache[2] = true;

  sanctum::testing::cache_line_size[0] = 1 << 6;  // irrelevant to tests
  sanctum::testing::cache_line_size[1] = 1 << 6;  // irrelevant to tests
  sanctum::testing::cache_line_size[2] = 1 << 6;  // must be a power of 2

  sanctum::testing::cache_set_count[0] = 1 << 6;  // irrelevant to tests
  sanctum::testing::cache_set_count[1] = 1 << 8;  // irrelevant to tests
  sanctum::testing::cache_set_count[2] = 1 << 9;  // must be a power of 2

  sanctum::testing::min_cache_index_shift = 0;
  sanctum::testing::max_cache_index_shift = 16;

  sanctum::testing::set_core_count(4);
}

void check_dram_geometry_invariants() {
  constexpr size_t page_mask = page_size() - 1;

  // The masks must be disjoint.
  ASSERT_EQ(page_mask & g_dram_stripe_page_mask, 0);
  if (g_dram_stripe_page_mask != 0)
    ASSERT_LT(page_mask, g_dram_stripe_page_mask);
  ASSERT_EQ(g_dram_stripe_page_mask & g_dram_region_mask, 0);
  if (g_dram_stripe_page_mask != 0)
    ASSERT_LT(g_dram_stripe_page_mask, g_dram_region_mask);
  ASSERT_EQ(g_dram_region_mask & g_dram_stripe_mask, 0);
  if (g_dram_stripe_mask != 0)
    ASSERT_LT(g_dram_region_mask, g_dram_stripe_mask);

  // The shifts must match the masks.
  if (g_dram_stripe_page_mask != 0) {
    ASSERT_EQ(g_dram_stripe_page_mask & (1 << page_shift()),
              1 << page_shift());
    ASSERT_GE(g_dram_stripe_page_mask, 1 << page_shift());
  }
  ASSERT_EQ(g_dram_region_mask & (1 << g_dram_region_shift),
            1 << g_dram_region_shift);
  ASSERT_GE(g_dram_region_mask, 1 << g_dram_region_shift);
  if (g_dram_stripe_mask != 0) {
    ASSERT_EQ(g_dram_stripe_mask & (1 << g_dram_stripe_shift),
              1 << g_dram_stripe_shift);
    ASSERT_GE(g_dram_stripe_mask, 1 << g_dram_stripe_shift);
  }

  // The counts must match the masks.
  ASSERT_EQ((g_dram_region_mask >> g_dram_region_shift) + 1,
      g_dram_region_count);

  // The masks must combine to cover all the DRAM address bits.
  ASSERT_EQ(page_mask | g_dram_stripe_page_mask | g_dram_region_mask |
      g_dram_stripe_mask, g_dram_size - 1);
}

};  // anonymous namespace

TEST(BootInitTest, DramRegionsHappyPath) {
  set_up_paper_memory_model();
  sanctum::testing::max_cache_index_shift = 3;
  boot_init_dram_regions();

  ASSERT_EQ(g_dram_region_count, 8);
  ASSERT_EQ(g_dram_region_shift, 15);
  ASSERT_EQ(g_dram_stripe_shift, 18);
  ASSERT_EQ(g_dram_region_bitmap_words, 1);
  check_dram_geometry_invariants();
}

TEST(BootInitTest, DramRegionsNoShift) {
  set_up_paper_memory_model();
  sanctum::testing::max_cache_index_shift = 0;
  boot_init_dram_regions();

  ASSERT_EQ(g_dram_region_count, 8);
  ASSERT_EQ(g_dram_region_shift, 12);
  ASSERT_EQ(g_dram_stripe_shift, 15);
  check_dram_geometry_invariants();
}

TEST(BootInitTest, DramRegionsShiftOne) {
  set_up_paper_memory_model();
  sanctum::testing::max_cache_index_shift = 1;
  boot_init_dram_regions();

  ASSERT_EQ(g_dram_region_count, 8);
  ASSERT_EQ(g_dram_region_shift, 13);
  ASSERT_EQ(g_dram_stripe_shift, 16);
  check_dram_geometry_invariants();
}

TEST(BootInitTest, DramRegionsShiftTwo) {
  set_up_paper_memory_model();
  sanctum::testing::max_cache_index_shift = 2;
  boot_init_dram_regions();

  ASSERT_EQ(g_dram_region_count, 8);
  ASSERT_EQ(g_dram_region_shift, 14);
  ASSERT_EQ(g_dram_stripe_shift, 17);
  check_dram_geometry_invariants();
}

TEST(BootInitTest, MetadataHappyPath) {
  set_up_paper_memory_model();
  boot_init_dram_regions();
  boot_init_metadata();

  ASSERT_EQ(g_metadata_region_pages, 8);
  ASSERT_EQ(g_metadata_region_start, 1);
}

TEST(BootInitTest, MetadataNoShift) {
  set_up_paper_memory_model();
  sanctum::testing::max_cache_index_shift = 0;
  boot_init_dram_regions();
  boot_init_metadata();

  ASSERT_EQ(g_metadata_region_pages, 8);
  ASSERT_EQ(g_metadata_region_start, 1);
}

TEST(BootInitTest, MetadataShiftOne) {
  set_up_paper_memory_model();
  sanctum::testing::max_cache_index_shift = 1;
  boot_init_dram_regions();
  boot_init_metadata();

  ASSERT_EQ(g_metadata_region_pages, 8);
  ASSERT_EQ(g_metadata_region_start, 1);
}
TEST(BootInitTest, MetadataShiftTwo) {
  set_up_paper_memory_model();
  sanctum::testing::max_cache_index_shift = 2;
  boot_init_dram_regions();
  boot_init_metadata();

  ASSERT_EQ(g_metadata_region_pages, 8);
  ASSERT_EQ(g_metadata_region_start, 1);
}

TEST(BootInitTest, DynamicArrays) {
  set_up_paper_memory_model();

  boot_init_dram_regions();
  boot_init_metadata();

  g_monitor_top = 0x800;
  boot_init_dynamic_arrays();

  ASSERT_EQ(g_core_count, 4);
  ASSERT_EQ(static_cast<uintptr_t>(g_core), static_cast<uintptr_t>(0x800));
  ASSERT_EQ(static_cast<uintptr_t>(g_core + 4),
            static_cast<uintptr_t>(g_dram_region));
  ASSERT_EQ(g_dram_region_count, 8);
  ASSERT_EQ(static_cast<uintptr_t>(g_dram_region + 8),
            static_cast<uintptr_t>(g_dram_regions));
  ASSERT_EQ(static_cast<uintptr_t>(g_dram_regions + 1),
            static_cast<uintptr_t>(g_os_region_bitmap));
  ASSERT_EQ(static_cast<uintptr_t>(g_os_region_bitmap + 1), g_monitor_top);
}

TEST(BootInitTest, Protection) {
  set_up_paper_memory_model();

  boot_init_dram_regions();
  boot_init_metadata();

  g_monitor_top = 0x7363;
  boot_init_protection();

  ASSERT_EQ(current_core(), 0);
  ASSERT_EQ(g_monitor_top, 0x8000);
  ASSERT_EQ(sanctum::testing::core_par_base[0], 0);
  ASSERT_EQ(~sanctum::testing::core_par_mask[0], 0x7fff);

  ASSERT_EQ(g_dma_range_start, 0x8000);
  ASSERT_EQ(g_dma_range_end, 0x8001);
  ASSERT_EQ(sanctum::testing::dmar_base, 0x8000);
  ASSERT_EQ(~sanctum::testing::dmar_mask, 0);
}
