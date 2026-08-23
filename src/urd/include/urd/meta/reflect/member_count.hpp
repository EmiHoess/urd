#pragma once
#include <cstddef>
#include <tuple>
#include <urd/meta/reflect/detail/to_any.hpp>
#include <urd/meta/reflect/detail/aggregate_ctor.hpp>

namespace urd::reflect {

namespace detail {

template<typename T, typename... Args>
constexpr size_t member_count_impl() {
    if constexpr (urd::reflect::detail::is_aggregate_ctor<T, std::tuple<Args...>>::value)
        return member_count_impl<T, urd::reflect::detail::to_any, Args...>();
    else
        return sizeof...(Args) - 1;
}

} // namespace detail

template<typename T>
inline constexpr size_t member_count_v =
    detail::member_count_impl<T, detail::to_any>();

} // namespace urd::reflect
