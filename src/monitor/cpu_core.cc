#include "cpu_core.h"

#include "cpu_core_inl.h"

using sanctum::bare::phys_ptr;
using sanctum::bare::size_t;

namespace sanctum {
namespace internal {

phys_ptr<core_info_t> g_core{0};

size_t g_core_count;


};  // namespace sanctum::internal
};  // namespace sanctum
