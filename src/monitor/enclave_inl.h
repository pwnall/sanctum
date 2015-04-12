#if !defined(MONITOR_ENCLAVE_INL_H_INCLUDED)
#define MONITOR_ENCLAVE_INL_H_INCLUDED

namespace sanctum {
namespace internal {

using sanctum::bare::page_size;
using sanctum::bare::page_shift;

// Checks if an argument is a valid range mask.
//
// Valid range masks are written in binary as a sequence of 0s, followed by a
// sequence of 1s. Equivalently, the size of a base/mask range must be a power
// of two.
constexpr inline bool is_valid_range_mask(uintptr_t mask) {
  return (mask & (mask + 1)) == 0;
}
// Checks if an address is aligned with regard to a mask.
constexpr inline bool is_aligned_to_mask(uintptr_t address, uintptr_t mask) {
  return (address & mask) == 0;
}
// Checks the validity of a base/mask range.
//
// Valid range masks are written in binary as a sequence of 0s, followed by a
// sequence of 1s. Equivalently, the size of a base/mask range must be a power
// of two.
//
// A valid range base has 0s in the positions where the range mask has 1s.
// Equivalently, the range's base must be size-aligned.
constexpr inline bool is_valid_range(uintptr_t base, uintptr_t mask) {
  return is_aligned_to_mask(base, mask) && is_valid_range_mask(mask);
}
// Checks if an address is aligned to an address translation page.
inline bool is_page_aligned(uintptr_t address) {
  return is_aligned_to_mask(address, page_size() - 1);
}

// Computes the physical address of an enclave's thread slots.
inline phys_ptr<thread_slot_t> enclave_thread_slots(enclave_id_t enclave_id) {
  phys_ptr<enclave_info_t> enclave_info{enclave_id};
  return phys_ptr<thread_slot_t>{uintptr_t{enclave_info + 1}};
}
// Computes the physical address of a specific enclave thread slot.
inline phys_ptr<thread_slot_t> enclave_thread_slot(enclave_id_t enclave_id,
    thread_id_t thread_id) {
  return enclave_thread_slots(enclave_id) + thread_id;
}

// The amount of memory used by the security monitor for an enclave.
//
// The monitor data consists of an enclave_info_t and a thread_slot_t array. It
// is stored at the beginning of an enclave's main DRAM region, and cannot be
// modified by the enclave.
inline size_t enclave_monitor_area_size(size_t max_threads) {
  return static_cast<size_t>(uintptr_t{enclave_thread_slots(0) + max_threads});
}

inline size_t enclave_monitor_area_pages(size_t max_threads) {
  return (enclave_monitor_area_size(max_threads) + page_size() - 1)
      >> page_shift();
}

};  // namespace sanctum::internal
};  // namespace sanctum
#endif  // !defined(MONITOR_ENCLAVE_INL_H_INCLUDED)
