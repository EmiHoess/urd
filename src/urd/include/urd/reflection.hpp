#pragma once

#include <cstddef>
#include <type_traits>

#include <urd/meta/reflect/reflect.hpp>

namespace urd {

template<typename type>
inline constexpr std::size_t member_count_v =
    reflect::member_count_v<std::remove_cvref_t<type>>;

template<typename type>
constexpr auto tie_members(type& value) noexcept {
    return reflect::as_tuple(value);
}

template<std::size_t index, typename type>
constexpr decltype(auto) member_at(type& value) noexcept {
    return reflect::member_at<index>(value);
}

template<typename owner, typename member>
constexpr std::size_t member_index(member owner::* pointer) {
    return reflect::member_index(pointer);
}

}
