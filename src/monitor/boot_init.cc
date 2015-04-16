#include "boot_init.h"

namespace sanctum {
namespace internal {

size_t g_monitor_top;

void boot_panic() {
  // TODO: come up with a better way to signal panic
  while(true) { }
}

};  // namespace sanctum::internal
};  // namespace sanctum
