#include "bare/phys_ptr.h"

#include "gtest/gtest.h"

using sanctum::bare::phys_ptr;
using sanctum::bare::phys_ref;
using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;
using sanctum::testing::phys_buffer;
using sanctum::testing::phys_buffer_size;

TEST(PhysPtrTest, HandlesUintptr) {
  uintptr_t addr = 160;  // Aligned by 32, should satisfy most archs.
  uintptr_t value = 0xbeef, write_value = 0xdeed;
  uintptr_t zero = 0;
  memset(phys_buffer, 0, phys_buffer_size);
  *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)) = value;

  phys_ptr<uintptr_t> ptr_addr{addr};
  ASSERT_EQ(addr, uintptr_t{ptr_addr});
  ASSERT_EQ(value, *ptr_addr);

  phys_ptr<uintptr_t> ptr_copy_ctor{ptr_addr};
  ASSERT_EQ(addr, uintptr_t{ptr_copy_ctor});
  ASSERT_EQ(value, *ptr_copy_ctor);

  phys_ptr<uintptr_t> ptr_assign_copy{zero};
  ptr_assign_copy = ptr_addr;
  ASSERT_EQ(addr, uintptr_t{ptr_assign_copy});
  ASSERT_EQ(value, *ptr_assign_copy);

  phys_ptr<uintptr_t> ptr_assign_move{zero};
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

  ASSERT_EQ(&ptr_addr[16], ptr_addr + 16);

  phys_ptr<uintptr_t> ptr_sum = ptr_addr;
  ptr_sum += 16;
  ASSERT_EQ(&ptr_addr[16], ptr_sum);

  *ptr_addr += write_value;
  ASSERT_EQ(value + write_value,
            *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));
  ASSERT_EQ(value + write_value, *ptr_addr);
  *ptr_addr = value;

  *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)) = 0;
}

TEST(PhysPtrTest, HandlesSize) {
  uintptr_t addr = 160;  // Aligned by 32, should satisfy most archs.
  size_t value = 0xbeef, write_value = 0xdeed;
  size_t zero = 0;
  memset(phys_buffer, 0, phys_buffer_size);
  *(reinterpret_cast<size_t*>(phys_buffer + addr)) = value;

  phys_ptr<size_t> ptr_addr{addr};
  ASSERT_EQ(addr, uintptr_t{ptr_addr});
  ASSERT_EQ(value, *ptr_addr);

  phys_ptr<size_t> ptr_copy_ctor{ptr_addr};
  ASSERT_EQ(addr, uintptr_t{ptr_copy_ctor});
  ASSERT_EQ(value, *ptr_copy_ctor);

  phys_ptr<size_t> ptr_assign_copy{zero};
  ptr_assign_copy = ptr_addr;
  ASSERT_EQ(addr, uintptr_t{ptr_assign_copy});
  ASSERT_EQ(value, *ptr_assign_copy);

  phys_ptr<size_t> ptr_assign_move{zero};
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

  ASSERT_EQ(&ptr_addr[16], ptr_addr + 16);

  phys_ptr<size_t> ptr_sum = ptr_addr;
  ptr_sum += 16;
  ASSERT_EQ(&ptr_addr[16], ptr_sum);

  *ptr_addr += write_value;
  ASSERT_EQ(value + write_value,
            *(reinterpret_cast<uintptr_t*>(phys_buffer + addr)));
  ASSERT_EQ(value + write_value, *ptr_addr);
  *ptr_addr = value;

  *(reinterpret_cast<size_t*>(phys_buffer + addr)) = 0;
}

typedef struct {
  uintptr_t a;
  size_t b;
} test_struct;

