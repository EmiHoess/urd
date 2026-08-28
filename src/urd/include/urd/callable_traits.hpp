#pragma once

#include <functional>
#include <type_traits>

#include <urd/type_list.hpp>

namespace urd {

template<typename type>
struct callable_traits;

template<typename result, typename... arguments>
struct callable_traits<result(arguments...)> {
    using result_type = result;
    using parameter_types = type_list<arguments...>;
};

template<typename result, typename... arguments>
struct callable_traits<result(arguments...) noexcept>
    : callable_traits<result(arguments...)> {};

template<typename result, typename... arguments>
struct callable_traits<result (*)(arguments...)>
    : callable_traits<result(arguments...)> {};

template<typename result, typename... arguments>
struct callable_traits<result (*)(arguments...) noexcept>
    : callable_traits<result(arguments...)> {};

template<typename result, typename... arguments>
struct callable_traits<std::function<result(arguments...)>>
    : callable_traits<result(arguments...)> {};

template<typename owner, typename result, typename... arguments>
struct callable_traits<result (owner::*)(arguments...)>
    : callable_traits<result(arguments...)> {
    using owner_type = owner;
};

template<typename owner, typename result, typename... arguments>
struct callable_traits<result (owner::*)(arguments...) const>
    : callable_traits<result(arguments...)> {
    using owner_type = owner;
};

template<typename owner, typename result, typename... arguments>
struct callable_traits<result (owner::*)(arguments...) noexcept>
    : callable_traits<result(arguments...)> {
    using owner_type = owner;
};

template<typename owner, typename result, typename... arguments>
struct callable_traits<result (owner::*)(arguments...) const noexcept>
    : callable_traits<result(arguments...)> {
    using owner_type = owner;
};

template<typename type>
    requires requires { &std::remove_reference_t<type>::operator(); }
struct callable_traits<type>
    : callable_traits<decltype(&std::remove_reference_t<type>::operator())> {};

template<typename callable>
using callable_parameter_types_t =
    typename callable_traits<std::remove_cvref_t<callable>>::parameter_types;

template<typename callable>
using callable_result_t =
    typename callable_traits<std::remove_cvref_t<callable>>::result_type;

}
