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

// A thread ID is the physial address of the thread's info structure.
typedef uintptr_t thread_id_t;

// A mailbox ID is the index of the mailbox's structure in an array.
typedef size_t mailbox_id_t;

// Error codes returned from monitor API calls.
typedef enum {
  // API call succeeded.
  monitor_ok = 0,

  // A parameter given to the API call was invalid.
  //
  // This most likely reflects a bug in the caller code.
  monitor_invalid_value = 1,

  // A resource referenced by the API call is in an unsuitable state.
  //
  // The API call will not succeed if the caller simply retries it. However,
  // the caller may be able to perform other API calls to get the resources in
  // a state that will allow this call to succeed.
  monitor_invalid_state = 2,

  // Failed to acquire a lock. Retrying with the same arguments might succeed.
  //
  // The monitor returns this instead of blocking a hardware thread when a
  // resource lock is acquired by another thread. This approach eliminates any
  // possibility of having the monitor deadlock. The caller is responsible for
  // retrying the API call.
  //
  // This is also sometime returned instead of monitor_invalid_value, in the
  // interest of reducing edge cases in monitor implementation.
  monitor_concurrent_call = 3,

  // The call was interrupted due to an asynchronous enclave exit (AEX).
  //
  // This is only returned by enter_enclave, and can be considered a more
  // specific case of monitor_concurrent_call. The caller should retry the
  // enclave_enter call, so the enclave thread can make progress.
  monitor_async_exit = 4,

  // The caller is not allowed to access a resource referenced by the API call.
  //
  // This is a more specific version of monitor_invalid_value. The monitor does
  // its best to identify these cases, but may fail.
  monitor_access_denied = 4,

  // The current monitor implementation does not support the request.
  //
  // The caller made a reasonable API request that exercises an unhandled edge
  // case in the monitor implementaiton. Some edge cases that would require
  // complex or difficult-to-test implementations are detected and handled by
  // returning monitor_unsupported.
  //
  // The documentation for API calls states the edge cases that result in a
  // monitor_unsupported response.
  monitor_unsupported = 5,
} api_result_t;

// Returns the amount of DRAM installed on the system.
size_t dram_size();

// Returns the bits in a physical address that determine the DRAM region.
//
// dram_region_shift() can be computed by right-shifting the mask by 1 until
// the least significant bit is 1. dram_region_count() can be computed by
// adding 1 to the shifted mask mentioned above. Therefore, no security monitor
// calls are provided for dram_region_{count,shift}().
size_t dram_region_mask();

// Locks a DRAM region that was previously owned by the caller.
//
// Enclaves calling this API are responsible for wiping any confidential
// information from the relinquished DRAM region.
//
// Before issuing this call, the OS is responsible for wiping its own
// confidential information from the DRAM region.
api_result_t block_dram_region(size_t dram_region);

namespace enclave {  // sanctum::api::enclave

// Returns monitor_ok if the given DRAM region is owned by the calling enclave.
//
// This is used by enclaves to confirm that they own a DRAM region when the OS
// tells them that they do. The enclave should that assume something went wrong
// if it sees any return value other than monitor_ok.
api_result_t dram_region_check_ownership(size_t dram_region);

// Allocates a thread info slot.
//
// `phys_addr` is the physical address of a range of pages that will hold the
// thread_init_info_t structure. The address must be page-aligned, and must not
// overlap with the pages used by the monitor.
api_result_t accept_thread(thread_id_t thread_id, uintptr_t thread_info_addr);

// Deallocates a thread info slot.
//
// The thread must not be running on any core.
api_result_t delete_thread(thread_id_t thread_id);

// Ends the currently running enclave thread and returns control to the OS.
api_result_t exit_enclave();

// Reads the monitor's private attestation key.
//
// This API call will only succeed if the calling enclave is the special
// enclave designated by the security monitor to be the signing enclave.
//
// `phys_addr` must point into a buffer large enough to store the attestation
// key. The entire buffer must be contained in a single DRAM region that
// belongs to the enclave.
api_result_t get_attestation_key(uintptr_t phys_addr);

// Prepares a mailbox to receive a message from another enclave.
//
// The mailbox will discard any message that it might contain.
//
// `phys_addr` must point into a buffer large enough to store a
// mailbox_identity_t structure. The entire buffer must be contained in a
// single DRAM region that belongs to the enclave.
api_result_t accept_message(mailbox_id_t mailbox_id, uintptr_t phys_addr);

// Attempts to read a message received in a mailbox.
//
// If the read succeeds, the mailbox will transition into the free state.
//
// `phys_addr` must point into a buffer large enough to store a
// mailbox_identity_t structure. The entire buffer must be contained in a
// single DRAM region that belongs to the enclave.
api_result_t read_message(mailbox_id_t mailbox_id, uintptr_t phys_addr);

// Sends a message to another enclave's mailbox.
//
// `enclave_id` and `mailbox_id` identify the destination mailbox.
//
// `phys_addr` must point into a buffer large enough to store a
// mailbox_identity_t structure. The entire buffer must be contained in a
// single DRAM region that belongs to the enclave.
//
// The structure contains the destination enclave's expected identity. The
// monitor will refuse to deliver the message
api_result_t send_message(enclave_id_t enclave_id, mailbox_id_t mailbox_id,
    uintptr_t phys_addr);

// Enclave-supplied information used to initialize a thread's metadata.
typedef struct {
  // The virtual address of the thread's entry point.
  void (*entry_pc)();
  // The virtual address of the thread's stack top.
  void *entry_stack;
  // The virtual address of the thread's fault handler.
  void (*fault_pc)();
  // The virtual address of the fault handler's stack top.
  void *fault_stack;

  // The EPTBR value to be loaded for this thread.
  //
  // The EPTBR contains the physical address of the enclave's page table base.
  uintptr_t eptbr;
} thread_init_info_t;

//
typedef struct {
  // The enclave ID is supplied by the OS.
  //
  // This untrusted ID is used to distinguish between multiple instances of the
  // same enclave.
  enclave_id_t enclave_id;

  // The enclave's measurement.
  //
  // This ensures that the identity of the enclave on the other side is as
  // expected.
  uint8_t enclave_hash[64];
} mailbox_identity_t;

};  // namespace sanctum::api::enclave


