#include "../../cpu_context.h"

#include <cassert>  // Core configuration use assert for bound checking.

using namespace sanctum::bare;

namespace sanctum {
namespace testing {

size_t total_cores = 0;
size_t current_core = 0;

size_t core_tlb_flush_count[max_cores];
size_t core_cache_flush_count[max_cores];

void set_current_core(size_t core_id) {
  assert(core_id < total_cores);
  current_core = core_id;
}

void set_total_cores(size_t new_total) {
  assert(new_total > 0 && new_total <= max_cores);
  total_cores = new_total;
  current_core = 0;
}

};  // namespace sanctum::testing
};  // namespace sanctum


