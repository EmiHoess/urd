#pragma once
#include <array>
#include <tuple>
#include <type_traits>
#include <urd/meta/traits/array.hpp>

namespace urd {

// instantiate_one_item — produce a default-constructed item of the element
// type: unwraps C-arrays, std::array / urd::array, or returns T directly.
template<typename T>
constexpr auto instantiate_one_item() {
    if constexpr (std::is_array_v<T>)
        return typename std::remove_all_extents<T>::type{};
    else if constexpr (urd::is_array<T>::value)
        return typename T::value_type{};
    else
        return T{};
}

// collection_item_type — element type of an array-like T, or T itself.
template<typename T>
using collection_item_type = decltype(instantiate_one_item<T>());

// collection_size — compile-time element count for array-like T, 1 otherwise.
template<typename Type>
constexpr auto collection_size() {
    if constexpr (urd::is_urd_array<Type>::value)
        return Type::size();
    else if constexpr (urd::is_array<Type>::value)
        return std::tuple_size<std::remove_reference_t<Type>>::value;
    else
        return 1;
}

} // namespace urd
