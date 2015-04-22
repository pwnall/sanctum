#include "hash.h"

#include "bare/base_types.h"
#include "bare/memory.h"

using sanctum::bare::bcopy;
using sanctum::bare::bzero;
using sanctum::bare::phys_ptr;
using sanctum::bare::size_t;
using sanctum::bare::uint32_t;
using sanctum::bare::uintptr_t;

namespace {

// SHA-256 constants table.
static uint32_t hash_k[] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static_assert(sizeof(hash_k) == 64 * sizeof(uint32_t),
    "Incorrectly sized constant table");

constexpr inline uint32_t rotate_right(uint32_t value, size_t size) {
  return (value >> size) | (value << (32 - size));
}
constexpr inline uint32_t clamp_to_32(uint32_t value) {
  return (sizeof(uint32_t) == 4) ? value :
      (value & ((1 << 31) | ((1 << 31) - 1)));
}
constexpr inline uint32_t to_big_endian(uint32_t value) {
  return ((value & 0xff) << 24) | ((value & 0xff00) << 8) |
      ((value & 0xff0000) >> 8) | ((value & 0xff000000) >> 24);
}

};  // anonymous namespace

namespace sanctum {
namespace crypto {  // sanctum::crypto

void init_hash(phys_ptr<hash_state_t> state) {
  (state->*(&hash_state_t::h))[0] = 0x6a09e667;
  (state->*(&hash_state_t::h))[1] = 0xbb67ae85;
  (state->*(&hash_state_t::h))[2] = 0x3c6ef372;
  (state->*(&hash_state_t::h))[3] = 0xa54ff53a;
  (state->*(&hash_state_t::h))[4] = 0x510e527f;
  (state->*(&hash_state_t::h))[5] = 0x9b05688c;
  (state->*(&hash_state_t::h))[6] = 0x1f83d9ab;
  (state->*(&hash_state_t::h))[7] = 0x5be0cd19;

  uintptr_t padding_addr = uintptr_t(state->*(&hash_state_t::padding));
  phys_ptr<uint32_t> padding_start{padding_addr};
  bzero(padding_start, hash_block_size);
  *padding_start =
      to_big_endian((static_cast<uint32_t>(1) << (sizeof(uint32_t) * 8 - 1)));
}

void hash_block(phys_ptr<hash_state_t> state, phys_ptr<uint32_t> block) {
  phys_ptr<uint32_t> hptr = state->*(&hash_state_t::h);
  uint32_t a = hptr[0];
  uint32_t b = hptr[1];
  uint32_t c = hptr[2];
  uint32_t d = hptr[3];
  uint32_t e = hptr[4];
  uint32_t f = hptr[5];
  uint32_t g = hptr[6];
  uint32_t h = hptr[7];

  phys_ptr<uint32_t> w = state->*(&hash_state_t::work_area);
  for (size_t i = 0; i < 64; ++i) {
    uint32_t wi;
    if (i < 16) {
      wi = w[i & 0xf] = to_big_endian(block[i]);
    } else {
      uint32_t w15 = w[(i - 15) & 0xf], w2 = w[(i - 2) & 0xf];
      uint32_t gamma0 = rotate_right(w15, 7) ^ rotate_right(w15, 18) ^
          clamp_to_32(w15 >> 3);
      uint32_t gamma1 = rotate_right(w2, 17) ^ rotate_right(w2, 19) ^
          clamp_to_32(w2 >> 10);
      uint32_t w7 = w[(i - 7) & 0xf];
      uint32_t ii = i & 0xf;
      wi = w[ii] = clamp_to_32(w[ii] + gamma0 + w7 + gamma1);
    }

    uint32_t ch = (e & f) ^ ((~e) & g);
    uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
    uint32_t sigma0 = rotate_right(a, 2) ^ rotate_right(a, 13) ^
        rotate_right(a, 22);
    uint32_t sigma1 = rotate_right(e, 6) ^ rotate_right(e, 11) ^
        rotate_right(e, 25);
    uint32_t temp1 = h + sigma1 + ch + hash_k[i] + wi;
    uint32_t temp2 = sigma0 + maj;

    h = g; g = f; f = e;
    e = d + temp1;
    d = c; c = b; b = a;
    a = temp1 + temp2;
  }

  hptr[0] = clamp_to_32(hptr[0] + a);
  hptr[1] = clamp_to_32(hptr[1] + b);
  hptr[2] = clamp_to_32(hptr[2] + c);
  hptr[3] = clamp_to_32(hptr[3] + d);
  hptr[4] = clamp_to_32(hptr[4] + e);
  hptr[5] = clamp_to_32(hptr[5] + f);
  hptr[6] = clamp_to_32(hptr[6] + g);
  hptr[7] = clamp_to_32(hptr[7] + h);

  // The bit counter is the last 64 bits of the hash.
  uintptr_t counter_addr = uintptr_t(state->*(&hash_state_t::padding)) +
      hash_block_size - 4;
  phys_ptr<uint32_t> counter_ptr{counter_addr};

  // TODO: handle overflow
  uint32_t hashed_bits = to_big_endian(*counter_ptr);
  *counter_ptr = to_big_endian(hashed_bits + hash_block_size * 8);
}

void finalize_hash(phys_ptr<hash_state_t> state) {
  uintptr_t padding_addr = uintptr_t(state->*(&hash_state_t::padding));
  hash_block(state, phys_ptr<uint32_t>{padding_addr});
}

};  // namespace sanctum::crypto
};  // namespace sanctum
