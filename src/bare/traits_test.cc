#include "traits.h"

#include "gtest/gtest.h"

using sanctum::bare::is_same;
using sanctum::bare::remove_const;
using sanctum::bare::remove_volatile;
using sanctum::bare::remove_cv;
using sanctum::bare::is_class_or_union;
using sanctum::bare::is_union;
using sanctum::bare::is_class;


typedef union {
  int a;
  int b;
} TestUnion;
struct TestStruct {
  int a;
  int b;
};
class TestClass {
  int a;
  int b;
};

TEST(TraitsTest, IsSame) {
  static_assert(true == is_same<int, int>::value, "is_same<int, int>");
  static_assert(false == is_same<int, int*>::value, "is_same<int, int*>");
  static_assert(false == is_same<int, int&>::value, "is_same<int, int*>");

  static_assert(true == is_same<TestClass, TestClass>::value,
                "is_same<TestClass, TestClass>");
  static_assert(false == is_same<TestClass, TestClass*>::value,
                "is_same<TestClass, TestClass*>");
  static_assert(false == is_same<TestClass, TestClass&>::value,
                "is_same<TestClass, TestClass*>");

  static_assert(true == is_same<TestClass*, TestClass*>::value,
                "is_same<TestClass*, TestClass*>");
  static_assert(false == is_same<TestClass*, const TestClass*>::value,
                "is_same<TestClass*, const TestClass*>");
  static_assert(false == is_same<TestClass*, volatile TestClass*>::value,
                "is_same<TestClass*, volatile TestClass*>");
}

TEST(TraitsTest, RemoveConst) {
  static_assert(true == is_same<TestClass,
                remove_const<TestClass>::type>::value,
                "remove_const<TestClass>");
  static_assert(true == is_same<TestClass,
                remove_const<const TestClass>::type>::value,
                "remove_const<const TestClass>");
  static_assert(true == is_same<volatile TestClass,
                remove_const<volatile TestClass>::type>::value,
                "remove_const<volatile TestClass>");
  static_assert(true == is_same<volatile TestClass,
                remove_const<const volatile TestClass>::type>::value,
                "remove_const<const volatile TestClass>");
}

TEST(TraitsTest, RemoveVolatile) {
  static_assert(true == is_same<TestClass,
                remove_volatile<TestClass>::type>::value,
                "remove_volatile<TestClass>");
  static_assert(true == is_same<const TestClass,
                remove_volatile<const TestClass>::type>::value,
                "remove_volatile<const TestClass>");
  static_assert(true == is_same<TestClass,
                remove_volatile<volatile TestClass>::type>::value,
                "remove_volatile<volatile TestClass>");
  static_assert(true == is_same<const TestClass,
                remove_volatile<const volatile TestClass>::type>::value,
                "remove_volatile<const volatile TestClass>");
}

TEST(TraitsTest, RemoveCv) {
  static_assert(true == is_same<TestClass,
                remove_cv<TestClass>::type>::value,
                "remove_cv<TestClass>");
  static_assert(true == is_same<TestClass,
                remove_cv<const TestClass>::type>::value,
                "remove_cv<const TestClass>");
  static_assert(true == is_same<TestClass,
                remove_cv<volatile TestClass>::type>::value,
                "remove_cv<volatile TestClass>");
  static_assert(true == is_same<TestClass,
                remove_cv<const volatile TestClass>::type>::value,
                "remove_cv<const volatile TestClass>");
}

TEST(TraitsTest, IsClassOrUnion) {
  static_assert(true == is_class_or_union<TestUnion>::value,
                "is_class_or_union<TestUnion>");
  static_assert(true == is_class_or_union<TestStruct>::value,
                "is_class_or_union<TestStruct>");
  static_assert(true == is_class_or_union<TestClass>::value,
                "is_class_or_union<TestClass>");
  static_assert(false == is_class_or_union<TestUnion*>::value,
                "is_class_or_union<TestUnion*>");
  static_assert(false == is_class_or_union<TestStruct*>::value,
                "is_class_or_union<TestStruct*>");
  static_assert(false == is_class_or_union<TestClass*>::value,
                "is_class_or_union<TestClass*>");
  static_assert(false == is_class_or_union<TestUnion&>::value,
                "is_class_or_union<TestUnion&>");
  static_assert(false == is_class_or_union<TestStruct&>::value,
                "is_class_or_union<TestStruct&>");
  static_assert(false == is_class_or_union<TestClass&>::value,
                "is_class_or_union<TestClass&>");
  static_assert(false == is_class_or_union<int>::value,
                "is_class_or_union<int>");
  static_assert(false == is_class_or_union<int*>::value,
                "is_class_or_union<int *>");
  static_assert(false == is_class_or_union<int&>::value,
                "is_class_or_union<int &>");
}

TEST(TraitsTest, IsUnion) {
#if __has_feature(is_union) || (_GNUC_VER >= 403)
  // Unions can't be classified correctly without compiler support.
  static_assert(true == is_union<TestUnion>::value, "is_union<TestUnion>");
#endif  // __has_feature(is_union) || (_GNUC_VER >= 403)

  static_assert(false == is_union<TestStruct>::value, "is_union<TestStruct>");
  static_assert(false == is_union<TestClass>::value, "is_union<TestClass>");
  static_assert(false == is_union<TestUnion*>::value, "is_union<TestUnion*>");
  static_assert(false == is_union<TestStruct*>::value,
                "is_union<TestStruct*>");
  static_assert(false == is_union<TestClass*>::value, "is_union<TestClass*>");
  static_assert(false == is_union<TestUnion&>::value, "is_union<TestUnion&>");
  static_assert(false == is_union<TestStruct&>::value,
                "is_union<TestStruct&>");
  static_assert(false == is_union<TestClass&>::value, "is_union<TestClass&>");
  static_assert(false == is_union<int>::value, "is_union<int>");
  static_assert(false == is_union<int*>::value, "is_union<int *>");
  static_assert(false == is_union<int&>::value, "is_union<int &>");
}

TEST(TraitsTest, IsClass) {
#if __has_feature(is_union) || (_GNUC_VER >= 403)
  // Unions can't be classified correctly without compiler support.
  static_assert(false == is_class<TestUnion>::value, "is_class<TestUnion>");
#endif  // __has_feature(is_union) || (_GNUC_VER >= 403)

  static_assert(true == is_class<TestStruct>::value, "is_class<TestStruct>");
  static_assert(true == is_class<TestClass>::value, "is_class<TestClass>");
  static_assert(false == is_class<TestUnion*>::value, "is_class<TestUnion*>");
  static_assert(false == is_class<TestStruct*>::value,
                "is_class<TestStruct*>");
  static_assert(false == is_class<TestClass*>::value, "is_class<TestClass*>");
  static_assert(false == is_class<TestUnion&>::value, "is_class<TestUnion&>");
  static_assert(false == is_class<TestStruct&>::value,
                "is_class<TestStruct&>");
  static_assert(false == is_class<TestClass&>::value, "is_class<TestClass&>");
  static_assert(false == is_class<int>::value, "is_class<int>");
  static_assert(false == is_class<int*>::value, "is_class<int *>");
  static_assert(false == is_class<int&>::value, "is_class<int &>");
}
