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
using sanctum::api::enclave::thread_info_t;
using sanctum::bare::atomic;
using sanctum::bare::atomic_flag;
using sanctum::bare::phys_ptr;
using sanctum::bare::register_state_t;
using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;
using sanctum::crypto::hash_block_size;
using sanctum::crypto::hash_state_t;

// The per-thread information stored in metadata regions.
struct thread_metadata_t {
  // We store the information used to create the thread as-is for simplicity.
  // thread_metadata_t is a monitor implementation detail, so its layout can
  // change later, if that makes sense.
  thread_info_t thread_info;

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
  // The base of the enclave's virtual address range.
  uintptr_t ev_base;

  // The mask of the enclave's virtual address range.
  uintptr_t ev_mask;

  // Number of mailbox_t structures following the thread_slot_t structures.
  size_t mailbox_count;

  // non-zero when the enclave was initialized and can execute threads.
  // NOTE: this isn't bool because we don't want to specialize phys_ptr<bool>.
  size_t is_initialized;

  // non-zero for debug enclaves.
  size_t is_debug;

  // Physical address of the enclave's page table base during loading.
  //
  // This is set by the first load_enclave_page_table() call, and forced as the
  // EPTBR value for enclave threads created by load_enclave_thread.
  uintptr_t load_eptbr;

  // The phyiscal address of the last page loaded into the enclave by the OS.
  uintptr_t last_load_addr;

  // Number of enclave threads running on cores.
  //
  // This field is only incremented while the enclave lock is held. However, it
  // is decremented without holding the enclave lock, in enclave exits.
  atomic<size_t> running_threads;

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
