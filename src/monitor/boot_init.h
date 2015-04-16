#if !defined(MONITOR_BOOT_INIT_H_INCLUDED)
#define MONITOR_BOOT_INIT_H_INCLUDED

#include "bare/base_types.h"

namespace sanctum {
namespace internal {

// The top of the memory used by the security monitor.
//
// This is used to track memory allocation during boot. It remains constant
// after the boot initialization.
extern size_t g_monitor_top;

// Stops the monitor's boot process.
//
// This is called when the monitor concludes that the platform violates a core
// assumption built into the monitor.
void boot_panic();

};  // namespace sanctum::internal
};  // namespace sanctum
#endif  // !defined(MONITOR_BOOT_INIT_H_INCLUDED)
