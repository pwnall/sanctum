#include "dram_regions_inl.h"

#include "boot_init.h"

#include "gtest/gtest.h"

using sanctum::bare::atomic_load;
using sanctum::bare::atomic_store;
using sanctum::bare::phys_ptr;
using sanctum::internal::boot_init_dram_regions;
using sanctum::internal::boot_init_metadata;
using sanctum::internal::boot_init_dynamic_arrays;
using sanctum::internal::bzero_dram_region;
using sanctum::internal::clamped_dram_region_for;
using sanctum::internal::clear_dram_region_lock;
using sanctum::internal::core_info_t;
using sanctum::internal::dram_region_for;
using sanctum::internal::dram_region_info_t;
using sanctum::internal::dram_regions_info_t;
using sanctum::internal::dram_region_page_for;
using sanctum::internal::dram_region_start;
using sanctum::internal::dram_region_tlb_flush;
using sanctum::internal::dram_stripe_for;
using sanctum::internal::dram_stripe_page_for;
using sanctum::internal::g_core;
using sanctum::internal::g_dram_region;
using sanctum::internal::g_dram_regions;
using sanctum::internal::is_dram_address;
using sanctum::internal::is_dynamic_dram_region;
using sanctum::internal::is_valid_dram_region;
using sanctum::internal::read_dram_region_owner;
using sanctum::internal::test_and_set_dram_region_lock;

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

  sanctum::testing::set_core_count(4);
}

}

class DramRegionInlTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    set_up_paper_memory_model();
    boot_init_dram_regions();
    boot_init_metadata();
    boot_init_dynamic_arrays();
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

TEST_F(DramRegionInlTest, DramRegionLocks) {
  clear_dram_region_lock(5);
  test_and_set_dram_region_lock(5);
  ASSERT_EQ(atomic_flag_test_and_set(
    &((g_dram_region + 5)->*(&dram_region_info_t::lock))), 1);
  clear_dram_region_lock(5);
  ASSERT_EQ(atomic_flag_test_and_set(
    &((g_dram_region + 5)->*(&dram_region_info_t::lock))), 0);
  clear_dram_region_lock(5);

  clear_dram_region_lock(0);
  clear_dram_region_lock(1);
  clear_dram_region_lock(2);
  ASSERT_EQ(test_and_set_dram_region_lock(0), 0);
  ASSERT_EQ(test_and_set_dram_region_lock(0), 1);
  ASSERT_EQ(test_and_set_dram_region_lock(1), 0);
  ASSERT_EQ(test_and_set_dram_region_lock(1), 1);
  ASSERT_EQ(test_and_set_dram_region_lock(2), 0);
  ASSERT_EQ(test_and_set_dram_region_lock(2), 1);
  clear_dram_region_lock(1);
  ASSERT_EQ(test_and_set_dram_region_lock(0), 1);
  ASSERT_EQ(test_and_set_dram_region_lock(1), 0);
  ASSERT_EQ(test_and_set_dram_region_lock(2), 1);
}

TEST_F(DramRegionInlTest, ReadDramRegionOwner) {
  (g_dram_region + 0)->*(&dram_region_info_t::owner) = 0x42424242;
  (g_dram_region + 1)->*(&dram_region_info_t::owner) = 0xabababab;
  (g_dram_region + 2)->*(&dram_region_info_t::owner) = 0x98765432;
  (g_dram_region + 5)->*(&dram_region_info_t::owner) = 0x12345678;
  (g_dram_region + 7)->*(&dram_region_info_t::owner) = 0xfccffccf;
  ASSERT_EQ(read_dram_region_owner(0), 0x42424242);
  ASSERT_EQ(read_dram_region_owner(1), 0xabababab);
  ASSERT_EQ(read_dram_region_owner(2), 0x98765432);
  ASSERT_EQ(read_dram_region_owner(5), 0x12345678);
  ASSERT_EQ(read_dram_region_owner(7), 0xfccffccf);
}

TEST_F(DramRegionInlTest, BzeroDramRegion) {
  for (size_t i = 0; i < 256 * 1024; i += sizeof(uintptr_t))
    *(phys_ptr<uintptr_t>{i}) = ~0;
  bzero_dram_region(1);

  for (size_t i = 0; i < 32 * 1024; i += sizeof(uintptr_t))
    ASSERT_EQ(*(phys_ptr<uintptr_t>{i}), ~0);
  for (size_t i = 32 * 1024; i < 64 * 1024; i += sizeof(uintptr_t))
    ASSERT_EQ(*(phys_ptr<uintptr_t>{i}), 0);
  for (size_t i = 64 * 1024; i < 256 * 1024; i += sizeof(uintptr_t))
    ASSERT_EQ(*(phys_ptr<uintptr_t>{i}), ~0);
}

TEST_F(DramRegionInlTest, DramRegionTlbFlush) {
  sanctum::testing::core_tlb_flush_count[0] = 16;
  sanctum::testing::core_tlb_flush_count[1] = 32;
  sanctum::testing::core_tlb_flush_count[2] = 64;
  sanctum::testing::core_tlb_flush_count[3] = 128;
  sanctum::testing::set_current_core(2);
  atomic_store(&(g_dram_regions->*(&dram_regions_info_t::block_clock)),
               static_cast<size_t>(0x12345678));
  atomic_store(&((g_core + 2)->*(&core_info_t::flushed_at)),
               static_cast<size_t>(0));

  dram_region_tlb_flush();

  ASSERT_EQ(sanctum::testing::core_tlb_flush_count[0], 16);
  ASSERT_EQ(sanctum::testing::core_tlb_flush_count[1], 32);
  ASSERT_EQ(sanctum::testing::core_tlb_flush_count[2], 65);
  ASSERT_EQ(sanctum::testing::core_tlb_flush_count[3], 128);
  ASSERT_EQ(atomic_load(&((g_core + 2)->*(&core_info_t::flushed_at))),
            static_cast<size_t>(0x12345678));
}

// TODO: test dram_stripe_for in multi-stripe memory setting
// TODO: test dram_region_page_for in multi-stripe memory setting
// TODO: test bzero_dram_region in multi-stripe memory setting
