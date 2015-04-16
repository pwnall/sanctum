#include "bit_masking.h"

#include "gtest/gtest.h"

using sanctum::bare::is_aligned_to_mask;
using sanctum::bare::is_page_aligned;
using sanctum::bare::is_valid_range;
using sanctum::bare::is_valid_range_mask;

TEST(BitMaskingTest, IsValidRangeMask) {
  ASSERT_EQ(true, is_valid_range_mask(0x00));
  ASSERT_EQ(true, is_valid_range_mask(0x01));
  ASSERT_EQ(true, is_valid_range_mask(0x03));
  ASSERT_EQ(true, is_valid_range_mask(0x07));
  ASSERT_EQ(true, is_valid_range_mask(0x0F));
  ASSERT_EQ(true, is_valid_range_mask(0x1F));
  ASSERT_EQ(true, is_valid_range_mask(0x3F));
  ASSERT_EQ(true, is_valid_range_mask(0x7F));
  ASSERT_EQ(true, is_valid_range_mask(0xFF));

  ASSERT_EQ(false, is_valid_range_mask(0x02));
  ASSERT_EQ(false, is_valid_range_mask(0x04));
  ASSERT_EQ(false, is_valid_range_mask(0x05));
  ASSERT_EQ(false, is_valid_range_mask(0x06));
  ASSERT_EQ(false, is_valid_range_mask(0x08));
  ASSERT_EQ(false, is_valid_range_mask(0x09));
  ASSERT_EQ(false, is_valid_range_mask(0x0A));
  ASSERT_EQ(false, is_valid_range_mask(0x0E));
  ASSERT_EQ(false, is_valid_range_mask(0xFE));
}

TEST(BitMaskingTest, IsAlignedToMask) {
  ASSERT_EQ(true, is_aligned_to_mask(0x10, 0x0F));
  ASSERT_EQ(false, is_aligned_to_mask(0x18, 0x0F));
  ASSERT_EQ(true, is_aligned_to_mask(0x100, 0xFF));
  ASSERT_EQ(false, is_aligned_to_mask(0x101, 0xFF));
  ASSERT_EQ(true, is_aligned_to_mask(0x200, 0xFF));
  ASSERT_EQ(false, is_aligned_to_mask(0x220, 0xFF));
  ASSERT_EQ(true, is_aligned_to_mask(0x1000, 0xFF));
}

TEST(BitMaskingTest, IsValidRange) {
  ASSERT_EQ(true, is_valid_range(0x10, 0x0F));
  ASSERT_EQ(false, is_valid_range(0x10, 0x08));
  ASSERT_EQ(false, is_valid_range(0x11, 0x0F));
  ASSERT_EQ(false, is_valid_range(0x11, 0x08));
}

TEST(BitMaskingTest, IsPageAligned) {
  ASSERT_EQ(true, is_page_aligned(0));
  ASSERT_EQ(false, is_page_aligned(1));
  ASSERT_EQ(true, is_page_aligned(0x1000));
  ASSERT_EQ(false, is_page_aligned(0x1001));
  ASSERT_EQ(true, is_page_aligned(0x2000));
  ASSERT_EQ(false, is_page_aligned(0x2001));
  ASSERT_EQ(true, is_page_aligned(0x3000));
  ASSERT_EQ(false, is_page_aligned(0x2800));
}
