#pragma once

#include <tuple>
#include <utility>

#include <urd/reflection.hpp>

namespace urd {

template<typename type, typename function>
constexpr void for_each_member(type& value, function&& operation) {
    std::apply(
        [&operation](auto&... members) {
            (operation(members), ...);
        },
        tie_members(value));
}

template<typename type, typename function>
constexpr void for_each_member_indexed(type& value, function&& operation) {
    [&]<std::size_t... indices>(std::index_sequence<indices...>) {
        (operation(member_at<indices>(value), indices), ...);
    }(std::make_index_sequence<member_count_v<type>>{});
}

}
