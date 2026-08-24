#pragma once
#include <vector>
#include <utility>
#include <urd/types/tuple.hpp>
#include <urd/types/struct_container/pool.hpp>

// compact() — convert deferred storage to unified storage.
//
// Takes an urd::v2::tuple produced by deferred_layout and returns a single
// flat urd::v2::tuple:
//   urd::v2::tuple<
//     ...,          unified fields (span fields → std::array<uint32_t,N>)
//     pool<T0>,     one pool per span field, appended in field order
//     pool<T1>, ...
//   >
//
// expand() — reverse: unified + pools → deferred.
//
// Example:
//   auto deferred = urd::v2::to_tuple<N, deferred_layout, stack_storage>(proto);
//   deferred.weights[5].push_back(1.0f);            // edit freely
//   auto unified = urd::v2::compact(deferred);       // snapshot for GPU

namespace urd::v2 {

namespace detail {

// --- compact_one ---

// Non-deferred field: pass through unchanged, contribute empty pool tuple.
template<typename T>
auto compact_one(T const& field)
    -> std::pair<T, urd::v2::tuple<>>
{
    return {field, {}};
}

// Deferred field std::array<std::vector<T>, N>:
// flatten all per-slot vectors into a pool, produce index array.
template<typename T, size_t N>
auto compact_one(std::array<std::vector<T>, N> const& field)
    -> std::pair<std::array<uint32_t, N>, urd::v2::tuple<pool<T>>>
{
    pool<T> p;
    p.offsets.resize(N);
    p.counts.resize(N);
    std::array<uint32_t, N> indices{};

    uint32_t offset = 0;
    for (size_t i = 0; i < N; ++i) {
        p.offsets[i] = offset;
        p.counts[i]  = static_cast<uint32_t>(field[i].size());
        indices[i]   = offset;
        p.data.insert(p.data.end(), field[i].begin(), field[i].end());
        offset      += p.counts[i];
    }
    return {std::move(indices), urd::v2::make_tuple(std::move(p))};
}

// --- expand_one ---

// Non-index field: pass through unchanged.
template<typename T>
T expand_one(T const& field, urd::v2::tuple<> const&) { return field; }

// Index field std::array<uint32_t, N> + pool<T>:
// rebuild per-slot vectors from the pool.
template<typename T, size_t N>
std::array<std::vector<T>, N> expand_one(
    std::array<uint32_t, N> const& /*indices*/, urd::v2::tuple<pool<T>> const& pool_tuple)
{
    auto const& p = urd::v2::get<0>(pool_tuple);
    std::array<std::vector<T>, N> result;
    for (size_t i = 0; i < N; ++i) {
        auto const* src = p.slot_data(i);
        result[i].assign(src, src + p.slot_count(i));
    }
    return result;
}

// --- compact_impl ---

template<typename... Ts, size_t... Is>
auto compact_impl(urd::v2::tuple<Ts...> const& deferred, std::index_sequence<Is...>)
{
    auto results = urd::v2::make_tuple(compact_one(urd::v2::get<Is>(deferred))...);

    auto unified = urd::v2::make_tuple(std::move(urd::v2::get<Is>(results).first)...);
    auto pools   = urd::v2::tuple_cat(std::move(urd::v2::get<Is>(results).second)...);

    return urd::v2::tuple_cat(std::move(unified), std::move(pools));
}

} // namespace detail

// Convert deferred → (unified tuple, pool tuple).
template<typename... Ts>
auto compact(urd::v2::tuple<Ts...> const& deferred)
{
    return detail::compact_impl(deferred, std::index_sequence_for<Ts...>{});
}

} // namespace urd::v2
