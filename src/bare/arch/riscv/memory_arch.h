#if !defined(BARE_ARCH_RISCV_MEMORY_ARCH_H_INCLUDED)
#define BARE_ARCH_RISCV_MEMORY_ARCH_H_INCLUDED

namespace sanctum {
namespace bare {

inline size_t read_dram_size() {
  // TODO: asm intrinsics
}
inline size_t read_cache_levels() {
  // TODO: asm intrinsics
}
inline bool is_shared_cache(size_t cache_level) {
  // TODO: asm intrinsics
}
inline size_t read_cache_line_size(size_t cache_level) {
  // TODO: asm intrinsics
}
inline size_t read_cache_set_count(size_t cache_level) {
  // TODO: asm intrinsics
}
inline size_t read_min_cache_index_shift() {
  // TODO: asm intrinsics
}
inline size_t read_max_cache_index_shift() {
  // TODO: asm intrinsics
}

template<typename T> inline void bzero(phys_ptr<T> start, size_t bytes) {
  // TODO: asm intrinsics
}
template<typename T> inline void bcopy(phys_ptr<T> dest, phys_ptr<T> source,
    size_t bytes) {
  // TODO: asm intrinsics
}

};  // namespace sanctum::bare
};  // namespace sanctum

#endif  // !defined(BARE_ARCH_RISCV_MEMORY_ARCH_H_INCLUDED)
