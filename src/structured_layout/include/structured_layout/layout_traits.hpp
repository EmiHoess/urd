#pragma once

#include <type_traits>

#include <structured_layout/detail/default_layout.hpp>

namespace structured_layout {

template<typename value_type>
struct layout_traits : detail::default_layout<std::remove_cvref_t<value_type>> {};

template<typename value_type>
using layout_of_t = std::remove_cv_t<
    typename layout_traits<std::remove_cvref_t<value_type>>::type>;

}
