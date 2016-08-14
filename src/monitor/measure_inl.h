#if !defined(MONITOR_MEASURE_INL_H_INCLUDED)
#define MONITOR_MEASURE_INL_H_INCLUDED

#include "bare/memory.h"
#include "bare/page_tables.h"
#include "crypto/hash.h"
#include "enclave.h"
#include "public/api.h"

namespace sanctum {
namespace internal {

using sanctum::api::thread_id_t;
using sanctum::bare::page_size;
using sanctum::crypto::extend_hash;
using sanctum::crypto::finalize_hash;
using sanctum::crypto::hash_block_size;
using sanctum::crypto::hash_state_t;
using sanctum::crypto::init_hash;

// The layout of a hash block used to measure an enclave operation.
//
// The structure may not overlap the entire hash block, so init_enclave_hash()
// zeroes the entire hash block. Every other function that uses the structure
// is responsible for zeroing out the arguments that it sets after it's done
// extending the enclave hash. The opcode field does not need to be zeroed out,
// because every operation needs to set it.
struct measurement_block_t {
  size_t opcode;
  uintptr_t ptr1, ptr2, ptr3, ptr4;
  size_t size1, size2;
};
static_assert(sizeof(measurement_block_t) <= hash_block_size,
    "measurement_block_t does not fit in a hash block");

// Opcodes for enclave operations.
constexpr size_t enclave_init_opcode = 0xAAAAAAAA;
constexpr size_t load_enclave_page_table_opcode = 0xBBBBBBBB;
constexpr size_t load_enclave_page_opcode = 0xCCCCCCCC;
constexpr size_t load_enclave_thread_opcode = 0xDDDDDDDD;
constexpr size_t finalize_enclave_opcode = 0xEEEEEEEE;

// Computes the address of an enclave's buffer for measurement hashing.
//
// The buffer is used to put together
inline phys_ptr<measurement_block_t> enclave_measurement_block(
    phys_ptr<enclave_info_t> enclave_info) {
  phys_ptr<uint32_t> block_ptr = enclave_info->*(&enclave_info_t::hash_block);
  return phys_ptr<measurement_block_t>{uintptr_t(block_ptr)};
}

// Initializes an enclave's measurement hash.
//
// The caller must hold the lock of the enclave's main DRAM region.
inline void init_enclave_hash(phys_ptr<enclave_info_t> enclave_info,
    uintptr_t ev_base, uintptr_t ev_mask, size_t mailbox_count, bool debug) {
  // NOTE: 32-bit operations may be slow on 64-bit architectures, so we convert
  //       the pointer to an architecture-native type before instantiating the
  //       bzero template
  phys_ptr<size_t> fast_hash_block = phys_ptr<size_t>{uintptr_t(
      enclave_info->*(&enclave_info_t::hash_block))};
  bzero(fast_hash_block, hash_block_size);
  init_hash(&(enclave_info->*(&enclave_info_t::hash)));

  phys_ptr<measurement_block_t> block =
      enclave_measurement_block(enclave_info);
  block->*(&measurement_block_t::opcode) = enclave_init_opcode;
  block->*(&measurement_block_t::ptr1) = ev_base;
  block->*(&measurement_block_t::ptr2) = ev_mask;
  block->*(&measurement_block_t::size1) = mailbox_count;
  block->*(&measurement_block_t::size2) = debug;

  extend_hash(&(enclave_info->*(&enclave_info_t::hash)),
      enclave_info->*(&enclave_info_t::hash_block));
  block->*(&measurement_block_t::ptr1) = 0;
  block->*(&measurement_block_t::ptr2) = 0;
  block->*(&measurement_block_t::size1) = 0;
  block->*(&measurement_block_t::size2) = 0;
}

// Adds a page table creation operation to an enclave's measurement hash.
//
// The caller must hold the lock of the encalve's main DRAM region.
inline void extend_enclave_hash_with_page_table(
    phys_ptr<enclave_info_t> enclave_info, uintptr_t virtual_addr,
    size_t level, uintptr_t acl) {
  phys_ptr<measurement_block_t> block =
      enclave_measurement_block(enclave_info);
  block->*(&measurement_block_t::opcode) = load_enclave_page_table_opcode;
  block->*(&measurement_block_t::ptr1) = virtual_addr;
  block->*(&measurement_block_t::ptr2) = acl;
  block->*(&measurement_block_t::size1) = level;

  extend_hash(&(enclave_info->*(&enclave_info_t::hash)),
      enclave_info->*(&enclave_info_t::hash_block));
  block->*(&measurement_block_t::ptr1) = 0;
  block->*(&measurement_block_t::ptr2) = 0;
  block->*(&measurement_block_t::size1) = 0;
}

// Adds a page creation operation to an enclave's measurement hash.
//
// The caller must hold the lock of the encalve's main DRAM region.
//
// `phys_addr` is not included in the measurement. It's used to read in the
// page and hash its contents.
inline void extend_enclave_hash_with_page(
    phys_ptr<enclave_info_t> enclave_info, uintptr_t virtual_addr,
    uintptr_t acl, uintptr_t phys_addr) {
  phys_ptr<measurement_block_t> block =
      enclave_measurement_block(enclave_info);
  block->*(&measurement_block_t::opcode) = load_enclave_page_opcode;
  block->*(&measurement_block_t::ptr1) = virtual_addr;
  block->*(&measurement_block_t::ptr2) = acl;

  extend_hash(&(enclave_info->*(&enclave_info_t::hash)),
      enclave_info->*(&enclave_info_t::hash_block));
  block->*(&measurement_block_t::ptr1) = 0;
  block->*(&measurement_block_t::ptr2) = 0;

  phys_ptr<uint32_t> page_end{phys_addr + page_size()};
  for(phys_ptr<uint32_t> page_ptr{phys_addr}; page_ptr != page_end;
      page_ptr += hash_block_size / sizeof(uint32_t)) {
    extend_hash(&(enclave_info->*(&enclave_info_t::hash)), page_ptr);
  }
}

// Adds a thread creation operation to an enclave's measurement hash.
//
// The caller must hold the lock of the encalve's main DRAM region.
inline void extend_enclave_hash_with_thread(
    phys_ptr<enclave_info_t> enclave_info, uintptr_t entry_pc,
    uintptr_t entry_stack, uintptr_t fault_pc, uintptr_t fault_stack) {
  phys_ptr<measurement_block_t> block =
      enclave_measurement_block(enclave_info);
  block->*(&measurement_block_t::opcode) = load_enclave_thread_opcode;
  block->*(&measurement_block_t::ptr1) = entry_pc;
  block->*(&measurement_block_t::ptr2) = entry_stack;
  block->*(&measurement_block_t::ptr3) = fault_pc;
  block->*(&measurement_block_t::ptr4) = fault_stack;

  extend_hash(&(enclave_info->*(&enclave_info_t::hash)),
      enclave_info->*(&enclave_info_t::hash_block));
  block->*(&measurement_block_t::ptr1) = 0;
  block->*(&measurement_block_t::ptr2) = 0;
  block->*(&measurement_block_t::ptr3) = 0;
  block->*(&measurement_block_t::ptr4) = 0;
}

// Finalizes the enclave's measurement hash.
inline void finalize_enclave_hash(phys_ptr<enclave_info_t> enclave_info) {
  phys_ptr<measurement_block_t> block =
      enclave_measurement_block(enclave_info);
  block->*(&measurement_block_t::opcode) = finalize_enclave_opcode;

  extend_hash(&(enclave_info->*(&enclave_info_t::hash)),
      enclave_info->*(&enclave_info_t::hash_block));
  finalize_hash(&(enclave_info->*(&enclave_info_t::hash)));
}

};  // namespace sanctum::internal
};  // namespace sanctum
#endif  // !defined(MONITOR_MEASURE_INL_H_INCLUDED)
