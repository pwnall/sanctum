#if !defined(BARE_CPU_CONTEXT_H_INCLUDED)
#define BARE_CPU_CONTEXT_H_INCLUDED

#include "base_types.h"

namespace sanctum {
namespace bare {

// Obtains the number of cores installed in the system.
size_t total_cores();

// Cores are numbered starting from 0.
size_t current_core();

// The execution state saved by an asynchronous enclave exit.
struct enclave_exit_state_t;

};  // namespace sanctum::bare
};  // namespace sanctum

// Per-architecture definitions and operations.
#include "cpu_context_arch.h"

#endif  // !defined(BARE_CPU_CONTEXT_H_INCLUDED)
