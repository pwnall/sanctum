#include "../../memory.h"

using namespace sanctum::bare;

namespace sanctum {
namespace testing {

size_t dmar_base, dmar_mask;
size_t dram_size;
size_t cache_levels;
size_t min_cache_index_shift;
size_t max_cache_index_shift;

bool is_shared_cache[max_cache_levels];
size_t cache_line_size[max_cache_levels];
size_t cache_set_count[max_cache_levels];

};  // namespace sanctum::testing
};  // namespace sanctum

