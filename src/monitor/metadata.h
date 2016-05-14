#if !defined(MONITOR_METADATA_H_INCLUDED)
#define MONITOR_METADATA_H_INCLUDED

#include "bare/base_types.h"
#include "bare/phys_atomics.h"
#include "public/api.h"

namespace sanctum {
namespace internal {

using sanctum::bare::phys_ptr;
using sanctum::bare::uintptr_t;

// The page map entry for a metadata page is packed in a single pointer.
//
// This is possible because each enclave ID must be a multiple of a page, so
// the bottom bits are available.
typedef uintptr_t metadata_page_info_t;

// Mask that selects the metadata page type bits.
constexpr metadata_page_info_t metadata_page_type_mask = 3;

// Type for metadata pages that have been assigned to enclaves but not used.
//
// These pages are waiting to be turned into threat_info_t pages by the enclave
// software.
constexpr metadata_page_info_t empty_metadata_page_type = 0;

// Type for metadata pages that hold an enclave's enclave_info_t.
constexpr metadata_page_info_t enclave_info_metadata_page_type = 1;

// Type for metadata pages that hold a thread_info_t for an enclave.
constexpr metadata_page_info_t thread_info_metadata_page_type = 2;

// Bit that distinguishes between the first page in a structure and other
// pages.
constexpr metadata_page_info_t metadata_page_start_mask = 4;

// Total number of metadata pages in a DRAM region dedicated to metadata.
extern size_t g_metadata_region_pages;

// The first usable metadata page in a DRAM region dedicated to metadata.
//
// Unusable pages contain a map of all the pages, ensuring that metadata pages
// aren't double-allocated.
extern size_t g_metadata_region_start;

};  // namespace sanctum::internal
};  // namespace sanctum
#endif  // !defined(MONITOR_METADATA_H_INCLUDED)
