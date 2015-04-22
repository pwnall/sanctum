#include "bare/phys_ptr.h"

#include "gtest/gtest.h"

using sanctum::bare::phys_ptr;
using sanctum::bare::phys_ref;
using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;
using sanctum::testing::phys_buffer;
using sanctum::testing::phys_buffer_size;

TEST(PhysPtrTest, HandlesUintptr) {
  constexpr uintptr_t addr = 160;  // Aligned by 32, should satisfy most archs.
  constexpr uintptr_t value = 0xbeef, write_value = 0xdeed;
  constexpr uintptr_t zero_addr = 0;
  ASSERT_LE(256, phys_buffer_size);
  memset(phys_buffer, 0, 256);
  *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)) = value;

  phys_ptr<uintptr_t> ptr_addr{addr};
  ASSERT_EQ(addr, uintptr_t{ptr_addr});
  ASSERT_EQ(value, *ptr_addr);

  phys_ptr<uintptr_t> ptr_copy_ctor{ptr_addr};
  ASSERT_EQ(addr, uintptr_t{ptr_copy_ctor});
  ASSERT_EQ(value, *ptr_copy_ctor);

  phys_ptr<uintptr_t> ptr_assign_copy{zero_addr};
  ptr_assign_copy = ptr_addr;
  ASSERT_EQ(addr, uintptr_t{ptr_assign_copy});
  ASSERT_EQ(value, *ptr_assign_copy);

  phys_ptr<uintptr_t> ptr_assign_move{zero_addr};
  ptr_assign_move = phys_ptr<uintptr_t>(addr);
  ASSERT_EQ(addr, uintptr_t{ptr_assign_move});
  ASSERT_EQ(value, *ptr_assign_move);

  *ptr_addr = write_value;
  ASSERT_EQ(write_value, *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));
  ASSERT_EQ(write_value, *ptr_addr);
  *ptr_addr = value;

  phys_ref<uintptr_t> ref = *ptr_addr;
  ASSERT_EQ(&ref, ptr_addr);

  *(reinterpret_cast<uintptr_t*>(phys_buffer + addr) + 1) = write_value;
  ASSERT_EQ(value, ptr_addr[0]);
  ASSERT_EQ(write_value, ptr_addr[1]);
  *(reinterpret_cast<uintptr_t*>(phys_buffer + addr) + 1) = 0;
  ptr_addr[2] = write_value;
  ASSERT_EQ(write_value,
            *(reinterpret_cast<uintptr_t*>(phys_buffer + addr) + 2));
  ptr_addr[2] = 0;

  phys_ref<uintptr_t> other_ref = ptr_addr[1];
  *(reinterpret_cast<uintptr_t*>(phys_buffer + addr) + 1) = write_value;
  *ptr_addr = ptr_addr[1];
  *ptr_addr = other_ref;
  ASSERT_EQ(write_value, ptr_addr[0]);
  ASSERT_EQ(write_value, ptr_addr[1]);
  *ptr_addr = value;
  ptr_addr[1] = 0;

  *(reinterpret_cast<uintptr_t*>(phys_buffer + addr) + 1) = write_value;
  *ptr_addr = ptr_addr[1];
  ASSERT_EQ(write_value, ptr_addr[0]);
  ASSERT_EQ(write_value, ptr_addr[1]);
  *ptr_addr = value;
  ptr_addr[1] = 0;

  ASSERT_EQ(true, ptr_addr == ptr_copy_ctor);
  ASSERT_EQ(false, ptr_addr == phys_ptr<uintptr_t>{addr + 1});

  ASSERT_EQ(false, ptr_addr != ptr_copy_ctor);
  ASSERT_EQ(true, ptr_addr != phys_ptr<uintptr_t>{addr + 1});

  ASSERT_EQ(&ptr_addr[16], ptr_addr + 16);

  phys_ptr<uintptr_t> ptr_sum = ptr_addr;
  ptr_sum += 16;
  ASSERT_EQ(&ptr_addr[16], ptr_sum);

  ASSERT_EQ(&ptr_addr[8], ptr_sum - 8);

  phys_ptr<uintptr_t> ptr_diff = ptr_sum;
  ptr_diff -= 8;
  ASSERT_EQ(&ptr_addr[8], ptr_diff);

  *ptr_addr += write_value;
  ASSERT_EQ(value + write_value,
            *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));
  ASSERT_EQ(value + write_value, *ptr_addr);
  *ptr_addr = value;

  *ptr_addr -= write_value;
  ASSERT_EQ(value - write_value,
            *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));
  ASSERT_EQ(value - write_value, *ptr_addr);
  *ptr_addr = value;

  static_assert(value != (value | write_value), "Invalid test setup.");
  *ptr_addr |= write_value;
  ASSERT_EQ(value | write_value,
            *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));
  ASSERT_EQ(value | write_value, *ptr_addr);
  *ptr_addr = value;

  static_assert(value != (value & write_value), "Invalid test setup.");
  *ptr_addr &= write_value;
  ASSERT_EQ(value & write_value,
            *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));
  ASSERT_EQ(value & write_value, *ptr_addr);
  *ptr_addr = value;

  ASSERT_EQ(0, uintptr_t{phys_ptr<uintptr_t*>::null()});

  *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)) = 0;
}

