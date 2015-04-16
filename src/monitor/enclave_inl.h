#if !defined(MONITOR_ENCLAVE_INL_H_INCLUDED)
#define MONITOR_ENCLAVE_INL_H_INCLUDED

#include "dram_regions.h"
#include "enclave.h"

namespace sanctum {
namespace internal {

using sanctum::bare::page_size;
using sanctum::bare::page_shift;

// Computes the physical address of an enclave's DRAM region bitmap.
//
// The DRAM region bitmap has 1 bit for every DRAM region in the system. The
// bits corresponding to DRAM regions allocated to the enclave are set to 1.
inline phys_ptr<size_t> enclave_region_bitmap(enclave_id_t enclave_id) {
  phys_ptr<enclave_info_t> enclave_info{enclave_id};
  return phys_ptr<size_t>{uintptr_t(enclave_info + 1)};
}
// Computes the physical address of an enclave's thread slots.
inline phys_ptr<thread_slot_t> enclave_thread_slots(enclave_id_t enclave_id) {
  phys_ptr<enclave_info_t> enclave_info{enclave_id};
  return phys_ptr<thread_slot_t>{uintptr_t(
      enclave_region_bitmap(enclave_id) + g_dram_region_bitmap_words)};
}
// Computes the physical address of a specific enclave thread slot.
inline phys_ptr<thread_slot_t> enclave_thread_slot(enclave_id_t enclave_id,
    thread_id_t thread_id) {
  return enclave_thread_slots(enclave_id) + thread_id;
}


// The amount of memory used by the security monitor for an enclave.
//
// The monitor data consists of an enclave_info_t, a DRAM region bitmap, and a
// thread_slot_t array. It is stored at the beginning of an enclave's main DRAM
// region, and cannot be modified by the enclave.
inline size_t enclave_monitor_area_size(size_t max_threads) {
  return static_cast<size_t>(uintptr_t(enclave_thread_slots(0) + max_threads));
}

inline size_t enclave_monitor_area_pages(size_t max_threads) {
  return (enclave_monitor_area_size(max_threads) + page_size() - 1)
      >> page_shift();
}


// Sets a bit in a DRAM region bitmap.
//
// A null enclave_id causes a bit to be set in the OS' DRAM region bitmap.
inline void set_enclave_region_bitmap_bit(enclave_id_t enclave_id,
    size_t dram_region, bool true_for_set) {
  if (enclave_id == null_enclave_id)
    set_bitmap_bit(g_os_region_bitmap, dram_region, true_for_set);
  else
    set_bitmap_bit(enclave_region_bitmap(enclave_id), dram_region, true_for_set);
}

// Reads a bit from a DRAM region bitmap.
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
#endif  // !defined(MONITOR_ENCLAVE_INL_H_INCLUDED)
