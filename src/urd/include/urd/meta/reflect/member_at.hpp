#pragma once
#include <tuple>
#include <urd/meta/reflect/as_tuple.hpp>

namespace urd::reflect {

// Returns a reference to the I-th field of t.
// Constness of t is preserved.
template<size_t I, typename T>
decltype(auto) member_at(T& t) noexcept {
    return std::get<I>(as_tuple(t));
}

} // namespace urd::reflect
