#pragma once

#if !defined(__cpp_impl_reflection) || __cpp_impl_reflection < 202506L
#error "reflected named views require C++26 reflection support"
#else

#include <array>
#include <cstddef>
#include <meta>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

#include <structured_layout/detail/storage.hpp>
#include <structured_layout/schema.hpp>

namespace structured_layout::experimental::detail {

template<typename type>
struct is_vector : std::false_type {};

template<typename type, typename allocator>
struct is_vector<std::vector<type, allocator>> : std::true_type {};

template<typename type>
inline constexpr bool is_indexed_v =
    is_array_field_v<type> || is_std_array_v<type> || is_std_span_v<type> ||
    is_vector<std::remove_cv_t<type>>::value || std::is_array_v<type>;

template<typename access_type, auto... members>
decltype(auto) access_field(access_type const& access) {
    if constexpr (sizeof...(members) == 0)
        return access.value();
    else
        return access.template get<members...>();
}

template<typename access_type>
struct cursor_arm {
    access_type access;

    cursor_arm& operator=(cursor_arm const&) = delete;
};

template<typename access_type, auto... members>
struct field_reference {
    access_type access;

    field_reference(field_reference const&) = default;

    using reference = decltype(access_field<access_type, members...>(
        std::declval<access_type const&>()));

    decltype(auto) get() const {
        return access_field<access_type, members...>(access);
    }

    operator reference() const {
        return get();
    }

    field_reference& operator=(field_reference const& other)
        requires requires (reference left, reference right) { left = right; }
    {
        get() = other.get();
        return *this;
    }

    template<typename value_type>
    field_reference& operator=(value_type&& value)
        requires requires (reference target, value_type&& source) {
            target = std::forward<value_type>(source);
        }
    {
        get() = std::forward<value_type>(value);
        return *this;
    }

#define structured_layout_named_compound(symbol) \
    template<typename value_type> \
    field_reference& operator symbol(value_type&& value) \
        requires requires (reference target, value_type&& source) { \
            target symbol std::forward<value_type>(source); \
        } \
    { \
        get() symbol std::forward<value_type>(value); \
        return *this; \
    }

    structured_layout_named_compound(+=)
    structured_layout_named_compound(-=)
    structured_layout_named_compound(*=)
    structured_layout_named_compound(/=)
    structured_layout_named_compound(%=)
    structured_layout_named_compound(&=)
    structured_layout_named_compound(|=)
    structured_layout_named_compound(^=)
    structured_layout_named_compound(<<=)
    structured_layout_named_compound(>>=)

#undef structured_layout_named_compound

    field_reference& operator++()
        requires requires (reference value) { ++value; }
    {
        ++get();
        return *this;
    }

    auto operator++(int)
        requires requires (reference value) { value++; }
    {
        return get()++;
    }

    field_reference& operator--()
        requires requires (reference value) { --value; }
    {
        --get();
        return *this;
    }

    auto operator--(int)
        requires requires (reference value) { value--; }
    {
        return get()--;
    }
};

template<typename source_type, typename access_type, auto... members>
struct record_reference;

template<typename source_type, typename access_type, auto... members>
consteval std::meta::info reflected_reference_type();

template<typename source_type, typename access_type, auto... members>
decltype(auto) make_reference(access_type access);

template<typename source_type, typename access_type, auto... members>
struct array_reference {
    access_type access;

    array_reference& operator=(array_reference const&) = delete;

    std::size_t size() const {
        if constexpr (is_array_field_v<source_type>)
            return array_field_traits<std::remove_cv_t<source_type>>::extent;
        else if constexpr (std::is_array_v<source_type>)
            return std::extent_v<source_type>;
        else
            return access_field<access_type, members...>(access).size();
    }

    decltype(auto) operator[](std::size_t index) const {
        auto&& elements = access_field<access_type, members...>(access);
        if constexpr (is_array_field_v<source_type> &&
                      !is_array_field_v<decltype(elements)>) {
            auto element = elements[index];
            return make_reference<typename decltype(element)::value_type>(element);
        } else {
            auto& value = elements[index];
            using element_type = std::remove_reference_t<decltype(value)>;
            auto element = structured_layout::detail::direct_element_reference<
                element_type>{value};
            return make_reference<std::remove_cv_t<element_type>>(element);
        }
    }
};

template<typename source_type, typename access_type, auto... members>
struct record_reference {
    union type;

    consteval {
        using record_type = std::remove_cv_t<source_type>;
        static_assert(std::is_class_v<record_type> && std::is_aggregate_v<record_type> &&
                      !std::is_union_v<record_type>,
                      "named records must be non-union aggregates");
        static_assert(std::is_standard_layout_v<access_type> &&
                      std::is_trivially_copyable_v<access_type>,
                      "named view accessors must be standard-layout trivial cursors");
        constexpr auto context = std::meta::access_context::current();
        static_assert(std::meta::bases_of(^^record_type, context).empty(),
                      "named records with base classes are unsupported");
        constexpr auto fields = std::define_static_array(
            std::meta::nonstatic_data_members_of(^^record_type, context));
        constexpr bool valid_fields = [=] {
            for (auto member : fields) {
                if (std::meta::is_bit_field(member) ||
                    std::meta::identifier_of(member) == "sl_cursor")
                    return false;
            }
            return true;
        }();
        static_assert(valid_fields,
                      "named records cannot contain bit-fields or a field named sl_cursor");

        auto descriptions = []<std::size_t... indices>(std::index_sequence<indices...>) {
            constexpr auto fields = std::define_static_array(
                std::meta::nonstatic_data_members_of(
                    ^^record_type, std::meta::access_context::current()));
            std::vector<std::meta::info> result;
            result.push_back(std::meta::data_member_spec(
                ^^cursor_arm<access_type>, {.name = "sl_cursor"}));
            (result.push_back(std::meta::data_member_spec(
                reflected_reference_type<
                    typename [:std::meta::type_of(fields[indices]):],
                    access_type, members..., &[:fields[indices]:]>(),
                {.name = std::meta::identifier_of(fields[indices])})), ...);
            return result;
        }(std::make_index_sequence<fields.size()>{});
        std::meta::define_aggregate(^^type, descriptions);
    }
};

template<typename source_type, typename access_type, auto... members>
consteval std::meta::info reflected_reference_type() {
    if constexpr (is_indexed_v<source_type>)
        return ^^array_reference<source_type, access_type, members...>;
    else if constexpr (schema_kind_v<source_type> == schema_kind::record)
        return ^^typename record_reference<source_type, access_type, members...>::type;
    else
        return ^^field_reference<access_type, members...>;
}

template<typename source_type, typename access_type, auto... members>
decltype(auto) make_reference(access_type access) {
    if constexpr (is_indexed_v<source_type>) {
        return array_reference<source_type, access_type, members...>{access};
    } else if constexpr (schema_kind_v<source_type> == schema_kind::record) {
        using result_type = typename record_reference<
            source_type, access_type, members...>::type;
        static_assert(sizeof(result_type) == sizeof(cursor_arm<access_type>));
        static_assert(alignof(result_type) == alignof(cursor_arm<access_type>));
        static_assert(std::is_standard_layout_v<result_type>);
        static_assert(std::is_trivially_copy_constructible_v<result_type>);
        return result_type{{access}};
    } else {
        return access_field<access_type, members...>(access);
    }
}

}

#endif
