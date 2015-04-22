#if !defined(CRYPTO_HASH_H_INCLUDED)
#define CRYPTO_HASH_H_INCLUDED

#include "bare/base_types.h"
#include "bare/phys_ptr.h"

namespace sanctum {
namespace crypto {

using sanctum::bare::phys_ptr;
using sanctum::bare::size_t;
using sanctum::bare::uint32_t;

// The block size for the hashing algorithm, in bytes.
//
// The hashing algorithm implemented here processes data in blocks.
constexpr size_t hash_block_size = 64;  // 512 bits

// The result
constexpr size_t hash_result_size = 32;  // 256 bits

// The data structure used by the SHA-256 funtions.
struct hash_state_t {
  uint32_t h[8];  // The final hash is stored here.
  uint32_t work_area[16];  // The message schedule is computed here.
  char padding[hash_block_size];  // The padding block is stored here.
};

// Initializes a hashing data structure.
//
// This must be called before hash_block() is used.
void init_hash(phys_ptr<hash_state_t> state);

// Extends a crypto hash by a block.
//
// The block is exactly hash_block_size bytes. init_hash() must be called to
// set up the hashing data structure before this function is used.
void hash_block(phys_ptr<hash_state_t> state, phys_ptr<uint32_t> block);

// Finalizes the value in a hashing data structure.
//
// After this is called, hash_block() must not be called again. The
void finalize_hash(phys_ptr<hash_state_t> state);

};  // namespace sanctum::crypto
};  // namespace sanctum
#endif  // !defined(CRYPTO_HASH_H_INCLUDED)
