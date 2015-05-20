#if !defined(BARE_CPU_CONTEXT_H_INCLUDED)
#define BARE_CPU_CONTEXT_H_INCLUDED

#include "base_types.h"

namespace sanctum {
namespace bare {

// Obtains the number of cores installed in the system.
//
// The implementation may be very slow, so the return value should be cached.
//
// The implementation may use privileged instructions.
size_t read_core_count();

// Cores are numbered starting from 0.
size_t current_core();

// Flush all TLBs on the current core.
//
// This does not flush any cache.
void flush_tlbs();

// Flush all the caches belonging to the current core.
//
// This does not flush TLBs, and does not flush the shared last-level cache.
void flush_private_caches();

// Sets the core's cache index shift.
//
// This must be set to identical values on all cores. Undefined behavior will
// occur otherwise.
//
// This can only be issued by the security monitor.
void set_cache_index_shift(size_t cache_index_shift);

// Sets the EPTBR (enclave page table base register).
//
// This can only be issued by the security monitor. An invalid DRAM address
// will lock up or reboot the machine.
void set_eptbr(uintptr_t value);

// Sets the PTBR (page table base register).
//
// This can only be issued by the security monitor. An invalid DRAM address
// will lock up or reboot the machine.
void set_ptbr(uintptr_t value);

// Sets the EPARBASE (enclave protected address range base) register.
//
// This can only be issued by the security monitor. An invalid DRAM address
// will lock up or reboot the machine.
void set_epar_base(uintptr_t value);

// Sets the PARBASE (protected address range base) register.
//
// This can only be issued by the security monitor. An invalid DRAM address
// will lock up or reboot the machine.
void set_par_base(uintptr_t value);

// Sets the EPARMASK (enclave protected address range mask) register.
//
// This can only be issued by the security monitor. An invalid DRAM address
// will lock up or reboot the machine.
void set_epar_mask(uintptr_t value);

// Sets the PARMASK (protected address range mask) register.
//
// This can only be issued by the security monitor. An invalid DRAM address
// will lock up or reboot the machine.
void set_par_mask(uintptr_t value);

// Sets the EPARPMASK (enclave protected address range permission mask) reg.
//
// This can only be issued by the security monitor. An invalid DRAM address
// will lock up or reboot the machine.
void set_epar_pmask(uintptr_t value);

// Sets the PARPMASK (protected address range permission mask) register.
//
// This can only be issued by the security monitor. An invalid DRAM address
// will lock up or reboot the machine.
void set_par_pmask(uintptr_t value);

// Sets the EVBASE (enclave virtual address base register).
//
// This can only be issued by the security monitor.
void set_ev_base(uintptr_t value);

// Sets the EVMASK (enclave virtual address mask register).
//
// This can only be issued by the security monitor.
void set_ev_mask(uintptr_t value);

// Loads the DRBMAP (DMA region bitmap) register from memory.
void set_drb_map(uintptr_t phys_addr);

// Loads the EDRBMAP (enclave DMA region bitmap) register from memory.
void set_edrb_map(uintptr_t phys_addr);

// The execution state saved by an asynchronous enclave exit.
struct enclave_exit_state_t;

};  // namespace sanctum::bare
};  // namespace sanctum

// Per-architecture definitions and operations.
#include "cpu_context_arch.h"

#endif  // !defined(BARE_CPU_CONTEXT_H_INCLUDED)