TEST(PhysPtrTest, HandlesSize) {
  constexpr uintptr_t addr = 160;  // Aligned by 32, should satisfy most archs.
  constexpr size_t value = 0xbeef, write_value = 0xdeed;
  constexpr uintptr_t zero_addr = 0;
  ASSERT_LE(256, phys_buffer_size);
  memset(phys_buffer, 0, 256);
  *(reinterpret_cast<size_t*>(phys_buffer + addr)) = value;

  phys_ptr<size_t> ptr_addr{addr};
  ASSERT_EQ(addr, uintptr_t{ptr_addr});
  ASSERT_EQ(value, *ptr_addr);

  phys_ptr<size_t> ptr_copy_ctor{ptr_addr};
  ASSERT_EQ(addr, uintptr_t{ptr_copy_ctor});
  ASSERT_EQ(value, *ptr_copy_ctor);

  phys_ptr<size_t> ptr_assign_copy{zero_addr};
  ptr_assign_copy = ptr_addr;
  ASSERT_EQ(addr, uintptr_t{ptr_assign_copy});
  ASSERT_EQ(value, *ptr_assign_copy);

  phys_ptr<size_t> ptr_assign_move{zero_addr};
  ptr_assign_move = phys_ptr<size_t>(addr);
  ASSERT_EQ(addr, uintptr_t{ptr_assign_move});
  ASSERT_EQ(value, *ptr_assign_move);

  *ptr_addr = write_value;
  ASSERT_EQ(write_value, *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));
  ASSERT_EQ(write_value, *ptr_addr);
  *ptr_addr = value;

  phys_ref<size_t> ref = *ptr_addr;
  ASSERT_EQ(&ref, ptr_addr);

  *(reinterpret_cast<size_t*>(phys_buffer + addr) + 1) = write_value;
  ASSERT_EQ(value, ptr_addr[0]);
  ASSERT_EQ(write_value, ptr_addr[1]);
  *(reinterpret_cast<size_t*>(phys_buffer + addr) + 1) = 0;
  ptr_addr[2] = write_value;
  ASSERT_EQ(write_value,
            *(reinterpret_cast<size_t*>(phys_buffer + addr) + 2));
  ptr_addr[2] = 0;

  phys_ref<size_t> other_ref = ptr_addr[1];
  *(reinterpret_cast<size_t*>(phys_buffer + addr) + 1) = write_value;
  *ptr_addr = ptr_addr[1];
  *ptr_addr = other_ref;
  ASSERT_EQ(write_value, ptr_addr[0]);
  ASSERT_EQ(write_value, ptr_addr[1]);
  *ptr_addr = value;
  ptr_addr[1] = 0;

  *(reinterpret_cast<size_t*>(phys_buffer + addr) + 1) = write_value;
  *ptr_addr = ptr_addr[1];
  ASSERT_EQ(write_value, ptr_addr[0]);
  ASSERT_EQ(write_value, ptr_addr[1]);
  *ptr_addr = value;
  ptr_addr[1] = 0;

  ASSERT_EQ(true, ptr_addr == ptr_copy_ctor);
  ASSERT_EQ(false, ptr_addr == phys_ptr<size_t>{addr + 1});

  ASSERT_EQ(false, ptr_addr != ptr_copy_ctor);
  ASSERT_EQ(true, ptr_addr != phys_ptr<size_t>{addr + 1});

  ASSERT_EQ(&ptr_addr[16], ptr_addr + 16);

  phys_ptr<size_t> ptr_sum = ptr_addr;
  ptr_sum += 16;
  ASSERT_EQ(&ptr_addr[16], ptr_sum);

  ASSERT_EQ(&ptr_addr[8], ptr_sum - 8);

  phys_ptr<size_t> ptr_diff = ptr_sum;
  ptr_diff -= 8;
  ASSERT_EQ(&ptr_addr[8], ptr_diff);

  *ptr_addr += write_value;
  ASSERT_EQ(value + write_value,
            *(reinterpret_cast<size_t*>(phys_buffer + addr)));
  ASSERT_EQ(value + write_value, *ptr_addr);
  *ptr_addr = value;

  *ptr_addr -= write_value;
  ASSERT_EQ(value - write_value,
            *(reinterpret_cast<size_t*>(phys_buffer + addr)));
  ASSERT_EQ(value - write_value, *ptr_addr);
  *ptr_addr = value;

  static_assert(value != (value | write_value), "Invalid test setup.");
  *ptr_addr |= write_value;
  ASSERT_EQ(value | write_value,
            *(reinterpret_cast<size_t*>(phys_buffer + addr)));
  ASSERT_EQ(value | write_value, *ptr_addr);
  *ptr_addr = value;

  static_assert(value != (value & write_value), "Invalid test setup.");
  *ptr_addr &= write_value;
  ASSERT_EQ(value & write_value,
            *(reinterpret_cast<size_t*>(phys_buffer + addr)));
  ASSERT_EQ(value & write_value, *ptr_addr);
  *ptr_addr = value;

  ASSERT_EQ(0, uintptr_t{phys_ptr<size_t*>::null()});

  *(reinterpret_cast<size_t*>(phys_buffer + addr)) = 0;
}

