#include "hash.h"

#include "gtest/gtest.h"

using sanctum::bare::phys_ptr;
using sanctum::bare::uint32_t;
using sanctum::crypto::finalize_hash;
using sanctum::crypto::hash_block;
using sanctum::crypto::hash_block_size;
using sanctum::crypto::hash_state_t;
using sanctum::crypto::init_hash;
using sanctum::testing::phys_buffer;
using sanctum::testing::phys_buffer_size;

TEST(HashTest, EmptyString) {
  uintptr_t hash_addr = 160;

  ASSERT_LE(160 + sizeof(hash_state_t), phys_buffer_size);

  phys_ptr<hash_state_t> state{hash_addr};
  init_hash(state);
  finalize_hash(state);

  phys_ptr<uint32_t> h = state->*(&hash_state_t::h);
  EXPECT_EQ(0xe3b0c442, h[0]);
  EXPECT_EQ(0x98fc1c14, h[1]);
  EXPECT_EQ(0x9afbf4c8, h[2]);
  EXPECT_EQ(0x996fb924, h[3]);
  EXPECT_EQ(0x27ae41e4, h[4]);
  EXPECT_EQ(0x649b934c, h[5]);
  EXPECT_EQ(0xa495991b, h[6]);
  EXPECT_EQ(0x7852b855, h[7]);
}

TEST(HashTest, BlockOfA) {
  uintptr_t hash_addr = 160;
  uintptr_t block_addr = 400;

  ASSERT_LE(400 + hash_block_size, phys_buffer_size);
  memset(phys_buffer + 400, 'A', hash_block_size);

  phys_ptr<hash_state_t> state{hash_addr};
  init_hash(state);
  phys_ptr<uint32_t> block{block_addr};
  hash_block(state, block);
  finalize_hash(state);

  phys_ptr<uint32_t> h = state->*(&hash_state_t::h);
  EXPECT_EQ(0xd53eda7a, h[0]);
  EXPECT_EQ(0x637c99cc, h[1]);
  EXPECT_EQ(0x7fb566d9, h[2]);
  EXPECT_EQ(0x6e9fa109, h[3]);
  EXPECT_EQ(0xbf15c478, h[4]);
  EXPECT_EQ(0x410a3f5e, h[5]);
  EXPECT_EQ(0xb4d4c4e2, h[6]);
  EXPECT_EQ(0x6cd081f6, h[7]);
}

TEST(HashTest, BlockOfLetters) {
  uintptr_t hash_addr = 160;
  uintptr_t block_addr = 400;
  char block_data[] =
      "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq01234567";

  ASSERT_LE(400 + hash_block_size, phys_buffer_size);
  memcpy(phys_buffer + 400, block_data, hash_block_size);

  phys_ptr<hash_state_t> state{hash_addr};
  init_hash(state);
  phys_ptr<uint32_t> block{block_addr};
  hash_block(state, block);
  finalize_hash(state);

  phys_ptr<uint32_t> h = state->*(&hash_state_t::h);
  EXPECT_EQ(0x9e411773, h[0]);
  EXPECT_EQ(0x2130ab9d, h[1]);
  EXPECT_EQ(0xc766d118, h[2]);
  EXPECT_EQ(0x4ecb8dcc, h[3]);
  EXPECT_EQ(0xf82bc6a4, h[4]);
  EXPECT_EQ(0x2a12378a, h[5]);
  EXPECT_EQ(0x0fd067b9, h[6]);
  EXPECT_EQ(0x5d04ebff, h[7]);
}

TEST(HashTest, PageOfU) {
  uintptr_t hash_addr = 160;
  uintptr_t block_addr = 400;

  ASSERT_LE(400 + 4096, phys_buffer_size);
  memset(phys_buffer + 400, 'U', 4096);

  phys_ptr<hash_state_t> state{hash_addr};
  init_hash(state);
  for (size_t i = 0; i < 4096; i += hash_block_size) {
    phys_ptr<uint32_t> block{block_addr + i};
    hash_block(state, block);
  }
  finalize_hash(state);

  phys_ptr<uint32_t> h = state->*(&hash_state_t::h);
  EXPECT_EQ(0x0561079e, h[0]);
  EXPECT_EQ(0x4fe3390b, h[1]);
  EXPECT_EQ(0xc1d8bb70, h[2]);
  EXPECT_EQ(0x6edb7d80, h[3]);
  EXPECT_EQ(0x243eeca7, h[4]);
  EXPECT_EQ(0xddf876ce, h[5]);
  EXPECT_EQ(0xfbaa8c16, h[6]);
  EXPECT_EQ(0x84db80c3, h[7]);
}

