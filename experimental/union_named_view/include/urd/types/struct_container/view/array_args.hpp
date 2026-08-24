#pragma once
#include <urd/types/struct_container/view/alias.hpp>

namespace urd {

enum array_layout { aos = 0, soa = 1 };

template<template<typename> typename Array_Type = no_template,
         array_layout _array_layout = array_layout::aos>
struct array_args {
    static constexpr bool is_atomic = ::v2::is_foo_template<Array_Type>::value;
    template<typename alias4>
    using element_alias = Array_Type<alias4>;
    static constexpr array_layout current_layout = _array_layout;
};

} // namespace urd
