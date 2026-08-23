#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace urd {

template<typename... types>
struct tuple;

template<>
struct tuple<> {};

template<typename first, typename... rest>
struct tuple<first, rest...> {
    first head;
    tuple<rest...> tail;
};

template<std::size_t index, typename first, typename... rest>
constexpr decltype(auto) get(tuple<first, rest...>& value) noexcept {
    if constexpr (index == 0)
        return (value.head);
    else
        return get<index - 1>(value.tail);
}

template<std::size_t index, typename first, typename... rest>
constexpr decltype(auto) get(tuple<first, rest...> const& value) noexcept {
    if constexpr (index == 0)
        return (value.head);
    else
        return get<index - 1>(value.tail);
}

template<std::size_t index, typename first, typename... rest>
constexpr decltype(auto) get(tuple<first, rest...>&& value) noexcept {
    if constexpr (index == 0)
        return std::move(value.head);
    else
        return get<index - 1>(std::move(value.tail));
}

template<typename... values>
constexpr auto make_tuple(values&&... items)
    -> tuple<std::remove_cvref_t<values>...> {
    if constexpr (sizeof...(values) == 0) {
        return {};
    } else {
        return [&]<typename first, typename... rest>(first&& head, rest&&... tail) {
            return tuple<std::remove_cvref_t<first>, std::remove_cvref_t<rest>...>{
                std::forward<first>(head),
                make_tuple(std::forward<rest>(tail)...)};
        }(std::forward<values>(items)...);
    }
}

template<std::size_t index, typename... types>
using tuple_element_t = std::tuple_element_t<index, tuple<types...>>;

}

namespace std {

template<typename... types>
struct tuple_size<urd::tuple<types...>>
    : integral_constant<size_t, sizeof...(types)> {};

template<size_t index, typename first, typename... rest>
struct tuple_element<index, urd::tuple<first, rest...>>
    : tuple_element<index - 1, urd::tuple<rest...>> {};

template<typename first, typename... rest>
struct tuple_element<0, urd::tuple<first, rest...>> {
    using type = first;
};

}
