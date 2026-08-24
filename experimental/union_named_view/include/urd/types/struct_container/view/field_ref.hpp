#pragma once
#include <cstddef>
#include <cstring>
#include <span>
#include <urd/types/struct_container/view/array_index.hpp>
#include <urd/meta/traits/member_pointer.hpp>
#include <urd/meta/traits/layout.hpp>
#include <urd/macros/platform.hpp>

namespace urd::v2 {

// field_ref — single named-field accessor within a SOA element.
//
// Template params:
//   coeff      — per-element byte coefficient; absolute array offset =
//                  fixed_off + coeff * N        (static)
//                  fixed_off + coeff * index.n  (dynamic, N == dynamic_extent)
//   N          — element count; std::dynamic_extent for runtime-sized buffers
//   ptr        — member pointer identifying the field
//   fixed_off  — constant byte offset added before the coeff * N term

template<size_t coeff, size_t N, auto ptr, size_t fixed_off = 0>
struct field_ref {
    using ReturnType = urd::member_field_t<decltype(ptr)>;
    using ParentType = urd::member_class_t<decltype(ptr)>;

    static constexpr bool   is_soa = urd::has_soa_layout_v<ParentType>;
    static constexpr size_t stride = is_soa ? sizeof(ReturnType) : sizeof(ParentType);

    urd::v2::array_index<ReturnType, N> index;

    __device_host__ size_t array_offset() const {
        if constexpr (N == std::dynamic_extent)
            return fixed_off + coeff * index.n;
        else
            return fixed_off + coeff * N;
    }

    __device_host__ void operator=(ReturnType const& o) {
        uint8_t* base = index.ref_collection + array_offset() + index.i * stride;
        if constexpr (is_soa)
            std::memcpy(base, &o, sizeof(ReturnType));
        else
            std::memcpy(&(reinterpret_cast<ParentType*>(base)->*ptr), &o, sizeof(ReturnType));
    }

    __device_host__ operator ReturnType() const {
        uint8_t const* base = index.ref_collection + array_offset() + index.i * stride;
        ReturnType ret;
        if constexpr (is_soa)
            std::memcpy(&ret, base, sizeof(ReturnType));
        else
            std::memcpy(&ret, &(reinterpret_cast<ParentType const*>(base)->*ptr), sizeof(ReturnType));
        return ret;
    }

    __device_host__ ReturnType operator++(int) {
        ReturnType val = *this;
        ReturnType ret = val++;
        *this = val;
        return ret;
    }

    __device_host__ ReturnType* operator&() {
        uint8_t* base = index.ref_collection + array_offset() + index.i * stride;
        if constexpr (is_soa)
            return reinterpret_cast<ReturnType*>(base);
        else
            return &(reinterpret_cast<ParentType*>(base)->*ptr);
    }

    __device_host__ ReturnType const* operator&() const {
        uint8_t const* base = index.ref_collection + array_offset() + index.i * stride;
        if constexpr (is_soa)
            return reinterpret_cast<ReturnType const*>(base);
        else
            return &(reinterpret_cast<ParentType const*>(base)->*ptr);
    }
};

} // namespace urd::v2
