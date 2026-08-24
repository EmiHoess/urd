#pragma once

#include <array>
#include <type_traits>

#include <urd/traits/array.hpp>
#include <urd/meta/traits/layout.hpp>
#include <urd/types/struct_container/stack_storage.hpp>
#include <urd/types/struct_container/to_tuple.hpp>

namespace urd::v2 {

struct soa_layout {
    template<size_t N, typename StoragePolicy = stack_storage, typename Field>
    static constexpr auto transform(Field const&) noexcept {
        using F = std::remove_cvref_t<Field>;

        if constexpr (std::is_arithmetic_v<F> || std::is_pointer_v<F>)
        {
            return typename StoragePolicy::template container<F, N>{};
        }
        else if constexpr (meh::is_urd_array<F>::value)
        {
            using elem_t = std::remove_cvref_t<meh::collection_item_type<F>>;
            constexpr size_t M = F::size();
            if constexpr (std::is_same_v<typename F::memory_layout, urd::layout::aos>)
                return typename StoragePolicy::template container<elem_t, N * M>{};
            else
            {
                elem_t proto{};
                return transform<N * M, StoragePolicy>(proto);
            }
        }
        else if constexpr (meh::is_array<F>::value)
        {
            using elem_t = std::remove_cvref_t<meh::collection_item_type<F>>;
            constexpr size_t M = std::tuple_size_v<F>;

            if constexpr (meh::is_array<elem_t>::value)
            {
                elem_t proto{};
                return transform<N * M, StoragePolicy>(proto);
            }
            else if constexpr (urd::has_soa_layout_v<elem_t>)
            {
                elem_t proto{};
                return urd::v2::to_tuple<N * M, soa_layout, StoragePolicy>(proto);
            }
            else
                return typename StoragePolicy::template container<elem_t, N * M>{};
        }
        else if constexpr (urd::has_soa_layout_v<F>)
        {
            F proto{};
            return urd::v2::to_tuple<N, soa_layout, StoragePolicy>(proto);
        }
        else
            return typename StoragePolicy::template container<F, N>{};
    }
};

} // namespace urd::v2
