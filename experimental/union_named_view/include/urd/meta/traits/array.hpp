#pragma once
#include <array>
#include <urd/types/array.hpp>

namespace urd {

// is_array — true for std::array<T,N> and urd::array<T,N[,Layout]>
template<class T> struct is_array : std::false_type {};

template<class T, std::size_t N>
struct is_array<std::array<T, N>> : std::true_type {};

template<class T, std::size_t N>
struct is_array<urd::array<T, N>> : std::true_type {};

template<class T, std::size_t N, typename Layout>
struct is_array<urd::array<T, N, Layout>> : std::true_type {};

template<class T> struct is_array<T const>         : is_array<T> {};
template<class T> struct is_array<T volatile>       : is_array<T> {};
template<class T> struct is_array<T volatile const> : is_array<T> {};

template<class T>
inline constexpr bool is_array_v = is_array<T>::value;

// is_urd_array — true only for urd::array
template<class T> struct is_urd_array : std::false_type {};

template<class T, std::size_t N>
struct is_urd_array<urd::array<T, N>> : std::true_type {};

template<class T, std::size_t N, typename Layout>
struct is_urd_array<urd::array<T, N, Layout>> : std::true_type {};

template<class T>
inline constexpr bool is_urd_array_v = is_urd_array<T>::value;

// element_type_of_array — element type of std::array<T,N>
template<typename Array>
struct element_type_of_array;

template<typename T, std::size_t N>
struct element_type_of_array<std::array<T, N>> { using type = T; };

template<typename Array>
using element_type_of_array_t = typename element_type_of_array<Array>::type;

} // namespace urd
