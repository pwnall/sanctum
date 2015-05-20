#include "bare/cpu_context.h"

#include "bare/memory.h"

#include "gtest/gtest.h"

using sanctum::bare::current_core;
using sanctum::bare::flush_tlbs;
using sanctum::bare::flush_private_caches;
using sanctum::bare::read_core_count;
using sanctum::bare::phys_ptr;
using sanctum::bare::set_cache_index_shift;
using sanctum::bare::set_drb_map;
using sanctum::bare::set_edrb_map;
using sanctum::bare::set_epar_base;
using sanctum::bare::set_epar_mask;
using sanctum::bare::set_epar_pmask;
using sanctum::bare::set_eptbr;
using sanctum::bare::set_ev_base;
using sanctum::bare::set_ev_mask;
using sanctum::bare::set_par_base;
using sanctum::bare::set_par_mask;
using sanctum::bare::set_par_pmask;
using sanctum::bare::set_ptbr;
using sanctum::testing::phys_buffer;
using sanctum::testing::phys_buffer_size;
using sanctum::testing::set_current_core;
using sanctum::testing::set_core_count;
using sanctum::testing::set_dram_region_bitmap_words;

TEST(CpuContextTest, VirtualCurrentTotalCores) {
  set_core_count(8);
  ASSERT_EQ(8, read_core_count());
  ASSERT_EQ(0, current_core());
  ASSERT_EQ(8, sanctum::testing::core_count);
  ASSERT_EQ(0, sanctum::testing::current_core);

  set_current_core(3);
  ASSERT_EQ(3, current_core());
  ASSERT_EQ(8, read_core_count());
  ASSERT_EQ(3, sanctum::testing::current_core);
  ASSERT_EQ(8, sanctum::testing::core_count);

  set_core_count(4);
  ASSERT_EQ(4, read_core_count());
  ASSERT_EQ(0, current_core());
  ASSERT_EQ(4, sanctum::testing::core_count);
  ASSERT_EQ(0, sanctum::testing::current_core);
}

TEST(CpuContextTest, FlushTlbs) {
  set_core_count(8);
  sanctum::testing::core_tlb_flush_count[0] = 0;
  sanctum::testing::core_tlb_flush_count[3] = 0;

  flush_tlbs();
  ASSERT_EQ(1, sanctum::testing::core_tlb_flush_count[0]);
  ASSERT_EQ(0, sanctum::testing::core_tlb_flush_count[3]);

  set_current_core(3);
  flush_tlbs();
  ASSERT_EQ(1, sanctum::testing::core_tlb_flush_count[0]);
  ASSERT_EQ(1, sanctum::testing::core_tlb_flush_count[3]);
  flush_tlbs();
  ASSERT_EQ(1, sanctum::testing::core_tlb_flush_count[0]);
  ASSERT_EQ(2, sanctum::testing::core_tlb_flush_count[3]);

  set_current_core(0);
  flush_tlbs();
  flush_tlbs();
  ASSERT_EQ(3, sanctum::testing::core_tlb_flush_count[0]);
  ASSERT_EQ(2, sanctum::testing::core_tlb_flush_count[3]);

  sanctum::testing::core_tlb_flush_count[0] = 0;
  sanctum::testing::core_tlb_flush_count[3] = 0;
}

TEST(CpuContextTest, FlushPrivateCaches) {
  set_core_count(8);
  sanctum::testing::core_cache_flush_count[0] = 0;
  sanctum::testing::core_cache_flush_count[3] = 0;

  flush_private_caches();
  ASSERT_EQ(1, sanctum::testing::core_cache_flush_count[0]);
  ASSERT_EQ(0, sanctum::testing::core_cache_flush_count[3]);

  set_current_core(3);
  flush_private_caches();
  ASSERT_EQ(1, sanctum::testing::core_cache_flush_count[0]);
  ASSERT_EQ(1, sanctum::testing::core_cache_flush_count[3]);
  flush_private_caches();
  ASSERT_EQ(1, sanctum::testing::core_cache_flush_count[0]);
  ASSERT_EQ(2, sanctum::testing::core_cache_flush_count[3]);

  set_current_core(0);
  flush_private_caches();
  flush_private_caches();
  ASSERT_EQ(3, sanctum::testing::core_cache_flush_count[0]);
  ASSERT_EQ(2, sanctum::testing::core_cache_flush_count[3]);

  sanctum::testing::core_cache_flush_count[0] = 0;
  sanctum::testing::core_cache_flush_count[3] = 0;
}

