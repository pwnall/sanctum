#include "bare/cpu_context.h"

#include "gtest/gtest.h"

using sanctum::bare::current_core;
using sanctum::bare::flush_tlbs;
using sanctum::bare::flush_private_caches;
using sanctum::bare::read_core_count;
using sanctum::bare::set_cache_index_shift;
using sanctum::testing::set_current_core;
using sanctum::testing::set_core_count;

TEST(CpuContextTest, VirtualCurrentTotalCores) {
  set_core_count(8);
  ASSERT_EQ(8, read_core_count());
  ASSERT_EQ(0, current_core());
  ASSERT_EQ(8, sanctum::testing::core_count);
  ASSERT_EQ(0, sanctum::testing::current_core);

  set_current_core(3);
  ASSERT_EQ(3, current_core());
  ASSERT_EQ(8, read_core_count());
  ASSERT_EQ(3, sanctum::testing::current_core);
  ASSERT_EQ(8, sanctum::testing::core_count);

  set_core_count(4);
  ASSERT_EQ(4, read_core_count());
  ASSERT_EQ(0, current_core());
  ASSERT_EQ(4, sanctum::testing::core_count);
  ASSERT_EQ(0, sanctum::testing::current_core);
}

TEST(CpuContextTest, FlushTlbs) {
  set_core_count(8);
  sanctum::testing::core_tlb_flush_count[0] = 0;
  sanctum::testing::core_tlb_flush_count[3] = 0;

  flush_tlbs();
  ASSERT_EQ(1, sanctum::testing::core_tlb_flush_count[0]);
  ASSERT_EQ(0, sanctum::testing::core_tlb_flush_count[3]);

  set_current_core(3);
  flush_tlbs();
  ASSERT_EQ(1, sanctum::testing::core_tlb_flush_count[0]);
  ASSERT_EQ(1, sanctum::testing::core_tlb_flush_count[3]);
  flush_tlbs();
  ASSERT_EQ(1, sanctum::testing::core_tlb_flush_count[0]);
  ASSERT_EQ(2, sanctum::testing::core_tlb_flush_count[3]);

  set_current_core(0);
  flush_tlbs();
  flush_tlbs();
  ASSERT_EQ(3, sanctum::testing::core_tlb_flush_count[0]);
  ASSERT_EQ(2, sanctum::testing::core_tlb_flush_count[3]);

  sanctum::testing::core_tlb_flush_count[0] = 0;
  sanctum::testing::core_tlb_flush_count[3] = 0;
}

TEST(CpuContextTest, FlushPrivateCaches) {
  set_core_count(8);
  sanctum::testing::core_cache_flush_count[0] = 0;
  sanctum::testing::core_cache_flush_count[3] = 0;

  flush_private_caches();
  ASSERT_EQ(1, sanctum::testing::core_cache_flush_count[0]);
  ASSERT_EQ(0, sanctum::testing::core_cache_flush_count[3]);

  set_current_core(3);
  flush_private_caches();
  ASSERT_EQ(1, sanctum::testing::core_cache_flush_count[0]);
  ASSERT_EQ(1, sanctum::testing::core_cache_flush_count[3]);
  flush_private_caches();
  ASSERT_EQ(1, sanctum::testing::core_cache_flush_count[0]);
  ASSERT_EQ(2, sanctum::testing::core_cache_flush_count[3]);

  set_current_core(0);
  flush_private_caches();
  flush_private_caches();
  ASSERT_EQ(3, sanctum::testing::core_cache_flush_count[0]);
  ASSERT_EQ(2, sanctum::testing::core_cache_flush_count[3]);

  sanctum::testing::core_cache_flush_count[0] = 0;
  sanctum::testing::core_cache_flush_count[3] = 0;
}

TEST(CpuContextTest, VirtualCacheIndexShift) {
  set_core_count(8);
  sanctum::testing::core_cache_index_shift[0] = 0;
  sanctum::testing::core_cache_index_shift[3] = 0;

  set_cache_index_shift(11);
  ASSERT_EQ(11, sanctum::testing::core_cache_index_shift[0]);
  ASSERT_EQ(0, sanctum::testing::core_cache_index_shift[3]);

  set_current_core(3);
  set_cache_index_shift(10);
  ASSERT_EQ(11, sanctum::testing::core_cache_index_shift[0]);
  ASSERT_EQ(10, sanctum::testing::core_cache_index_shift[3]);

  set_current_core(0);
  set_cache_index_shift(10);
  ASSERT_EQ(10, sanctum::testing::core_cache_index_shift[0]);
  ASSERT_EQ(10, sanctum::testing::core_cache_index_shift[3]);
}

