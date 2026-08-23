#pragma once

#include <type_traits>

#include <structured_layout/layout_tags.hpp>

namespace structured_layout::detail {

template<typename value_type, typename = void>
struct default_layout {
    using type = aos_layout;
};

template<typename value_type>
struct default_layout<value_type, std::void_t<typename value_type::layout>> {
    using type = std::remove_cv_t<typename value_type::layout>;

    static_assert(is_layout_v<type>,
                  "embedded layout must be a structured_layout layout tag");
};

}
