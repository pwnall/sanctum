#include "bare/phys_atomics.h"

#include "gtest/gtest.h"

using sanctum::bare::atomic;
using sanctum::bare::atomic_flag;
using sanctum::bare::atomic_flag_test_and_set;
using sanctum::bare::phys_ptr;
using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;
using sanctum::testing::phys_buffer;
using sanctum::testing::phys_buffer_size;

TEST(AtomicFlagTest, TestAndSet) {
  uintptr_t addr1 = 160, addr2 = 200;
  memset(phys_buffer, 0, phys_buffer_size);
  phys_ptr<atomic_flag> ptr1{addr1}, ptr2{addr2};

  ASSERT_EQ(false, atomic_flag_test_and_set(ptr1));
  ASSERT_EQ(true,
            (reinterpret_cast<atomic_flag*>(phys_buffer + addr1))->__flag);
  ASSERT_EQ(false,
            (reinterpret_cast<atomic_flag*>(phys_buffer + addr2))->__flag);
  ASSERT_EQ(false, atomic_flag_test_and_set(ptr2));
  ASSERT_EQ(true,
            (reinterpret_cast<atomic_flag*>(phys_buffer + addr1))->__flag);
  ASSERT_EQ(true,
            (reinterpret_cast<atomic_flag*>(phys_buffer + addr2))->__flag);
  ASSERT_EQ(true, atomic_flag_test_and_set(ptr1));
  ASSERT_EQ(true, atomic_flag_test_and_set(ptr2));
  ASSERT_EQ(true,
            (reinterpret_cast<atomic_flag*>(phys_buffer + addr1))->__flag);
  ASSERT_EQ(true,
            (reinterpret_cast<atomic_flag*>(phys_buffer + addr2))->__flag);

  (reinterpret_cast<atomic_flag*>(phys_buffer + addr1))->__flag = false;
  (reinterpret_cast<atomic_flag*>(phys_buffer + addr2))->__flag = false;
}

TEST(AtomicFlagTest, Clear) {
  uintptr_t addr1 = 160, addr2 = 200;
  memset(phys_buffer, 0, phys_buffer_size);
  phys_ptr<atomic_flag> ptr1{addr1}, ptr2{addr2};

  atomic_flag_test_and_set(ptr1);
  atomic_flag_test_and_set(ptr2);
  ASSERT_EQ(true,
            (reinterpret_cast<atomic_flag*>(phys_buffer + addr1))->__flag);
  ASSERT_EQ(true,
            (reinterpret_cast<atomic_flag*>(phys_buffer + addr2))->__flag);

  atomic_flag_clear(ptr1);
  ASSERT_EQ(false,
            (reinterpret_cast<atomic_flag*>(phys_buffer + addr1))->__flag);
  ASSERT_EQ(true,
            (reinterpret_cast<atomic_flag*>(phys_buffer + addr2))->__flag);

  atomic_flag_clear(ptr2);
  ASSERT_EQ(false,
            (reinterpret_cast<atomic_flag*>(phys_buffer + addr1))->__flag);
  ASSERT_EQ(false,
            (reinterpret_cast<atomic_flag*>(phys_buffer + addr2))->__flag);

  ASSERT_EQ(false, atomic_flag_test_and_set(ptr1));
  ASSERT_EQ(false, atomic_flag_test_and_set(ptr2));

  (reinterpret_cast<atomic_flag*>(phys_buffer + addr1))->__flag = false;
  (reinterpret_cast<atomic_flag*>(phys_buffer + addr2))->__flag = false;
}

