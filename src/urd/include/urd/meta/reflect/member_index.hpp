#pragma once
#include <cstddef>
#include <utility>
#include <urd/meta/reflect/as_tuple.hpp>
#include <urd/meta/reflect/member_count.hpp>

namespace urd::reflect {

namespace detail {

template<typename T, size_t... Is>
constexpr size_t member_index_impl(T& proto, void const* target, std::index_sequence<Is...>) {
    size_t result = member_count_v<T>; // sentinel: not found
    auto refs = as_tuple(proto);
    static_cast<void>(((static_cast<void const*>(&std::get<Is>(refs)) == target
        ? (result = Is, true) : false) || ...));
    return result;
}

} // namespace detail

// Returns the zero-based index of the field pointed to by ptr.
// Returns member_count_v<T> if the pointer does not match any field.
//
// Example:
//   struct Point { float x, y, z; };
//   member_index<Point>(&Point::y)  →  1
template<typename T, typename R>
constexpr size_t member_index(R T::* ptr) {
    T proto{};
    void const* target = static_cast<void const*>(&(proto.*ptr));
    return detail::member_index_impl(
        proto, target,
        std::make_index_sequence<member_count_v<T>>{});
}

} // namespace urd::reflect
