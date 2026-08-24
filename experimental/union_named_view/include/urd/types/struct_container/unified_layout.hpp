#pragma once
#include <array>
#include <cstdint>
#include <urd/meta/traits/span.hpp>
#include <urd/types/struct_container/soa_layout.hpp>
#include <urd/types/struct_container/stack_storage.hpp>

namespace urd::v2 {

struct unified_layout {
    template<size_t N, typename StoragePolicy = stack_storage, typename Field>
    static auto transform(Field const& f) noexcept {
        using F = std::remove_cvref_t<Field>;
        if constexpr (urd::v2::is_span_v<F>)
            return std::array<uint32_t, N>{};
        else
            return soa_layout::transform<N, StoragePolicy>(f);
    }
};

} // namespace urd::v2
