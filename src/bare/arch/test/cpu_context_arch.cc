#include "../../cpu_context.h"

#include <cassert>  // Core configuration use assert for bound checking.

using namespace sanctum::bare;

namespace sanctum {
namespace testing {

size_t core_count = 0;
size_t current_core = 0;

size_t core_tlb_flush_count[max_cores];
size_t core_cache_flush_count[max_cores];
size_t core_cache_index_shift[max_cores];

void set_current_core(size_t core_id) {
  assert(core_id < core_count);
  current_core = core_id;
}

void set_core_count(size_t new_core_count) {
  assert(new_core_count > 0 && new_core_count <= max_cores);
  core_count = new_core_count;
  current_core = 0;
}

};  // namespace sanctum::testing
};  // namespace sanctum


