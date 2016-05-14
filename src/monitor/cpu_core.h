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

struct thread_metadata_t;

// Per-core accounting information.
//
// Most information is only modified on the core that it corresponds to, so we
// don't need atomics or locking. The notable exception is the flushed_at
// timestamp.
struct core_info_t {
  enclave_id_t enclave_id;  // 0 if the core isn't executing enclave code
  thread_id_t thread_id;

  // The DRAM backing the thread_metadata_t is guaranteed to be pinned
  // while the thread is executing on a core.
  phys_ptr<thread_metadata_t> thread;

  // The value of block_clock when this core's TLB was last flushed.
  // This is read on other cores,
  atomic<size_t> flushed_at;
};

// Core costants.
//
// These values are computed during the boot process. Once computed, the values
// never change.

extern phys_ptr<core_info_t> g_core;

extern size_t g_core_count;

};  // namespace sanctum::internal
};  // namespace sanctum
#endif  // !defined(MONITOR_CPU_CORE_H_INCLUDED)
