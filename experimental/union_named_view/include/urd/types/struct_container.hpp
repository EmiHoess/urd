#pragma once
#include <vector>
#include <cstddef>
#include <utility>
#include <type_traits>
#include <urd/types/struct_container/to_tuple.hpp>
#include <urd/types/struct_container/ref_policy.hpp>
#include <urd/meta/reflect/member_count.hpp>
#include <urd/meta/reflect/detail/struct_tie.hpp>
#include <urd/types/struct_container/soa_layout.hpp>
#include <urd/types/struct_container/deferred_layout.hpp>
#include <urd/types/struct_container/unified_layout.hpp>
#include <urd/types/struct_container/stack_storage.hpp>
#include <urd/types/struct_container/heap_storage.hpp>
#include <urd/types/struct_container/compact.hpp>

namespace urd::v2 {

namespace detail {

template<typename ElemType, typename FieldRef>
void push_field(std::vector<ElemType>& vec, FieldRef const& v) {
    vec.push_back(static_cast<ElemType>(v));
}

template<typename ElemType, typename SpanElem>
void push_field(std::vector<std::vector<ElemType>>& vec, ::span<SpanElem> const& s) {
    vec.push_back(std::vector<ElemType>(s.mem, s.mem + s.size));
}

template<typename StorageTuple, typename RefTuple, size_t... Is>
void push_all(StorageTuple& s, RefTuple& refs, std::index_sequence<Is...>) {
    (push_field(urd::v2::get<Is>(s), std::get<Is>(refs)), ...);
}

template<typename StorageTuple, size_t... Is>
void pop_all(StorageTuple& s, std::index_sequence<Is...>) {
    (urd::v2::get<Is>(s).pop_back(), ...);
}

template<typename StorageTuple, size_t... Is>
void reserve_all(StorageTuple& s, size_t n, std::index_sequence<Is...>) {
    (urd::v2::get<Is>(s).reserve(n), ...);
}

template<typename StorageTuple, size_t... Is>
void clear_all(StorageTuple& s, std::index_sequence<Is...>) {
    (urd::v2::get<Is>(s).clear(), ...);
}

template<typename T, size_t N, typename LayoutPolicy, typename StoragePolicy>
struct storage_type_for {
    using _deferred = std::remove_cvref_t<
        decltype(urd::v2::to_tuple<N, deferred_layout, stack_storage>(std::declval<T&>()))>;
    using _direct   = std::remove_cvref_t<
        decltype(urd::v2::to_tuple<N, LayoutPolicy, StoragePolicy>(std::declval<T&>()))>;
    using _unified  = std::remove_cvref_t<
        decltype(urd::v2::compact(std::declval<_deferred const&>()))>;

    using type = std::conditional_t<std::is_same_v<LayoutPolicy, unified_layout>,
                                    _unified, _direct>;
};

template<typename T, size_t N, typename LayoutPolicy, typename StoragePolicy>
using storage_type_for_t =
    typename storage_type_for<T, N, LayoutPolicy, StoragePolicy>::type;

} // namespace detail

// ---------------------------------------------------------------------------

template<typename T, size_t N,
         typename LayoutPolicy  = deferred_layout,
         typename StoragePolicy = stack_storage>
struct struct_container {
    using storage_type = detail::storage_type_for_t<T, N, LayoutPolicy, StoragePolicy>;

    static constexpr size_t field_count = urd::reflect::member_count_v<T>;
    static_assert(field_count > 0, "T must have at least one field");

    storage_type storage;

    // size() / empty() — constexpr for stack_storage, runtime for heap_storage
    constexpr auto size() const noexcept {
        if constexpr (std::is_same_v<StoragePolicy, heap_storage>)
            return urd::v2::get<0>(storage).size();
        else
            return N;
    }

    auto capacity() const noexcept
        requires std::is_same_v<StoragePolicy, heap_storage>
    {
        return urd::v2::get<0>(storage).capacity();
    }

    constexpr bool empty() const noexcept {
        if constexpr (std::is_same_v<StoragePolicy, heap_storage>)
            return urd::v2::get<0>(storage).empty();
        else
            return N == 0;
    }

    // operator[] — disabled for unified_layout (pool indexing has no single-element semantics)
    auto operator[](size_t i)
        requires (!std::is_same_v<LayoutPolicy, unified_layout>)
    {
        return ref_policy::element(storage, i, std::make_index_sequence<field_count>{});
    }

    auto operator[](size_t i) const
        requires (!std::is_same_v<LayoutPolicy, unified_layout>)
    {
        return ref_policy::element(storage, i, std::make_index_sequence<field_count>{});
    }

    // push_back / pop_back / reserve / clear — heap_storage only
    void push_back(T const& t)
        requires std::is_same_v<StoragePolicy, heap_storage>
    {
        auto refs = urd::reflect::detail::struct_tie<field_count>(const_cast<T&>(t));
        detail::push_all(storage, refs, std::make_index_sequence<field_count>{});
    }

    void pop_back()
        requires std::is_same_v<StoragePolicy, heap_storage>
    {
        detail::pop_all(storage, std::make_index_sequence<field_count>{});
    }

    void reserve(size_t n)
        requires std::is_same_v<StoragePolicy, heap_storage>
    {
        detail::reserve_all(storage, n, std::make_index_sequence<field_count>{});
    }

    void clear()
        requires std::is_same_v<StoragePolicy, heap_storage>
    {
        detail::clear_all(storage, std::make_index_sequence<field_count>{});
    }

    // Constructor from deferred — only valid for unified_layout
    struct_container(
        struct_container<T, N, deferred_layout, stack_storage> const& d)
        requires std::is_same_v<LayoutPolicy, unified_layout>
        : storage(urd::v2::compact(d.storage)) {}

    struct_container() = default;

    template<size_t I>
    auto& get() noexcept { return urd::v2::get<I>(storage); }

    template<size_t I>
    auto const& get() const noexcept { return urd::v2::get<I>(storage); }
};

// --- aliases -----------------------------------------------------------------

template<typename T, size_t N>
using deferred_container = struct_container<T, N, deferred_layout, stack_storage>;

template<typename T, size_t N>
using unified_container = struct_container<T, N, unified_layout, stack_storage>;

template<typename T>
using dynamic_container = struct_container<T, 0, soa_layout, heap_storage>;

// --- make_struct_container ---------------------------------------------------

template<size_t N, typename T>
auto make_struct_container(T const&) {
    return struct_container<std::remove_cvref_t<T>, N>{};
}

template<size_t N = 1, typename T, size_t M>
auto make_struct_container(std::array<T, M> const&) {
    return struct_container<T, N * M>{};
}

} // namespace urd::v2
