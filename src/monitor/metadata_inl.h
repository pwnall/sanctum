#if !defined(MONITOR_METADATA_INL_H_INCLUDED)
#define MONITOR_METADATA_INL_H_INCLUDED

#include "bare/base_types.h"
#include "bare/bit_masking.h"
#include "bare/phys_ptr.h"
#include "dram_regions.h"
#include "enclave.h"
#include "metadata.h"

namespace sanctum {
namespace internal {  // sanctum::internal

using sanctum::api::null_enclave_id;
using sanctum::bare::pages_needed_for;

inline phys_ptr<metadata_page_info_t> metadata_page_info_for(
    uintptr_t phys_addr) {

}


// Computes the physical address of an enclave's DRAM region bitmap.
//
// The DRAM region bitmap has 1 bit for every DRAM region in the system. The
// bits corresponding to DRAM regions allocated to the enclave are set to 1.
inline phys_ptr<size_t> enclave_region_bitmap(enclave_id_t enclave_id) {
  phys_ptr<enclave_info_t> enclave_info{enclave_id};
  return phys_ptr<size_t>{uintptr_t(enclave_info + 1)};
}
// Computes the physical address of an enclave's mailboxes array.
inline phys_ptr<mailbox_t> enclave_mailboxes(enclave_id_t enclave_id) {
  phys_ptr<enclave_info_t> enclave_info{enclave_id};
  return phys_ptr<thread_slot_t>{uintptr_t(
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
constexpr inline size_t thread_info_size() {
  return sizeof(thread_info_t);
}

// The number of pages taken by an enclave hardware thread's metadata.
constexpr inline size_t thread_info_pages() {
  return pages_needed_for(thread_info_size());
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

};  // namespace sanctum::internal
};  // namespace sanctum
#endif  // !defined(MONITOR_METADATA_INL_H_INCLUDED)
