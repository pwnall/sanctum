#include "bare/base_types.h"

#include "gtest/gtest.h"

using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;

TEST(BaseTypesTest, Sizes) {
  static_assert(sizeof(uintptr_t) >= sizeof(void *), "uintptr_t too small");
  static_assert(sizeof(size_t) >= 2, "size_t too small");  // C99 lower limit
}
