// SPDX-FileCopyrightText: 2024 Emi Hoess <contact@emihoess.com>
// SPDX-License-Identifier: MIT

#pragma once
#include <type_traits>

namespace urd::layout {
    struct soa;
    struct aos;
} // namespace urd::layout

namespace urd {

template<typename T, typename = void>
struct has_layout : std::false_type {};
template<typename T>
struct has_layout<T, std::void_t<typename T::_memory_layout>> : std::true_type {};
template<typename T>
inline constexpr bool has_layout_v = has_layout<T>::value;

template<typename T, typename = void>
struct has_soa_layout : std::false_type {};
template<typename T>
struct has_soa_layout<T, std::void_t<typename T::_memory_layout>>
    : std::is_same<typename T::_memory_layout, urd::layout::soa> {};
template<typename T>
inline constexpr bool has_soa_layout_v = has_soa_layout<T>::value;

} // namespace urd