TEST(PhysPtrTest, HandlesStruct) {
  uintptr_t addr = 160;  // Aligned by 32, should satisfy most archs.
  uintptr_t a_value = 0xbeef, write_a_value = 0xdeed;
  size_t b_value = 0xcafe, write_b_value = 0xface;
  uintptr_t zero = 0;
  memset(phys_buffer, 0, phys_buffer_size);
  (reinterpret_cast<test_struct*>(phys_buffer + addr))->a = a_value;
  (reinterpret_cast<test_struct*>(phys_buffer + addr))->b = b_value;

  phys_ptr<test_struct> ptr_addr{addr};
  ASSERT_EQ(addr, uintptr_t{ptr_addr});
  ASSERT_EQ(a_value, ptr_addr->*(&test_struct::a));
  ASSERT_EQ(b_value, ptr_addr->*(&test_struct::b));

  // NOTE: phys_ptr<struct> uses a duplicated implementation, so all the
  //       operators must be tested.

  phys_ptr<test_struct> ptr_copy_ctor{ptr_addr};
  ASSERT_EQ(addr, uintptr_t{ptr_copy_ctor});
  ASSERT_EQ(a_value, ptr_copy_ctor->*(&test_struct::a));
  ASSERT_EQ(b_value, ptr_copy_ctor->*(&test_struct::b));

  phys_ptr<test_struct> ptr_assign_copy{zero};
  ptr_assign_copy = ptr_addr;
  ASSERT_EQ(addr, uintptr_t{ptr_assign_copy});
  ASSERT_EQ(a_value, ptr_assign_copy->*(&test_struct::a));
  ASSERT_EQ(b_value, ptr_assign_copy->*(&test_struct::b));

  phys_ptr<test_struct> ptr_assign_move{zero};
  ptr_assign_move = phys_ptr<test_struct>(addr);
  ASSERT_EQ(addr, uintptr_t{ptr_assign_move});
  ASSERT_EQ(a_value, ptr_assign_move->*(&test_struct::a));
  ASSERT_EQ(b_value, ptr_assign_move->*(&test_struct::b));

  ptr_addr->*(&test_struct::a) = write_a_value;
  ptr_addr->*(&test_struct::b) = write_b_value;
  ASSERT_EQ(write_a_value,
            (reinterpret_cast<test_struct*>(phys_buffer + addr))->a);
  ASSERT_EQ(write_b_value,
            (reinterpret_cast<test_struct*>(phys_buffer + addr))->b);
  ptr_addr->*(&test_struct::a) = a_value;
  ptr_addr->*(&test_struct::b) = b_value;

  // ptr_addr->a (commented out below) doesn't work because of the limitations
  // in overriding operator ->. See the phys_ptr source for more information.
  //
  // ptr_addr->a = write_a_value;

  phys_ref<test_struct> ref = *ptr_addr;
  ASSERT_EQ(&ref, ptr_addr);

  (reinterpret_cast<test_struct*>(phys_buffer + addr) + 1)->a = write_a_value;
  (reinterpret_cast<test_struct*>(phys_buffer + addr) + 1)->b = write_b_value;
  ASSERT_EQ(a_value, (&ptr_addr[0])->*(&test_struct::a));
  ASSERT_EQ(b_value, (&ptr_addr[0])->*(&test_struct::b));
  ASSERT_EQ(write_a_value, (&ptr_addr[1])->*(&test_struct::a));
  ASSERT_EQ(write_b_value, (&ptr_addr[1])->*(&test_struct::b));
  (reinterpret_cast<test_struct*>(phys_buffer + addr) + 1)->a = 0;
  (reinterpret_cast<test_struct*>(phys_buffer + addr) + 1)->b = 0;
  (&ptr_addr[2])->*(&test_struct::a) = write_a_value;
  (&ptr_addr[2])->*(&test_struct::b) = write_b_value;
  ASSERT_EQ(write_a_value,
            (reinterpret_cast<test_struct*>(phys_buffer + addr) + 2)->a);
  ASSERT_EQ(write_b_value,
            (reinterpret_cast<test_struct*>(phys_buffer + addr) + 2)->b);
  (&ptr_addr[2])->*(&test_struct::a) = 0;
  (&ptr_addr[2])->*(&test_struct::b) = 0;

  ASSERT_EQ(&ptr_addr[16], ptr_addr + 16);

  phys_ptr<test_struct> ptr_sum = ptr_addr;
  ptr_sum += 16;
  ASSERT_EQ(&ptr_addr[16], ptr_sum);

  (reinterpret_cast<test_struct*>(phys_buffer + addr))->a = 0;
  (reinterpret_cast<test_struct*>(phys_buffer + addr))->b = 0;
}
