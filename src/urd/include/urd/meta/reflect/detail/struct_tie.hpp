#pragma once
#include <tuple>
#include <type_traits>
#include <urd/macros/repeat.hpp>
#include <urd/meta/reflect/member_count.hpp>

// Generates struct_tie_n overloads for 1–20 fields.
// Each overload decomposes an aggregate via structured bindings
// and returns a std::tuple of lvalue references.
// T deduces as const-qualified for const inputs — constness is preserved.

namespace urd::reflect::detail {

#define URD_REFLECT_FIELD(n)    m##n
#define URD_REFLECT_FIELDS(K)   REPEAT_BY(URD_REFLECT_FIELD, COMMA, K)

#define URD_REFLECT_DEFINE_STRUCT_TIE(K)                                       \
template<typename T>                                                            \
constexpr auto struct_tie_n(T& val, std::integral_constant<size_t, K>) noexcept { \
    auto& [URD_REFLECT_FIELDS(K)] = val;                                        \
    return std::tie(URD_REFLECT_FIELDS(K));                                     \
};

FOR_EACH(URD_REFLECT_DEFINE_STRUCT_TIE,
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20)

#undef URD_REFLECT_DEFINE_STRUCT_TIE
#undef URD_REFLECT_FIELDS
#undef URD_REFLECT_FIELD

template<size_t N, typename T>
constexpr auto struct_tie(T& val) noexcept {
    return struct_tie_n(val, std::integral_constant<size_t, N>{});
}

} // namespace urd::reflect::detail
