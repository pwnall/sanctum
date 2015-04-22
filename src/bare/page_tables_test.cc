#include "bare/page_tables.h"

#include "gtest/gtest.h"

using sanctum::bare::is_valid_page_table_entry;
using sanctum::bare::page_size;
using sanctum::bare::page_shift;
using sanctum::bare::page_table_entries;
using sanctum::bare::page_table_entry_shift;
using sanctum::bare::page_table_entry_size;
using sanctum::bare::page_table_entry_target;
using sanctum::bare::page_table_levels;
using sanctum::bare::page_table_pages;
using sanctum::bare::page_table_shift;
using sanctum::bare::page_table_size;
using sanctum::bare::page_table_translated_bits;
using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;
using sanctum::bare::write_page_table_entry;
using sanctum::testing::phys_buffer;
using sanctum::testing::phys_buffer_size;
using sanctum::testing::set_current_core;
using sanctum::testing::set_core_count;

TEST(PageTablesTest, Geometry) {
  // NOTE: using static_assert to ensure that constexpr functions work as
  //       expected
  static_assert(page_table_levels() == 3, "page_table_levels");
  static_assert(page_shift() == 13, "page_shift");
  static_assert(page_size() == 8192, "page_size");

  static_assert(page_table_shift(0) == 11, "L0 page_table_shift");
  static_assert(page_table_entries(0) == 2048, "L0 page_table_entries");
  static_assert(page_table_entry_shift(0) == 4, "L0 page_table_entry_shift");
  static_assert(page_table_entry_size(0) == 16, "L0 page_table_entry_size");
  static_assert(page_table_size(0) == 32768, "L0 page_table_size");
  static_assert(page_table_pages(0) == 4, "L0 page_table_pages");

  static_assert(page_table_shift(1) == 10, "L1 page_table_shift");
  static_assert(page_table_entries(1) == 1024, "L1 page_table_entries");
  static_assert(page_table_entry_shift(1) == 3, "L1 page_table_entry_shift");
  static_assert(page_table_entry_size(1) == 8, "L1 page_table_entry_size");
  static_assert(page_table_size(1) == 8192, "L1 page_table_size");
  static_assert(page_table_pages(1) == 1, "L1 page_table_pages");

  static_assert(page_table_shift(2) == 12, "L1 page_table_shift");
  static_assert(page_table_entries(2) == 4096, "L1 page_table_entries");
  static_assert(page_table_entry_shift(2) == 5, "L1 page_table_entry_shift");
  static_assert(page_table_entry_size(2) == 32, "L1 page_table_entry_size");
  static_assert(page_table_size(2) == 131072, "L1 page_table_size");
  static_assert(page_table_pages(2) == 16, "L1 page_table_pages");

  static_assert(page_table_translated_bits() == 46,
      "page_table_translated_bits");
}

TEST(PageTablesTest, IsValidPageTableEntry) {
  uintptr_t addr = 160;
  ASSERT_LE(256, phys_buffer_size);
  memset(phys_buffer, 0, 256);

  *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)) = 0;
  ASSERT_EQ(false, is_valid_page_table_entry(addr, 0));
  ASSERT_EQ(false, is_valid_page_table_entry(addr, 1));
  ASSERT_EQ(false, is_valid_page_table_entry(addr, 2));
  *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)) = 1;
  ASSERT_EQ(true, is_valid_page_table_entry(addr, 0));
  ASSERT_EQ(true, is_valid_page_table_entry(addr, 1));
  ASSERT_EQ(true, is_valid_page_table_entry(addr, 2));

  *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)) = 0xcafebabe000;
  ASSERT_EQ(false, is_valid_page_table_entry(addr, 0));
  ASSERT_EQ(false, is_valid_page_table_entry(addr, 1));
  ASSERT_EQ(false, is_valid_page_table_entry(addr, 2));
  *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)) = 0xcafebabe001;
  ASSERT_EQ(true, is_valid_page_table_entry(addr, 0));
  ASSERT_EQ(true, is_valid_page_table_entry(addr, 1));
  ASSERT_EQ(true, is_valid_page_table_entry(addr, 2));
}

TEST(PageTablesTest, PageTableEntryTarget) {
  uintptr_t addr = 160;
  ASSERT_LE(256, phys_buffer_size);
  memset(phys_buffer, 0, 256);

  *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)) = 0;
  ASSERT_EQ(0, page_table_entry_target(addr, 0));
  ASSERT_EQ(0, page_table_entry_target(addr, 1));
  ASSERT_EQ(0, page_table_entry_target(addr, 2));
  *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)) = 1;
  ASSERT_EQ(0, page_table_entry_target(addr, 0));
  ASSERT_EQ(0, page_table_entry_target(addr, 1));
  ASSERT_EQ(0, page_table_entry_target(addr, 2));

  *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)) = 0xcafebabe000;
  ASSERT_EQ(0xcafebabe000, page_table_entry_target(addr, 0));
  ASSERT_EQ(0xcafebabe000, page_table_entry_target(addr, 1));
  ASSERT_EQ(0xcafebabe000, page_table_entry_target(addr, 2));
  *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)) = 0xcafebabe001;
  ASSERT_EQ(0xcafebabe000, page_table_entry_target(addr, 0));
  ASSERT_EQ(0xcafebabe000, page_table_entry_target(addr, 1));
  ASSERT_EQ(0xcafebabe000, page_table_entry_target(addr, 2));
  *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)) = 0xcafebabffff;
  ASSERT_EQ(0xcafebabe000, page_table_entry_target(addr, 0));
  ASSERT_EQ(0xcafebabe000, page_table_entry_target(addr, 1));
  ASSERT_EQ(0xcafebabe000, page_table_entry_target(addr, 2));
}

TEST(PageTablesTest, WritePageTableEntry) {
  uintptr_t addr = 160;
  ASSERT_LE(256, phys_buffer_size);
  memset(phys_buffer, 0, 256);

  write_page_table_entry(addr, 0, 0xcafebabe000, 0);
  ASSERT_EQ(0xcafebabe001,
      *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));
  write_page_table_entry(addr, 1, 0xcafebabe000, 0);
  ASSERT_EQ(0xcafebabe001,
      *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));
  write_page_table_entry(addr, 2, 0xcafebabe000, 0);
  ASSERT_EQ(0xcafebabe001,
      *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));

  write_page_table_entry(addr, 0, 0xcafebabe000, 0xffffffffff);
  ASSERT_EQ(0xcafebabffff,
      *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));
  write_page_table_entry(addr, 1, 0xcafebabe000, 0xffffffffff);
  ASSERT_EQ(0xcafebabffff,
      *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));
  write_page_table_entry(addr, 2, 0xcafebabe000, 0xffffffffff);
  ASSERT_EQ(0xcafebabffff,
      *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));
}
