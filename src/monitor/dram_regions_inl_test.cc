#include "dram_regions_inl.h"

#include "boot_init.h"

#include "gtest/gtest.h"

using sanctum::internal::boot_init_dram_regions;
using sanctum::internal::clamped_dram_region_for;
using sanctum::internal::dram_region_for;
using sanctum::internal::dram_region_page_for;
using sanctum::internal::dram_region_start;
using sanctum::internal::dram_stripe_for;
using sanctum::internal::dram_stripe_page_for;
using sanctum::internal::is_dram_address;
using sanctum::internal::is_dynamic_dram_region;
using sanctum::internal::is_valid_dram_region;

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

}

class DramRegionInlTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    set_up_paper_memory_model();
    boot_init_dram_regions();
  }
};

TEST_F(DramRegionInlTest, IsDramAddress) {
  EXPECT_EQ(is_dram_address(0), true);
  EXPECT_EQ(is_dram_address(1), true);
  EXPECT_EQ(is_dram_address(0x1000), true);
  EXPECT_EQ(is_dram_address(0x3ffff), true);
  EXPECT_EQ(is_dram_address(0x40000), false);
  EXPECT_EQ(is_dram_address(0x40001), false);
  EXPECT_EQ(is_dram_address(~0), false);
}

TEST_F(DramRegionInlTest, IsValidDramRegion) {
  EXPECT_EQ(is_valid_dram_region(0), true);
  EXPECT_EQ(is_valid_dram_region(1), true);
  EXPECT_EQ(is_valid_dram_region(2), true);
  EXPECT_EQ(is_valid_dram_region(6), true);
  EXPECT_EQ(is_valid_dram_region(7), true);
  EXPECT_EQ(is_valid_dram_region(8), false);
  EXPECT_EQ(is_valid_dram_region(16), false);
  EXPECT_EQ(is_valid_dram_region(~0), false);
}

TEST_F(DramRegionInlTest, IsDynamicDramRegion) {
  EXPECT_EQ(is_dynamic_dram_region(0), false);
  EXPECT_EQ(is_dynamic_dram_region(1), true);
  EXPECT_EQ(is_dynamic_dram_region(2), true);
  EXPECT_EQ(is_dynamic_dram_region(6), true);
  EXPECT_EQ(is_dynamic_dram_region(7), true);
  EXPECT_EQ(is_dynamic_dram_region(8), false);
  EXPECT_EQ(is_dynamic_dram_region(16), false);
  EXPECT_EQ(is_dynamic_dram_region(~0), false);
}

TEST_F(DramRegionInlTest, DramRegionStart) {
  EXPECT_EQ(dram_region_start(0), 0);
  EXPECT_EQ(dram_region_start(1), 0x8000);
  EXPECT_EQ(dram_region_start(2), 0x10000);
  EXPECT_EQ(dram_region_start(32), 0x100000);
}

TEST_F(DramRegionInlTest, DramRegionFor) {
  EXPECT_EQ(dram_region_for(0), 0);
  EXPECT_EQ(dram_region_for(1), 0);
  EXPECT_EQ(dram_region_for(0x7fff), 0);
  EXPECT_EQ(dram_region_for(0x8000), 1);
  EXPECT_EQ(dram_region_for(0x8001), 1);
  EXPECT_EQ(dram_region_for(0x9000), 1);
  EXPECT_EQ(dram_region_for(0xa000), 1);
  EXPECT_EQ(dram_region_for(0xe000), 1);
  EXPECT_EQ(dram_region_for(0xffff), 1);
  EXPECT_EQ(dram_region_for(0x10000), 2);
  EXPECT_EQ(dram_region_for(0x10001), 2);
  EXPECT_EQ(dram_region_for(0x3ffff), 7);
}

TEST_F(DramRegionInlTest, ClampedDramRegionFor) {
  EXPECT_EQ(clamped_dram_region_for(0), 0);
  EXPECT_EQ(clamped_dram_region_for(1), 0);
  EXPECT_EQ(clamped_dram_region_for(0x7fff), 0);
  EXPECT_EQ(clamped_dram_region_for(0x8000), 1);
  EXPECT_EQ(clamped_dram_region_for(0x8001), 1);
  EXPECT_EQ(clamped_dram_region_for(0x9000), 1);
  EXPECT_EQ(clamped_dram_region_for(0xa000), 1);
  EXPECT_EQ(clamped_dram_region_for(0xe000), 1);
  EXPECT_EQ(clamped_dram_region_for(0xffff), 1);
  EXPECT_EQ(clamped_dram_region_for(0x10000), 2);
  EXPECT_EQ(clamped_dram_region_for(0x10001), 2);
  EXPECT_EQ(clamped_dram_region_for(0x3ffff), 7);

  ASSERT_EQ(is_dram_address(0x40000), false);
  EXPECT_EQ(clamped_dram_region_for(0x40000), 0);
  EXPECT_EQ(clamped_dram_region_for(0x40001), 0);
  EXPECT_EQ(clamped_dram_region_for(0x48000), 0);
  EXPECT_EQ(clamped_dram_region_for(0x7ffff), 0);
}