namespace os {  // sanctum::api::os

// Per-DRAM region accounting information.
typedef enum {
  dram_region_invalid = 0,
  dram_region_free = 1,
  dram_region_blocked = 2,
  dram_region_locked = 3,
  dram_region_owned = 4,
} dram_region_state_t;

// Sets the memory range that allows DMA transfers.
//
// The range must be entirely contained in DRAM regions allocated to the OS.
api_result_t set_dma_range(uintptr_t base, uintptr_t mask);

// Returns the state of the DRAM region with the given index.
//
// Returns dram_region_invalid if the given DRAM region index is invalid.
// Returns dram_region_locked if the given DRAM region is currently locked by
// another API call.
dram_region_state_t dram_region_state(size_t dram_region);

// Returns the owner of the DRAM region with the given index.
//
// Returns null_enclave_id if the given DRAM region index is invalid, locked by
// another operation, or if the region is not in the owned state.
enclave_id_t dram_region_owner(size_t dram_region);

// Assigns a free DRAM region to an enclave or to the OS.
//
// `new_owner` is the enclave ID of the enclave that will own the DRAM region.
// 0 means that the DRAM region will be assigned to the OS.
//
api_result_t assign_dram_region(size_t dram_region, enclave_id_t new_owner);

// Frees a DRAM region that was previously locked.
api_result_t free_dram_region(size_t dram_region);

// Performs the TLB flushes needed to free a locked region.
//
// System software must invoke this call instead of flushing the TLB directly,
// as the monitor's state must be updated to reflect the fact that a TLB flush
// has occurred.
api_result_t flush_cached_dram_regions();

// Reserves a free DRAM region to hold enclave metadata.
//
// DRAM regions that hold enclave metadata can be freed directly by calling
// free_dram_region(). Calling block_dram_region() on them will fail.
api_result_t create_metadata_region(size_t dram_region);

// Returns the number of addressable metadata pages in a DRAM metadata region.
//
// This may be smaller than the number of total pages in a DRAM region, if the
// computer does not have continuous DRAM regions and the security monitor does
// not support using non-continuous regions.
size_t metadata_region_pages();

// Returns the first usable metadata page in a DRAM metadata region.
//
// The beginning of each DRAM metadata region is reserved for the monitor's
// use. This returns the first page number that can be used to store
// enclave_info_t and thread_info_t structures.
size_t metadata_region_start();

// Returns the number of pages used by a thread metadata structure.
size_t thread_metadata_pages();

// Returns the number of pages used by an enclave metadata structure.
size_t enclave_metadata_pages(size_t mailbox_count);

// Creates an enclave's metadata structure.
//
// `enclave_id` must be the physical address of the first page in a sequence of
// free pages in the same DRAM metadata region stripe. It becomes the enclave's
// ID used for subsequent API calls. The required number of free metadata pages
// can be obtained by calling `enclave_metadata_pages`.
//
// `ev_base` and `ev_mask` indicate the range of enclave virtual addresses. The
// addresses this range get translated using the enclave page tables, and must
// point into enclave memory.
//
// `mailbox_count` is the number of mailboxes that the enclave will have. Valid
// mailbox IDs for this enclave will range from 0 to mailbox_count - 1.
//
// `debug` is set for debug enclaves. A security monitor that supports
// enclave debugging implements copy_debug_enclave_page, which can only be used
// on debug enclaves.
//
// All arguments become a part of the enclave's measurement.
api_result_t create_enclave(enclave_id_t enclave_id, uintptr_t ev_base,
    uintptr_t ev_mask, size_t mailbox_count, bool debug);

// Allocates a page in the enclave's main DRAM region for page tables.
//
// `enclave_id` must be an enclave that has not yet been initialized.
//
// `phys_addr` must be higher than the last physical address passed to a
// load_enclave_ function, must be page-aligned, and must point into a DRAM
// region owned by the enclave.
//
// `virtual_addr` is the lowest virtual address mapped by the newly created
// page table.
//
// `level` indicates the page table level (e.g., in x86, 0 for PT, 1 for PD, 2
// for PDPT, 3 for PML).
//
// `virtual_addr`, `level` and `acl` become a part of the enclave's
// measurement.
api_result_t load_page_table(enclave_id_t enclave_id, uintptr_t phys_addr,
    uintptr_t virtual_addr, size_t level, uintptr_t acl);

// Allocates and initializes a page in the enclave's main DRAM region.
//
// `enclave_id` must be an enclave that has not yet been initialized.
//
// `phys_addr` must be higher than the last physical address passed to a
// load_enclave_ function, must be page-aligned, and must point into a DRAM
// region owned by the enclave.
//
// `virtual_addr`, `acl`, and the contents of the page at `os_addr` become a
// part of the enclave's measurement.
api_result_t load_page(enclave_id_t enclave_id, uintptr_t phys_addr,
    uintptr_t virtual_addr, uintptr_t os_addr, uintptr_t acl);

// Creates a hardware thread in an enclave.
//
// `enclave_id` must be an enclave that has not yet been initialized.
//
// `thread_id` must be the physical address of the first page in a sequence of
// free pages in the same DRAM metadata region stripe. It becomes the thread's
// ID used for subsequent API calls. The required number of free metadata pages
// can be obtained by calling `thread_metadata_pages`.
//
// `entry_pc`, `entry_stack`, `fault_pc` and `fault_stack` are virtual
// addresses in the enclave's address space. They are used to set the
// corresponding fields in thread_init_info_t.
//
// This must be called after the enclave's root page table is set by a call to
// load_page_table().
api_result_t load_thread(enclave_id_t enclave_id, thread_id_t thread_id,
    uintptr_t entry_pc, uintptr_t entry_stack, uintptr_t fault_pc,
    uintptr_t fault_stack);

// Allocates a thread metadata structure to be used by an enclave.
//
// `thread_id` must be the physical address of the first page in a sequence of
// free pages in the same DRAM metadata region. It becomes the enclave's ID
// used for subsequent API calls. The required number of free metadata pages
// can be obtained by calling `enclave_metadata_pages`.
//
// `enclave_id` must be an enclave that has been initialized and has not yet
// been killed.
//
// `phys_addr` is the physical address of a range of pages that will hold the
// thread_init_info_t structure. The address must be page-aligned, and must not
// overlap with the pages used by the monitor.
api_result_t assign_thread(enclave_id_t enclave_id, thread_id_t thread_id);

// Marks the given enclave as initialized and ready to execute.
//
// `enclave_id` must identify an enclave that has not yet been initialized.
api_result_t init_enclave(enclave_id_t enclave_id);

// Starts executing enclave code on the current hardware thread.
//
// The application thread performing this system call will be suspended until
// the enclave code executes an enclave exit, or is interrupted by an
// asynchronous enclave exit.
//
// `enclave_id` must identify an enclave that has been initialized.
//
// `thread_id` must identify a hardware thread that was created but is not
// executing on any core.
api_result_t enter_enclave(enclave_id_t enclave_id, thread_id_t thread_id);

// Frees up all DRAM regions and the metadata associated with an enclave.
//
// This can only be called when there is no thread metadata associated with the
// enclave.
api_result_t delete_enclave(enclave_id_t enclave_id);

// Reads/writes a page from/to a debug enclave's memory.
//
// If the given enclave is a debug enclave, a page from its memory is copied
// into OS memory. `enclave_addr` must point to a page in a DRAM region c
// by the enclave, and `os_addr` must point to a page in a DRAM region owned by
// the OS.
api_result_t copy_debug_enclave_page(enclave_id_t enclave_id,
    uintptr_t enclave_addr, uintptr_t os_addr, bool read_from_enclave);

};  // namespace sanctum::api::os
};  // namespace sanctum::api
};  // namespace sanctum
#endif   // !defined(SANCTUM_PUBLIC_API_H_INCLUDED)
