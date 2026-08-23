#pragma once

#include <array>
#include <cstddef>

#include <structured_layout/layout.hpp>

namespace structured_layout {

template<typename type, std::size_t extent_value, typename layout_type = layout_of_t<type>>
struct array_field {
    using value_type = type;
    using layout = layout_type;

    static constexpr std::size_t extent = extent_value;

    static_assert(is_layout_v<layout>);

    std::array<type, extent_value> values;

    constexpr type& operator[](std::size_t index) noexcept {
        return values[index];
    }

    constexpr type const& operator[](std::size_t index) const noexcept {
        return values[index];
    }

    constexpr auto begin() noexcept {
        return values.begin();
    }

    constexpr auto begin() const noexcept {
        return values.begin();
    }

    constexpr auto end() noexcept {
        return values.end();
    }

    constexpr auto end() const noexcept {
        return values.end();
    }
};

template<typename value_type, std::size_t extent, typename layout>
struct layout_traits<array_field<value_type, extent, layout>> {
    using type = layout;
};

}
