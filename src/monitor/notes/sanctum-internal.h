#include "monitor/public/api_os.h"
#include "monitor/public/api_enclave.h"


// The number of pages taken up by the enclave_info structure.
constexpr size_t enclave_info_pages =
    (sizeof(enclave_info) + page_size - 1) / page_size;

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


