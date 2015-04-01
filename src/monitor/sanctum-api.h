#include "sanctum-arch.h"

// An enclave ID is the physical address of the enclave's info structure.
typedef uintptr_t enclaveid_t;
constexpr enclave_id_t null_enclave_id = 0;

// A thread ID is the physical address of the thread's info structure.
typedef uintptr_t threadid_t;
constexpr thread_id_t null_thread_id = 0;

// Error codes returned from monitor API calls.
typedef enum {
  monitor_success = 0,
  monitor_invalid_value = 1,
  monitor_invalid_state = 2,
  monitor_concurrent_call = 3,
  monitor_access_denied = 4
} api_result_t;

// Number of DRAM regions recognized by security monitor.
//
// This API is not secured against cache timing attacks because enclaves should
// only call it in initialization code, and the OS already knows when an
// enclave is getting initialized, because it issues the enclave_enter() API
// call.
size_t dram_region_count();
// The size of a DRAM region. Always a power of two.
//
// This API is not secured against cache timing attacks because enclaves should
// only call it in initialization code, and the OS already knows when an
// enclave is getting initialized, because it issues the enclave_enter() API
// call.
size_t dram_region_size();

// Locks a DRAM region that was previously owned by the caller.
//
// This API is not secured against cache timing attacks because enclaves should
// only call it when the OS asks them to relinquish a DRAM region, so the OS
// already knows that the call will occur, and knows what DRAM region will be
// locked.
api_result_t lock_dram_region(size_t dram_region);