TEST(CpuContextTest, VirtualCacheIndexShift) {
  set_core_count(8);
  sanctum::testing::core_cache_index_shift[0] = 0;
  sanctum::testing::core_cache_index_shift[3] = 0;

  set_cache_index_shift(11);
  ASSERT_EQ(11, sanctum::testing::core_cache_index_shift[0]);
  ASSERT_EQ(0, sanctum::testing::core_cache_index_shift[3]);

  set_current_core(3);
  set_cache_index_shift(10);
  ASSERT_EQ(11, sanctum::testing::core_cache_index_shift[0]);
  ASSERT_EQ(10, sanctum::testing::core_cache_index_shift[3]);

  set_current_core(0);
  set_cache_index_shift(10);
  ASSERT_EQ(10, sanctum::testing::core_cache_index_shift[0]);
  ASSERT_EQ(10, sanctum::testing::core_cache_index_shift[3]);
}

TEST(CpuContextTest, VirtualPtbr) {
  uintptr_t value = 0xcafe;

  set_core_count(8);

  set_ptbr(value);
  ASSERT_EQ(value, sanctum::testing::core_ptbr[0]);
  ASSERT_EQ(0, sanctum::testing::core_ptbr[3]);
  set_ptbr(0);
  ASSERT_EQ(0, sanctum::testing::core_ptbr[0]);
  set_current_core(3);
  set_ptbr(value);
  ASSERT_EQ(0, sanctum::testing::core_ptbr[0]);
  ASSERT_EQ(value, sanctum::testing::core_ptbr[3]);
  set_ptbr(0);
  ASSERT_EQ(0, sanctum::testing::core_ptbr[3]);
  set_current_core(0);
}

TEST(CpuContextTest, VirtualEptbr) {
  uintptr_t value = 0xcafe;

  set_core_count(8);

  set_eptbr(value);
  ASSERT_EQ(value, sanctum::testing::core_eptbr[0]);
  ASSERT_EQ(0, sanctum::testing::core_eptbr[3]);
  set_eptbr(0);
  ASSERT_EQ(0, sanctum::testing::core_eptbr[0]);
  set_current_core(3);
  set_eptbr(value);
  ASSERT_EQ(0, sanctum::testing::core_eptbr[0]);
  ASSERT_EQ(value, sanctum::testing::core_eptbr[3]);
  set_eptbr(0);
  ASSERT_EQ(0, sanctum::testing::core_eptbr[3]);
  set_current_core(0);
}

TEST(CpuContextTest, VirtualParBase) {
  uintptr_t value = 0x21000000;

  set_core_count(8);

  set_par_base(value);
  ASSERT_EQ(value, sanctum::testing::core_par_base[0]);
  ASSERT_EQ(0, sanctum::testing::core_par_base[3]);
  set_par_base(0);
  ASSERT_EQ(0, sanctum::testing::core_par_base[0]);
  set_current_core(3);
  set_par_base(value);
  ASSERT_EQ(0, sanctum::testing::core_par_base[0]);
  ASSERT_EQ(value, sanctum::testing::core_par_base[3]);
  set_par_base(0);
  ASSERT_EQ(0, sanctum::testing::core_par_base[3]);
  set_current_core(0);
}

