#pragma once
#include <cstddef>
#include <type_traits>

// Minimal span type: a non-owning view with pointer + size.
// Detected structurally by is_span_v below.
template<typename T>
struct span {
    using value_type = T;
    T*     mem  = nullptr;
    size_t size = 0;
};

// v2-local span detection by structure, not type identity.
// Matches any type with:
//   using value_type = T;
//   T*     mem;
//   size_t size;

namespace urd::v2 {

template<typename T, typename = void>
struct is_span : std::false_type {};

template<typename T>
struct is_span<T, std::void_t<
    typename T::value_type,
    decltype(std::declval<T&>().mem),
    decltype(std::declval<T&>().size)
>> : std::bool_constant<
    std::is_same_v<
        std::remove_reference_t<decltype(std::declval<T&>().mem)>,
        typename T::value_type*
    >
> {};

template<typename T>
inline constexpr bool is_span_v = is_span<T>::value;

template<typename T>
using span_element_t = typename T::value_type;

} // namespace urd::v2
