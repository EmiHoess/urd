#pragma once
#include <cstddef>
#include <type_traits>

// Standard-layout heterogeneous tuple.
//
// tuple<Ts...> is standard-layout (and POD/trivial) whenever all Ts are.
// No multiple inheritance, no virtual functions, no user-provided constructors.
//
// Limitations:
//  - Reference members (T&) make the struct non-standard-layout; use for value
//    storage only. For reflection/tie use meh::tie_tuple (unchanged).
//  - get<I> is O(I) in template depth; fine for small field counts.

namespace urd::v2 {

// --- tuple ----------------------------------------------------------------

template<typename... Ts>
struct tuple;

template<>
struct tuple<> {
    static constexpr std::size_t size_v = 0;
};

template<typename Head, typename... Tail>
struct tuple<Head, Tail...> {
    static constexpr std::size_t size_v = 1 + sizeof...(Tail);

    Head          head;
    tuple<Tail...> tail;
};

// --- get ------------------------------------------------------------------

template<std::size_t I, typename Head, typename... Tail>
constexpr decltype(auto) get(tuple<Head, Tail...>& t) noexcept {
    if constexpr (I == 0) return (t.head);
    else                  return get<I - 1>(t.tail);
}

template<std::size_t I, typename Head, typename... Tail>
constexpr decltype(auto) get(const tuple<Head, Tail...>& t) noexcept {
    if constexpr (I == 0) return (t.head);
    else                  return get<I - 1>(t.tail);
}

// --- make_tuple -----------------------------------------------------------

inline auto make_tuple() -> tuple<> { return {}; }

template<typename T>
auto make_tuple(T&& v) -> tuple<std::remove_cvref_t<T>> {
    return {std::forward<T>(v), {}};
}

template<typename T, typename... Ts>
auto make_tuple(T&& v, Ts&&... rest) -> tuple<std::remove_cvref_t<T>, std::remove_cvref_t<Ts>...> {
    return {std::forward<T>(v), make_tuple(std::forward<Ts>(rest)...)};
}

// --- tuple_cat ------------------------------------------------------------

inline auto tuple_cat() -> tuple<> { return {}; }

template<typename... Ts>
auto tuple_cat(tuple<Ts...> t) -> tuple<Ts...> { return t; }

template<typename... As, typename... Bs, std::size_t... IAi, std::size_t... IBi>
auto tuple_cat_impl(tuple<As...> a, tuple<Bs...> b,
                    std::index_sequence<IAi...>, std::index_sequence<IBi...>)
    -> tuple<As..., Bs...>
{
    return make_tuple(std::move(get<IAi>(a))..., std::move(get<IBi>(b))...);
}

template<typename... As, typename... Bs>
auto tuple_cat(tuple<As...> a, tuple<Bs...> b) -> tuple<As..., Bs...>
{
    return tuple_cat_impl(
        std::move(a), std::move(b),
        std::make_index_sequence<sizeof...(As)>{},
        std::make_index_sequence<sizeof...(Bs)>{});
}

template<typename T1, typename T2, typename... Rest>
auto tuple_cat(T1 t1, T2 t2, Rest... rest)
{
    return tuple_cat(tuple_cat(std::move(t1), std::move(t2)), std::move(rest)...);
}

// --- type_at<I> -----------------------------------------------------------
// Retrieve the Ith type without instantiating a tuple value.

template<std::size_t I, typename... Ts>
struct type_at;

template<typename Head, typename... Tail>
struct type_at<0, Head, Tail...> {
    using type = Head;
};

template<std::size_t I, typename Head, typename... Tail>
struct type_at<I, Head, Tail...> {
    using type = typename type_at<I - 1, Tail...>::type;
};

template<std::size_t I, typename... Ts>
using type_at_t = typename type_at<I, Ts...>::type;

// --- structured binding support -------------------------------------------
// Enables: auto [x, y, z] = my_tuple;
// Requires std::tuple_size, std::tuple_element, and ADL get<I> in urd::v2.

} // namespace urd::v2

namespace std {

template<typename... Ts>
struct tuple_size<urd::v2::tuple<Ts...>>
    : integral_constant<size_t, sizeof...(Ts)> {};

template<size_t I, typename... Ts>
struct tuple_element<I, urd::v2::tuple<Ts...>> {
    using type = urd::v2::type_at_t<I, Ts...>;
};

} // namespace std

namespace urd::v2 {

// --- static_assert proofs -------------------------------------------------

static_assert(std::is_standard_layout_v<tuple<>>);
static_assert(std::is_standard_layout_v<tuple<int>>);
static_assert(std::is_standard_layout_v<tuple<int, float>>);
static_assert(std::is_standard_layout_v<tuple<int, float, double>>);

static_assert(std::is_trivial_v<tuple<>>);
static_assert(std::is_trivial_v<tuple<int>>);
static_assert(std::is_trivial_v<tuple<int, float>>);

} // namespace urd::v2
