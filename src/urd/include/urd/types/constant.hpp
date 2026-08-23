#pragma once

template <auto I>
using constant = std::integral_constant<decltype(I), I>;