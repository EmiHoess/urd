#pragma once
#include <array>

namespace urd::v2 {

struct stack_storage {
    template<typename T, size_t N>
    using container = std::array<T, N>;
};

} // namespace urd::v2
