#if !defined(MONITOR_DRAM_REGIONS_H_INCLUDED)
#define MONITOR_DRAM_REGIONS_H_INCLUDED

#include "bare/base_types.h"
#include "bare/phys_atomics.h"
#include "public/api.h"

namespace sanctum {
namespace internal {

using sanctum::api::enclave_id_t;
using sanctum::api::os::dram_region_state_t;
using sanctum::bare::atomic;
using sanctum::bare::atomic_flag;
using sanctum::bare::phys_ptr;
using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;

// Per-DRAM region accounting information.
typedef struct {
  atomic_flag lock;             // lock for all the DRAM region's state
  size_t state;                 // actually a dram_region_state_t
  atomic<enclave_id_t> owner;   // nullptr if not owned by enclave
  enclave_id_t previous_owner;  // nullptr if previously owned by OS
  size_t monitor_pages;         // pages reserved for the monitor
  size_t pinned_pages;          // pages that can't be removed from DRAM
  size_t blocked_at;            // only valid for blocked regions
} dram_region_info_t;

// Accounting information for all DRAM regions.
typedef struct {
  // NOTE: the lock generation counter is NOT protected by a lock, because it
  //       must be accessible in dram_region_flush(), which must be lock-free
  atomic<size_t> block_clock;
} dram_regions_info_t;

// The regions are allocated at boot time, so the physical pointers never
// change.

extern phys_ptr<dram_region_info_t> g_dram_region;
extern phys_ptr<dram_regions_info_t> g_dram_regions;

// The fields below are set by boot_init_dram_regions() and never change
// afterwards. Therefore, they do not require locking.
extern size_t g_dram_region_count;
extern size_t g_dram_region_size;
extern size_t g_dram_region_mask;  // size - 1
extern size_t g_dram_region_shift; // size == (1 << shift)

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

// Acquires the lock for a DRAM region.
//
// Invalid DRAM region indices will cause memory thrashing.
//
// Returns false if the lock was acquired, and true if it was already held by
// someone else.
inline bool test_and_set_dram_region_lock(size_t dram_region) {
  phys_ptr<dram_region_info_t> region = &g_dram_region[dram_region];
  return atomic_flag_test_and_set(&(region->*(&dram_region_info_t::lock)));
}

// Releases the lock for a DRAM region.
//
// Invalid DRAM region indices will cause memory thrashing.
inline void clear_dram_region_lock(size_t dram_region) {
  phys_ptr<dram_region_info_t> region = &g_dram_region[dram_region];
  atomic_flag_clear(&(region->*(&dram_region_info_t::lock)));
}

// Verifies the validity of an enclave ID. 0 is considered a valid ID.
//
// Returns true if the given enclave ID is valid, and false otherwise. 0 is
// used to indicate OS ownership of DRAM areas, so it is considered a valid ID.
inline bool is_valid_enclave_id(enclave_id_t enclave_id) {
  size_t dram_region = checked_dram_region_for(enclave_id);
  phys_ptr<dram_region_info_t> region = &g_dram_region[dram_region];
  enclave_id_t region_owner = atomic_load(
      &(region->*(&dram_region_info_t::owner)));

  // NOTE: the first DRAM region always belongs to the OS, so this returns true
  //       when enclave_id is 0 (indicating OS ownership)
  return region_owner == enclave_id;
}

};  // namespace sanctum::internal
};  // namespace sanctum

#endif  // !defined(MONITOR_DRAM_REGIONS_H_INCLUDED)