TEST(CpuContextTest, VirtualEparBase) {
  uintptr_t value = 0x21000000;

  set_core_count(8);

  set_epar_base(value);
  ASSERT_EQ(value, sanctum::testing::core_epar_base[0]);
  ASSERT_EQ(0, sanctum::testing::core_epar_base[3]);
  set_epar_base(0);
  ASSERT_EQ(0, sanctum::testing::core_epar_base[0]);
  set_current_core(3);
  set_epar_base(value);
  ASSERT_EQ(0, sanctum::testing::core_epar_base[0]);
  ASSERT_EQ(value, sanctum::testing::core_epar_base[3]);
  set_epar_base(0);
  ASSERT_EQ(0, sanctum::testing::core_epar_base[3]);
  set_current_core(0);
}

TEST(CpuContextTest, VirtualParMask) {
  uintptr_t value = 0x3fff;

  set_core_count(8);

  set_par_mask(value);
  ASSERT_EQ(value, sanctum::testing::core_par_mask[0]);
  ASSERT_EQ(0, sanctum::testing::core_par_mask[3]);
  set_par_mask(0);
  ASSERT_EQ(0, sanctum::testing::core_par_mask[0]);
  set_current_core(3);
  set_par_mask(value);
  ASSERT_EQ(0, sanctum::testing::core_par_mask[0]);
  ASSERT_EQ(value, sanctum::testing::core_par_mask[3]);
  set_par_mask(0);
  ASSERT_EQ(0, sanctum::testing::core_par_mask[3]);
  set_current_core(0);
}

TEST(CpuContextTest, VirtualEparMask) {
  uintptr_t value = 0x3fff;

  set_core_count(8);

  set_epar_mask(value);
  ASSERT_EQ(value, sanctum::testing::core_epar_mask[0]);
  ASSERT_EQ(0, sanctum::testing::core_epar_mask[3]);
  set_epar_mask(0);
  ASSERT_EQ(0, sanctum::testing::core_epar_mask[0]);
  set_current_core(3);
  set_epar_mask(value);
  ASSERT_EQ(0, sanctum::testing::core_epar_mask[0]);
  ASSERT_EQ(value, sanctum::testing::core_epar_mask[3]);
  set_epar_mask(0);
  ASSERT_EQ(0, sanctum::testing::core_epar_mask[3]);
  set_current_core(0);
}

TEST(CpuContextTest, VirtualParPmask) {
  uintptr_t value = 0xfffffe00;

  set_core_count(8);

  set_par_pmask(value);
  ASSERT_EQ(value, sanctum::testing::core_par_pmask[0]);
  ASSERT_EQ(0, sanctum::testing::core_par_pmask[3]);
  set_par_pmask(0);
  ASSERT_EQ(0, sanctum::testing::core_par_pmask[0]);
  set_current_core(3);
  set_par_pmask(value);
  ASSERT_EQ(0, sanctum::testing::core_par_pmask[0]);
  ASSERT_EQ(value, sanctum::testing::core_par_pmask[3]);
  set_par_pmask(0);
  ASSERT_EQ(0, sanctum::testing::core_par_pmask[3]);
  set_current_core(0);
}

TEST(CpuContextTest, VirtualEparPmask) {
  uintptr_t value = 0xfffffe00;

  set_core_count(8);

  set_epar_pmask(value);
  ASSERT_EQ(value, sanctum::testing::core_epar_pmask[0]);
  ASSERT_EQ(0, sanctum::testing::core_epar_pmask[3]);
  set_epar_pmask(0);
  ASSERT_EQ(0, sanctum::testing::core_epar_pmask[0]);
  set_current_core(3);
  set_epar_pmask(value);
  ASSERT_EQ(0, sanctum::testing::core_epar_pmask[0]);
  ASSERT_EQ(value, sanctum::testing::core_epar_pmask[3]);
  set_epar_pmask(0);
  ASSERT_EQ(0, sanctum::testing::core_epar_pmask[3]);
  set_current_core(0);
}

