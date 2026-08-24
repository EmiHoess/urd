#pragma once

#include <utility>
#include <urd/types/struct_container/view/alias.hpp>
#include <urd/types/struct_container/view/alias_size.hpp>
#include <urd/types/struct_container/view/array_args.hpp>
#include <urd/types/struct_container/view/array_index.hpp>
#include <urd/types/struct_container/view/element_ref.hpp>
#include <urd/types/struct_container.hpp>
#include <urd/meta/traits/array.hpp>
#include <urd/meta/traits/collection.hpp>
#include <urd/meta/traits/layout.hpp>
#include <urd/macros/platform.hpp>

namespace urd::v2 {

// ---------------------------------------------------------------------------
// array_view
// ---------------------------------------------------------------------------
//
// Non-owning view over a struct_container, returning
//   elem_alias< alias_size<0, N, T> >
// from operator[](i) — the same type the old contrafactum array_view returned.
//
// Also handles inner array traversal: when alias::aliased_type is a
// std::array<T,M> or urd::array<T,M> field (detected via urd::is_array_v),
// operator[](i) computes the sub-element index and descends into element i,
// replacing the need for a separate urd_array_view type.
//
// Why the reinterpret_cast is safe:
//   struct_container::storage is a urd::v2::tuple<array<F0,N>, array<F1,N>,...>
//   urd::v2::tuple is a standard-layout recursive struct:
//     head F0[N] at byte 0
//     head F1[N] at byte N*sizeof(F0)
//     ...
//   get_soa_offset_of<N>(member_ptr) computes exactly those offsets (sum of
//   N*sizeof(field_j) for each preceding field), so the alias chain reads the
//   correct bytes.

template<typename alias, typename TARGS>
struct array_view {
    urd::v2::array_index<typename alias::aliased_type, alias::previous_N> index;

    // --- inner-array detection -----------------------------------------------

    static constexpr bool   is_array_field = urd::is_array_v<typename alias::aliased_type>;
    static constexpr size_t size_of_array  = urd::collection_size<typename alias::aliased_type>();
    static constexpr bool   arty_is_soa    =
        urd::has_soa_layout_v<typename alias::aliased_type>;

    // element type when descending into an array field; aliased_type itself otherwise
    using arty = std::conditional_t<is_array_field,
        urd::collection_item_type<typename alias::aliased_type>,
        typename alias::aliased_type>;

    // N for the inner alias produced by operator[] in array-field mode:
    //   SOA sub-array  → previous_N * array_size  (parallel arrays stay parallel)
    //   non-SOA array  → array_size               (one contiguous block)
    static constexpr size_t next_N = is_array_field
        ? (arty_is_soa ? alias::previous_N * size_of_array : size_of_array)
        : alias::previous_N;

    // inner alias used as the template argument for element_alias in array-field mode
    using _inner_alias = urd::v2::rename_type<alias::current_offset, next_N, nullptr, arty>;

    // --- return type ---------------------------------------------------------

    using _atomic4       = urd::v2::element_ref<alias,        alias::current_coeff, alias::previous_N>;
    // fixed_off encodes the SOA base offset exactly; coeff=0 so no N-multiply loss.
    using _atomic4_inner = urd::v2::element_ref<_inner_alias, 0, next_N, alias::current_offset>;

    using return_type = std::conditional_t<
        is_array_field,
        std::conditional_t<TARGS::is_atomic, _atomic4_inner, typename TARGS::template element_alias<_inner_alias>>,
        std::conditional_t<TARGS::is_atomic, _atomic4,       typename TARGS::template element_alias<alias>>
    >;

    // --- operator[] ----------------------------------------------------------

    __device_host__ return_type operator[](size_t i) {
        if constexpr (is_array_field) {
            size_t new_i = arty_is_soa ? index.i + i * alias::previous_N
                                       : index.i * size_of_array + i;
            if constexpr (TARGS::is_atomic) {
                return _atomic4_inner{ .index = { new_i, index.ref_collection } };
            } else {
                urd::v2::array_index<arty, next_N> new_index{ new_i, index.ref_collection };
                return return_type{{new_index}};
            }
        } else if constexpr (TARGS::is_atomic) {
            return _atomic4{ .index = { i, index.ref_collection } };
        } else {
            urd::v2::array_index<typename alias::aliased_type, alias::previous_N> new_index{
                i, index.ref_collection
            };
            return return_type{{new_index}};
        }
    }

    __device_host__ return_type operator[](size_t i) const {
        if constexpr (is_array_field) {
            size_t new_i = arty_is_soa ? index.i + i * alias::previous_N
                                       : index.i * size_of_array + i;
            if constexpr (TARGS::is_atomic) {
                return _atomic4_inner{ .index = { new_i, index.ref_collection } };
            } else {
                urd::v2::array_index<arty, next_N> new_index{ new_i, index.ref_collection };
                return return_type{{new_index}};
            }
        } else if constexpr (TARGS::is_atomic) {
            return _atomic4{ .index = { i, index.ref_collection } };
        } else {
            urd::v2::array_index<typename alias::aliased_type, alias::previous_N> new_index{
                i, index.ref_collection
            };
            return return_type{{new_index}};
        }
    }

    constexpr size_t size() const noexcept { return alias::previous_N; }
};

// ---------------------------------------------------------------------------
// make_view
// ---------------------------------------------------------------------------

template<alias_header typename elem_alias = no_template,
         typename T, size_t N,
         typename LayoutPolicy  = deferred_layout,
         typename StoragePolicy = stack_storage>
    requires (!urd::is_array_v<T>)
auto make_view(struct_container<T, N, LayoutPolicy, StoragePolicy>& c)
{
    using alias = urd::v2::alias_size<0, N, T>;
    return array_view<alias, urd::array_args<elem_alias, urd::array_layout::soa>>
    {
        .index = {
            .i              = 0,
            .ref_collection = reinterpret_cast<uint8_t*>(&c.storage)
        }
    };
}

} // namespace urd::v2