TEST_F(DramRegionInlTest, DramStripePageFor) {
  EXPECT_EQ(dram_stripe_page_for(0), 0);
  EXPECT_EQ(dram_stripe_page_for(1), 0);
  EXPECT_EQ(dram_stripe_page_for(0xfff), 0);
  EXPECT_EQ(dram_stripe_page_for(0x1000), 1);
  EXPECT_EQ(dram_stripe_page_for(0x1001), 1);
  EXPECT_EQ(dram_stripe_page_for(0x1fff), 1);
  EXPECT_EQ(dram_stripe_page_for(0x2000), 2);
  EXPECT_EQ(dram_stripe_page_for(0x3000), 3);
  EXPECT_EQ(dram_stripe_page_for(0x4000), 4);
  EXPECT_EQ(dram_stripe_page_for(0x6000), 6);
  EXPECT_EQ(dram_stripe_page_for(0x7fff), 7);
  EXPECT_EQ(dram_stripe_page_for(0x8000), 0);
  EXPECT_EQ(dram_stripe_page_for(0x8001), 0);
  EXPECT_EQ(dram_stripe_page_for(0x9000), 1);
  EXPECT_EQ(dram_stripe_page_for(0xa000), 2);
  EXPECT_EQ(dram_stripe_page_for(0xe000), 6);
  EXPECT_EQ(dram_stripe_page_for(0xffff), 7);
  EXPECT_EQ(dram_stripe_page_for(0x10000), 0);
  EXPECT_EQ(dram_stripe_page_for(0x10001), 0);
  EXPECT_EQ(dram_stripe_page_for(0x3ffff), 7);
}

TEST_F(DramRegionInlTest, DramStripeFor) {
  EXPECT_EQ(dram_stripe_for(0), 0);
  EXPECT_EQ(dram_stripe_for(1), 0);
  EXPECT_EQ(dram_stripe_for(0xfff), 0);
  EXPECT_EQ(dram_stripe_for(0x1000), 0);
  EXPECT_EQ(dram_stripe_for(0x1001), 0);
  EXPECT_EQ(dram_stripe_for(0x1fff), 0);
  EXPECT_EQ(dram_stripe_for(0x2000), 0);
  EXPECT_EQ(dram_stripe_for(0x3000), 0);
  EXPECT_EQ(dram_stripe_for(0x4000), 0);
  EXPECT_EQ(dram_stripe_for(0x6000), 0);
  EXPECT_EQ(dram_stripe_for(0x7fff), 0);
  EXPECT_EQ(dram_stripe_for(0x8000), 0);
  EXPECT_EQ(dram_stripe_for(0x8001), 0);
  EXPECT_EQ(dram_stripe_for(0x9000), 0);
  EXPECT_EQ(dram_stripe_for(0xa000), 0);
  EXPECT_EQ(dram_stripe_for(0xe000), 0);
  EXPECT_EQ(dram_stripe_for(0xffff), 0);
  EXPECT_EQ(dram_stripe_for(0x10000), 0);
  EXPECT_EQ(dram_stripe_for(0x10001), 0);
  EXPECT_EQ(dram_stripe_for(0x3ffff), 0);
}

TEST_F(DramRegionInlTest, DramRegionPageFor) {
  EXPECT_EQ(dram_region_page_for(0), 0);
  EXPECT_EQ(dram_region_page_for(1), 0);
  EXPECT_EQ(dram_region_page_for(0xfff), 0);
  EXPECT_EQ(dram_region_page_for(0x1000), 1);
  EXPECT_EQ(dram_region_page_for(0x1001), 1);
  EXPECT_EQ(dram_region_page_for(0x1fff), 1);
  EXPECT_EQ(dram_region_page_for(0x2000), 2);
  EXPECT_EQ(dram_region_page_for(0x3000), 3);
  EXPECT_EQ(dram_region_page_for(0x4000), 4);
  EXPECT_EQ(dram_region_page_for(0x6000), 6);
  EXPECT_EQ(dram_region_page_for(0x7fff), 7);
  EXPECT_EQ(dram_region_page_for(0x8000), 0);
  EXPECT_EQ(dram_region_page_for(0x8001), 0);
  EXPECT_EQ(dram_region_page_for(0x9000), 1);
  EXPECT_EQ(dram_region_page_for(0xa000), 2);
  EXPECT_EQ(dram_region_page_for(0xe000), 6);
  EXPECT_EQ(dram_region_page_for(0xffff), 7);
  EXPECT_EQ(dram_region_page_for(0x10000), 0);
  EXPECT_EQ(dram_region_page_for(0x10001), 0);
  EXPECT_EQ(dram_region_page_for(0x3ffff), 7);
}

// TODO: test dram_stripe_for in multi-stripe memory setting
// TODO: test dram_region_page_for in multi-stripe memory setting
