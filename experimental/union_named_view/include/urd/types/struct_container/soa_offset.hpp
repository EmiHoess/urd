#pragma once
#include <array>
#include <cstddef>
#include <urd/types/tuple.hpp>
#include <urd/types/struct_container/to_tuple.hpp>
#include <urd/types/struct_container/soa_layout.hpp>
#include <urd/types/struct_container/stack_storage.hpp>
#include <urd/meta/reflect/member_index.hpp>

namespace urd::v2 {

namespace detail {

// Byte offset of the Ith element inside urd::v2::tuple<Ts...>,
// computed purely from the types via sizeof/alignof — no construction needed.

constexpr size_t align_up(size_t x, size_t a) noexcept {
    return (x + a - 1) & ~(a - 1);
}

template<size_t I, typename... Ts>
struct tuple_elem_offset;

template<typename Head, typename... Tail>
struct tuple_elem_offset<0, Head, Tail...> {
    static constexpr size_t value = 0;
};

template<size_t I, typename Head, typename... Tail>
struct tuple_elem_offset<I, Head, Tail...> {
    static constexpr size_t tail_start = align_up(sizeof(Head), alignof(urd::v2::tuple<Tail...>));
    static constexpr size_t value = tail_start + tuple_elem_offset<I - 1, Tail...>::value;
};

template<typename... Ts, size_t... Is>
constexpr auto make_offsets_array(std::index_sequence<Is...>) noexcept {
    return std::array<size_t, sizeof...(Ts)>{ tuple_elem_offset<Is, Ts...>::value... };
}

// Unpacks tuple<Ts...> to build the offset array — no value construction.
template<typename TupleType>
struct storage_offsets;

template<typename... Ts>
struct storage_offsets<urd::v2::tuple<Ts...>> {
    static constexpr auto value =
        make_offsets_array<Ts...>(std::make_index_sequence<sizeof...(Ts)>{});
};

} // namespace detail

template<size_t N>
constexpr size_t soa_offset_of(nullptr_t) noexcept { return 0; }

// Returns the byte offset of `member` in the SOA storage produced by
// to_tuple<N, LayoutPolicy, StoragePolicy>.  Uses decltype to derive the
// storage tuple type without constructing it, so the result is a genuine
// compile-time constant even when make_tuple is not constexpr.
template<size_t N,
         typename LayoutPolicy  = soa_layout,
         typename StoragePolicy = stack_storage,
         typename T, typename R>
constexpr size_t soa_offset_of(R T::* member) noexcept {
    using storage_t = decltype(to_tuple<N, LayoutPolicy, StoragePolicy>(std::declval<T&>()));
    constexpr auto offsets = detail::storage_offsets<storage_t>::value;
    return offsets[urd::reflect::member_index(member)];
}

} // namespace urd::v2