TEST(CpuContextTest, VirtualEvBase) {
  uintptr_t value = 0x20000000;

  set_core_count(8);

  set_ev_base(value);
  ASSERT_EQ(value, sanctum::testing::core_ev_base[0]);
  ASSERT_EQ(0, sanctum::testing::core_ev_base[3]);
  set_ev_base(0);
  ASSERT_EQ(0, sanctum::testing::core_ev_base[0]);
  set_current_core(3);
  set_ev_base(value);
  ASSERT_EQ(0, sanctum::testing::core_ev_base[0]);
  ASSERT_EQ(value, sanctum::testing::core_ev_base[3]);
  set_ev_base(0);
  ASSERT_EQ(0, sanctum::testing::core_ev_base[3]);
  set_current_core(0);
}

TEST(CpuContextTest, VirtualEvMask) {
  uintptr_t value = 0x1fffffff;

  set_core_count(8);

  set_ev_mask(value);
  ASSERT_EQ(value, sanctum::testing::core_ev_mask[0]);
  ASSERT_EQ(0, sanctum::testing::core_ev_mask[3]);
  set_ev_mask(0);
  ASSERT_EQ(0, sanctum::testing::core_ev_mask[0]);
  set_current_core(3);
  set_ev_mask(value);
  ASSERT_EQ(0, sanctum::testing::core_ev_mask[0]);
  ASSERT_EQ(value, sanctum::testing::core_ev_mask[3]);
  set_ev_mask(0);
  ASSERT_EQ(0, sanctum::testing::core_ev_mask[3]);
  set_current_core(0);
}

TEST(CpuContextTest, VirtualDrbMap) {
  ASSERT_LE(256 * sizeof(size_t), phys_buffer_size);
  for (size_t i = 0; i < 128; ++i)
    *(phys_ptr<size_t>{0} + i) = i;
  for (size_t i = 0; i < 128; ++i)
    *(phys_ptr<size_t>{0} + 128 + i) = 0;

  set_core_count(8);

  set_dram_region_bitmap_words(2);
  set_drb_map(42 * sizeof(size_t));
  ASSERT_EQ(42, sanctum::testing::core_drb_map[0][0]);
  ASSERT_EQ(43, sanctum::testing::core_drb_map[0][1]);
  ASSERT_EQ(0, sanctum::testing::core_drb_map[0][2]);
  ASSERT_EQ(0, sanctum::testing::core_drb_map[0][3]);

  set_dram_region_bitmap_words(3);
  set_drb_map(64 * sizeof(size_t));
  ASSERT_EQ(64, sanctum::testing::core_drb_map[0][0]);
  ASSERT_EQ(65, sanctum::testing::core_drb_map[0][1]);
  ASSERT_EQ(66, sanctum::testing::core_drb_map[0][2]);
  ASSERT_EQ(0, sanctum::testing::core_drb_map[0][3]);

  ASSERT_EQ(0, sanctum::testing::core_drb_map[3][0]);
  ASSERT_EQ(0, sanctum::testing::core_drb_map[3][1]);
  ASSERT_EQ(0, sanctum::testing::core_drb_map[3][2]);
  ASSERT_EQ(0, sanctum::testing::core_drb_map[3][3]);

  set_drb_map(128 * sizeof(size_t));
  ASSERT_EQ(0, sanctum::testing::core_drb_map[0][0]);
  ASSERT_EQ(0, sanctum::testing::core_drb_map[0][1]);
  ASSERT_EQ(0, sanctum::testing::core_drb_map[0][2]);
  ASSERT_EQ(0, sanctum::testing::core_drb_map[0][3]);

  set_current_core(3);
  set_drb_map(29 * sizeof(size_t));
  ASSERT_EQ(29, sanctum::testing::core_drb_map[3][0]);
  ASSERT_EQ(30, sanctum::testing::core_drb_map[3][1]);
  ASSERT_EQ(31, sanctum::testing::core_drb_map[3][2]);
  ASSERT_EQ(0, sanctum::testing::core_drb_map[3][3]);

  ASSERT_EQ(0, sanctum::testing::core_drb_map[0][0]);
  ASSERT_EQ(0, sanctum::testing::core_drb_map[0][1]);
  ASSERT_EQ(0, sanctum::testing::core_drb_map[0][2]);
  ASSERT_EQ(0, sanctum::testing::core_drb_map[0][3]);

  set_drb_map(128 * sizeof(size_t));
  ASSERT_EQ(0, sanctum::testing::core_drb_map[3][0]);
  ASSERT_EQ(0, sanctum::testing::core_drb_map[3][1]);
  ASSERT_EQ(0, sanctum::testing::core_drb_map[3][2]);
  ASSERT_EQ(0, sanctum::testing::core_drb_map[3][3]);

  set_current_core(0);

  memset(phys_buffer, 0, 256 * sizeof(size_t));
}

