#pragma once

#include <cstddef>

#include <structured_layout/detail/storage.hpp>
#include <structured_layout/layout.hpp>
#include <structured_layout/view.hpp>

namespace structured_layout {

template<
    typename type,
    std::size_t extent_value,
    typename layout_type = layout_of_t<type>>
class array {
public:
    using value_type = type;
    using layout = layout_type;

    static constexpr std::size_t extent = extent_value;

    static_assert(is_layout_v<layout>);
    static_assert(schema_kind_v<type> != schema_kind::layout_array);

    constexpr std::size_t size() const noexcept {
        return extent;
    }

    constexpr auto operator[](std::size_t index) noexcept {
        return basic_element_reference<storage_type>{storage_, index};
    }

    constexpr auto operator[](std::size_t index) const noexcept {
        return basic_element_reference<storage_type const>{storage_, index};
    }

    constexpr type load(std::size_t index) const {
        return storage_.load(index);
    }

    constexpr void store(std::size_t index, type const& value) {
        storage_.store(index, value);
    }

    template<auto member>
    constexpr auto field() noexcept {
        return basic_field_view<storage_type, member>{storage_, extent};
    }

    template<auto member>
    constexpr auto field() const noexcept {
        return basic_field_view<storage_type const, member>{storage_, extent};
    }

private:
    using storage_type = detail::storage_node<type, extent, layout>;

    storage_type storage_{};
};

}
