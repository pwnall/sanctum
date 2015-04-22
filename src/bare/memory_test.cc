#include "bare/memory.h"

#include "gtest/gtest.h"

using sanctum::bare::is_shared_cache;
using sanctum::bare::phys_ptr;
using sanctum::bare::read_cache_levels;
using sanctum::bare::read_cache_line_size;
using sanctum::bare::read_cache_set_count;
using sanctum::bare::read_dram_size;
using sanctum::bare::read_min_cache_index_shift;
using sanctum::bare::read_max_cache_index_shift;
using sanctum::testing::phys_buffer;
using sanctum::testing::phys_buffer_size;

TEST(MemoryTest, ReadDramSize) {
  sanctum::testing::dram_size = 1 << 30;
  ASSERT_EQ(1 << 30, read_dram_size());

  sanctum::testing::dram_size = 1 << 28;
  ASSERT_EQ(1 << 28, read_dram_size());
}

TEST(MemoryTest, ReadCacheLevels) {
  sanctum::testing::cache_levels = 3;
  ASSERT_EQ(3, read_cache_levels());

  sanctum::testing::cache_levels = 2;
  ASSERT_EQ(2, read_cache_levels());
}

TEST(MemoryTest, IsSharedCache) {
  sanctum::testing::cache_levels = 3;
  sanctum::testing::is_shared_cache[0] = false;
  sanctum::testing::is_shared_cache[1] = false;
  sanctum::testing::is_shared_cache[2] = true;
  ASSERT_EQ(false, is_shared_cache(0));
  ASSERT_EQ(false, is_shared_cache(1));
  ASSERT_EQ(true, is_shared_cache(2));
}

TEST(MemoryTest, ReadCacheLineSize) {
  sanctum::testing::cache_levels = 3;
  sanctum::testing::cache_line_size[0] = 64;
  sanctum::testing::cache_line_size[1] = 32;
  sanctum::testing::cache_line_size[2] = 16;
  ASSERT_EQ(64, read_cache_line_size(0));
  ASSERT_EQ(32, read_cache_line_size(1));
  ASSERT_EQ(16, read_cache_line_size(2));
}

TEST(MemoryTest, ReadCacheSetCount) {
  sanctum::testing::cache_levels = 3;
  sanctum::testing::cache_set_count[0] = 64;
  sanctum::testing::cache_set_count[1] = 512;
  sanctum::testing::cache_set_count[2] = 8192;
  ASSERT_EQ(64, read_cache_set_count(0));
  ASSERT_EQ(512, read_cache_set_count(1));
  ASSERT_EQ(8192, read_cache_set_count(2));
}

TEST(MemoryTest, ReadMinCacheIndexShift) {
  sanctum::testing::min_cache_index_shift = 9;
  ASSERT_EQ(9, read_min_cache_index_shift());

  sanctum::testing::min_cache_index_shift = 7;
  ASSERT_EQ(7, read_min_cache_index_shift());
}

TEST(MemoryTest, ReadMaxCacheIndexShift) {
  sanctum::testing::max_cache_index_shift = 13;
  ASSERT_EQ(13, read_max_cache_index_shift());

  sanctum::testing::max_cache_index_shift = 11;
  ASSERT_EQ(11, read_max_cache_index_shift());
}

TEST(MemoryTest, Bzero) {
  ASSERT_LE(256, phys_buffer_size);
  memset(phys_buffer, 0xcc, 256);

  phys_ptr<size_t> ptr{64};
  bzero(ptr, 0);
  ASSERT_NE(0, *(ptr - 2));
  ASSERT_NE(0, *(ptr - 1));
  ASSERT_NE(0, *ptr);
  ASSERT_NE(0, *(ptr + 1));
  ASSERT_NE(0, *(ptr + 2));

  bzero(ptr, sizeof(size_t));
  ASSERT_NE(0, *(ptr - 2));
  ASSERT_NE(0, *(ptr - 1));
  ASSERT_EQ(0, *ptr);
  ASSERT_NE(0, *(ptr + 1));
  ASSERT_NE(0, *(ptr + 1));
  ASSERT_NE(0, *(ptr + 2));

  bzero(ptr, 96);
  ASSERT_NE(0, *(ptr - 2));
  ASSERT_NE(0, *(ptr - 1));
  ASSERT_EQ(0, *ptr);
  ASSERT_EQ(0, *(ptr + 1));
  ASSERT_EQ(0, *(ptr + 2));

  phys_ptr<size_t> ptr2{64 + 96};
  ASSERT_NE(0, *ptr2);
  ASSERT_NE(0, *(ptr2 + 1));
  ASSERT_NE(0, *(ptr2 + 2));
  ASSERT_EQ(0, *(ptr2 - 1));
  ASSERT_EQ(0, *(ptr2 - 2));

  memset(phys_buffer, 0, 256);
}

TEST(MemoryTest, Bcopy) {
  ASSERT_LE(256, phys_buffer_size);
  memset(phys_buffer, 0xcc, 128);
  memset(phys_buffer + 128, 0xdd, 128);

  phys_ptr<size_t> ptr1{32};
  phys_ptr<size_t> ptr2{160};
  size_t value1 = *ptr1, value2 = *ptr2;

  bcopy(ptr1, ptr2, 0);
  ASSERT_EQ(value1, *(ptr1 - 2));
  ASSERT_EQ(value1, *(ptr1 - 1));
  ASSERT_EQ(value1, *ptr1);
  ASSERT_EQ(value1, *(ptr1 + 1));
  ASSERT_EQ(value1, *(ptr1 + 2));

  bcopy(ptr1, ptr2, sizeof(size_t));
  ASSERT_EQ(value1, *(ptr1 - 2));
  ASSERT_EQ(value1, *(ptr1 - 1));
  ASSERT_EQ(value2, *ptr1);
  ASSERT_EQ(value1, *(ptr1 + 1));
  ASSERT_EQ(value1, *(ptr1 + 2));

  bcopy(ptr1, ptr2, 48);
  ASSERT_EQ(value1, *(ptr1 - 2));
  ASSERT_EQ(value1, *(ptr1 - 1));
  ASSERT_EQ(value2, *ptr1);
  ASSERT_EQ(value2, *(ptr1 + 1));
  ASSERT_EQ(value2, *(ptr1 + 2));

  phys_ptr<size_t> ptr12{32 + 48};
  ASSERT_EQ(value2, *(ptr12 - 2));
  ASSERT_EQ(value2, *(ptr12 - 1));
  ASSERT_EQ(value1, *ptr12);
  ASSERT_EQ(value1, *(ptr12 + 1));
  ASSERT_EQ(value1, *(ptr12 + 2));

  memset(phys_buffer, 0, 256);
}
