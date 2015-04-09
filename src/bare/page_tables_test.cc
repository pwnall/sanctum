#include "bare/page_tables.h"

#include "gtest/gtest.h"

using sanctum::bare::bits_in_a_page;
using sanctum::bare::page_size;
using sanctum::bare::page_shift;
using sanctum::bare::page_table_entries;
using sanctum::bare::page_table_entry_size;
using sanctum::bare::page_table_levels;
using sanctum::bare::page_table_shift;
using sanctum::bare::page_table_size;
using sanctum::bare::set_eptrr;
using sanctum::bare::set_ptrr;
using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;
using sanctum::testing::set_current_core;
using sanctum::testing::set_total_cores;

TEST(PageTablesTest, Geometry) {
  static_assert(page_table_levels() >= 1, "no page tables");
  static_assert(page_size() == (1 << page_shift()), "inconsistent page_size");
  static_assert(bits_in_a_page() == (page_size() * 8),
                "inconsistent bits_in_a_page");

  for (size_t level = 0; level < page_table_levels(); ++level) {
    ASSERT_EQ(page_table_entries(level), 1 << page_table_shift(level));
    ASSERT_EQ(page_table_size(level),
              page_table_entry_size(level) * page_table_entries(level));
  }
}

TEST(PageTablesTest, VirtualPtrr) {
  uintptr_t value = 0xcafe;

  set_total_cores(8);

  set_ptrr(value);
  ASSERT_EQ(value, sanctum::testing::core_ptrr[0]);
  ASSERT_EQ(0, sanctum::testing::core_ptrr[3]);
  set_ptrr(0);
  ASSERT_EQ(0, sanctum::testing::core_ptrr[0]);
  set_current_core(3);
  set_ptrr(value);
  ASSERT_EQ(0, sanctum::testing::core_ptrr[0]);
  ASSERT_EQ(value, sanctum::testing::core_ptrr[3]);
  set_ptrr(0);
  ASSERT_EQ(0, sanctum::testing::core_ptrr[3]);
  set_current_core(0);
}

TEST(PageTablesTest, VirtualEptrr) {
  uintptr_t value = 0xcafe;

  set_total_cores(8);

  set_eptrr(value);
  ASSERT_EQ(value, sanctum::testing::core_eptrr[0]);
  ASSERT_EQ(0, sanctum::testing::core_eptrr[3]);
  set_eptrr(0);
  ASSERT_EQ(0, sanctum::testing::core_eptrr[0]);
  set_current_core(3);
  set_eptrr(value);
  ASSERT_EQ(0, sanctum::testing::core_eptrr[0]);
  ASSERT_EQ(value, sanctum::testing::core_eptrr[3]);
  set_eptrr(0);
  ASSERT_EQ(0, sanctum::testing::core_eptrr[3]);
  set_current_core(0);
}
