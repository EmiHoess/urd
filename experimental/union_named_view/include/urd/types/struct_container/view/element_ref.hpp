#pragma once
#include <cstddef>
#include <cstring>
#include <span>
#include <urd/types/struct_container/view/array_index.hpp>
#include <urd/macros/platform.hpp>

namespace urd::v2 {

// element_ref — atomic whole-element accessor for a SOA buffer.
//
// Template params:
//   alias      — rename_type / alias_size; provides original_type
//   coeff      — per-element byte coefficient; absolute array offset =
//                  fixed_off + coeff * N        (static)
//                  fixed_off + coeff * index.n  (dynamic, N == dynamic_extent)
//   N          — element count; std::dynamic_extent for runtime-sized buffers
//   fixed_off  — constant byte offset added before the coeff * N term
//                (used when a sub-element index contributes a fixed offset)

template<typename alias, size_t coeff, size_t N, size_t fixed_off = 0>
struct element_ref {
    using ReturnType = typename alias::original_type;

    urd::v2::array_index<ReturnType, N> index;

    static constexpr size_t size_of_ = sizeof(ReturnType);

    __device_host__ size_t array_offset() const {
        if constexpr (N == std::dynamic_extent)
            return fixed_off + coeff * index.n;
        else
            return fixed_off + coeff * N;
    }

    __device_host__ void operator=(ReturnType const& o) {
        std::memcpy(index.ref_collection + array_offset() + index.i * size_of_, &o, size_of_);
    }

    __device_host__ operator ReturnType() const {
        ReturnType ret;
        std::memcpy(&ret, index.ref_collection + array_offset() + index.i * size_of_, size_of_);
        return ret;
    }

    __device_host__ ReturnType operator++(int) {
        ReturnType val;
        std::memcpy(&val, index.ref_collection + array_offset() + index.i * size_of_, size_of_);
        ReturnType ret = val++;
        std::memcpy(index.ref_collection + array_offset() + index.i * size_of_, &val, size_of_);
        return ret;
    }

    __device_host__ ReturnType* operator&() {
        return reinterpret_cast<ReturnType*>(index.ref_collection + array_offset() + index.i * size_of_);
    }

    __device_host__ ReturnType const* operator&() const {
        return reinterpret_cast<ReturnType const*>(index.ref_collection + array_offset() + index.i * size_of_);
    }
};

} // namespace urd::v2
