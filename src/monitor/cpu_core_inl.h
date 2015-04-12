#if !defined(MONITOR_CPU_CORE_INL_H_INCLUDED)
#define MONITOR_CPU_CORE_INL_H_INCLUDED

#include "cpu_core.h"

namespace sanctum {
namespace internal {

using sanctum::api::enclave_id_t;
using sanctum::bare::current_core;
using sanctum::bare::phys_ptr;

// The physical address of the core_info_t for the current core.
inline phys_ptr<core_info_t> current_core_info() {
  return &g_core[current_core()];
}

// The enclave running on the current core.
//
// Returns null_enclave_id if no enclave is running on the current core. This
// implies that the caller is the OS.
inline enclave_id_t current_enclave() {
  phys_ptr<core_info_t> core_info{current_core_info()};
  return core_info->*(&core_info_t::enclave_id);
}

};  // namespace sanctum::internal
};  // namespace sanctum
#endif  // !defined(MONITOR_CPU_CORE_INL_H_INCLUDED)
