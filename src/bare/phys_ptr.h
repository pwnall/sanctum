#if !defined(BARE_PHYS_PTR_H_INCLUDED)
#define BARE_PHYS_PTR_H_INCLUDED

#include "base_types.h"
#include "traits.h"

namespace sanctum {
namespace bare {

template<typename T, typename E = void> class phys_ptr;
template<typename T> class phys_ptr<T,
    typename enable_if<is_class_or_union<T>::value>::type>;
template<typename T> class phys_ref;


// References to the physical address space.
//
// This reference type shouldn't be used directly. It's better to use the
// pointer type phys_ptr, and use pointer operations explicitly.
template<typename T> class phys_ref {
public:
  // Guarantee the existence of a default copy constructor.
  constexpr inline phys_ref(const phys_ref<T>&) noexcept = default;
  // Guarantee the existence of a move constructor.
  //
  // This is used when constructing a reference to be returned, e.g.
  //     return phys_ref<T>{address};
  constexpr inline phys_ref(phys_ref<T>&&) noexcept = default;

  // Dereference.
  operator T() const;
  // Reference assignment.
  phys_ref<T>& operator =(const T&);

  // Copy assignment.
  //
  // This is used by code that creates references explicitly, like:
  //   phys_ref<size_t> ref = *b;
  //   *a = ref;
  //
  // Implementing this lets use phys_ref explicitly, in case we ever decide to
  // e.g., use it to express non-null pointers, like WebKit does.
  phys_ref<T>& operator =(const phys_ref<T>& other) {
    const T value = other.operator T();
    return this->operator =(value);
  }
  // Move-assignment.
  //
  // This is used for de-referencing phys_ptr assignment (*a = *b).
  inline phys_ref<T>& operator =(phys_ref<T>&& other) {
    // NOTE: we write this in terms of standard de-referencing and assignment
    //       so we don't have to write assembly for those twice; it would only
    //       make sense to implement this directly if the underlying
    //       architecture had a memory-to-memory move, which isn't true for the
    //       load/store machines that we're targeting
    const T value = other.operator T();
    return this->operator =(value);
  }

  // Address-of.
  constexpr inline phys_ptr<T> operator &() const noexcept {
    return phys_ptr<T>{addr};
  }

  // Convenience overloads.
  //
  // The RISC V ISA doesn't have any opcodes that use memory directly, except
  // for load/store, so there's no optimization opportunity in implementing
  // operators such as += in assembly. Therefore, we implement what we need
  // once, here.
  template<typename U> inline phys_ref<T>& operator +=(const U& other) {
    this->operator =(this->operator T() + other);
    return *this;
  }
  template<typename U> inline phys_ref<T>& operator -=(const U& other) {
    this->operator =(this->operator T() - other);
    return *this;
  }
  template<typename U> inline phys_ref<T>& operator |=(const U& other) {
    this->operator =(this->operator T() | other);
    return *this;
  }
  template<typename U> inline phys_ref<T>& operator &=(const U& other) {
    this->operator =(this->operator T() & other);
    return *this;
  }

private:
  // Initialize from a physical address.
  constexpr explicit inline phys_ref(const uintptr_t& phys_addr) noexcept
      : addr(phys_addr) {};

  // Friendship needed to access the constructor that takes an address.
  template<typename U, typename V> friend class phys_ptr;

  const uintptr_t addr;
};


// Pointers to the physical address space.
template<typename T, typename E> class phys_ptr {
public:
  // Guarantee the existence of a default copy constructor.
  constexpr inline phys_ptr(const phys_ptr<T, E>&) noexcept = default;
  // Guarantee the existence of a move constructor.
  //
  // This is used when returning a newly created pointer, e.g.
  //     return phys_ptr<size_t>{addr};
  constexpr inline phys_ptr(phys_ptr<T, E>&&) noexcept = default;
  // Guarantee the existence of a default copy assignment.
  inline phys_ptr<T, E>& operator =(const phys_ptr<T, E>&) noexcept = default;
  // Guarantee the existence of a move assignment.
  //
  // This is used when assigning a uintptr_t to an existing pointer, e.g.
  //     ptr = phys_ptr<size_t>(address);
  inline phys_ptr<T, E>& operator =(phys_ptr<T, E>&&) noexcept = default;

  // Initialize from a physical address.
  constexpr inline phys_ptr(const uintptr_t phys_addr) noexcept
      : addr(phys_addr) {};
  // Convert to a numerical physical address.
  constexpr explicit inline operator uintptr_t() const noexcept {
    return addr;
  }

  // The * operator returns references.
  constexpr inline phys_ref<T> operator *() const noexcept {
    return phys_ref<T>{addr};
  }
  constexpr inline phys_ref<T> operator [](size_t offset) const noexcept {
    return phys_ref<T>{reinterpret_cast<uintptr_t>(
        reinterpret_cast<T*>(addr) + offset)};
  }

  // Pointer comparsion.
  constexpr inline bool operator ==(const phys_ptr<T>& other) const noexcept {
    return addr == other.addr;
  }
  constexpr inline bool operator !=(const phys_ptr<T>& other) const noexcept {
    return addr != other.addr;
  }

  // Pointer arithmetic.
  inline phys_ptr<T, E>& operator +=(const size_t offset) noexcept {
    addr = reinterpret_cast<uintptr_t>(reinterpret_cast<T*>(addr) + offset);
    return *this;
  }
  constexpr inline phys_ptr<T, E> operator +(const size_t offset) const
      noexcept {
    return phys_ptr<T, E>{reinterpret_cast<uintptr_t>(
        reinterpret_cast<T*>(addr) + offset)};
  }
  inline phys_ptr<T, E>& operator -=(const size_t offset) noexcept {
    addr = reinterpret_cast<uintptr_t>(reinterpret_cast<T*>(addr) - offset);
    return *this;
  }
  constexpr inline phys_ptr<T, E> operator -(const size_t offset) const
      noexcept {
    return phys_ptr<T, E>{reinterpret_cast<uintptr_t>(
        reinterpret_cast<T*>(addr) - offset)};
  }

  // nullptr replacement.
  static constexpr phys_ptr<T> null() noexcept { return phys_ptr<T, E>{0}; }

private:
  uintptr_t addr;
};

template<typename T> class phys_ptr<T,
    typename enable_if<is_class_or_union<T>::value>::type> {
public:
  // The code in the master template, slightly tweaked.
  // NOTE: there's a lot of redundancy here; using a base class wouldn't help
  //       too much, because constructors and assignment operators would have
  //       to be redefined
  constexpr inline phys_ptr(const phys_ptr<T>&) noexcept = default;
  constexpr inline phys_ptr(phys_ptr<T>&&) noexcept = default;
  inline phys_ptr<T>& operator =(const phys_ptr<T>&) noexcept = default;
  inline phys_ptr<T>& operator =(phys_ptr<T>&&) noexcept = default;
  constexpr inline phys_ptr(const uintptr_t phys_addr) noexcept
      : addr(phys_addr) {};
  constexpr explicit inline operator uintptr_t() const noexcept {
    return addr;
  }
  constexpr inline phys_ref<T> operator *() const noexcept {
    return phys_ref<T>{addr};
  }
  constexpr inline phys_ref<T> operator [](size_t offset) const noexcept {
    return phys_ref<T>{reinterpret_cast<uintptr_t>(
        reinterpret_cast<T*>(addr) + offset)};
  }
  constexpr inline bool operator ==(const phys_ptr<T>& other) const noexcept {
    return addr == other.addr;
  }
  constexpr inline bool operator !=(const phys_ptr<T>& other) const noexcept {
    return addr != other.addr;
  }
  inline phys_ptr<T>& operator +=(const size_t offset) noexcept {
    addr = reinterpret_cast<uintptr_t>(reinterpret_cast<T*>(addr) + offset);
    return *this;
  }
  constexpr inline phys_ptr<T> operator +(const size_t offset) const noexcept {
    return phys_ptr<T>{reinterpret_cast<uintptr_t>(
        reinterpret_cast<T*>(addr) + offset)};
  }
  inline phys_ptr<T>& operator -=(const size_t offset) noexcept {
    addr = reinterpret_cast<uintptr_t>(reinterpret_cast<T*>(addr) - offset);
    return *this;
  }
  constexpr inline phys_ptr<T> operator -(const size_t offset) const
      noexcept {
    return phys_ptr<T>{reinterpret_cast<uintptr_t>(
        reinterpret_cast<T*>(addr) - offset)};
  }
  static constexpr phys_ptr<T> null() noexcept { return phys_ptr<T>{0}; }

public:  // Extensions for pointers to types with members.
  template<typename U> constexpr inline phys_ref<U>
      operator ->*(U T::* member_offset) const noexcept {
    return phys_ref<U>{reinterpret_cast<uintptr_t>(
        &((reinterpret_cast<T*>(this->addr))->*(member_offset)))};
  }

  // NOTE: It's tempting to add a reference operator (commented out below).
  //       Unfortunately, -> must (ultimately) return a pointer type, so we
  //       can't get the ptr->field notation to work. Instead, we must use the
  //       clunky notation below, which invokes ->* instead.
  //            ptr->*(&struct_name::field)
  //
  // constexpr inline phys_ref<T> operator ->() const noexcept {
  //   return phys_ref<T>{addr};
  // }

private:
  uintptr_t addr;
};

};  // namespace sanctum::bare
};  // namespace sanctum

