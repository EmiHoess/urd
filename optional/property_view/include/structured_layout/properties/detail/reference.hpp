#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include <structured_layout/detail/storage.hpp>
#include <structured_layout/schema.hpp>

namespace structured_layout::properties::detail {

template<typename type>
struct is_vector : std::false_type {};

template<typename type, typename allocator>
struct is_vector<std::vector<type, allocator>> : std::true_type {};

template<typename type>
inline constexpr bool is_indexed_v =
    is_array_field_v<type> || is_std_array_v<type> || is_std_span_v<type> ||
    is_vector<std::remove_cv_t<type>>::value || std::is_array_v<type>;

template<typename access_type, auto... members>
constexpr decltype(auto) access_field(access_type const& access) {
    if constexpr (sizeof...(members) == 0)
        return access.value();
    else
        return access.template get<members...>();
}

template<typename source_type, typename access_type, auto... members>
class generated_view;

template<typename source_type, typename access_type, auto... members>
constexpr decltype(auto) make_property_reference(access_type access);

template<typename source_type, typename access_type, auto... members>
class array_reference {
public:
    constexpr explicit array_reference(access_type access) noexcept
        : access_(access) {}

    array_reference(array_reference const&) = default;
    array_reference& operator=(array_reference const&) = delete;

    constexpr std::size_t size() const {
        if constexpr (is_array_field_v<source_type>)
            return array_field_traits<std::remove_cv_t<source_type>>::extent;
        else if constexpr (std::is_array_v<source_type>)
            return std::extent_v<source_type>;
        else
            return access_field<access_type, members...>(access_).size();
    }

    constexpr decltype(auto) operator[](std::size_t index) const {
        auto&& elements = access_field<access_type, members...>(access_);
        if constexpr (is_array_field_v<source_type> &&
                      !is_array_field_v<decltype(elements)>) {
            auto element = elements[index];
            return make_property_reference<typename decltype(element)::value_type>(
                element);
        } else {
            using reference_type = decltype(elements[index]);
            static_assert(std::is_lvalue_reference_v<reference_type>,
                          "property views require addressable array elements; vector<bool> is unsupported");
            using stored_type = std::remove_reference_t<reference_type>;
            using element_type = std::conditional_t<
                std::is_const_v<std::remove_reference_t<decltype(elements)>>,
                std::add_const_t<stored_type>, stored_type>;
            auto element = structured_layout::detail::direct_element_reference<
                element_type>{elements[index]};
            return make_property_reference<std::remove_cv_t<element_type>>(element);
        }
    }

private:
    access_type access_;
};

template<typename source_type, typename access_type, auto... members>
constexpr decltype(auto) make_property_reference(access_type access) {
    if constexpr (is_indexed_v<source_type>) {
        return array_reference<source_type, access_type, members...>{access};
    } else if constexpr (schema_kind_v<source_type> == schema_kind::record) {
        using result_type = generated_view<
            std::remove_cv_t<source_type>, access_type, members...>;
        static_assert(sizeof(result_type) == sizeof(access_type),
                      "property views must contain only cursor storage");
        static_assert(alignof(result_type) == alignof(access_type),
                      "property views must preserve cursor alignment");
        return result_type{access};
    } else {
        return access_field<access_type, members...>(access);
    }
}

}