TEST(PhysPtrTest, HandlesPhysPtrOfSize) {
  constexpr uintptr_t addr = 160;  // Aligned by 32, should satisfy most archs.
  constexpr phys_ptr<size_t> value{0xbeef}, write_value{0xdeed};
  constexpr uintptr_t zero_addr = 0;
  ASSERT_LE(256, phys_buffer_size);
  memset(phys_buffer, 0, 256);
  *(reinterpret_cast<phys_ptr<size_t>*>(phys_buffer + addr)) = value;

  phys_ptr<phys_ptr<size_t>> ptr_addr{addr};
  ASSERT_EQ(addr, uintptr_t{ptr_addr});
  ASSERT_EQ(value, phys_ptr<size_t>(*ptr_addr));

  phys_ptr<phys_ptr<size_t>> ptr_copy_ctor{ptr_addr};
  ASSERT_EQ(addr, uintptr_t{ptr_copy_ctor});
  ASSERT_EQ(value, phys_ptr<size_t>(*ptr_copy_ctor));

  phys_ptr<phys_ptr<size_t>> ptr_assign_copy{zero_addr};
  ptr_assign_copy = ptr_addr;
  ASSERT_EQ(addr, uintptr_t{ptr_assign_copy});
  ASSERT_EQ(value, phys_ptr<size_t>(*ptr_assign_copy));

  phys_ptr<phys_ptr<size_t>> ptr_assign_move{zero_addr};
  ptr_assign_move = phys_ptr<phys_ptr<size_t>>(addr);
  ASSERT_EQ(addr, uintptr_t{ptr_assign_move});
  ASSERT_EQ(value, phys_ptr<size_t>(*ptr_assign_move));

  *ptr_addr = write_value;
  ASSERT_EQ(write_value, *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));
  ASSERT_EQ(write_value, phys_ptr<size_t>(*ptr_addr));
  *ptr_addr = value;

  phys_ref<phys_ptr<size_t>> ref = *ptr_addr;
  ASSERT_EQ(&ref, ptr_addr);

  *(reinterpret_cast<phys_ptr<size_t>*>(phys_buffer + addr) + 1) = write_value;
  ASSERT_EQ(value, phys_ptr<size_t>(ptr_addr[0]));
  ASSERT_EQ(write_value, phys_ptr<size_t>(ptr_addr[1]));
  *(reinterpret_cast<phys_ptr<size_t>*>(phys_buffer + addr) + 1) = 0;
  ptr_addr[2] = write_value;
  ASSERT_EQ(write_value,
            *(reinterpret_cast<phys_ptr<size_t>*>(phys_buffer + addr) + 2));
  ptr_addr[2] = 0;

  phys_ref<phys_ptr<size_t>> other_ref = ptr_addr[1];
  *(reinterpret_cast<phys_ptr<size_t>*>(phys_buffer + addr) + 1) = write_value;
  *ptr_addr = ptr_addr[1];
  *ptr_addr = other_ref;
  ASSERT_EQ(write_value, phys_ptr<size_t>(ptr_addr[0]));
  ASSERT_EQ(write_value, phys_ptr<size_t>(ptr_addr[1]));
  *ptr_addr = value;
  ptr_addr[1] = 0;

  *(reinterpret_cast<phys_ptr<size_t>*>(phys_buffer + addr) + 1) = write_value;
  *ptr_addr = ptr_addr[1];
  ASSERT_EQ(write_value, phys_ptr<size_t>(ptr_addr[0]));
  ASSERT_EQ(write_value, phys_ptr<size_t>(ptr_addr[1]));
  *ptr_addr = value;
  ptr_addr[1] = 0;

  ASSERT_EQ(true, ptr_addr == ptr_copy_ctor);
  ASSERT_EQ(false, ptr_addr == phys_ptr<phys_ptr<size_t>>{addr + 1});

  ASSERT_EQ(false, ptr_addr != ptr_copy_ctor);
  ASSERT_EQ(true, ptr_addr != phys_ptr<phys_ptr<size_t>>{addr + 1});

  ASSERT_EQ(&ptr_addr[16], ptr_addr + 16);

  phys_ptr<phys_ptr<size_t>> ptr_sum = ptr_addr;
  ptr_sum += 16;
  ASSERT_EQ(&ptr_addr[16], ptr_sum);

  ASSERT_EQ(&ptr_addr[8], ptr_sum - 8);

  phys_ptr<phys_ptr<size_t>> ptr_diff = ptr_sum;
  ptr_diff -= 8;
  ASSERT_EQ(&ptr_addr[8], ptr_diff);

  size_t offset = 0xfade;
  *ptr_addr += offset;
  ASSERT_EQ(value + offset,
            *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));
  ASSERT_EQ(value + offset, phys_ptr<size_t>(*ptr_addr));
  *ptr_addr = value;

  *ptr_addr -= offset;
  ASSERT_EQ(value - offset,
            *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));
  ASSERT_EQ(value - offset, phys_ptr<size_t>(*ptr_addr));
  *ptr_addr = value;

  ASSERT_EQ(0, uintptr_t{phys_ptr<phys_ptr<size_t>*>::null()});

  *(reinterpret_cast<phys_ptr<size_t>*>(phys_buffer + addr)) = 0;
}

