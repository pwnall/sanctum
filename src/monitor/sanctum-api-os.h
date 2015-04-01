#include "sanctum-api.h"

// Per-DRAM region accounting information.
typedef enum {
  dram_region_invalid = 0,
  dram_region_free = 1,
  dram_region_locked = 2,
  dram_region_owned = 3,
} dram_region_state_t;

// The state of the DRAM region with the given index.
dram_region_state_t dram_region_state(size_t dram_region);

// The owner of the DRAM region with the given index.
enclave_id_t dram_region_owner(size_t dram_region);

// Assigns a free DRAM region to an enclave or to the OS.
//
// `new_owner` is the enclave ID of the enclave that will own the DRAM region.
// 0 means that the DRAM region will be assigned to the OS.
//
// Returns monitor_ok for success.
api_result_t assign_dram_region(size_t dram_region, enclave_id_t new_owner);

// Frees a DRAM region that was previously locked.
//
// Returns monitor_ok for success.
api_result_t free_dram_region(size_t dram_region);

// Performs the TLB flush needed to free a locked region.
//
// Returns monitor_ok for success.
api_result_t dram_region_flush();


// Creates an enclave using the given free DRAM region.
//
// Returns an enclave ID, or null_enclave_id in case of an error.
enclave_id_t make_enclave(size_t dram_region);

// Allocates a page in the enclave's main DRAM region for page tables.
//
// `lvl` indicates the page table level (e.g., in x86, 1 for PT, 2 for PD, 3
// for PDPT). The top level is already created by `make_enclave`.
//
// `addr` is the lowest virtual address mapped by the newly created page table.
//
// Returns 0 for success.
api_result_t load_enclave_page_table(enclave_id eid, uintptr_t addr, int lvl);

//
thread_id_t load_enclave_thread(enclave_id eid, uintptr_t addr);

int load_enclave_page(enclave_id eid, ptr_t dst, uintptr_t src, int access);

// Marks the given enclave as initialized and ready to execute.
int init_enclave(enclave_id eid);
