#if !defined(MONITOR_CPU_CORE_H_INCLUDED)
#define MONITOR_CPU_CORE_H_INCLUDED

#include "bare/base_types.h"
#include "bare/cpu_context.h"
#include "bare/phys_atomics.h"
#include "public/api.h"

namespace sanctum {
namespace internal {

using sanctum::api::enclave_id_t;
using sanctum::api::thread_id_t;
using sanctum::bare::atomic;
using sanctum::bare::current_core;
using sanctum::bare::phys_ptr;
using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;

struct thread_private_info_t;

// Per-core accounting information.
//
// This information is only modified on the core that it corresponds to, so we
// don't need atomics or locking.
typedef struct {
  enclave_id_t enclave_id;  // 0 if the core isn't executing enclave code
  thread_id_t thread_id;

  // The DRAM backing the thread_private_info_t is guaranteed to be pinned
  // while the thread is executing on a core.
  phys_ptr<thread_private_info_t> thread_info;

  // block_clock when this core's TLB was last flushed
  atomic<size_t> flushed_at;
} core_info_t;

// Core costants.
//
// These values are computed during the boot process. Once computed, the values
// never change.
extern phys_ptr<core_info_t> g_core_info;

extern size_t g_core_count;

// The enclave running on the current core.
//
// Returns 0 if no enclave is running on the current core. This implies that
// the caller is the OS.
inline enclave_id_t current_enclave() {
  phys_ptr<core_info_t> core = &g_core_info[current_core()];
  return core->*(&core_info_t::enclave_id);
}

};  // namespace sanctum::internal
};  // namespace sanctum

#endif  // !defined(MONITOR_CPU_CORE_H_INCLUDED)
