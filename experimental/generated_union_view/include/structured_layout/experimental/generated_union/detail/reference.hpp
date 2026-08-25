#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include <structured_layout/detail/storage.hpp>
#include <structured_layout/schema.hpp>

namespace structured_layout::experimental::generated_union::detail {

template<typename type>
struct is_vector : std::false_type {};

template<typename type, typename allocator>
struct is_vector<std::vector<type, allocator>> : std::true_type {};

template<typename type>
struct is_bool_vector : std::false_type {};

template<typename allocator>
struct is_bool_vector<std::vector<bool, allocator>> : std::true_type {};

template<typename type>
inline constexpr bool is_indexed_v =
    is_array_field_v<type> || is_std_array_v<type> || is_std_span_v<type> ||
    is_vector<std::remove_cv_t<type>>::value || std::is_array_v<type>;

template<typename source_type, auto... members>
inline constexpr bool schema_validation_v = [] {
    if constexpr (!std::is_aggregate_v<source_type> ||
                  std::is_union_v<source_type>) {
        return false;
    } else if constexpr (!(std::is_same_v<
                               source_type, urd::member_owner_t<members>> && ...)) {
        return false;
    } else if constexpr ((std::is_const_v<urd::member_value_t<members>> || ...) ||
                         (std::is_volatile_v<urd::member_value_t<members>> || ...)) {
        static_assert((!std::is_const_v<urd::member_value_t<members>> && ...),
                      "generated union views do not support const data members");
        static_assert((!std::is_volatile_v<urd::member_value_t<members>> && ...),
                      "generated union views do not support volatile data members");
        return false;
    } else if constexpr ((std::is_array_v<urd::member_value_t<members>> || ...)) {
        return false;
    } else if constexpr (sizeof...(members) != urd::member_count_v<source_type>) {
        return false;
    } else {
        std::size_t index = 0;
        return ((urd::member_index(members) == index++) && ...);
    }
}();

template<typename access_type, auto... members>
constexpr decltype(auto) access_field(access_type const& access) {
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

    constexpr decltype(auto) get() const {
        return access_field<access_type, members...>(access);
    }

    constexpr operator reference() const {
        return get();
    }

    constexpr field_reference& operator=(field_reference const& other)
        requires requires (reference left, reference right) { left = right; }
    {
        get() = other.get();
        return *this;
    }

    template<typename value_type>
    constexpr field_reference& operator=(value_type&& value)
        requires requires (reference target, value_type&& source) {
            target = std::forward<value_type>(source);
        }
    {
        get() = std::forward<value_type>(value);
        return *this;
    }

#define structured_layout_generated_union_compound(symbol) \
    template<typename value_type> \
    constexpr field_reference& operator symbol(value_type&& value) \
        requires requires (reference target, value_type&& source) { \
            target symbol std::forward<value_type>(source); \
        } \
    { \
        get() symbol std::forward<value_type>(value); \
        return *this; \
    }

    structured_layout_generated_union_compound(+=)
    structured_layout_generated_union_compound(-=)
    structured_layout_generated_union_compound(*=)
    structured_layout_generated_union_compound(/=)
    structured_layout_generated_union_compound(%=)
    structured_layout_generated_union_compound(&=)
    structured_layout_generated_union_compound(|=)
    structured_layout_generated_union_compound(^=)
    structured_layout_generated_union_compound(<<=)
    structured_layout_generated_union_compound(>>=)

#undef structured_layout_generated_union_compound

    constexpr field_reference& operator++()
        requires requires (reference value) { ++value; }
    {
        ++get();
        return *this;
    }

    constexpr auto operator++(int)
        requires requires (reference value) { value++; }
    {
        return get()++;
    }

    constexpr field_reference& operator--()
        requires requires (reference value) { --value; }
    {
        --get();
        return *this;
    }

    constexpr auto operator--(int)
        requires requires (reference value) { value--; }
    {
        return get()--;
    }
};

template<typename source_type, typename access_type, auto... members>
union generated_view;

template<typename source_type, typename access_type, auto... members>
constexpr decltype(auto) make_reference(access_type access);

template<typename source_type, typename access_type, auto... members>
struct array_reference {
    static_assert(!std::is_array_v<source_type>,
                  "generated union views require std::array or array_field instead of raw arrays");
    static_assert(!is_bool_vector<std::remove_cv_t<source_type>>::value,
                  "generated union views require addressable elements; vector<bool> is unsupported");

    access_type access;

    constexpr explicit array_reference(access_type value) : access(value) {}

    array_reference(array_reference const&) = default;
    array_reference& operator=(array_reference const&) = delete;

    constexpr std::size_t size() const {
        if constexpr (is_array_field_v<source_type>)
            return array_field_traits<std::remove_cv_t<source_type>>::extent;
        else
            return access_field<access_type, members...>(access).size();
    }

    constexpr decltype(auto) operator[](std::size_t index) const {
        auto&& elements = access_field<access_type, members...>(access);
        if constexpr (is_array_field_v<source_type> &&
                      !is_array_field_v<decltype(elements)>) {
            auto element = elements[index];
            return make_reference<typename decltype(element)::value_type>(element);
        } else {
            using reference_type = decltype(elements[index]);
            static_assert(std::is_lvalue_reference_v<reference_type>,
                          "generated union views require addressable array elements");
            using stored_type = std::remove_reference_t<reference_type>;
            using element_type = std::conditional_t<
                std::is_const_v<std::remove_reference_t<decltype(elements)>>,
                std::add_const_t<stored_type>, stored_type>;
            auto element = structured_layout::detail::direct_element_reference<
                element_type>{elements[index]};
            return make_reference<std::remove_cv_t<element_type>>(element);
        }
    }
};

template<typename source_type, typename access_type, auto... members>
struct reference_type {
    using type = std::conditional_t<
        is_indexed_v<source_type>,
        array_reference<source_type, access_type, members...>,
        std::conditional_t<
            schema_kind_v<source_type> == schema_kind::record,
            generated_view<std::remove_cv_t<source_type>, access_type, members...>,
            field_reference<access_type, members...>>>;
};

template<typename source_type, typename access_type, auto... members>
using reference_type_t = typename reference_type<
    source_type, access_type, members...>::type;

template<typename source_type, typename access_type, auto... members>
constexpr decltype(auto) make_reference(access_type access) {
    static_assert(std::is_standard_layout_v<access_type> &&
                  std::is_trivially_copyable_v<access_type>,
                  "generated union views require standard-layout trivially copyable cursors");
    if constexpr (is_indexed_v<source_type>) {
        return array_reference<source_type, access_type, members...>{access};
    } else if constexpr (schema_kind_v<source_type> == schema_kind::record) {
        using result_type = generated_view<
            std::remove_cv_t<source_type>, access_type, members...>;
        if constexpr (requires { sizeof(result_type); }) {
            static_assert(sizeof(result_type) == sizeof(cursor_arm<access_type>),
                          "generated union views must contain only cursor storage");
            static_assert(alignof(result_type) == alignof(cursor_arm<access_type>),
                          "generated union views must preserve cursor alignment");
            static_assert(std::is_standard_layout_v<result_type>);
            static_assert(std::is_trivially_copy_constructible_v<result_type>);
            static_assert(std::is_aggregate_v<result_type>);
            return result_type{{access}};
        } else {
            static_assert(requires { sizeof(result_type); },
                          "generated union record is missing from the manifest or generated header");
        }
    } else {
        return access_field<access_type, members...>(access);
    }
}

}
