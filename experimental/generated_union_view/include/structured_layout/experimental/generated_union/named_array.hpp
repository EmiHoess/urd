#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

#include <structured_layout/array.hpp>
#include <structured_layout/experimental/generated_union/detail/reference.hpp>

namespace structured_layout::experimental::generated_union {

namespace detail {

template<typename array_type>
class named_iterator {
public:
    using reference = decltype(make_reference<typename array_type::value_type>(
        std::declval<array_type&>()[0]));
    using value_type = reference;
    using difference_type = std::ptrdiff_t;
    using iterator_concept = std::input_iterator_tag;
    using iterator_category = std::input_iterator_tag;

    constexpr named_iterator() = default;

    constexpr named_iterator(array_type& values, std::size_t index) noexcept
        : values_(&values), index_(index) {}

    constexpr reference operator*() const {
        return make_reference<typename array_type::value_type>(
            (*values_)[index_]);
    }

    constexpr named_iterator& operator++() noexcept {
        ++index_;
        return *this;
    }

    constexpr named_iterator operator++(int) noexcept {
        auto previous = *this;
        ++*this;
        return previous;
    }

    constexpr bool operator==(named_iterator const&) const = default;

private:
    array_type* values_ = nullptr;
    std::size_t index_ = 0;
};

}

template<typename array_type>
class named_view {
public:
    using value_type = typename array_type::value_type;

    static_assert(schema_kind_v<value_type> == schema_kind::record);

    constexpr explicit named_view(array_type& values) noexcept
        : values_(&values) {}

    named_view(array_type&&) = delete;

    constexpr auto operator[](std::size_t index) const {
        return detail::make_reference<value_type>((*values_)[index]);
    }

    constexpr std::size_t size() const noexcept {
        return values_->size();
    }

    constexpr auto begin() const noexcept {
        return detail::named_iterator<array_type>{*values_, 0};
    }

    constexpr auto end() const noexcept {
        return detail::named_iterator<array_type>{*values_, size()};
    }

private:
    array_type* values_;
};

template<typename type, std::size_t extent, typename layout>
constexpr auto make_named_view(
    structured_layout::array<type, extent, layout>& values) noexcept {
    return named_view{values};
}

template<typename type, std::size_t extent, typename layout>
constexpr auto make_named_view(
    structured_layout::array<type, extent, layout> const& values) noexcept {
    return named_view{values};
}

template<typename type, std::size_t extent, typename layout>
auto make_named_view(structured_layout::array<type, extent, layout>&&) = delete;

template<typename type, std::size_t extent, typename layout>
auto make_named_view(structured_layout::array<type, extent, layout> const&&) = delete;

template<typename type, std::size_t extent_value, typename layout_type = layout_of_t<type>>
class named_array {
public:
    using value_type = type;
    using layout = layout_type;

    static constexpr std::size_t extent = extent_value;

    static_assert(schema_kind_v<type> == schema_kind::record,
                  "generated union arrays require a record element type");

    constexpr auto operator[](std::size_t index) & {
        return detail::make_reference<type>(values_[index]);
    }

    constexpr auto operator[](std::size_t index) const& {
        return detail::make_reference<type>(values_[index]);
    }

    auto operator[](std::size_t) && = delete;
    auto operator[](std::size_t) const&& = delete;

    constexpr std::size_t size() const noexcept {
        return extent;
    }

    constexpr type load(std::size_t index) const {
        return values_.load(index);
    }

    constexpr void store(std::size_t index, type const& value) {
        values_.store(index, value);
    }

    constexpr auto begin() & noexcept {
        return detail::named_iterator<storage_type>{values_, 0};
    }

    constexpr auto end() & noexcept {
        return detail::named_iterator<storage_type>{values_, extent};
    }

    constexpr auto begin() const& noexcept {
        return detail::named_iterator<storage_type const>{values_, 0};
    }

    constexpr auto end() const& noexcept {
        return detail::named_iterator<storage_type const>{values_, extent};
    }

    auto begin() && = delete;
    auto end() && = delete;
    auto begin() const&& = delete;
    auto end() const&& = delete;

    constexpr auto cbegin() const& noexcept {
        return begin();
    }

    constexpr auto cend() const& noexcept {
        return end();
    }

    auto cbegin() const&& = delete;
    auto cend() const&& = delete;

private:
    using storage_type = structured_layout::array<type, extent, layout>;
    storage_type values_{};
};

}
