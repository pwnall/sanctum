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


