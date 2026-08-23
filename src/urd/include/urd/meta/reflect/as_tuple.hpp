#pragma once
#include <urd/meta/reflect/member_count.hpp>
#include <urd/meta/reflect/detail/struct_tie.hpp>

namespace urd::reflect {

// Returns a std::tuple of lvalue references to each field of t.
// Constness of t is preserved: as_tuple(const T&) yields tuple<F0 const&, ...>.
template<typename T>
constexpr auto as_tuple(T& t) noexcept {
    return detail::struct_tie<member_count_v<std::remove_cvref_t<T>>>(t);
}

} // namespace urd::reflect
