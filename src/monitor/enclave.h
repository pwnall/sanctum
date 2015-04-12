#if !defined(MONITOR_ENCLAVE_H_INCLUDED)
#define MONITOR_ENCLAVE_H_INCLUDED

#include "bare/base_types.h"
#include "bare/phys_atomics.h"
#include "public/api.h"

namespace sanctum {
namespace internal {

using sanctum::api::enclave::thread_info_t;
using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;
using sanctum::bare::atomic;
using sanctum::bare::atomic_flag;
using sanctum::bare::phys_ptr;

// Extended version of thred_info_t.
struct thread_private_info_t {
  // The public thread_info_t must be at the beginning of the structure.
  thread_info_t ti;

  atomic_flag exit_state_used;  // Set on AEX.
};


// Pointers to threads.
struct thread_slot_t {
  phys_ptr<thread_private_info_t> thread_info;
  atomic_flag lock;
};

// Per-enclave accounting information.
//
// This is synchronized by the lock of the enclave's main DRAM region.
struct enclave_info_t {
  // Physical address of the enclave's page table base.
  uintptr_t eptbr;

  // Number of thread_slot_t structures following the enclave_info_t.
  //
  // Thread IDs are between 1 and max_threads.
  size_t max_threads;

  // non-zero when the enclave was initialized and can execute threads.
  // NOTE: this isn't bool because we don't want to specialize phys_ptr<bool>.
  size_t is_initialized;

  // non-zero for debug enclaves.
  size_t is_debug;

  // Number of enclave threads running on cores.
  //
  // This field is only incremented while the enclave lock is held. However, it
  // is decremented without holding the enclave lock, in enclave exits.
  atomic<size_t> running_threads;
};

};  // namespace sanctum::internal
};  // namespace sanctum
#endif  // !defined(MONITOR_ENCLAVE_H_INCLUDED)
