#pragma once
#include <tuple>
#include <type_traits>
#include <urd/meta/reflect/as_tuple.hpp>
#include <urd/meta/reflect/member_count.hpp>
#include <urd/meta/traits/array.hpp>
#include <urd/types/struct_container/soa_layout.hpp>
#include <urd/types/struct_container/stack_storage.hpp>
#include <urd/meta/traits/span.hpp>
#include <urd/types/tuple.hpp>

namespace urd::v2 {

// recursive_layout — layout policy that mirrors struct hierarchy as tuple-of-tuples.
//
// transform<N, StoragePolicy>(field) recurses into aggregate structs instead of
// treating them as atomic units, producing a nested std::tuple that structurally
// mirrors T at every level.
//
// Stopping conditions (leaf → delegate to BaseLayout::transform):
//   - arithmetic / pointer
//   - std::array / urd::array  (urd::is_array_v)
//   - span-like types          (urd::v2::is_span_v)
//   - aggregate with zero instance members (e.g. EdgeRef — only static data)
//
// Recursive case:
//   - aggregate with ≥1 instance members → std::tuple<transform(f0), transform(f1), ...>

template<typename BaseLayout = soa_layout>
struct recursive_layout {
    template<size_t N, typename StoragePolicy = stack_storage, typename Field>
    static constexpr auto transform(Field const&) noexcept {
        using F = std::remove_cvref_t<Field>;

        if constexpr (std::is_arithmetic_v<F>  ||
                      std::is_pointer_v<F>      ||
                      urd::is_array_v<F>         ||
                      urd::v2::is_span_v<F>) {
            return BaseLayout::template transform<N, StoragePolicy>(F{});
        } else if constexpr (std::is_aggregate_v<F>) {
            if constexpr (urd::reflect::member_count_v<F> == 0) {
                // no instance data (e.g. EdgeRef with only static constexpr members)
                return F{};
            } else {
                F proto{};
                auto members = urd::reflect::as_tuple(proto);
                return std::apply([](auto const&... fields) {
                    return urd::v2::make_tuple(
                        recursive_layout::template transform<N, StoragePolicy>(fields)...
                    );
                }, members);
            }
        } else {
            return BaseLayout::template transform<N, StoragePolicy>(F{});
        }
    }
};

} // namespace urd::v2
