#if !defined(MONITOR_ENCLAVE_H_INCLUDED)
#define MONITOR_ENCLAVE_H_INCLUDED

#include "bare/base_types.h"
#include "bare/phys_atomics.h"
#include "public/api.h"

namespace sanctum {
namespace internal {

using sanctum::api::enclave::thread_info_t;
using sanctum::bare::atomic;
using sanctum::bare::atomic_flag;
using sanctum::bare::phys_ptr;
using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;

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
// This structure is stored at the beginning of an enclave's main DRAM region,
// followed by the enclave's DRAM region bitmap and thread slots. The monitor
// ensures that the pages holding the structure are not evicted while the
// enclave is alive.
//
// This is synchronized by the lock of the enclave's main DRAM region.
struct enclave_info_t {
  // Number of thread_slot_t structures following the enclave_info_t.
  //
  // Thread IDs are between 1 and max_threads.
  size_t max_threads;

  // The base of the enclave's virtual address range.
  uintptr_t ev_base;

  // The mask of the enclave's virtual address range.
  uintptr_t ev_mask;

  // non-zero when the enclave was initialized and can execute threads.
  // NOTE: this isn't bool because we don't want to specialize phys_ptr<bool>.
  size_t is_initialized;

  // non-zero for debug enclaves.
  size_t is_debug;

  // Physical address of the enclave's page table base during loading.
  //
  // This is set by the first load_enclave_page_table() call, and forced as the
  // EPTBR value for enclave threads created by load_enclave_thread.
  uintptr_t loading_eptbr;

  // The phyiscal address of the last page loaded into the enclave by the OS.
  uintptr_t loading_last_addr;

  // The end of the monitor area at the beginning of the enclave.
  uintptr_t monitor_area_top;

  // Number of enclave threads running on cores.
  //
  // This field is only incremented while the enclave lock is held. However, it
  // is decremented without holding the enclave lock, in enclave exits.
  atomic<size_t> running_threads;
};

// The DRAM region bitmap for the OS.
//
// This is synchronized by the lock of DRAM region 0, which must belong to the
// OS. The pointer itself is allocated at boot time, so it never changes.
extern phys_ptr<size_t> g_os_region_bitmap;


};  // namespace sanctum::internal
};  // namespace sanctum
#endif  // !defined(MONITOR_ENCLAVE_H_INCLUDED)
