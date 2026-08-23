#pragma once
#include <tuple>
#include <type_traits>

namespace urd::reflect::detail {

template<typename T, typename... Args>
auto aggregate_ctor_probe(std::tuple<Args...>)
    -> decltype(T(std::declval<Args>()...));

template<typename T, typename ArgTuple, typename = void>
struct is_aggregate_ctor : std::false_type {};

template<typename T, typename ArgTuple>
struct is_aggregate_ctor<T, ArgTuple,
    std::void_t<decltype(aggregate_ctor_probe<T>(std::declval<ArgTuple>()))>>
    : std::true_type {};

} // namespace urd::reflect::detail