// Per-architecture implementations of physical loads and stores.
#include "phys_ptr_arch.h"

namespace sanctum {
namespace bare {

static_assert(sizeof(phys_ref<size_t>) == sizeof(uintptr_t),
    "phys_ref has unnecessary storge overhead");
static_assert(sizeof(phys_ptr<size_t>) == sizeof(uintptr_t),
    "phys_ptr has unnecessary storge overhead");

// Specialize phys_ref for phys_ptr so we can de-reference phys_ptr refs.
//
// This allows us to have phys_ptr members in structures pointed by phys_ptr.
template<typename T> class phys_ref<phys_ptr<T>> {
public:
  constexpr inline phys_ref(const phys_ref<phys_ptr<T>>&) noexcept = default;
  constexpr inline phys_ref(phys_ref<phys_ptr<T>>&&) noexcept = default;
  inline operator phys_ptr<T>() const {
    return phys_ptr<T>{
        (reinterpret_cast<const phys_ref<uintptr_t>*>(this))->
        operator uintptr_t()};
  }
  inline phys_ref<phys_ptr<T>>& operator =(const phys_ptr<T>& value) {
    (reinterpret_cast<phys_ref<uintptr_t>*>(this))->operator =(
        value.operator uintptr_t());
    return *this;
  }
  inline phys_ref<phys_ptr<T>>& operator =(
      const phys_ref<phys_ptr<T>>& other) {
    const phys_ptr<T> value = other.operator phys_ptr<T>();
    return this->operator =(value);
  }
  inline phys_ref<phys_ptr<T>>& operator =(phys_ref<phys_ptr<T>>&& other) {
    const phys_ptr<T> value = other.operator phys_ptr<T>();
    return this->operator =(value);
  }
  constexpr inline phys_ptr<phys_ptr<T>> operator &() const noexcept {
    return phys_ptr<phys_ptr<T>>{addr};
  }
  template<typename U> inline phys_ref<phys_ptr<T>>& operator +=(
      const U& other) {
    this->operator =(this->operator phys_ptr<T>() + other);
    return *this;
  }
  template<typename U> inline phys_ref<phys_ptr<T>>& operator -=(
      const U& other) {
    this->operator =(this->operator phys_ptr<T>() - other);
    return *this;
  }

private:
  // Initialize from a physical address.
  constexpr explicit inline phys_ref(const uintptr_t& phys_addr) noexcept
      : addr(phys_addr) {};

  // Friendship needed to access the constructor that takes an address.
  template<typename U, typename V> friend class phys_ptr;

  const uintptr_t addr;
};

};  // namespace sanctum::bare
};  // namespace sanctum
#endif  // !defined(BARE_PHYS_PTR_H_INCLUDED)
