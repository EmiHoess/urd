#pragma once
#include <vector>
#include <cstdint>

// Contiguous pool for variable-per-slot data.
//
// All N slots share one flat allocation. Slot i owns the range
//   [data[offsets[i]], data[offsets[i] + counts[i]])
//
// Used by unified_layout to back span<T> fields. The SOA struct stores
// one uint32_t index per slot; this pool holds the actual pointed-to data.

namespace urd::v2 {

template<typename T>
struct pool {
    std::vector<T>        data;
    std::vector<uint32_t> offsets;
    std::vector<uint32_t> counts;

    size_t   slot_count(size_t i) const noexcept { return counts[i]; }
    T*       slot_data (size_t i)       noexcept { return data.data() + offsets[i]; }
    T const* slot_data (size_t i) const noexcept { return data.data() + offsets[i]; }
    size_t   total()               const noexcept { return data.size(); }
};

} // namespace urd::v2
