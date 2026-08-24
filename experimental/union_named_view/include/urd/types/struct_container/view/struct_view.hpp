#pragma once

#include <span>
#include <urd/types/struct_container/view/alias_size.hpp>
#include <urd/types/struct_container/view/array_index.hpp>
#include <urd/types/struct_container/view/array_args.hpp>
#include <urd/types/struct_container.hpp>
#include <urd/meta/traits/array.hpp>
#include <urd/macros/platform.hpp>

namespace urd::v2 {

// struct_view — non-owning view over a struct_container<Type, N> where Type
// is a plain struct (not std::array / urd::array).
//
// operator[](i) sets the element index to i and forwards the alias to
// elem_alias for named field access via the contrafactum dub mechanism.

template<typename alias, typename TARGS>
struct struct_view {
    urd::v2::array_index<typename alias::aliased_type, alias::previous_N> index;

    using return_type = typename TARGS::template element_alias<alias>;

    __device_host__ return_type operator[](size_t i) {
        if constexpr (alias::previous_N == std::dynamic_extent) {
            urd::v2::array_index<typename alias::aliased_type, std::dynamic_extent> new_index{
                i, index.n, index.ref_collection
            };
            return return_type{{new_index}};
        } else {
            urd::v2::array_index<typename alias::aliased_type, alias::previous_N> new_index{
                i, index.ref_collection
            };
            return return_type{{new_index}};
        }
    }

    __device_host__ return_type operator[](size_t i) const {
        if constexpr (alias::previous_N == std::dynamic_extent) {
            urd::v2::array_index<typename alias::aliased_type, std::dynamic_extent> new_index{
                i, index.n, index.ref_collection
            };
            return return_type{{new_index}};
        } else {
            urd::v2::array_index<typename alias::aliased_type, alias::previous_N> new_index{
                i, index.ref_collection
            };
            return return_type{{new_index}};
        }
    }

    uint8_t* data_ptr() const noexcept { return index.ref_collection; }

    constexpr size_t size() const noexcept {
        if constexpr (alias::previous_N == std::dynamic_extent)
            return index.n;
        else
            return alias::previous_N;
    }
};

// make_view for non-array plain struct types backed by a struct_container.
// Usage:
//   struct_container<MyStruct, N, soa_layout, stack_storage> c;
//   auto v = urd::v2::make_view<my_alias>(c);
//   v[i].field = value;

template<alias_header typename elem_alias = no_template,
         typename Type, size_t N,
         typename LayoutPolicy  = deferred_layout,
         typename StoragePolicy = stack_storage>
    requires (!urd::is_array_v<Type>)
auto make_struct_view(struct_container<Type, N, LayoutPolicy, StoragePolicy>& c)
{
    using alias = urd::v2::alias_size<0, N, Type>;
    return struct_view<alias, urd::array_args<elem_alias, urd::array_layout::soa>>
    {
        .index = {
            .i              = 0,
            .ref_collection = reinterpret_cast<uint8_t*>(&c.storage)
        }
    };
}

// make_pool_struct_view — view over a flat contiguous SOA buffer with runtime N.
// The buffer must hold n elements laid out as to_tuple<n, soa_layout> would produce.
// Offsets are computed at access time using field coefficients × n.
template<alias_header typename elem_alias = no_template, typename Type>
    requires (!urd::is_array_v<Type>)
auto make_pool_struct_view(uint8_t* buffer, size_t n) {
    using alias = urd::v2::alias_size<0, std::dynamic_extent, Type>;
    return struct_view<alias, urd::array_args<elem_alias, urd::array_layout::soa>>
    {
        .index = {
            .i              = 0,
            .n              = n,
            .ref_collection = buffer
        }
    };
}

} // namespace urd::v2
