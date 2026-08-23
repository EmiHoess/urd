#pragma once

namespace urd::reflect::detail {

struct to_any {
    template<typename T>
    operator T() const noexcept;
};

} // namespace urd::reflect::detail
