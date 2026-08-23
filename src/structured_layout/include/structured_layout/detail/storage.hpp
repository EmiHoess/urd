#pragma once

#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include <structured_layout/detail/element_access.hpp>
#include <structured_layout/layout.hpp>
#include <structured_layout/schema.hpp>

#include <urd/member_pointer.hpp>
#include <urd/reflection.hpp>

namespace structured_layout::detail {

template<typename type, std::size_t extent, typename layout>
class storage_node;

template<typename type, std::size_t outer_extent, std::size_t inner_extent, typename layout>
class array_field_storage;

template<auto first, auto... rest, typename type>
constexpr decltype(auto) access_object(type& value) noexcept;

template<typename type>
class direct_element_reference {
public:
    using value_type = std::remove_const_t<type>;

    constexpr explicit direct_element_reference(type& value) noexcept
        : value_(&value) {}

    template<auto... members>
    constexpr decltype(auto) get() const noexcept {
        static_assert(sizeof...(members) > 0);
        return access_object<members...>(*value_);
    }

    constexpr type& value() const noexcept {
        return *value_;
    }

    constexpr value_type load() const {
        return *value_;
    }

    constexpr void store(value_type const& value) const
        requires (!std::is_const_v<type>)
    {
        *value_ = value;
    }

private:
    type* value_;
};

template<typename field_type>
class direct_array_field_reference {
public:
    using value_type = std::remove_const_t<field_type>;
    using traits = array_field_traits<value_type>;
    using stored_type = typename traits::value_type;
    using element_type = std::conditional_t<
        std::is_const_v<field_type>,
        stored_type const,
        stored_type>;

    constexpr explicit direct_array_field_reference(field_type& value) noexcept
        : value_(&value) {}

    constexpr std::size_t size() const noexcept {
        return traits::extent;
    }

    constexpr auto operator[](std::size_t index) const noexcept {
        return direct_element_reference<element_type>{(*value_)[index]};
    }

    constexpr value_type load() const {
        return *value_;
    }

    constexpr void store(value_type const& value) const
        requires (!std::is_const_v<field_type>)
    {
        *value_ = value;
    }

private:
    field_type* value_;
};

template<typename storage_type>
class basic_array_field_reference {
public:
    using value_type = typename std::remove_const_t<storage_type>::value_type;

    constexpr basic_array_field_reference(
        storage_type& storage,
        std::size_t outer_index) noexcept
        : storage_(&storage), outer_index_(outer_index) {}

    constexpr std::size_t size() const noexcept {
        return std::remove_const_t<storage_type>::extent;
    }

    constexpr auto operator[](std::size_t inner_index) const noexcept {
        return storage_->element(outer_index_, inner_index);
    }

    constexpr value_type load() const {
        return storage_->load(outer_index_);
    }

    constexpr void store(value_type const& value) const
        requires (!std::is_const_v<storage_type>)
    {
        storage_->store(outer_index_, value);
    }

private:
    storage_type* storage_;
    std::size_t outer_index_;
};

template<typename type>
using member_tuple_t = decltype(urd::tie_members(std::declval<type&>()));

template<typename type, std::size_t index>
using member_type_t = std::remove_cvref_t<
    std::tuple_element_t<index, member_tuple_t<type>>>;

template<typename value_type, std::size_t extent>
struct member_storage {
    using result = storage_node<value_type, extent, layout_of_t<value_type>>;
};

template<
    typename value_type,
    std::size_t inner_extent,
    typename layout,
    std::size_t outer_extent>
struct member_storage<array_field<value_type, inner_extent, layout>, outer_extent> {
    using result = array_field_storage<
        value_type,
        outer_extent,
        inner_extent,
        layout>;
};

template<typename type, std::size_t extent>
using member_storage_t = typename member_storage<type, extent>::result;

template<typename type, std::size_t extent, std::size_t... indices>
auto make_field_storage_type(std::index_sequence<indices...>)
    -> std::tuple<member_storage_t<member_type_t<type, indices>, extent>...>;

template<typename type, std::size_t extent>
using field_storage_t = decltype(make_field_storage_type<type, extent>(
    std::make_index_sequence<urd::member_count_v<type>>{}));

template<auto first, auto... rest, typename type>
constexpr decltype(auto) access_object(type& value) noexcept {
    static_assert(std::is_same_v<
        std::remove_cv_t<type>,
        urd::member_owner_t<first>>);
    auto& member = value.*first;
    if constexpr (sizeof...(rest) == 0)
        return (member);
    else
        return access_object<rest...>(member);
}

template<typename type, std::size_t extent, schema_kind kind>
class contiguous_storage {
public:
    using value_type = type;

