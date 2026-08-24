#pragma once

#include <utility>
#include <type_traits>
#include <urd/types/tuple.hpp>
#include <urd/meta/reflect/member_count.hpp>
#include <urd/meta/reflect/detail/struct_tie.hpp>

namespace urd::v2 {

namespace detail {

// refs is a std::tuple (from urd::reflect::detail::struct_tie via std::tie)
template<typename LayoutPolicy, size_t N, typename StoragePolicy,
         typename RefTuple, size_t... Is>
constexpr auto apply_policy(RefTuple& refs, std::index_sequence<Is...>) {
    return urd::v2::make_tuple(
        LayoutPolicy::template transform<N, StoragePolicy>(std::get<Is>(refs))...);
}

} // namespace detail

// Decompose prototype into semantic fields via urd::reflect::detail::struct_tie,
// then apply LayoutPolicy::transform<N, StoragePolicy> to each field.
template<size_t N, typename LayoutPolicy, typename StoragePolicy, typename T>
constexpr auto to_tuple(T& prototype) {
    constexpr size_t count = urd::reflect::member_count_v<T>;
    auto refs = urd::reflect::detail::struct_tie<count>(prototype);
    return detail::apply_policy<LayoutPolicy, N, StoragePolicy>(
        refs, std::make_index_sequence<count>{});
}

} // namespace urd::v2
