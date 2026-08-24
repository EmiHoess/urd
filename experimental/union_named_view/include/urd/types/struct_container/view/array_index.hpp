#pragma once
#include <cstddef>
#include <cstdint>
#include <span>

namespace urd::v2 {

// array_index — element position + pointer to the backing SOA buffer.
//
// Static form  (N != dynamic_extent): N is compile-time; no size field.
// Dynamic form (N == dynamic_extent): n holds the runtime element count,
//   used by element_ref / field_ref to compute SOA array offsets at runtime.

template<typename Type, size_t N>
struct array_index {
    size_t   i;
    uint8_t* ref_collection;
};

template<typename Type>
struct array_index<Type, std::dynamic_extent> {
    size_t   i;
    size_t   n;
    uint8_t* ref_collection;
};

} // namespace urd::v2