    constexpr type& reference(std::size_t index) noexcept {
        return values_[index];
    }

    constexpr type const& reference(std::size_t index) const noexcept {
        return values_[index];
    }

    template<auto... members>
    constexpr decltype(auto) get(std::size_t index) noexcept {
        static_assert(sizeof...(members) > 0);
        auto& member = access_object<members...>(values_[index]);
        if constexpr (is_array_field_v<decltype(member)>)
            return direct_array_field_reference<std::remove_reference_t<
                decltype(member)>>{member};
        else
            return (member);
    }

    template<auto... members>
    constexpr decltype(auto) get(std::size_t index) const noexcept {
        static_assert(sizeof...(members) > 0);
        auto& member = access_object<members...>(values_[index]);
        if constexpr (is_array_field_v<decltype(member)>)
            return direct_array_field_reference<std::remove_reference_t<
                decltype(member)>>{member};
        else
            return (member);
    }

    constexpr type load(std::size_t index) const {
        return values_[index];
    }

    constexpr void store(std::size_t index, type const& value) {
        values_[index] = value;
    }

private:
    std::array<type, extent> values_{};
};

template<typename type, std::size_t extent>
class storage_node<type, extent, aos_layout>
    : public contiguous_storage<type, extent, schema_kind_v<type>> {};

template<typename type, std::size_t extent>
    requires (schema_kind_v<type> != schema_kind::record)
class storage_node<type, extent, soa_layout>
    : public contiguous_storage<type, extent, schema_kind_v<type>> {};

template<typename type, std::size_t extent>
    requires (schema_kind_v<type> == schema_kind::record)
class storage_node<type, extent, soa_layout> {
public:
    using value_type = type;

    template<auto first, auto... rest>
    constexpr decltype(auto) get(std::size_t index) noexcept {
        static_assert(std::is_same_v<urd::member_owner_t<first>, type>);
        constexpr std::size_t field_index = urd::member_index(first);
        static_assert(field_index < urd::member_count_v<type>);
        auto& field = std::get<field_index>(fields_);
        if constexpr (sizeof...(rest) == 0)
            return field.reference(index);
        else
            return field.template get<rest...>(index);
    }

    template<auto first, auto... rest>
    constexpr decltype(auto) get(std::size_t index) const noexcept {
        static_assert(std::is_same_v<urd::member_owner_t<first>, type>);
        constexpr std::size_t field_index = urd::member_index(first);
        static_assert(field_index < urd::member_count_v<type>);
        auto const& field = std::get<field_index>(fields_);
        if constexpr (sizeof...(rest) == 0)
            return field.reference(index);
        else
            return field.template get<rest...>(index);
    }

    constexpr type load(std::size_t index) const {
        return load_impl(index, std::make_index_sequence<urd::member_count_v<type>>{});
    }

    constexpr void store(std::size_t index, type const& value) {
        auto members = urd::tie_members(value);
        store_impl(index, members, std::make_index_sequence<urd::member_count_v<type>>{});
    }

private:
    template<std::size_t... indices>
    constexpr type load_impl(
        std::size_t index,
        std::index_sequence<indices...>) const {
        return type{std::get<indices>(fields_).load(index)...};
    }

    template<typename tuple_type, std::size_t... indices>
    constexpr void store_impl(
        std::size_t index,
        tuple_type const& members,
        std::index_sequence<indices...>) {
        (std::get<indices>(fields_).store(index, std::get<indices>(members)), ...);
    }

