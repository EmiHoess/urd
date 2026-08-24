#pragma once
#include <vector>

namespace urd::v2 {

struct heap_storage {
    template<typename T, size_t /*N*/>
    using container = std::vector<T>;
};

} // namespace urd::v2
