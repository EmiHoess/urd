#pragma once

#include <type_traits>

namespace urd {

template<auto value>
using constant = std::integral_constant<decltype(value), value>;

}
