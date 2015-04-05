#include "../../phys_ptr.h"

using namespace sanctum::bare;

namespace sanctum {
namespace testing {

char* phys_buffer = nullptr;
size_t phys_buffer_size = 0;

void init_phys_buffer(size_t buffer_size) {
  phys_buffer = new char[buffer_size];
  phys_buffer_size = buffer_size;
}

};  // namespace sanctum::testing
};  // namespace sanctum
