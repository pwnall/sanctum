#if !defined(MONITOR_BOOT_INIT_H_INCLUDED)
#define MONITOR_BOOT_INIT_H_INCLUDED

#include "bare/base_types.h"

namespace sanctum {
namespace internal {

using sanctum::bare::uintptr_t;

// The top of the memory used by the security monitor.
//
// This is used to track memory allocation during boot. It remains constant
// after the boot initialization.
extern uintptr_t g_monitor_top;

// Computes the initial top of monitor memory, based on the monitor's header.
void boot_init_monitor_top();

// Sets up DRAM regions.
//
// This computes the constants related to DRAM regions and sets the cache shift
// index.
void boot_init_dram_regions();

// Computes the constants related to enclave metadata DRAM regions.
//
// This must be called after the DRAM region constants are set.
void boot_init_metadata();

// Allocates the monitor arrays whose sizes depend on system parameters.
//
// This must be called after the top of monitor memory and the DRAM region
// constants are set.
void boot_init_dynamic_arrays();

// Sets up the hardware registers needed to protect the monitor from the OS.
//
// This must be called after all the monitor's memory is allocated.
void boot_init_protection();

// Stops the monitor's boot process.
//
// This is called when the monitor concludes that the platform violates a core
// assumption built into the monitor.
void boot_panic();

};  // namespace sanctum::internal
};  // namespace sanctum
#endif  // !defined(MONITOR_BOOT_INIT_H_INCLUDED)
