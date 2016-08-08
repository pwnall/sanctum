#include "metadata_inl.h"

#include "bare/bit_masking.h"

#include "gtest/gtest.h"

using sanctum::api::enclave_id_t;
using sanctum::bare::is_power_of_two;
using sanctum::bare::page_size;
using sanctum::internal::empty_metadata_page_info;
using sanctum::internal::empty_metadata_page_type;
using sanctum::internal::enclave_metadata_page_type;
using sanctum::internal::inner_metadata_page_type;
using sanctum::internal::metadata_page_info;
using sanctum::internal::metadata_page_info_t;
using sanctum::internal::metadata_page_type_mask;
using sanctum::internal::thread_metadata_page_type;

TEST(MetadataPageInfo, SizeAndMasks) {
  static_assert(is_power_of_two(sizeof(metadata_page_info_t)),
      "metadata_page_info_t's size must be a power of two");
  static_assert(sizeof(metadata_page_info_t) >= sizeof(enclave_id_t),
      "metadata_page_info_t must be able to hold an enclave_id_t");

  static_assert(metadata_page_type_mask < (page_size() - 1),
      "metadata_page_type_mask must fit into untranslated address bits");
  static_assert((empty_metadata_page_type & metadata_page_type_mask)
      == empty_metadata_page_type,
      "metadata_page_type_mask must cover empty_metadata_page_type");
  static_assert((inner_metadata_page_type & metadata_page_type_mask)
      == inner_metadata_page_type,
      "metadata_page_type_mask must cover inner_metadata_page_type");
  static_assert((enclave_metadata_page_type & metadata_page_type_mask)
      == enclave_metadata_page_type,
      "metadata_page_type_mask must cover enclave_metadata_page_type");
  static_assert((thread_metadata_page_type & metadata_page_type_mask)
      == thread_metadata_page_type,
      "metadata_page_type_mask must cover thread_metadata_page_type");
}

TEST(MetadataPageInfo, MetadataPageInfo) {
  static_assert(metadata_page_info(static_cast<enclave_id_t>(0xAA0000),
      static_cast<metadata_page_info_t>(0x3)) ==
      static_cast<metadata_page_info_t>(0xAA0003),
      "incorrect metadata_page_info implementation");
}

TEST(MetadataPageInfo, EmptyMetadataPageInfo) {
  static_assert(empty_metadata_page_info ==
      static_cast<metadata_page_info_t>(0),
      "empty_metadata_page_info must be zero");
}
