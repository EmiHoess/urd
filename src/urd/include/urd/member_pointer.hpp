#pragma once

#include <type_traits>

namespace urd {

template<typename type>
struct member_pointer_traits;

template<typename class_type, typename member_type>
struct member_pointer_traits<member_type class_type::*> {
    using owner_type = class_type;
    using value_type = member_type;
};

template<typename type>
using member_pointer_owner_t =
    typename member_pointer_traits<std::remove_cv_t<type>>::owner_type;

template<typename type>
using member_pointer_value_t =
    typename member_pointer_traits<std::remove_cv_t<type>>::value_type;

template<auto member>
using member_owner_t = member_pointer_owner_t<decltype(member)>;

template<auto member>
using member_value_t = member_pointer_value_t<decltype(member)>;

}
