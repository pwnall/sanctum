#include "sanctum-api.h"


// Sets up a free page in the enclave's DRAM regions as a page table.
//
// `lvl` indicates the page table level (e.g., in x86, 1 for PT, 2 for PD, 3
// for PDPT). The top level (e.g., in x86, 4 for PML4) is already created by
// `make_enclave`.
//
// `phys_addr` is the physical address of a free page in one of the enclave's
// DRAM regions.
//
// `addr` is the lowest virtual address mapped by the newly created page table.
//
// Returns 0 for success.
int make_enclave_page_table(ptr_t phys_addr, ptr_t addr, int lvl);

// Sets up a free page in the enclave' DRAM regions as a thread info structure.
//
// `phys_addr` is the physical address of a free page in one of the enclave's
// DRAM regions.
//
// `addr` is the address that the thread info will be mapped to.
thread_id_t make_enclave_thread(ptr_t phys_addr, ptr_t addr);

// Maps a free page in the enclave's DRAM regions for enclave use.
//
// Returns 0
int map_enclave_page(ptr_t phys_addr, ptr_t addr);


typedef enum {
  enclave_loading = 2,      // The OS is loading pages into the enclave.
  enclave_initialized = 3,  // The enclave is fully set up and can run.
} enclave_state_t;

// Metadata for each enclave.
typedef struct {
  // The enclave lock must be the first field in the structure.
  enum {
    // The lock must be acquired before accessing mutable enclave state.
    atomic_flag lock;
    // The lock is initialized using a size_t physical store instruction.
    size_t __alloc_size_t_for_lock;
  };

  uintptr_t enclave_ptrr;         // Physical address of the page table root.
  uintptr_t virtual_address;      // Virtual address of this structure.
  uintptr_t dram_region_bitmaps;  // Virtual address of the DRAM bitmaps.
  // The virtual address of each level of the enclave's page tables.
  uintptr_t page_tables[page_table_levels];

  enclave_state_t state;
  void (*entry_point)();
  void (*fault_handler)();
} enclave_info_t;

// Metadata for each hardware thread in an enclave.
typedef struct {
  atomic_flag exit_state_used;
  enclave_exit_state_t exit_state;

} thread_info_t;


