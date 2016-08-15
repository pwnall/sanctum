#if !defined(MONITOR_ENCLAVE_H_INCLUDED)
#define MONITOR_ENCLAVE_H_INCLUDED

#include "bare/base_types.h"
#include "bare/cpu_context.h"
#include "bare/phys_atomics.h"
#include "crypto/hash.h"
#include "public/api.h"

namespace sanctum {
namespace internal {

using sanctum::api::enclave_id_t;
using sanctum::api::enclave::thread_init_info_t;
using sanctum::bare::atomic;
using sanctum::bare::atomic_flag;
using sanctum::bare::phys_ptr;
using sanctum::bare::register_state_t;
using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;
using sanctum::crypto::hash_block_size;
using sanctum::crypto::hash_state_t;

// The per-thread information stored in metadata regions.
struct thread_info_t {
  // Protects this structure from data races.
  //
  // This lock should be acquired using lock_thread_metadata(), which
  // guarantees that the thread_info_t is valid at lock acquisition time by
  // holding the metadata region's lock while acquiring the thread's lock.
  atomic_flag lock;

  // The fields below get initialized from thread_init_info_t.

  // The virtual address of the thread's entry point.
  uintptr_t entry_pc;
  // The virtual address of the thread's stack top.
  uintptr_t entry_stack;
  // The virtual address of the thread's fault handler.
  uintptr_t fault_pc;
  // The virtual address of the fault handler's stack top.
  uintptr_t fault_stack;

  // The EPTBR value to be loaded for this thread.
  //
  // The EPTBR contains the physical address of the enclave's page table base.
  uintptr_t eptbr;

  register_state_t exit_state;  // enter_enclave caller state
  register_state_t aex_state;   // enclave state saved on AEX
  size_t can_resume;            // true if the AEX state is valid
};

// Secure inter-enclave communication.
struct mailbox_t {
  atomic_flag lock;
  size_t state;

  // The OS-assigned enclave ID of the expected sender.
  //
  // This is necessary to prevent a malicious enclave from DoSing other
  // enclaves in the system by spamming their mailboxes. This enclave ID should
  // not be trusted to identify the software inside the sender.
  enclave_id_t sender_id;
  // The measurement of the expected sender.
  //
  // This is a secure identifier for the software inside the sender enclave.
  hash_state_t sender_hash;
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
  // Protects this structure from data races.
  //
  // This lock should be acquired using lock_enclave_info(), which guarantees
  // that the enclave_info_t is valid at lock acquisition time by holding the
  // metadata region's lock while acquiring the enclave's lock.
  atomic_flag lock;

  // Number of mailbox_t structures following the thread_slot_t structures.
  size_t mailbox_count;

  // non-zero when the enclave was initialized and can execute threads.
  // NOTE: this isn't bool because we don't want to specialize phys_ptr<bool>.
  size_t is_initialized;

  // non-zero for debug enclaves.
  size_t is_debug;

  // Number of thread metadata structures assigned to the enclave.
  //
  // This must be zero for the enclave to be killed.
  size_t thread_count;

  // Number of DRAM regions assigned to the enclave.
  //
  // This must be zero for the enclave's metadata to be removed.
  size_t dram_region_count;

  // The base of the enclave's virtual address range.
  uintptr_t ev_base;

  // The mask of the enclave's virtual address range.
  uintptr_t ev_mask;

  // Physical address of the enclave's page table base during loading.
  //
  // This is set by the first load_page_table() call, and forced as the
  // EPTBR value for enclave threads created by load_thread.
  uintptr_t load_eptbr;

  // The phyiscal address of the last page loaded into the enclave by the OS.
  uintptr_t last_load_addr;

  // The enclave's measurement hash.
  //
  // This is updated by the enclave loading API calls, and finalized by
  // enclave_init(). It does not change afterwards.
  hash_state_t hash;

  // Working area for the enclave measurement process.
  uint32_t hash_block[hash_block_size / sizeof(uint32_t)];
};

// The DRAM region bitmap for the OS.
//
// This is synchronized by the lock of DRAM region 0, which must belong to the
// OS. The pointer itself is allocated at boot time, so it never changes.
extern phys_ptr<size_t> g_os_region_bitmap;


};  // namespace sanctum::internal
};  // namespace sanctum
#endif  // !defined(MONITOR_ENCLAVE_H_INCLUDED)
