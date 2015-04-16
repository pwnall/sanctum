#include "../../memory.h"

using namespace sanctum::bare;

namespace sanctum {
namespace testing {

size_t dram_size;
size_t cache_levels;

bool is_shared_cache[max_cache_levels];
size_t cache_line_bits[max_cache_levels];
size_t cache_set_bits[max_cache_levels];

};  // namespace sanctum::testing
};  // namespace sanctum