TEST(PhysPtrTest, HandlesPhysPtrOfUint32) {
  constexpr uintptr_t addr = 160;  // Aligned by 32, should satisfy most archs.
  constexpr phys_ptr<uint32_t> value{0xbeef}, write_value{0xdeed};
  constexpr uintptr_t zero_addr = 0;
  ASSERT_LE(256, phys_buffer_size);
  memset(phys_buffer, 0, 256);
  *(reinterpret_cast<phys_ptr<uint32_t>*>(phys_buffer + addr)) = value;

  phys_ptr<phys_ptr<uint32_t>> ptr_addr{addr};
  ASSERT_EQ(addr, uintptr_t{ptr_addr});
  ASSERT_EQ(value, phys_ptr<uint32_t>(*ptr_addr));

  phys_ptr<phys_ptr<uint32_t>> ptr_copy_ctor{ptr_addr};
  ASSERT_EQ(addr, uintptr_t{ptr_copy_ctor});
  ASSERT_EQ(value, phys_ptr<uint32_t>(*ptr_copy_ctor));

  phys_ptr<phys_ptr<uint32_t>> ptr_assign_copy{zero_addr};
  ptr_assign_copy = ptr_addr;
  ASSERT_EQ(addr, uintptr_t{ptr_assign_copy});
  ASSERT_EQ(value, phys_ptr<uint32_t>(*ptr_assign_copy));

  phys_ptr<phys_ptr<uint32_t>> ptr_assign_move{zero_addr};
  ptr_assign_move = phys_ptr<phys_ptr<uint32_t>>(addr);
  ASSERT_EQ(addr, uintptr_t{ptr_assign_move});
  ASSERT_EQ(value, phys_ptr<uint32_t>(*ptr_assign_move));

  *ptr_addr = write_value;
  ASSERT_EQ(write_value, *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));
  ASSERT_EQ(write_value, phys_ptr<uint32_t>(*ptr_addr));
  *ptr_addr = value;

  phys_ref<phys_ptr<uint32_t>> ref = *ptr_addr;
  ASSERT_EQ(&ref, ptr_addr);

  *(reinterpret_cast<phys_ptr<uint32_t>*>(phys_buffer + addr) + 1) = write_value;
  ASSERT_EQ(value, phys_ptr<uint32_t>(ptr_addr[0]));
  ASSERT_EQ(write_value, phys_ptr<uint32_t>(ptr_addr[1]));
  *(reinterpret_cast<phys_ptr<uint32_t>*>(phys_buffer + addr) + 1) = 0;
  ptr_addr[2] = write_value;
  ASSERT_EQ(write_value,
            *(reinterpret_cast<phys_ptr<uint32_t>*>(phys_buffer + addr) + 2));
  ptr_addr[2] = 0;

  phys_ref<phys_ptr<uint32_t>> other_ref = ptr_addr[1];
  *(reinterpret_cast<phys_ptr<uint32_t>*>(phys_buffer + addr) + 1) = write_value;
  *ptr_addr = ptr_addr[1];
  *ptr_addr = other_ref;
  ASSERT_EQ(write_value, phys_ptr<uint32_t>(ptr_addr[0]));
  ASSERT_EQ(write_value, phys_ptr<uint32_t>(ptr_addr[1]));
  *ptr_addr = value;
  ptr_addr[1] = 0;

  *(reinterpret_cast<phys_ptr<uint32_t>*>(phys_buffer + addr) + 1) = write_value;
  *ptr_addr = ptr_addr[1];
  ASSERT_EQ(write_value, phys_ptr<uint32_t>(ptr_addr[0]));
  ASSERT_EQ(write_value, phys_ptr<uint32_t>(ptr_addr[1]));
  *ptr_addr = value;
  ptr_addr[1] = 0;

  ASSERT_EQ(true, ptr_addr == ptr_copy_ctor);
  ASSERT_EQ(false, ptr_addr == phys_ptr<phys_ptr<uint32_t>>{addr + 1});

  ASSERT_EQ(false, ptr_addr != ptr_copy_ctor);
  ASSERT_EQ(true, ptr_addr != phys_ptr<phys_ptr<uint32_t>>{addr + 1});

  ASSERT_EQ(&ptr_addr[16], ptr_addr + 16);

  phys_ptr<phys_ptr<uint32_t>> ptr_sum = ptr_addr;
  ptr_sum += 16;
  ASSERT_EQ(&ptr_addr[16], ptr_sum);

  ASSERT_EQ(&ptr_addr[8], ptr_sum - 8);

  phys_ptr<phys_ptr<uint32_t>> ptr_diff = ptr_sum;
  ptr_diff -= 8;
  ASSERT_EQ(&ptr_addr[8], ptr_diff);

  uint32_t offset = 0xfade;
  *ptr_addr += offset;
  ASSERT_EQ(value + offset,
            *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));
  ASSERT_EQ(value + offset, phys_ptr<uint32_t>(*ptr_addr));
  *ptr_addr = value;

  *ptr_addr -= offset;
  ASSERT_EQ(value - offset,
            *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));
  ASSERT_EQ(value - offset, phys_ptr<uint32_t>(*ptr_addr));
  *ptr_addr = value;

  ASSERT_EQ(0, uintptr_t{phys_ptr<phys_ptr<uint32_t>*>::null()});

  *(reinterpret_cast<phys_ptr<uint32_t>*>(phys_buffer + addr)) = 0;
}

