#if !defined(BARE_MEMORY_H_INCLUDED)
#define BARE_MEMORY_H_INCLUDED

#include "base_types.h"
#include "phys_ptr.h"

namespace sanctum {
namespace bare {

// Obtains the DRAM size from the memory subsystem.
//
// The implementation may be very slow, so the return value should be cached.
//
// The implementation may use privileged instructions.
size_t read_dram_size();

// The number of levels of cache memory.
//
// The implementation may be very slow, so the return value should be cached.
//
// The implementation may use privileged instructions.
size_t read_cache_levels();

// True if the given cache level is shared among cores.
//
// The implementation may be very slow, so the return value should be cached.
//
// The implementation may use privileged instructions.
bool is_shared_cache(size_t cache_level);

// The number of physical address bits used for the cache line index.
//
// The implementation may be very slow, so the return value should be cached.
//
// The implementation may use privileged instructions.
size_t read_cache_line_bits(size_t cache_level);

// The number of physical address bits used for the cache set index.
size_t read_cache_set_bits(size_t cache_level);

// Fills a buffer in physical memory with zeros.
//
// In order to allow for optimized assembly implementations, both the starting
// address and buffer size must be a multiple of the cache line size.
template<typename T> void bzero(phys_ptr<T> start, size_t bytes);

// Copies data between two non-overlaping buffers in physical memory.
//
// In order to allow for optimized assembly implementations, both addresses, as
// well as the buffer size must be a multiple of the cache line size.
template<typename T> void bcopy(phys_ptr<T> dest, phys_ptr<T> source,
    size_t bytes);

};  // namespace sanctum::bare
};  // namespace sanctum

// Per-architecture implementations of memory operations.
#include "memory_arch.h"

#endif  // !defined(BARE_MEMORY_H_INCLUDED)
