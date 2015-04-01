#include "sanctum-arch.h"
#include "sanctum-api-os.h"
#include "sanctum-api-enclave.h"


// An enclave ID is the physical address of the enclave's info structure.
typedef uintptr_t enclave_id_t;
// A thread ID is the physical address of the thread's info structure.
typedef uintptr_t thread_id_t;

// DRAM region accounting information.
typedef struct {
  dram_region_state_t state;
  atomic_uintptr_t owner;       // nullptr if not owned by enclave
  enclave_id_t previous_owner;  // nullptr if previously owned by OS
  size_t mapped_pages;          // only valid for enclave-owned regions
  size_t locked_at;             // only valid for locked regions
} region_info_t;

// The number of pages taken up by the enclave_info structure.
constexpr size_t enclave_info_pages =
    (sizeof(enclave_info) + page_size - 1) / page_size;

// DRAM region constants.
//
// These values are computed during the boot process because systems include
// various amounts of DRAM. Once computed, the values never change.

// Lock for the region info array and the lock clock.
atomic_flag g_region_info_lock;
region_info_t *g_region_info;

// NOTE: the lock generation counter is NOT protected by a lock, because it
//       must be accessible in dram_region_flush(), which must be lock-free
atomic_size_t g_lock_clock;

size_t g_dram_region_count;
size_t g_dram_region_size;
size_t g_dram_region_mask;   // g_dram_region_size - 1
// g_dram_region_size == (1 << g_dram_region_shift)
size_t g_dram_region_shift;
// (g_dram_region_size >> page_shift) <=
//     (g_dram_region_bitmap_pages * bits_in_a_page)
size_t g_dram_region_bitmap_pages;


// Per-core accounting information.
typedef struct {
  atomic_uintptr_t enclave;  // 0 if the core isn't executing enclave code
  thread_id_t thread;        // 0 if the core isn't executing enclave code
  atomic_uint flushed_at;    // lock_clock when this core's TLB was last flushed
} core_info_t;

// Core costants.
//
// These values are computed during the boot process. Once computed, the values
// never change.
core_info_t* g_core_info;
size_t g_core_count;

// Flushes the core's TLBs and updates the relevant flush generation counter.
//
// This code is guaranteed to be lock-free, as it is used in enclave exits.
inline void core_tlb_flush() {
  core_info_t& core_info = g_core_info[current_core()];
  unsigned lock_clock = atomic_load(&g_lock_clock);
  atomic_store(&core_info.flushed_at, lock_clock);

  // TODO: do the actual TLB flush
}

// Computes the physical start address of a DRAM region.
//
// Invalid DRAM region indices will yield invalid pointers.
inline uintptr_t dram_region_start(size_t dram_region) {
  return static_cast<uintptr_t>(dram_region) << g_dram_region_shift;
}

// Computes the DRAM region index for a pointer.
//
// Pointers outside DRAM will yield invalid region indices.
inline size_t dram_region_for(uintptr_t address) {
  return (address & g_dram_region_mask) >> g_dram_region_shift;
}

// Computes the DRAM region index for a pointer.
//
// Region index 0 will be returned for pointers outside DRAM.
inline size_t checked_dram_region_for(uintptr_t address) {
  size_t region = dram_region_for(address);
  return (region < g_dram_region_count) ? region : 0;
}

// Verifies the validity of an enclave ID. 0 is considered a valid ID.
//
// The DRAM region info lock must be held while calling this function.
// check_enclave_id_lockless can be used when the lock cannot be acquired.
//
// Returns true if the given enclave ID is valid, and false otherwise. 0 is
// used to indicate OS ownership of DRAM areas, so it is considered a valid ID.
inline bool check_enclave_id(enclave_id_t enclave_id) {
  size_t dram_region = checked_dram_region_for((ptr_t)enclave_id);

  // NOTE: the first DRAM region always belongs to the OS, so this returns true
  //       when enclave_id is 0 (indicating OS ownership)
  uintptr_t region_owner = &g_region_info[dram_region];
  return region_owner == enclave_id;
}

// Verifies the validity of an enclave ID. 0 is considered a valid ID.
//
// Returns true if the given enclave ID is valid, and false otherwise. 0 is
// used to indicate OS ownership of DRAM areas, so it is considered a valid ID.
inline bool check_enclave_id_lockless(enclave_id_t enclave_id) {
  size_t dram_region = checked_dram_region_for((ptr_t)enclave_id);

  // NOTE: the first DRAM region always belongs to the OS, so this returns true
  //       when enclave_id is 0 (indicating OS ownership)
  uintptr_t region_owner = atomic_load(&g_region_info[dram_region].owner);
  return region_owner == enclave_id;
}

// Computes the physical address of an enclave's lock.
//
// Invalid enclave IDs will yield invalid pointers.
inline uintptr_t enclave_lock_addr(enclave_id_t enclave_id) {
  enclave_info_t* enclave_info = static_cast<enclave_info_t*>(enclave_id);
  return static_cast<uintptr_t>(&(enclave_info->lock));
}

// Computes the physical address of an enclave's ECR3 register.
//
// Invalid enclave IDs will yield invalid pointers.
inline uintptr_t enclave_ptrr_addr(enclave_id_t enclave_id) {
  enclave_info_t* enclave_info = static_cast<enclave_info_t*>(enclave_id);
  return static_cast<uintptr_t>(&(enclave_info->enclave_ptrr));
}

// Computes the physical address of the enclave_info structure virtual address.
//
// Invalid enclave IDs will yield invalid pointers.
inline enclave_info_t *enclave_info_addr(enclave_id_t enclave_id) {
  enclave_info_t* enclave_info = static_cast<enclave_info_t*>(enclave_id);
  return static_cast<uintptr_t>(&(enclave_info->virtual_addr));
}

// Convenience method for acquiring an enclave lock.
//
// Invalid enclave IDs will trash DRAM.
inline bool enclave_test_and_set_lock(enclave_id_t enclave_id) {
  return phys_atomic_flag_test_and_set(enclave_lock_addr(enclave_id));
}

// Convenience method for releasing an enclave lock.
//
// Invalid enclave IDs will trash DRAM.
inline void enclave_clear_lock(enclave_id_t enclave_id) {
  phys_atomic_flag_clear(enclave_lock_addr(enclave_id));
}

// Convenience method for activating an enclave's page tables.
//
// The enclave must be set as the active enclave in the core's core_info_t
// structure, otherwise the monitor's PMC will reject the page table's
// mappings.
//
// TODO: what lock should be held for this?
inline enclave_info_t* enable_enclave_pages(enclave_id_t enclave_id) {
  uintptr_t ptrr_addr = enclave_ptrr_addr(enclave_id);
  uintptr_t info_addr = enclave_info_addr(enclave_id);

  enclave_info_t* enclave_info = load_phys_ptr(info_addr);
  uintptr_t eptrr = load_phys_ptr(ptrr_addr);

  set_eptr_register(eptrr);
  return enclave_info;
}

// Convenience method for deactivating an enclave's page tables.
//
// TODO: what lock should be held for this?
inline void disable_enclave_pages() {
  set_eptr_register(0);
  core_tlb_flush();
  return enclave_info;
}
