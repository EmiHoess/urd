#pragma once
#include <tuple>
#include <utility>
#include <urd/meta/reflect/as_tuple.hpp>
#include <urd/meta/reflect/member_at.hpp>
#include <urd/meta/reflect/member_count.hpp>

namespace urd::algorithms {

// Calls fn(field) for each field of t, in declaration order.
// fn may be a generic lambda: [](auto& field) { ... }
template<typename T, typename Fn>
void for_each(T& t, Fn&& fn) {
    std::apply([&fn](auto&... fields) {
        (fn(fields), ...);
    }, urd::reflect::as_tuple(t));
}

// Calls fn(field, index) for each field of t, in declaration order.
template<typename T, typename Fn>
void for_each_indexed(T& t, Fn&& fn) {
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        (fn(urd::reflect::member_at<Is>(t), Is), ...);
    }(std::make_index_sequence<urd::reflect::member_count_v<std::remove_cvref_t<T>>>{});
}

} // namespace urd::algorithms