TEST(AtomicTest, HandlesUintptr) {
  uintptr_t addr = 160;
  uintptr_t value = 0xbeef, write_value = 0xdeed;
  memset(phys_buffer, 0, phys_buffer_size);
  phys_ptr<atomic<uintptr_t>> ptr{addr};

  atomic_init(ptr, value);
  ASSERT_EQ(value,
      (reinterpret_cast<atomic<uintptr_t>*>(phys_buffer + addr))->__value);
  ASSERT_EQ(value, atomic_load(ptr));

  atomic_store(ptr, write_value);
  ASSERT_EQ(write_value,
      (reinterpret_cast<atomic<uintptr_t>*>(phys_buffer + addr))->__value);
  ASSERT_EQ(write_value, atomic_load(ptr));

  ASSERT_EQ(write_value, atomic_fetch_add(ptr, value));
  ASSERT_EQ(write_value + value,
      (reinterpret_cast<atomic<uintptr_t>*>(phys_buffer + addr))->__value);
  ASSERT_EQ(write_value + value, atomic_load(ptr));

  ASSERT_EQ(write_value + value, atomic_fetch_add(ptr, value));
  ASSERT_EQ(write_value + 2 * value,
      (reinterpret_cast<atomic<uintptr_t>*>(phys_buffer + addr))->__value);
  ASSERT_EQ(write_value + 2 * value, atomic_load(ptr));

  atomic_store(ptr, write_value);
  ASSERT_EQ(write_value,
      (reinterpret_cast<atomic<uintptr_t>*>(phys_buffer + addr))->__value);
  ASSERT_EQ(write_value, atomic_load(ptr));

  ASSERT_EQ(write_value, atomic_fetch_sub(ptr, value));
  ASSERT_EQ(write_value - value,
      (reinterpret_cast<atomic<uintptr_t>*>(phys_buffer + addr))->__value);
  ASSERT_EQ(write_value - value, atomic_load(ptr));

  ASSERT_EQ(write_value - value, atomic_fetch_sub(ptr, value));
  ASSERT_EQ(write_value - 2 * value,
      (reinterpret_cast<atomic<uintptr_t>*>(phys_buffer + addr))->__value);
  ASSERT_EQ(write_value - 2 * value, atomic_load(ptr));

  (reinterpret_cast<atomic<uintptr_t>*>(phys_buffer + addr))->__value = 0;
}

TEST(AtomicTest, HandlesSize) {
  uintptr_t addr = 160;
  size_t value = 0xbeef, write_value = 0xdeed;
  memset(phys_buffer, 0, phys_buffer_size);
  phys_ptr<atomic<size_t>> ptr{addr};

  atomic_init(ptr, value);
  ASSERT_EQ(value,
      (reinterpret_cast<atomic<size_t>*>(phys_buffer + addr))->__value);
  ASSERT_EQ(value, atomic_load(ptr));

  atomic_store(ptr, write_value);
  ASSERT_EQ(write_value,
      (reinterpret_cast<atomic<size_t>*>(phys_buffer + addr))->__value);
  ASSERT_EQ(write_value, atomic_load(ptr));

  ASSERT_EQ(write_value, atomic_fetch_add(ptr, value));
  ASSERT_EQ(write_value + value,
      (reinterpret_cast<atomic<size_t>*>(phys_buffer + addr))->__value);
  ASSERT_EQ(write_value + value, atomic_load(ptr));

  ASSERT_EQ(write_value + value, atomic_fetch_add(ptr, value));
  ASSERT_EQ(write_value + 2 * value,
      (reinterpret_cast<atomic<size_t>*>(phys_buffer + addr))->__value);
  ASSERT_EQ(write_value + 2 * value, atomic_load(ptr));

  atomic_store(ptr, write_value);
  ASSERT_EQ(write_value,
      (reinterpret_cast<atomic<size_t>*>(phys_buffer + addr))->__value);
  ASSERT_EQ(write_value, atomic_load(ptr));

  ASSERT_EQ(write_value, atomic_fetch_sub(ptr, value));
  ASSERT_EQ(write_value - value,
      (reinterpret_cast<atomic<size_t>*>(phys_buffer + addr))->__value);
  ASSERT_EQ(write_value - value, atomic_load(ptr));

  ASSERT_EQ(write_value - value, atomic_fetch_sub(ptr, value));
  ASSERT_EQ(write_value - 2 * value,
      (reinterpret_cast<atomic<size_t>*>(phys_buffer + addr))->__value);
  ASSERT_EQ(write_value - 2 * value, atomic_load(ptr));

  (reinterpret_cast<atomic<size_t>*>(phys_buffer + addr))->__value = 0;
}

