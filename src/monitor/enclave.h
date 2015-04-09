#if !defined(MONITOR_ENCLAVE_H_INCLUDED)
#define MONITOR_ENCLAVE_H_INCLUDED

#include "bare/base_types.h"
#include "public/api.h"

namespace sanctum {
namespace internal {

using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;
using sanctum::bare::atomic;
using sanctum::bare::atomic_flag;

// Pointers to threads.
typedef struct {
  phys_ptr<thread_private_info_t> thread_info;

  // Acquired when this thread is running on a core.
  //
  // This cannot be in thread_info_t because we use it to track the number of
  // enclave threads, and that count must be trusted.
  atomic_flag run_lock;
} thread_slot_t;

// Extended version of thred_info_t.
typedef struct {
  // The public thread_info_t must be at the beginning of the structure.
  thread_info_t ti;

  atomic_flag exit_state_used;  // Set on AEX.
} thread_private_info_t;

// Per-enclave accounting information.
typedef struct {
  // The enclave lock must be the first field in the structure.
  // The lock must be acquired before accessing mutable enclave state.
  atomic_flag lock;

  uintptr_t enclave_ptrr;   // Physical address of the page table root.
  enclave_state_t state;
} enclave_info_t;

typedef enum {
  enclave_loading = 2,      // The OS is loading pages into the enclave.
  enclave_initialized = 3,  // The enclave is fully set up and can run.
} enclave_state_t;

};  // namespace sanctum::internal
};  // namespace sanctum


#endif  // !defined(MONITOR_ENCLAVE_H_INCLUDED)