TEST(CpuContextTest, VirtualEdrbMap) {
  ASSERT_LE(256 * sizeof(size_t), phys_buffer_size);
  for (size_t i = 0; i < 128; ++i)
    *(phys_ptr<size_t>{0} + i) = i;
  for (size_t i = 0; i < 128; ++i)
    *(phys_ptr<size_t>{0} + 128 + i) = 0;

  set_core_count(8);

  set_dram_region_bitmap_words(2);
  set_edrb_map(42 * sizeof(size_t));
  ASSERT_EQ(42, sanctum::testing::core_edrb_map[0][0]);
  ASSERT_EQ(43, sanctum::testing::core_edrb_map[0][1]);
  ASSERT_EQ(0, sanctum::testing::core_edrb_map[0][2]);
  ASSERT_EQ(0, sanctum::testing::core_edrb_map[0][3]);

  set_dram_region_bitmap_words(3);
  set_edrb_map(64 * sizeof(size_t));
  ASSERT_EQ(64, sanctum::testing::core_edrb_map[0][0]);
  ASSERT_EQ(65, sanctum::testing::core_edrb_map[0][1]);
  ASSERT_EQ(66, sanctum::testing::core_edrb_map[0][2]);
  ASSERT_EQ(0, sanctum::testing::core_edrb_map[0][3]);

  ASSERT_EQ(0, sanctum::testing::core_edrb_map[3][0]);
  ASSERT_EQ(0, sanctum::testing::core_edrb_map[3][1]);
  ASSERT_EQ(0, sanctum::testing::core_edrb_map[3][2]);
  ASSERT_EQ(0, sanctum::testing::core_edrb_map[3][3]);

  set_edrb_map(128 * sizeof(size_t));
  ASSERT_EQ(0, sanctum::testing::core_edrb_map[0][0]);
  ASSERT_EQ(0, sanctum::testing::core_edrb_map[0][1]);
  ASSERT_EQ(0, sanctum::testing::core_edrb_map[0][2]);
  ASSERT_EQ(0, sanctum::testing::core_edrb_map[0][3]);

  set_current_core(3);
  set_edrb_map(29 * sizeof(size_t));
  ASSERT_EQ(29, sanctum::testing::core_edrb_map[3][0]);
  ASSERT_EQ(30, sanctum::testing::core_edrb_map[3][1]);
  ASSERT_EQ(31, sanctum::testing::core_edrb_map[3][2]);
  ASSERT_EQ(0, sanctum::testing::core_edrb_map[3][3]);

  ASSERT_EQ(0, sanctum::testing::core_edrb_map[0][0]);
  ASSERT_EQ(0, sanctum::testing::core_edrb_map[0][1]);
  ASSERT_EQ(0, sanctum::testing::core_edrb_map[0][2]);
  ASSERT_EQ(0, sanctum::testing::core_edrb_map[0][3]);

  set_edrb_map(128 * sizeof(size_t));
  ASSERT_EQ(0, sanctum::testing::core_edrb_map[3][0]);
  ASSERT_EQ(0, sanctum::testing::core_edrb_map[3][1]);
  ASSERT_EQ(0, sanctum::testing::core_edrb_map[3][2]);
  ASSERT_EQ(0, sanctum::testing::core_edrb_map[3][3]);

  set_current_core(0);

  memset(phys_buffer, 0, 256 * sizeof(size_t));
}
