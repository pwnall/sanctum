// This header relies on the standard types size_t and uintptr_t. Use the
// includes below:
//
//     #include <cstddef>
//     #include <cstdint>
//
// or this include inside the Sanctum source tree:
//
//     #include "bare/base_types.h"

#if !defined(SANCTUM_PUBLIC_API_H_INCLUDED)
#define SANCTUM_PUBLIC_API_H_INCLUDED

namespace sanctum {
namespace api {

// An enclave ID is the physical address of the enclave's info structure.
typedef uintptr_t enclave_id_t;
constexpr enclave_id_t null_enclave_id = 0;

// A thread ID is the index of the thread's info structure pointer in an array.
typedef size_t thread_id_t;
constexpr thread_id_t null_thread_id = 0;

// Error codes returned from monitor API calls.
typedef enum {
  monitor_ok = 0,
  monitor_invalid_value = 1,
  monitor_invalid_state = 2,
  monitor_concurrent_call = 3,
  monitor_access_denied = 4
} api_result_t;

// The amount of DRAM installed on the system.
//
// This API is not secured against cache timing attacks because enclaves should
// only call it in initialization code, and the OS already knows when an
// enclave is getting initialized, because it issues the enclave_enter() API
// call.
size_t dram_size();

// The bits in a physical address that determine the DRAM region.
//
// dram_region_shift() can be computed by right-shifting the mask by 1 until
// the least significant bit is 1. dram_region_count() can be computed by
// adding 1 to the shifted mask mentioned above. Therefore, no security monitor
// calls are provided for dram_region_{count,shift}().
//
// This API is not secured against cache timing attacks because enclaves should
// only call it in initialization code, and the OS already knows when an
// enclave is getting initialized, because it issues the enclave_enter() API
// call.
size_t dram_region_mask();

// Locks a DRAM region that was previously owned by the caller.
//
// Enclaves calling this API are responsible for wiping any confidential
// information from the relinquished DRAM region.
//
// This API is not secured against cache timing attacks because enclaves should
// only call it when the OS asks them to relinquish a DRAM region, so the OS
// already knows that the call will occur, and knows what DRAM region will be
// locked.
api_result_t block_dram_region(size_t dram_region);


namespace enclave {  // sanctum::api::enclave

// Returns 0 if the given DRAM region is owned by the calling enclave.
//
// This is used by enclaves to confirm that they own a DRAM region when the OS
// tells them that they do.
api_result_t dram_region_check_ownership(size_t dram_region);

// Allocates a thread info slot.
//
// `phys_addr` is the physical address of a range of pages that will hold the
// thread_info_t structure. The address must be page-aligned, and must not
// overlap with the pages used by the monitor.
api_result_t create_enclave_thread(thread_id_t thread_id, uintptr_t phys_addr);

// Deallocates a thread info slot.
//
// The thread must not be running on any core.
api_result_t delete_enclave_thread(thread_id_t thread_id);

// Ends an enclave thread and returns control to the OS.
api_result_t exit_enclave();

// Metadata for each hardware thread in an enclave.
typedef struct {
  void (*entry_point)();
  void *entry_stack;
  void (*fault_handler)();
  void *fault_stack;

  //enclave_exit_state_t exit_state;
} thread_info_t;

};  // namespace sanctum::api::enclave


namespace os {  // sanctum::api::os

// Per-DRAM region accounting information.
typedef enum {
  dram_region_invalid = 0,
  dram_region_free = 1,
  dram_region_blocked = 2,
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
// Before issuing this call, the OS is responsible for wiping confidential
// information from DRAM the region, if it was previously assigned to the OS.
// The OS is not responsible for wiping enclave-owned DRAM regions.
api_result_t assign_dram_region(size_t dram_region, enclave_id_t new_owner);

// Frees a DRAM region that was previously locked.
api_result_t free_dram_region(size_t dram_region);

// Performs the TLB flush needed to free a locked region.
api_result_t dram_region_flush();

// Creates an enclave using the given free DRAM region.
//
// `ev_base` and `ev_mask` indicate the range of enclave virtual addresses. The
// addresses this range get translated using the enclave page tables, and must
// point into enclave memory.
//
// `max_thread_count` is the maximum number of enclave threads that can be
// allocated. This argument directs the number of 2-pointer slots created for
// threads. For example, on a 64-bit machine, a max_thread_count of 256 will
// allocate 4KB of data.
//
// `debug` is set for debug enclaves. The security monitor allows debug reads
// and writes in debug enclaves, to facilitate testing and debugging.
//
// Returns an enclave ID, or null_enclave_id in case of an error.
enclave_id_t create_enclave(size_t dram_region, uintptr_t ev_base,
    uintptr_t ev_mask, size_t max_thread_count, bool debug);

// Allocates a page in the enclave's main DRAM region for page tables.
//
// `phys_addr` must be higher than the last physical address passed to a
// load_enclave_ function, must be page-aligned, and must point into a DRAM
// region owned by the enclave.
//
// `virtual_addr` is the lowest virtual address mapped by the newly created
// page table.
//
// `lvl` indicates the page table level (e.g., in x86, 1 for PT, 2 for PD, 3
// for PDPT).
//
// This API will not check if a page table entry was already created for the
// given virtual address and level. It is assumed that the enclave creator will
// not expect duplicate calls
api_result_t load_enclave_page_table(enclave_id_t enclave_id,
    uintptr_t phys_addr, uintptr_t virtual_addr, int lvl);

// Allocates and initializes a page in the enclave's main DRAM region.
//
// `phys_addr` must be higher than the last physical address passed to a
// load_enclave_ function, must be page-aligned, and must point into a DRAM
// region owned by the enclave.
api_result_t load_enclave_page(enclave_id_t enclave_id, uintptr_t phys_addr,
    uintptr_t virtual_addr, uintptr_t os_addr, uintptr_t access_bits);

// Creates a hardware thread in an enclave.
thread_id_t load_enclave_thread(enclave_id_t enclave_id, thread_id_t thread_id,
      uintptr_t thread_info_addr);

// Marks the given enclave as initialized and ready to execute.
api_result_t init_enclave(enclave_id_t enclave_id);

// Starts an enclave thread.
api_result_t run_enclave_thread(enclave_id_t enclave_id,
    thread_id_t thread_id);

// Frees up all DRAM regions associated with an enclave.
//
// This can only be called when no enclave thread is running on any core. The
// API wipes the DRAM regions belonging to the enclave and marks them as free.
api_result_t delete_enclave(enclave_id_t enclave_id);


// Reads/writes a page from/to a debug enclave's memory.
//
// If the given enclave is a debug enclave, a page from its memory is copied
// into OS memory. `enclave_addr` must point to a page in a DRAM region c
// by the enclave, and `os_addr` must point to a page in a DRAM region owned by
// the OS.
api_result_t debug_enclave_copy_page(enclave_id_t enclave_id,
    uintptr_t enclave_addr, uintptr_t os_addr, bool read_from_enclave);

};  // namespace sanctum::api::os
};  // namespace sanctum::api
};  // namespace sanctum
#endif   // !defined(SANCTUM_PUBLIC_API_H_INCLUDED)
