#if !defined(MONITOR_METADATA_INL_H_INCLUDED)
#define MONITOR_METADATA_INL_H_INCLUDED

#include "bare/base_types.h"
#include "bare/bit_masking.h"
#include "bare/phys_ptr.h"
#include "dram_regions_inl.h"
#include "enclave.h"
#include "metadata.h"

namespace sanctum {
namespace internal {  // sanctum::internal

using sanctum::api::api_result_t;
using sanctum::api::mailbox_id_t;
using sanctum::api::monitor_concurrent_call;
using sanctum::api::monitor_invalid_state;
using sanctum::api::monitor_invalid_value;
using sanctum::api::monitor_ok;
using sanctum::api::null_enclave_id;
using sanctum::bare::is_page_aligned;
using sanctum::bare::pages_needed_for;

constexpr inline metadata_page_info_t metadata_page_info(enclave_id_t owner,
    metadata_page_info_t page_type) {
  return static_cast<metadata_page_info_t>(owner) | page_type;
}

// The value used to indicate empty pages.
//
// This must be zero.
constexpr metadata_page_info_t empty_metadata_page_info =
    metadata_page_info(null_enclave_id, empty_metadata_page_type);

// Initializes a DRAM region to be used as a metadata region.
//
// Invalid DRAM region indices will cause memory trashing.
//
// The caller should hold the given DRAM region's lock. The DRAM region should
// be free.
inline void init_metadata_region(size_t dram_region) {
  phys_ptr<dram_region_info_t> region = &g_dram_region[dram_region];
  region->*(&dram_region_info_t::owner) = metadata_enclave_id;
  region->*(&dram_region_info_t::pinned_pages) = 0;

  phys_ptr<metadata_page_info_t> metadata_map{dram_region_start(dram_region)};
  bzero(metadata_map, g_metadata_region_start << page_shift());
}

// Attempts to locks the metadata region for a metadata page address.
//
// Returns a monitor API call error code. If the code is not monitor_ok, it can
// be passed as-is to the caller. This can happen if the given address is not a
// valid metadata page address, or if the metadata region is locked.
//
// If this succeeds, it sets dram_region to the DRAM region index of the
// metadata region that the given address belongs to.
inline api_result_t lock_metadata_region_for(uintptr_t phys_addr,
    size_t& dram_region) {
  if (!is_page_aligned(phys_addr) || !is_dram_address(phys_addr))
    return monitor_invalid_value;

  dram_region = dram_region_for(phys_addr);
  if (test_and_set_dram_region_lock(dram_region))
    return monitor_concurrent_call;

  phys_ptr<dram_region_info_t> region = &g_dram_region[dram_region];
  if (region->*(&dram_region_info_t::owner) != metadata_enclave_id) {
    clear_dram_region_lock(dram_region);
    return monitor_invalid_state;
  }
  return monitor_ok;
}

inline phys_ptr<metadata_page_info_t> metadata_page_info_for(
    uintptr_t phys_addr) {
  return phys_ptr<metadata_page_info_t>{phys_addr & g_dram_region_mask} +
      dram_region_page_for(phys_addr);
}

// Attempts to allocate pages for a metadata structure.
//
// The caller must ensure that phys_addr falls into a metadata region, and must
// hold the lock for that DRAM region. The caller must not release the DRAM
// region lock until it finishes setting up the metadata structure.
//
// Returns a monitor API call error code. If the code is not monitor_ok, it can
// be passed as-is to the caller. This can happen if the metadata region does
// not have enough continuous pages for the structure, or if any of the
// metadata pages are allocated to something else.
inline api_result_t alloc_metadata_item(uintptr_t phys_addr, size_t page_count,
    enclave_id_t owner, metadata_page_info_t type) {

  if (dram_stripe_page_for(phys_addr) + page_count > g_dram_stripe_pages)
    return monitor_invalid_value;

  phys_ptr<metadata_page_info_t> page_info = metadata_page_info_for(phys_addr);
  for (size_t i = 0; i < page_count; ++i) {
    if (page_info[i]) return monitor_invalid_state;
  }

  *page_info = metadata_page_info(owner, type);

  metadata_page_info_t inner_page_info =
      metadata_page_info(owner, inner_metadata_page_type);
  for (size_t i = 1; i < page_count; ++i, page_info += 1)
    *page_info = inner_metadata_page_type;
  return monitor_ok;
}

// Computes the physical address of an enclave's DRAM region bitmap.
//
// The DRAM region bitmap has 1 bit for every DRAM region in the system. The
// bits corresponding to DRAM regions allocated to the enclave are set to 1.
inline phys_ptr<size_t> enclave_region_bitmap(enclave_id_t enclave_id) {
  const phys_ptr<enclave_info_t> enclave_info{enclave_id};
  return phys_ptr<size_t>{uintptr_t(enclave_info + 1)};
}
// Computes the physical address of an enclave's mailboxes array.
inline phys_ptr<mailbox_t> enclave_mailboxes(enclave_id_t enclave_id) {
  return phys_ptr<mailbox_t>{uintptr_t(
      enclave_region_bitmap(enclave_id) + g_dram_region_bitmap_words)};
}
// Computes the physical address of an enclave mailbox.
inline phys_ptr<mailbox_t> enclave_mailbox(enclave_id_t enclave_id,
      mailbox_id_t mailbox_id) {
  return enclave_mailboxes(enclave_id) + mailbox_id;
}

// The amount of memory used by the security monitor for an enclave.
//
// The monitor data consists of an enclave_info_t, a DRAM region bitmap, and a
// thread_slot_t array. It is stored at the beginning of an enclave's main DRAM
// region, and cannot be modified by the enclave.
//
// This returns the precise amount of memory used by the monitor. However, all
// metadata memory management happens at page granularity, so
// enclave_info_pages() is a better reflection of the amount of DRAM
// allocated to monitor pages.
inline size_t enclave_info_size(size_t mailbox_count) {
  return static_cast<size_t>(uintptr_t(enclave_mailbox(0, mailbox_count)));
}

// The number of pages used by the security monitor for an enclave.
//
// See enclave_info_size() for an explanation of the security monitor's data.
inline size_t enclave_info_pages(size_t mailbox_count) {
  return pages_needed_for(enclave_info_size(mailbox_count));
}

// The size of an enclave hardware thread's metadata, in bytes.
constexpr inline size_t thread_metadata_size() {
  return sizeof(thread_metadata_t);
}

// The number of pages taken by an enclave hardware thread's metadata.
constexpr inline size_t thread_metadata_pages() {
  return pages_needed_for(thread_metadata_size());
}

// Sets a bit in a DRAM region bitmap.
//
// The caller should hold the lock of the enclave metadata's DRAM region.
//
// A null enclave_id causes a bit to be set in the OS' DRAM region bitmap.
inline void set_enclave_region_bitmap_bit(enclave_id_t enclave_id,
    size_t dram_region, bool true_for_set) {
  if (enclave_id == null_enclave_id) {
    set_bitmap_bit(g_os_region_bitmap, dram_region, true_for_set);
  } else {
    set_bitmap_bit(enclave_region_bitmap(enclave_id), dram_region,
        true_for_set);
  }
}

// Reads a bit from a DRAM region bitmap.
//
// The caller should hold the lock of the enclave's main DRAM region.
//
// A null enclave_id causes a bit to be read from the OS' DRAM region bitmap.
inline bool read_enclave_region_bitmap_bit(enclave_id_t enclave_id,
    size_t dram_region) {
  if (enclave_id == null_enclave_id)
    return read_bitmap_bit(g_os_region_bitmap, dram_region);
  else
    return read_bitmap_bit(enclave_region_bitmap(enclave_id), dram_region);
}

// Verifies the validity of an enclave ID. 0 is considered a valid ID.
//
// This should be called while holding the DRAM region lock for the region
// corresponding to the given ID. This is computed by
// clamped_dram_region_for(enclave_id).
//
// Returns true if the given enclave ID is valid, and false otherwise. 0 is
// used to indicate OS ownership of DRAM areas, so it is considered a valid ID.
inline bool is_valid_enclave_id(enclave_id_t enclave_id) {
  size_t dram_region = clamped_dram_region_for(enclave_id);
  phys_ptr<dram_region_info_t> region = &g_dram_region[dram_region];

  // NOTE: the first DRAM region always belongs to the OS, so this returns true
  //       when enclave_id is 0 / null_enclave_id (indicating OS ownership)
  return region->*(&dram_region_info_t::owner) == enclave_id;
}

};  // namespace sanctum::internal
};  // namespace sanctum
#endif  // !defined(MONITOR_METADATA_INL_H_INCLUDED)
