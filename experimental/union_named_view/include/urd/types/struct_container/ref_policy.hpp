#pragma once
#include <urd/types/tuple.hpp>

// ref_policy — element-access policy for SOA containers.
//
// tie(args...) constructs tuple<T0&, T1&, ...> binding to the arguments.
// T deduces as const-qualified for const lvalues, so one function covers
// both mutable and read-only access.
//
// ref_policy wraps tie() as a named policy so operator[] and other
// accessors can be parameterised on access semantics.

namespace urd::v2 {

// --- tie ------------------------------------------------------------------

template<typename T>
auto tie(T& v) -> tuple<T&> {
    return {v, {}};
}

template<typename T, typename... Ts>
auto tie(T& v, Ts&... rest) -> tuple<T&, Ts&...> {
    return {v, tie(rest...)};
}

// --- ref_policy -----------------------------------------------------------

struct ref_policy {
    template<typename StorageTuple, size_t... Is>
    static auto element(StorageTuple& s, size_t i, std::index_sequence<Is...>) {
        return urd::v2::tie(urd::v2::get<Is>(s)[i]...);
    }
};

} // namespace urd::v2
