#include "bit_masking.h"

#include "gtest/gtest.h"

using sanctum::bare::address_bits_for;
using sanctum::bare::is_aligned_to_mask;
using sanctum::bare::is_page_aligned;
using sanctum::bare::is_valid_range;
using sanctum::bare::is_valid_range_mask;
using sanctum::bare::phys_ptr;
using sanctum::bare::read_bitmap_bit;
using sanctum::bare::set_bitmap_bit;
using sanctum::testing::phys_buffer;
using sanctum::testing::phys_buffer_size;

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
  ASSERT_EQ(true, is_page_aligned(0x2000));
  ASSERT_EQ(false, is_page_aligned(0x2001));
  ASSERT_EQ(true, is_page_aligned(0x4000));
  ASSERT_EQ(false, is_page_aligned(0x4001));
  ASSERT_EQ(true, is_page_aligned(0x6000));
  ASSERT_EQ(false, is_page_aligned(0x7000));
}

TEST(BitMaskingTest, AddressBitsFor) {
  ASSERT_EQ(0, address_bits_for(1));
  ASSERT_EQ(1, address_bits_for(2));
  ASSERT_EQ(2, address_bits_for(3));
  ASSERT_EQ(2, address_bits_for(4));
  ASSERT_EQ(3, address_bits_for(5));
  ASSERT_EQ(3, address_bits_for(8));
  ASSERT_EQ(4, address_bits_for(9));
  ASSERT_EQ(4, address_bits_for(16));
  ASSERT_EQ(16, address_bits_for(65536));
}

TEST(BitMaskingTest, ReadSetBitmapBit) {
  constexpr uintptr_t addr = 160, addr2 = 200;
  constexpr uintptr_t zero_addr = 0;
  memset(phys_buffer, 0, phys_buffer_size);
  *(reinterpret_cast<size_t*>(phys_buffer + addr)) = 0xFFFF;

  phys_ptr<size_t> ptr{addr};
  ASSERT_EQ(true, read_bitmap_bit(ptr, 0));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 1));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 2));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 3));

  set_bitmap_bit(ptr, 0, false);
  ASSERT_EQ(0xFFFE, *ptr);
  ASSERT_EQ(false, read_bitmap_bit(ptr, 0));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 1));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 2));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 3));

  set_bitmap_bit(ptr, 3, false);
  ASSERT_EQ(0xFFF6, *ptr);
  ASSERT_EQ(false, read_bitmap_bit(ptr, 0));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 1));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 2));
  ASSERT_EQ(false, read_bitmap_bit(ptr, 3));

  set_bitmap_bit(ptr, 0, true);
  ASSERT_EQ(0xFFF7, *ptr);
  ASSERT_EQ(true, read_bitmap_bit(ptr, 0));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 1));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 2));
  ASSERT_EQ(false, read_bitmap_bit(ptr, 3));

  phys_ptr<size_t> ptr2{addr2};
  set_bitmap_bit(ptr, 320, true);
  ASSERT_EQ(1, *ptr2);
  ASSERT_EQ(false, read_bitmap_bit(ptr, 319));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 320));
  ASSERT_EQ(false, read_bitmap_bit(ptr, 321));
  ASSERT_EQ(false, read_bitmap_bit(ptr, 322));

  set_bitmap_bit(ptr, 321, true);
  ASSERT_EQ(3, *ptr2);
  ASSERT_EQ(false, read_bitmap_bit(ptr, 319));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 320));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 321));
  ASSERT_EQ(false, read_bitmap_bit(ptr, 322));

  set_bitmap_bit(ptr, 320, false);
  ASSERT_EQ(2, *ptr2);
  ASSERT_EQ(false, read_bitmap_bit(ptr, 319));
  ASSERT_EQ(false, read_bitmap_bit(ptr, 320));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 321));
  ASSERT_EQ(false, read_bitmap_bit(ptr, 322));

  size_t msb = 1;
  while(true) {
    size_t msb2 = msb << 1;
    if (msb2 == 0)
      break;
    msb = msb2;
  }
  set_bitmap_bit(ptr, 319, true);
  ASSERT_EQ(msb, *(ptr2 - 1));
  ASSERT_EQ(false, read_bitmap_bit(ptr, 317));
  ASSERT_EQ(false, read_bitmap_bit(ptr, 318));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 319));
  ASSERT_EQ(false, read_bitmap_bit(ptr, 320));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 321));
  ASSERT_EQ(false, read_bitmap_bit(ptr, 322));

  set_bitmap_bit(ptr, 320, true);
  ASSERT_EQ(msb, *(ptr2 - 1));
  ASSERT_EQ(3, *ptr2);
  ASSERT_EQ(false, read_bitmap_bit(ptr, 317));
  ASSERT_EQ(false, read_bitmap_bit(ptr, 318));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 319));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 320));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 321));
  ASSERT_EQ(false, read_bitmap_bit(ptr, 322));

  set_bitmap_bit(ptr, 318, true);
  ASSERT_EQ(size_t(msb + size_t(msb >> 1)), *(ptr2 - 1));
  ASSERT_EQ(3, *ptr2);
  ASSERT_EQ(false, read_bitmap_bit(ptr, 317));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 318));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 319));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 320));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 321));
  ASSERT_EQ(false, read_bitmap_bit(ptr, 322));

  set_bitmap_bit(ptr, 319, false);
  ASSERT_EQ(size_t(msb >> 1), *(ptr2 - 1));
  ASSERT_EQ(3, *ptr2);
  ASSERT_EQ(false, read_bitmap_bit(ptr, 317));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 318));
  ASSERT_EQ(false, read_bitmap_bit(ptr, 319));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 320));
  ASSERT_EQ(true, read_bitmap_bit(ptr, 321));
  ASSERT_EQ(false, read_bitmap_bit(ptr, 322));

  *ptr = 0;
  *ptr2 = 0;
  *(ptr2 - 1) = 0;
}

