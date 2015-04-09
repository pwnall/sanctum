#include "monitor/public/api_os.h"
#include "monitor/public/api_enclave.h"


// The number of pages taken up by the enclave_info structure.
constexpr size_t enclave_info_pages =
    (sizeof(enclave_info) + page_size - 1) / page_size;

// Flushes the core's TLBs and updates the relevant flush generation counter.
//
// This code is guaranteed to be lock-free, as it is used in enclave exits.
inline void core_tlb_flush() {
  core_info_t& core_info = g_core_info[current_core()];
  unsigned lock_clock = atomic_load(&g_lock_clock);
  atomic_store(&core_info.flushed_at, lock_clock);

  // TODO: do the actual TLB flush
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
