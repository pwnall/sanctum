// This file is here to ensure that the public/api.h header compiles without
// any dependencies.

#include "bare/base_types.h"

using size_t = sanctum::bare::size_t;
using uintptr_t = sanctum::bare::uintptr_t;
using uint8_t = sanctum::bare::uint8_t;

#include "public/api.h"