typedef struct {
  uintptr_t a;
  size_t b;
  uint32_t c[3];
} test_struct;

TEST(PhysPtrTest, HandlesStruct) {
  uintptr_t addr = 160;  // Aligned by 32, should satisfy most archs.
  uintptr_t a_value = 0xbeef, write_a_value = 0xdeed;
  size_t b_value = 0xcafe, write_b_value = 0xface;
  uint32_t c0_value = 0x1111, c1_value = 0x2222, c2_value = 0x3333;
  uint32_t write_c0_value = 0x4444, write_c1_value = 0x5555,
           write_c2_value = 0x6666;
  uintptr_t zero_addr = 0;
  ASSERT_LE(256, phys_buffer_size);
  memset(phys_buffer, 0, 256);
  (reinterpret_cast<test_struct*>(phys_buffer + addr))->a = a_value;
  (reinterpret_cast<test_struct*>(phys_buffer + addr))->b = b_value;
  (reinterpret_cast<test_struct*>(phys_buffer + addr))->c[0] = c0_value;
  (reinterpret_cast<test_struct*>(phys_buffer + addr))->c[1] = c1_value;
  (reinterpret_cast<test_struct*>(phys_buffer + addr))->c[2] = c2_value;

  phys_ptr<test_struct> ptr_addr{addr};
  ASSERT_EQ(addr, uintptr_t{ptr_addr});
  ASSERT_EQ(a_value, ptr_addr->*(&test_struct::a));
  ASSERT_EQ(b_value, ptr_addr->*(&test_struct::b));
  ASSERT_EQ(c0_value, (ptr_addr->*(&test_struct::c))[0]);
  ASSERT_EQ(c1_value, (ptr_addr->*(&test_struct::c))[1]);
  ASSERT_EQ(c2_value, (ptr_addr->*(&test_struct::c))[2]);

  phys_ptr<uint32_t> ptr_addr_c = ptr_addr->*(&test_struct::c);
  ASSERT_EQ(reinterpret_cast<char*>(
      (reinterpret_cast<test_struct*>(phys_buffer + addr))->c) - phys_buffer,
      uintptr_t(ptr_addr_c));
  ASSERT_EQ(c0_value, ptr_addr_c[0]);
  ASSERT_EQ(c1_value, ptr_addr_c[1]);
  ASSERT_EQ(c2_value, ptr_addr_c[2]);

  // NOTE: phys_ptr<struct> uses a duplicated implementation, so all the
  //       operators must be tested.

  phys_ptr<test_struct> ptr_copy_ctor{ptr_addr};
  ASSERT_EQ(addr, uintptr_t{ptr_copy_ctor});
  ASSERT_EQ(a_value, ptr_copy_ctor->*(&test_struct::a));
  ASSERT_EQ(b_value, ptr_copy_ctor->*(&test_struct::b));
  ASSERT_EQ(c0_value, (ptr_copy_ctor->*(&test_struct::c))[0]);
  ASSERT_EQ(c1_value, (ptr_copy_ctor->*(&test_struct::c))[1]);
  ASSERT_EQ(c2_value, (ptr_copy_ctor->*(&test_struct::c))[2]);

  phys_ptr<test_struct> ptr_assign_copy{zero_addr};
  ptr_assign_copy = ptr_addr;
  ASSERT_EQ(addr, uintptr_t{ptr_assign_copy});
  ASSERT_EQ(a_value, ptr_assign_copy->*(&test_struct::a));
  ASSERT_EQ(b_value, ptr_assign_copy->*(&test_struct::b));
  ASSERT_EQ(c0_value, (ptr_assign_copy->*(&test_struct::c))[0]);
  ASSERT_EQ(c1_value, (ptr_assign_copy->*(&test_struct::c))[1]);
  ASSERT_EQ(c2_value, (ptr_assign_copy->*(&test_struct::c))[2]);

  phys_ptr<test_struct> ptr_assign_move{zero_addr};
  ptr_assign_move = phys_ptr<test_struct>(addr);
  ASSERT_EQ(addr, uintptr_t{ptr_assign_move});
  ASSERT_EQ(a_value, ptr_assign_move->*(&test_struct::a));
  ASSERT_EQ(b_value, ptr_assign_move->*(&test_struct::b));
  ASSERT_EQ(c0_value, (ptr_assign_move->*(&test_struct::c))[0]);
  ASSERT_EQ(c1_value, (ptr_assign_move->*(&test_struct::c))[1]);
  ASSERT_EQ(c2_value, (ptr_assign_move->*(&test_struct::c))[2]);

  ptr_addr->*(&test_struct::a) = write_a_value;
  ptr_addr->*(&test_struct::b) = write_b_value;
  (ptr_addr->*(&test_struct::c))[0] = write_c0_value;
  (ptr_addr->*(&test_struct::c))[1] = write_c1_value;
  (ptr_addr->*(&test_struct::c))[2] = write_c2_value;
  ASSERT_EQ(write_a_value,
            (reinterpret_cast<test_struct*>(phys_buffer + addr))->a);
  ASSERT_EQ(write_b_value,
            (reinterpret_cast<test_struct*>(phys_buffer + addr))->b);
  ASSERT_EQ(write_c0_value,
            (reinterpret_cast<test_struct*>(phys_buffer + addr))->c[0]);
  ASSERT_EQ(write_c1_value,
            (reinterpret_cast<test_struct*>(phys_buffer + addr))->c[1]);
  ASSERT_EQ(write_c2_value,
            (reinterpret_cast<test_struct*>(phys_buffer + addr))->c[2]);
  ptr_addr->*(&test_struct::a) = a_value;
  ptr_addr->*(&test_struct::b) = b_value;
  (ptr_addr->*(&test_struct::c))[0] = c0_value;
  (ptr_addr->*(&test_struct::c))[1] = c1_value;
  (ptr_addr->*(&test_struct::c))[2] = c2_value;

  // ptr_addr->a (commented out below) doesn't work because of the limitations
  // in overriding operator ->. See the phys_ptr source for more information.
  //
  // ptr_addr->a = write_a_value;

  phys_ref<test_struct> ref = *ptr_addr;
  ASSERT_EQ(&ref, ptr_addr);

  (reinterpret_cast<test_struct*>(phys_buffer + addr) + 1)->a = write_a_value;
  (reinterpret_cast<test_struct*>(phys_buffer + addr) + 1)->b = write_b_value;
  (reinterpret_cast<test_struct*>(phys_buffer + addr) + 1)->c[0] =
      write_c0_value;
  (reinterpret_cast<test_struct*>(phys_buffer + addr) + 1)->c[1] =
      write_c1_value;
  (reinterpret_cast<test_struct*>(phys_buffer + addr) + 1)->c[2] =
      write_c2_value;
  ASSERT_EQ(a_value, (&ptr_addr[0])->*(&test_struct::a));
  ASSERT_EQ(b_value, (&ptr_addr[0])->*(&test_struct::b));
  ASSERT_EQ(c0_value, ((&ptr_addr[0])->*(&test_struct::c))[0]);
  ASSERT_EQ(c1_value, ((&ptr_addr[0])->*(&test_struct::c))[1]);
  ASSERT_EQ(c2_value, ((&ptr_addr[0])->*(&test_struct::c))[2]);
  ASSERT_EQ(write_a_value, (&ptr_addr[1])->*(&test_struct::a));
  ASSERT_EQ(write_b_value, (&ptr_addr[1])->*(&test_struct::b));
  ASSERT_EQ(write_c0_value, ((&ptr_addr[1])->*(&test_struct::c))[0]);
  ASSERT_EQ(write_c1_value, ((&ptr_addr[1])->*(&test_struct::c))[1]);
  ASSERT_EQ(write_c2_value, ((&ptr_addr[1])->*(&test_struct::c))[2]);
  (reinterpret_cast<test_struct*>(phys_buffer + addr) + 1)->a = 0;
  (reinterpret_cast<test_struct*>(phys_buffer + addr) + 1)->b = 0;
  (reinterpret_cast<test_struct*>(phys_buffer + addr) + 1)->c[0] = 0;
  (reinterpret_cast<test_struct*>(phys_buffer + addr) + 1)->c[1] = 0;
  (reinterpret_cast<test_struct*>(phys_buffer + addr) + 1)->c[2] = 0;
  (&ptr_addr[2])->*(&test_struct::a) = write_a_value;
  (&ptr_addr[2])->*(&test_struct::b) = write_b_value;
  ((&ptr_addr[2])->*(&test_struct::c))[0] = write_c0_value;
  ((&ptr_addr[2])->*(&test_struct::c))[1] = write_c1_value;
  ((&ptr_addr[2])->*(&test_struct::c))[2] = write_c2_value;
  ASSERT_EQ(write_a_value,
            (reinterpret_cast<test_struct*>(phys_buffer + addr) + 2)->a);
  ASSERT_EQ(write_b_value,
            (reinterpret_cast<test_struct*>(phys_buffer + addr) + 2)->b);
  ASSERT_EQ(write_c0_value,
            (reinterpret_cast<test_struct*>(phys_buffer + addr) + 2)->c[0]);
  ASSERT_EQ(write_c1_value,
            (reinterpret_cast<test_struct*>(phys_buffer + addr) + 2)->c[1]);
  ASSERT_EQ(write_c2_value,
            (reinterpret_cast<test_struct*>(phys_buffer + addr) + 2)->c[2]);
  (&ptr_addr[2])->*(&test_struct::a) = 0;
  (&ptr_addr[2])->*(&test_struct::b) = 0;
  ((&ptr_addr[2])->*(&test_struct::c))[0] = 0;
  ((&ptr_addr[2])->*(&test_struct::c))[1] = 0;
  ((&ptr_addr[2])->*(&test_struct::c))[2] = 0;

  ASSERT_EQ(true, ptr_addr == ptr_copy_ctor);
  ASSERT_EQ(false, ptr_addr == phys_ptr<test_struct>{addr + 1});

  ASSERT_EQ(false, ptr_addr != ptr_copy_ctor);
  ASSERT_EQ(true, ptr_addr != phys_ptr<test_struct>{addr + 1});

  ASSERT_EQ(&ptr_addr[16], ptr_addr + 16);

  phys_ptr<test_struct> ptr_sum = ptr_addr;
  ptr_sum += 16;
  ASSERT_EQ(&ptr_addr[16], ptr_sum);

  ASSERT_EQ(&ptr_addr[8], ptr_sum - 8);

  phys_ptr<test_struct> ptr_diff = ptr_sum;
  ptr_diff -= 8;
  ASSERT_EQ(&ptr_addr[8], ptr_diff);

  ASSERT_EQ(0, uintptr_t{phys_ptr<test_struct*>::null()});

  (reinterpret_cast<test_struct*>(phys_buffer + addr))->a = 0;
  (reinterpret_cast<test_struct*>(phys_buffer + addr))->b = 0;
  (reinterpret_cast<test_struct*>(phys_buffer + addr))->c[0] = 0;
  (reinterpret_cast<test_struct*>(phys_buffer + addr))->c[1] = 0;
  (reinterpret_cast<test_struct*>(phys_buffer + addr))->c[2] = 0;
}