    field_storage_t<type, extent> fields_{};
};

template<typename type, std::size_t extent, std::size_t block_extent>
class storage_node<type, extent, aosoa_layout<block_extent>> {
public:
    using value_type = type;

    static constexpr std::size_t block_count =
        (extent + block_extent - 1) / block_extent;

    constexpr type& reference(std::size_t index) noexcept
        requires requires (storage_node<type, block_extent, soa_layout>& block) {
            block.reference(0);
        }
    {
        return blocks_[index / block_extent].reference(index % block_extent);
    }

    constexpr type const& reference(std::size_t index) const noexcept
        requires requires (
            storage_node<type, block_extent, soa_layout> const& block) {
            block.reference(0);
        }
    {
        return blocks_[index / block_extent].reference(index % block_extent);
    }

    template<auto... members>
    constexpr decltype(auto) get(std::size_t index) noexcept {
        return blocks_[index / block_extent]
            .template get<members...>(index % block_extent);
    }

    template<auto... members>
    constexpr decltype(auto) get(std::size_t index) const noexcept {
        return blocks_[index / block_extent]
            .template get<members...>(index % block_extent);
    }

    constexpr type load(std::size_t index) const {
        return blocks_[index / block_extent].load(index % block_extent);
    }

    constexpr void store(std::size_t index, type const& value) {
        blocks_[index / block_extent].store(index % block_extent, value);
    }

private:
    std::array<storage_node<type, block_extent, soa_layout>, block_count> blocks_{};
};

template<
    typename type,
    std::size_t outer_extent,
    std::size_t inner_extent,
    typename layout>
class array_field_storage {
public:
    using value_type = array_field<type, inner_extent, layout>;
    using element_storage_type =
        storage_node<type, outer_extent * inner_extent, layout>;

    static constexpr std::size_t extent = inner_extent;

    constexpr auto reference(std::size_t outer_index) noexcept {
        return basic_array_field_reference<array_field_storage>{*this, outer_index};
    }

    constexpr auto reference(std::size_t outer_index) const noexcept {
        return basic_array_field_reference<array_field_storage const>{*this, outer_index};
    }

    constexpr auto element(
        std::size_t outer_index,
        std::size_t inner_index) noexcept {
        return storage_element_reference<element_storage_type>{
            elements_,
            element_index(outer_index, inner_index)};
    }

    constexpr auto element(
        std::size_t outer_index,
        std::size_t inner_index) const noexcept {
        return storage_element_reference<element_storage_type const>{
            elements_,
            element_index(outer_index, inner_index)};
    }

    constexpr value_type load(std::size_t outer_index) const {
        value_type result{};
        for (std::size_t inner_index = 0; inner_index < inner_extent; ++inner_index)
            result[inner_index] =
                elements_.load(element_index(outer_index, inner_index));
        return result;
    }

    constexpr void store(std::size_t outer_index, value_type const& value) {
        for (std::size_t inner_index = 0; inner_index < inner_extent; ++inner_index)
            elements_.store(
                element_index(outer_index, inner_index),
                value[inner_index]);
    }

private:
    static constexpr std::size_t element_index(
        std::size_t outer_index,
        std::size_t inner_index) noexcept {
        if constexpr (std::is_same_v<layout, aos_layout>) {
            return outer_index * inner_extent + inner_index;
        } else if constexpr (std::is_same_v<layout, soa_layout>) {
            return inner_index * outer_extent + outer_index;
        } else {
            constexpr std::size_t block_extent = layout::extent;
            std::size_t const block_index = outer_index / block_extent;
            std::size_t const block_begin = block_index * block_extent;
            std::size_t const block_size =
                outer_extent - block_begin < block_extent
                    ? outer_extent - block_begin
                    : block_extent;
            return block_begin * inner_extent +
                inner_index * block_size +
                outer_index % block_extent;
        }
    }

    element_storage_type elements_{};
};

}
