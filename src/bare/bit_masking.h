#if !defined(BARE_BIT_MASKING_H_INCLUDED)
#define BARE_BIT_MASKING_H_INCLUDED

#include "base_types.h"
#include "page_tables.h"
#include "phys_ptr.h"

namespace sanctum {
namespace bare {

// Checks if an argument is a valid range mask.
//
// Valid range masks are written in binary as a sequence of 0s, followed by a
// sequence of 1s. Equivalently, the size of a base/mask range must be a power
// of two.
constexpr inline bool is_valid_range_mask(uintptr_t mask) {
  return (mask & (mask + 1)) == 0;
}
// Checks if an address is aligned with regard to a mask.
constexpr inline bool is_aligned_to_mask(uintptr_t address, uintptr_t mask) {
  return (address & mask) == 0;
}
// Checks the validity of a base/mask range.
//
// Valid range masks are written in binary as a sequence of 0s, followed by a
// sequence of 1s. Equivalently, the size of a base/mask range must be a power
// of two.
//
// A valid range base has 0s in the positions where the range mask has 1s.
// Equivalently, the range's base must be size-aligned.
constexpr inline bool is_valid_range(uintptr_t base, uintptr_t mask) {
  return is_aligned_to_mask(base, mask) && is_valid_range_mask(mask);
}
// Checks if an address is aligned to an address translation page.
constexpr inline bool is_page_aligned(uintptr_t address) {
  return is_aligned_to_mask(address, page_size() - 1);
}

// Sets or clears a bit in a bitmap.
//
// `value` is true for setting the bit, or false for clearing the bit.
inline void set_bitmap_bit(phys_ptr<size_t> bitmap, size_t bit, bool value) {
  constexpr size_t bits_in_size_t = sizeof(size_t) * 8;

  // NOTE: relying on the compiler to optimize division to bitwise shift
  size_t offset = bit / bits_in_size_t;
  size_t mask = 1 << (bit % bits_in_size_t);

  if (value)
    *(bitmap + offset) |= mask;
  else
    *(bitmap + offset) &= ~mask;
}
// Returns the value of a bit in a bitmap.
inline bool read_bitmap_bit(phys_ptr<size_t> bitmap, size_t bit) {
  constexpr size_t bits_in_size_t = sizeof(size_t) * 8;

  // NOTE: relying on the compiler to optimize division to bitwise shift
  size_t offset = bit / bits_in_size_t;
  size_t mask = 1 << (bit % bits_in_size_t);

  return (*(bitmap + offset) & mask) != 0;
}

};  // namespace sanctum::bare
};  // namespace sanctum
#endif  // !defined(BARE_BIT_MASKING_H_INCLUDED)
