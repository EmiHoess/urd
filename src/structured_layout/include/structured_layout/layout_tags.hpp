#pragma once

#include <cstddef>
#include <type_traits>

namespace structured_layout {

struct aos_layout {};

struct soa_layout {};

template<std::size_t block_extent>
struct aosoa_layout {
    static_assert(block_extent > 0);
    static constexpr std::size_t extent = block_extent;
};

template<typename type>
struct is_layout : std::false_type {};

template<>
struct is_layout<aos_layout> : std::true_type {};

template<>
struct is_layout<soa_layout> : std::true_type {};

template<std::size_t block_extent>
struct is_layout<aosoa_layout<block_extent>> : std::true_type {};

template<typename type>
inline constexpr bool is_layout_v = is_layout<std::remove_cv_t<type>>::value;

}
