#pragma once

#include <type_traits>

template <auto I>
using constant = std::integral_constant<decltype(I), I>;