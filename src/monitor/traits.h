#if !defined(MONITOR_TRAITS_H_INCLUDED)
#define MONITOR_TRAITS_H_INCLUDED

// Useful subset of the type traits in the C++ 11/14 stdlib.

// enable_if
template<bool B, typename T = void> struct enable_if {};
template<typename T> struct enable_if<true, T> { typedef T type; };

// remove_const, remove_volatile, remove_cv  (used below)
template<typename T> struct remove_const { typedef T type; };
template<typename T> struct remove_const<const T> { typedef T type; };
template<typename T> struct remove_volatile { typedef T type; };
template<typename T> struct remove_volatile<volatile T> { typedef T type; };
template<typename T> struct remove_cv {
  typedef typename remove_volatile<typename remove_const<T>::type>::type type;
};

// integral_constant  (used below)
template<typename T, T v> struct integral_constant {
    static constexpr const T value = v;
    typedef T value_type;
    typedef integral_constant type;
    inline constexpr operator value_type() const noexcept { return value; }
    inline constexpr value_type operator()() const noexcept { return value; }
};

// type_type, false_type
typedef integral_constant<bool, true>  true_type;
typedef integral_constant<bool, false> false_type;

// is_same (used in tests)
template<typename T, typename U> struct is_same : public false_type { };
template<typename T> struct is_same<T, T> : public true_type { };

// is_class_or_union
//
// This is not in the C++11/14 standard, but can be implemented without special
// compiler operators, and is more useful anyway.
namespace __is_class_or_union_impl {
  struct __two { char __x[2]; };
  template <class T> char  __test(int T::*);
  template <class T> __two __test(...);
};
template <class T> struct is_class_or_union : public integral_constant<
    bool, sizeof(__is_class_or_union_impl::__test<T>(0)) == 1> {
};

// TODO(pwnall): the last two definitions are here due to the research that
//               went into is_class_or_union; if they prove to be unnecessary,
//               remove them and their tests

// is_union
#if __has_feature(is_union) || (_GNUC_VER >= 403)
template <class T> struct is_union : public integral_constant<
    bool, __is_union(T)> {
};
#else
// It's impossible to do this correctly, so we always default this to false.
template<class T> struct is_union : public false_type {};
#endif  // __has_feature(is_union) || (_GNUC_VER >= 403)

// is_class
template <class T> struct is_class : public integral_constant<
    bool, is_class_or_union<T>::value && !is_union<T>::value> {
};

#endif  // !defined(MONITOR_TRAITS_H_INCLUDED)
