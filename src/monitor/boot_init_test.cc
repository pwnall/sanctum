#include "boot_init.h"

#include "bare/memory.h"
#include "bare/page_tables.h"
#include "dram_regions.h"
#include "metadata.h"

#include "gtest/gtest.h"

using sanctum::bare::page_size;
using sanctum::bare::page_shift;
using sanctum::internal::boot_init_dram_regions;
using sanctum::internal::boot_init_metadata;
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

namespace {

// Sets up the test rig with the toy memory parameters from the Sanctum paper.
void set_up_paper_memory_model() {
  sanctum::testing::dram_size = 1 << 18;
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
