#pragma once

#include <array>
#include <cstddef>
#include <span>
#include <type_traits>

#include <structured_layout/array_field.hpp>

namespace structured_layout {

enum class schema_kind {
    leaf,
    record,
    fixed_array,
    layout_array,
    borrowed_range
};

template<typename type>
struct is_array_field : std::false_type {};

template<typename type, std::size_t extent, typename layout>
struct is_array_field<array_field<type, extent, layout>> : std::true_type {};

template<typename type>
inline constexpr bool is_array_field_v =
    is_array_field<std::remove_cvref_t<type>>::value;

template<typename type>
struct array_field_traits;

template<typename type, std::size_t extent_value, typename layout_type>
struct array_field_traits<array_field<type, extent_value, layout_type>> {
    using value_type = type;
    using layout = layout_type;
    static constexpr std::size_t extent = extent_value;
};

template<typename type>
struct is_std_array : std::false_type {};

template<typename type, std::size_t extent>
struct is_std_array<std::array<type, extent>> : std::true_type {};

template<typename type>
inline constexpr bool is_std_array_v =
    is_std_array<std::remove_cvref_t<type>>::value;

template<typename type>
struct is_std_span : std::false_type {};

template<typename type, std::size_t extent>
struct is_std_span<std::span<type, extent>> : std::true_type {};

template<typename type>
inline constexpr bool is_std_span_v =
    is_std_span<std::remove_cvref_t<type>>::value;

template<typename type>
inline constexpr schema_kind schema_kind_v = [] {
    using value_type = std::remove_cvref_t<type>;
    if constexpr (is_array_field_v<value_type>)
        return schema_kind::layout_array;
    else if constexpr (is_std_array_v<value_type>)
        return schema_kind::fixed_array;
    else if constexpr (is_std_span_v<value_type>)
        return schema_kind::borrowed_range;
    else if constexpr (std::is_aggregate_v<value_type>)
        return schema_kind::record;
    else
        return schema_kind::leaf;
}();

}
