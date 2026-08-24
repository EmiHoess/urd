#pragma once
#include <urd/types/struct_container/view/rename_type.hpp>

namespace urd::v2 {

template<size_t offset, size_t N, typename Type>
using alias_size = rename_type<offset, N, nullptr, Type>;

} // namespace urd::v2
