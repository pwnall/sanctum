#include "dram_regions.h"

#include "cpu_core_inl.h"
#include "dram_regions_inl.h"

using sanctum::api::os::dram_region_blocked;
using sanctum::api::os::dram_region_owned;
using sanctum::bare::atomic_flag_test_and_set;
using sanctum::bare::phys_ptr;
using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;
using sanctum::internal::clear_dram_region_lock;
using sanctum::internal::current_enclave;
using sanctum::internal::dram_region_info_t;
using sanctum::internal::dram_region_tlb_flush;
using sanctum::internal::dram_regions_info_t;
using sanctum::internal::g_core_count;
using sanctum::internal::g_core;
using sanctum::internal::g_dram_regions;
using sanctum::internal::g_dram_region;
using sanctum::internal::g_dram_region_count;
using sanctum::internal::g_dram_region_mask;
using sanctum::internal::g_dram_region_shift;
using sanctum::internal::g_dram_size;
using sanctum::internal::is_dynamic_dram_region;
using sanctum::internal::is_valid_dram_region;
using sanctum::internal::is_valid_enclave_id;
using sanctum::internal::test_and_set_dram_region_lock;
using sanctum::internal::core_info_t;

namespace sanctum {
namespace internal {

phys_ptr<dram_region_info_t> g_dram_region{0};
phys_ptr<dram_regions_info_t> g_dram_regions{0};

size_t g_dram_size;
size_t g_dram_region_count;
size_t g_dram_region_mask;
size_t g_dram_region_shift;

};  // namespace sanctum::internal
};  // namespace sanctum

namespace sanctum {
namespace api {

size_t dram_size() {
  return g_dram_size;
}
size_t dram_region_mask() {
  return g_dram_region_mask;
}
api_result_t block_dram_region(size_t dram_region) {
  if (!is_dynamic_dram_region(dram_region))
    return monitor_invalid_value;
  if (test_and_set_dram_region_lock(dram_region))
    return monitor_concurrent_call;

  api_result_t result;
  phys_ptr<dram_region_info_t> region = &g_dram_region[dram_region];
  if (region->*(&dram_region_info_t::state) == dram_region_owned) {
    enclave_id_t owner = atomic_load(&(region->*(&dram_region_info_t::owner)));
    if (owner == 0 || region->*(&dram_region_info_t::pinned_pages) == 0) {
      if (owner == current_enclave()) {
        region->*(&dram_region_info_t::state) = dram_region_blocked;
        region->*(&dram_region_info_t::previous_owner) = owner;
        atomic_store(&(region->*(&dram_region_info_t::owner)),
            static_cast<enclave_id_t>(0));
        size_t block_clock = atomic_fetch_add(
            &(g_dram_regions->*(&dram_regions_info_t::block_clock)),
            static_cast<enclave_id_t>(1)) + 1;

        // TODO: panic if block_clock is max_size_t

        region->*(&dram_region_info_t::blocked_at) = block_clock;
        result = monitor_ok;
      } else {
        result = monitor_access_denied;
      }
    } else {
      result = monitor_invalid_state;
    }
  } else {
    result = monitor_invalid_state;
  }

  clear_dram_region_lock(dram_region);
  return result;
}

namespace enclave { // sanctum::api::enclave

api_result_t dram_region_check_ownership(size_t dram_region) {
  if (!is_dynamic_dram_region(dram_region))
    return monitor_invalid_value;
  if (test_and_set_dram_region_lock(dram_region))
    return monitor_concurrent_call;

  api_result_t result;
  phys_ptr<dram_region_info_t> region = &g_dram_region[dram_region];
  if (region->*(&dram_region_info_t::state) == dram_region_owned) {
    enclave_id_t owner = atomic_load(&(region->*(&dram_region_info_t::owner)));
    if (owner == current_enclave()) {
      result = monitor_ok;
    } else {
      result = monitor_invalid_state;
    }
  } else {
    result = monitor_invalid_state;
  }

  clear_dram_region_lock(dram_region);
  return result;
}

};  // namespace sanctum::api::enclave

namespace os {  // sanctum::api::os

api_result_t assign_dram_region(size_t dram_region, enclave_id_t new_owner) {
  // NOTE: non-dynamic DRAM regions will never be freed, so we don't need to
  //       explicitly check for them here
  if (!is_valid_dram_region(dram_region))
    return monitor_invalid_value;
  if (test_and_set_dram_region_lock(dram_region))
    return monitor_concurrent_call;

  api_result_t result;
  phys_ptr<dram_region_info_t> region = &g_dram_region[dram_region];
  if (region->*(&dram_region_info_t::state) == dram_region_free) {
    // NOTE: null_enclave_id is accepted here and assigns the DRAM region to
    //       the OS
    if (is_valid_enclave_id(new_owner)) {
      region->*(&dram_region_info_t::state) = dram_region_owned;
      atomic_store(&(region->*(&dram_region_info_t::owner)), new_owner);
      result = monitor_ok;
    } else {
      result = monitor_invalid_value;
    }
  } else {
    result = monitor_invalid_state;
  }

  clear_dram_region_lock(dram_region);
  return result;
}
api_result_t free_dram_region(size_t dram_region) {
  // NOTE: non-dynamic DRAM regions will never be blocked, so we don't need to
  //       explicitly check for them here
  if (!is_valid_dram_region(dram_region))
    return monitor_invalid_value;
  if (test_and_set_dram_region_lock(dram_region))
    return monitor_concurrent_call;

  api_result_t result;
  phys_ptr<dram_region_info_t> region = &g_dram_region[dram_region];
  if (region->*(&dram_region_info_t::state) == dram_region_blocked) {
    size_t blocked_at = region->*(&dram_region_info_t::blocked_at);

    bool can_free = true;
    // Mappings for OS-owned regions must be TLB-flushed from all cores.
    //
    // Mappings for enclave-owned regions must be TLB-flushed from cores that
    // execute enclave code. However, every enclave exit causes a TLB flush and
    // updates the core's clock.
    for (size_t i = 0; i < g_core_count; ++i) {
      phys_ptr<core_info_t> core = &g_core[i];
      if (atomic_load(&(core->*(&core_info_t::flushed_at))) < blocked_at) {
        can_free = false;
        break;
      }
    }
    if (can_free) {
      region->*(&dram_region_info_t::state) = dram_region_free;
      result = monitor_ok;
    } else {
      result = monitor_invalid_state;
    }
  } else {
    result = monitor_invalid_state;
  }

  clear_dram_region_lock(dram_region);
  return result;
}

api_result_t dram_region_flush() {
  dram_region_tlb_flush();
  return monitor_ok;
}


};  // namespace sanctum::api::os

};  // namespace sanctum::api
};  // namespace sanctum
