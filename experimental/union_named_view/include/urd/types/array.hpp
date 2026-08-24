#pragma once
#include <array>
#include <cstddef>
#include <urd/meta/traits/layout.hpp>

namespace urd {

template<typename T, std::size_t N, typename _memory_layout3 = urd::layout::soa>
struct array {
    using memory_layout = _memory_layout3;
    using _memory_layout = _memory_layout3;
    std::array<T, N> data;

    // type aliases
    using value_type      = T;
    using size_type       = std::size_t;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using iterator        = typename std::array<T, N>::iterator;
    using const_iterator  = typename std::array<T, N>::const_iterator;

    static constexpr size_type size() noexcept { return N; }

    constexpr       reference operator[](size_type i)       noexcept { return data[i]; }
    constexpr const_reference operator[](size_type i) const noexcept { return data[i]; }

    constexpr       iterator       begin()       noexcept { return data.begin(); }
    constexpr const_iterator       begin() const noexcept { return data.begin(); }
    constexpr       iterator       end()         noexcept { return data.end(); }
    constexpr const_iterator       end()   const noexcept { return data.end(); }

    constexpr       value_type*       data_ptr()       noexcept { return data.data(); }
    constexpr const value_type*       data_ptr() const noexcept { return data.data(); }
};

} // namespace urd
