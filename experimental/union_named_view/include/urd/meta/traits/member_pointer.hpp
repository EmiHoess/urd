#pragma once
#include <type_traits>

namespace urd {

template<typename T> struct member_field_impl;
template<typename C, typename T>
struct member_field_impl<T C::*> { using type = T; };

template<typename T>
using member_field_t = typename member_field_impl<typename std::remove_cv_t<T>>::type;

template<typename T> struct member_class_impl;
template<typename C, typename T>
struct member_class_impl<T C::*> { using type = C; };

template<typename T>
using member_class_t = typename member_class_impl<typename std::remove_cv_t<T>>::type;

// NTTP forms — take the member pointer directly as a value rather than a type.
// Usage: member_field_ptr_t<&Foo::x>  instead of  member_field_t<decltype(&Foo::x)>

template<auto Ptr> struct member_field_ptr_impl;
template<typename C, typename T, T C::*Ptr>
struct member_field_ptr_impl<Ptr> { using type = T; };

template<auto Ptr>
using member_field_ptr_t = typename member_field_ptr_impl<Ptr>::type;

template<auto Ptr> struct member_class_ptr_impl;
template<typename C, typename T, T C::*Ptr>
struct member_class_ptr_impl<Ptr> { using type = C; };

template<auto Ptr>
using member_class_ptr_t = typename member_class_ptr_impl<Ptr>::type;

} // namespace urd
