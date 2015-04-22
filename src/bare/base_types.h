#if !defined(BARE_BASE_TYPES_H_INCLUDED)
#define BARE_BASE_TYPES_H_INCLUDED

// This header defines size_t, uintptr_t, and uint32_t.
//
// These types are specified in the C11 standard.

// Per-architecture basic type definitions.
#include "base_types_arch.h"

static_assert(sizeof(uintptr_t) >= sizeof(void *), "uintptr_t too small");
static_assert(sizeof(uint32_t) == 4, "uint32_t is not exactly 4 bytes");

#endif  // !definded(BARE_BASE_TYPES_H_INCLUDED)
