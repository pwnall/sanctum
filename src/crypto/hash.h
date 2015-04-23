#if !defined(CRYPTO_HASH_H_INCLUDED)
#define CRYPTO_HASH_H_INCLUDED

#include "bare/base_types.h"
#include "bare/phys_ptr.h"

namespace sanctum {
namespace crypto {

using sanctum::bare::phys_ptr;
using sanctum::bare::size_t;
using sanctum::bare::uint32_t;
using sanctum::bare::uintptr_t;

// The block size for the hashing algorithm, in bytes.
//
// The hashing algorithm implemented here processes data in blocks.
//
// The block size is guaranteed to be a multiple of the sizes of size_t and
// uintptr_t.
constexpr size_t hash_block_size = 64;  // 512 bits
static_assert(hash_block_size % sizeof(size_t) == 0,
    "hash_block_size not a multiple of size_t");
static_assert(hash_block_size % sizeof(uintptr_t) == 0,
    "hash_block_size not a multiple of uintptr_t");

// The size of the result of a hashing operation.
constexpr size_t hash_result_size = 32;  // 256 bits

// The data structure used by the SHA-256 funtions.
struct hash_state_t {
  uint32_t h[8];  // Stores the final hash.
  uint32_t work_area[16];  // Stores the message schedule.
  size_t padding[hash_block_size / sizeof(size_t)];  // The padding block.
};

// Initializes a hashing data structure.
//
// This must be called before extend_hash() is used.
void init_hash(phys_ptr<hash_state_t> state);

// Extends a crypto hash by a block.
//
// The block is exactly hash_block_size bytes. init_hash() must be called to
// set up the hashing data structure before this function is used.
void extend_hash(phys_ptr<hash_state_t> state, phys_ptr<uint32_t> block);

// Finalizes the value in a hashing data structure.
//
// After this is called, extend_hash() must not be called again. The final hash
// value is stored in the first hash_result_size bytes of the hash_state
// structure.
void finalize_hash(phys_ptr<hash_state_t> state);

};  // namespace sanctum::crypto
};  // namespace sanctum
#endif  // !defined(CRYPTO_HASH_H_INCLUDED)
