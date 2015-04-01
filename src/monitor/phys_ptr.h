#if !defined(MONITOR_PHYS_PTR_H_INCLUDED)
#define MONITOR_PHYS_PTR_H_INCLUDED

#include "traits.h"

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
  inline phys_ref(const phys_ref<T>&) = default;
  // Guarantee the existence of a move constructor.
  //
  // This is used when constructing a reference to be returned, e.g.
  //     return phys_ref<T>{address};
  inline phys_ref(phys_ref<T>&&) noexcept = default;

  // Guarantee the existence of a default copy assignment.
  inline phys_ref<T>& operator =(const phys_ref<T>&) noexcept = default;
  // No move assign, the copy constructor is just as cheap.
  inline phys_ref<T>& operator =(phys_ref<T>&&) noexcept = delete;

  // Dereference.
  operator T() const;
  // Reference assignment.
  phys_ref<T>& operator =(const T&);
  // Address-of.
  phys_ptr<T> operator &() {
    return phys_ptr<T>{addr};
  }
private:
  // Initialize from a physical address.
  explicit inline phys_ref(const uintptr_t& phys_addr) noexcept
      : addr(phys_addr) {};

  // Friendship needed to access the constructor that takes an address.
  template<typename U, typename V> friend class phys_ptr;

  const uintptr_t addr;
};


// Pointers to the physical address space.
template<typename T, typename E> class phys_ptr {
public:
  // Guarantee the existence of a default copy constructor.
  inline phys_ptr(const phys_ptr<T, E>&) = default;
  // Guarantee the existence of a move constructor.
  //
  // This is used when returning a newly created pointer, e.g.
  //     return phys_ptr<size_t>{addr};
  inline phys_ptr(phys_ptr<T, E>&&) noexcept = default;
  // Guarantee the existence of a default copy assignment.
  inline phys_ptr<T, E>& operator =(const phys_ptr<T, E>&) noexcept = default;
  // Guarantee the existence of a move assignment.
  //
  // This is used when assigning a uintptr_t to an existing pointer, e.g.
  //     ptr = phys_ptr<size_t>(address);
  inline phys_ptr<T, E>& operator =(phys_ptr<T, E>&&) noexcept = default;

  // Initialize from a physical address.
  inline phys_ptr(const uintptr_t& phys_addr) noexcept : addr(phys_addr) {};
  // Convert to a numerical physical address.
  explicit inline operator uintptr_t() const { return addr; }

  // The * operator returns references.
  inline phys_ref<T> operator *() const { return phys_ref<T>{addr}; }
  inline phys_ref<T> operator [](size_t offset) const {
    return phys_ref<T>{reinterpret_cast<uintptr_t>(
        reinterpret_cast<T*>(addr) + offset)};
  }

  // Pointer comparsion.
  inline bool operator ==(const phys_ptr<T>& other) const {
    return addr == other.addr;
  }

  // Pointer arithmetic.
  inline phys_ptr<T, E>& operator +=(const size_t offset) {
    addr = reinterpret_cast<uintptr_t>(reinterpret_cast<T*>(addr) + offset);
    return *this;
  }
  inline phys_ptr<T, E> operator +(const size_t offset) const {
    return phys_ptr<T, E>{reinterpret_cast<uintptr_t>(
        reinterpret_cast<T*>(addr) + offset)};
  }

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
  inline phys_ptr(const phys_ptr<T>&) = default;
  inline phys_ptr(phys_ptr<T>&&) noexcept = default;
  inline phys_ptr<T>& operator =(const phys_ptr<T>&) noexcept = default;
  inline phys_ptr<T>& operator =(phys_ptr<T>&&) noexcept = default;
  inline phys_ptr(const uintptr_t& phys_addr) noexcept : addr(phys_addr) {};
  explicit inline operator uintptr_t() const { return addr; }
  inline phys_ref<T> operator *() const { return phys_ref<T>{addr}; }
  inline phys_ref<T> operator [](size_t offset) const {
    return phys_ref<T>{reinterpret_cast<uintptr_t>(
        reinterpret_cast<T*>(addr) + offset)};
  }
  inline bool operator ==(const phys_ptr<T>& other) const {
    return addr == other.addr;
  }
  inline phys_ptr<T>& operator +=(const size_t offset) {
    addr = reinterpret_cast<uintptr_t>(reinterpret_cast<T*>(addr) + offset);
    return *this;
  }
  inline phys_ptr<T> operator +(const size_t offset) const {
    return phys_ptr<T>{reinterpret_cast<uintptr_t>(
        reinterpret_cast<T*>(addr) + offset)};
  }

public:  // Extensions for pointers to types with members.
  template<typename U> inline phys_ref<U> operator ->*(U T::* member_offset)
      const {
    U& fake_ref = (reinterpret_cast<T*>(this->addr))->*(member_offset);
    return phys_ref<U>{reinterpret_cast<uintptr_t>(&fake_ref)};
  }

  // NOTE: It's tempting to add a reference operator (commented out below).
  //       Unfortunately, -> must (ultimately) return a pointer type, so we
  //       can't get the ptr->field notation to work. Instead, we must use the
  //       clunky notation below, which invokes ->* instead.
  //            ptr->*(&struct_name::field)
  //
  // inline phys_ref<T> operator ->() const { return phys_ref<T>{addr}; }

private:
  uintptr_t addr;
};

#endif  // !defined(MONITOR_PHYS_PTR_H_INCLUDED)
