// Architecture-specific types.
// These are normally defined in the standard library.

// C11 types.
typedef unsigned  uintptr_t;
typedef unsigned  size_t;

// C11 atomics.
typedef unsigned  atomic_flag;
typedef unsigned  atomic_size_t;
typedef unsigned  atomic_uintptr_t;
inline void atomic_flag_clear(volatile atomic_flag* flag) {
  // TODO: asm intrinsic
}
inline bool atomic_flag_test_and_set(volatile atomic_flag* flag) {
  // TODO: asm intrinsic
  return false;
}
inline void phys_atomic_flag_clear(uintptr_t flag_addr) {
  // TODO: asm intrinsic
}
inline bool phys_atomic_flag_test_and_set(uintptr_t flag_addr) {
  // TODO: asm intrinsic
  return false;
}
inline size_t atomic_load(atomic_size_t* obj) {
  // TODO: asm intrinsic
  return 0;
}
inline void atomic_store(volatile atomic_size_t* obj, size_t desr) {
  // TODO: asm intrinsic
}
inline size_t atomic_fetch_add(volatile atomic_size_t* obj, size_t arg) {
  // TODO: asm intrinsic
  return 0;
}
inline uintptr_t atomic_load(atomic_uintptr_t* obj) {
  // TODO: asm intrinsic
  return 0;
}
inline void atomic_store(volatile atomic_uintptr_t* obj, uintptr_t desr) {
  // TODO: asm intrinsic
}
// TODO: correct falue for ATOMIC_FLAG_INIT
#define ATOMIC_FLAG_INIT  0




// Architecture-specific constants.
//
// These values stay the same for all systems of a specific architecture, so
// they can be fixed at compilation time.

// Number of bits in an address that don't undergo address translation.
constexpr size_t page_shift = 12;

// Page size in bytes.
constexpr size_t page_size = (1 << page_shift);
// Page size in bits.
constexpr size_t bits_in_a_page = page_size * 8;
// Number of page table levels.
//
// We number levels from 0, which is assigned to the page table leaves.
//
// For example, in x86_64, the levels are 0 (PT), 1 (PD), 2 (PDPT), 3 (PML4),
// and page_table_levels is 4.
constexpr size_t page_table_levels = 4;

// The number of address bits translated by a page table level.
inline uintptr_t page_table_shift(size_t level) {
  return 9;
}
// The size of a page table at a given level, in pages.
inline uintptr_t page_table_pages(size_t level) {
  return 1;
}
// The number of page table entries at a given level.
inline uintptr_t page_table_entries(size_t level) {
  return 1 << page_table_shift(level);
}
// The size of a page table at a given level, in bytes.
inline uintptr_t page_table_size(size_t level) {
  return page_table_pages(level) << page_shift;
}
// The size of a page table entry, at a given level, in bytes.
inline uintptr_t page_table_entry_size(size_t level) {
  return sizeof(void *);
}

// The physical address of the next level table in a page table entry.
//
// `entry_addr` must be a valid physical address, otherwise invalid memory
// accesses will be issued.
inline uintptr_t page_table_next_addr(uintptr_t entry_addr, size_t level) {
  return load_phys_ptr(entry_addr) & (~static_cast<uintptr_t>(page_size));
}

// The physical address in a last level page table entry.
//
// `entry_addr` must be a valid physical address, otherwise invalid memory
// accesses will be issued.
inline uintptr_t page_table_phys_addr(uintptr_t entry_addr, size_t level) {
  return load_phys_ptr(entry_addr) & (~static_cast<uintptr_t>(page_size));
}

// Load physical (LDPHYS) wrapper that reads a size_t.
//
// This can only be issued by the security monitor. Invalid addresses result in
// invalid memory accesses.
inline size_t load_phys_size(uintptr_t addr) {
  // TODO: asm intrinsic
  return static_cast<size_t>(0);
}
// Store physical (STPHYS) wrapper that writes a size_t.
//
// This can only be issued by the security monitor. Invalid addresses will
// trash DRAM.
inline void store_phys_size(uintptr_t addr, size_t val) {
  // TODO: asm intrinsic
}
// Load physical (LDPHYS) wrapper that reads a uintptr_t.
//
// This can only be issued by the security monitor. Invalid addresses result in
// invalid memory accesses.
inline uintptr_t load_phys_ptr(uintptr_t addr) {
  // TODO: asm intrinsic
  return static_cast<uintptr_t>(0);
}
// Store physical (STPHYS) wrapper that writes a uintptr_t.
//
// This can only be issued by the security monitor. Invalid addresses will
// trash DRAM.
inline void store_phys_ptr(uintptr_t addr, uintptr_t val) {
  // TODO: asm intrinsic
}

// Sets up a wired page table entry.
//
// `entry_addr` and `dest_addr` must be valid physical addresses, otherwise
// invalid memory accesses will be issued.
//
// The access bits provided only allow write access from the security monitor.
// For x86, good defaults are S=0 (the enclave can read the wired pages), W=0
// (the enclave can't write the wired pages), NX=1 (the wired pages aren't
// executable).
inline void set_wired_page_table_entry(uintptr_t entry_addr, size_t level,
                                       uintptr_t dest_addr) {
  store_phys_ptr(entry_addr, dest_addr | 3);  // present, read-only
}

// Sets the EPTRR (enclave page table root register).
//
// This can only be issued by the security monitor. An invalid DRAM address
// will lock up or reboot the machine.
void set_eptr_register(uintptr_t value) {
  // TODO: asm intrinsic
}


inline size_t current_core() {
  // TODO: asm intrinsic
  return static_cast<size_t>(0);
}



// Execution state saved by an asynchronous enclave exit.
typedef struct {
  unsigned gprs[32];
} enclave_exit_state_t;


