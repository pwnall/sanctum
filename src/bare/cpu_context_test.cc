#include "bare/cpu_context.h"

#include "gtest/gtest.h"

using sanctum::testing::set_current_core;
using sanctum::testing::set_total_cores;

TEST(CpuContextTest, VirtualCurrentTotalCores) {
  set_total_cores(8);
  ASSERT_EQ(8, sanctum::testing::total_cores);
  ASSERT_EQ(0, sanctum::testing::current_core);

  set_current_core(3);
  ASSERT_EQ(3, sanctum::testing::current_core);
  ASSERT_EQ(8, sanctum::testing::total_cores);

  set_total_cores(4);
  ASSERT_EQ(4, sanctum::testing::total_cores);
  ASSERT_EQ(0, sanctum::testing::current_core);
}
