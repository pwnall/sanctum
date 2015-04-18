#include "bare/page_tables.h"

#include "gtest/gtest.h"

using sanctum::bare::bits_in_a_page;
using sanctum::bare::page_size;
using sanctum::bare::page_shift;
using sanctum::bare::page_table_entries;
using sanctum::bare::page_table_entry_size;
using sanctum::bare::page_table_levels;
using sanctum::bare::page_table_pages;
using sanctum::bare::page_table_shift;
using sanctum::bare::page_table_size;
using sanctum::bare::set_eptbr;
using sanctum::bare::set_ptbr;
using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;
using sanctum::testing::set_current_core;
using sanctum::testing::set_core_count;

TEST(PageTablesTest, Geometry) {
  static_assert(page_table_levels() >= 1, "no page tables");
  static_assert(page_size() == (1 << page_shift()), "inconsistent page_size");
  static_assert(bits_in_a_page() == (page_size() * 8),
                "inconsistent bits_in_a_page");

  for (size_t level = 0; level < page_table_levels(); ++level) {
    ASSERT_EQ(page_table_entries(level), 1 << page_table_shift(level));
    ASSERT_EQ(page_table_size(level),
              page_table_entry_size(level) * page_table_entries(level));
    ASSERT_EQ(page_table_size(level),
              page_table_pages(level) * page_size());
  }
}

TEST(PageTablesTest, VirtualPtbr) {
  uintptr_t value = 0xcafe;

  set_core_count(8);

  set_ptbr(value);
  ASSERT_EQ(value, sanctum::testing::core_ptbr[0]);
  ASSERT_EQ(0, sanctum::testing::core_ptbr[3]);
  set_ptbr(0);
  ASSERT_EQ(0, sanctum::testing::core_ptbr[0]);
  set_current_core(3);
  set_ptbr(value);
  ASSERT_EQ(0, sanctum::testing::core_ptbr[0]);
  ASSERT_EQ(value, sanctum::testing::core_ptbr[3]);
  set_ptbr(0);
  ASSERT_EQ(0, sanctum::testing::core_ptbr[3]);
  set_current_core(0);
}

TEST(PageTablesTest, VirtualEptbr) {
  uintptr_t value = 0xcafe;

  set_core_count(8);

  set_eptbr(value);
  ASSERT_EQ(value, sanctum::testing::core_eptbr[0]);
  ASSERT_EQ(0, sanctum::testing::core_eptbr[3]);
  set_eptbr(0);
  ASSERT_EQ(0, sanctum::testing::core_eptbr[0]);
  set_current_core(3);
  set_eptbr(value);
  ASSERT_EQ(0, sanctum::testing::core_eptbr[0]);
  ASSERT_EQ(value, sanctum::testing::core_eptbr[3]);
  set_eptbr(0);
  ASSERT_EQ(0, sanctum::testing::core_eptbr[3]);
  set_current_core(0);
}
