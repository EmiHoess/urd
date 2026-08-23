#pragma once

#include <cstddef>
#include <type_traits>

namespace urd {

template<typename... types>
struct type_list {
    static constexpr std::size_t size = sizeof...(types);
};

template<typename list>
inline constexpr std::size_t type_list_size_v = list::size;

template<typename list>
inline constexpr bool type_list_empty_v = type_list_size_v<list> == 0;

template<typename list>
struct type_list_front;

template<typename first, typename... rest>
struct type_list_front<type_list<first, rest...>> {
    using type = first;
};

template<typename list>
using type_list_front_t = typename type_list_front<list>::type;

template<typename list, typename type>
struct type_list_contains;

template<typename type>
struct type_list_contains<type_list<>, type> : std::false_type {};

template<typename first, typename... rest, typename type>
struct type_list_contains<type_list<first, rest...>, type>
    : std::bool_constant<
          std::is_same_v<first, type> ||
          type_list_contains<type_list<rest...>, type>::value> {};

template<typename list, typename type>
inline constexpr bool type_list_contains_v = type_list_contains<list, type>::value;

template<typename list, typename type>
struct type_list_append;

template<typename... types, typename type>
struct type_list_append<type_list<types...>, type> {
    using result = type_list<types..., type>;
};

template<typename list, typename type>
using type_list_append_t = typename type_list_append<list, type>::result;

template<typename list, typename type>
struct type_list_prepend;

template<typename... types, typename type>
struct type_list_prepend<type_list<types...>, type> {
    using result = type_list<type, types...>;
};

template<typename list, typename type>
using type_list_prepend_t = typename type_list_prepend<list, type>::result;

template<typename left, typename right>
struct type_list_concat;

template<typename... left, typename... right>
struct type_list_concat<type_list<left...>, type_list<right...>> {
    using result = type_list<left..., right...>;
};

template<typename left, typename right>
using type_list_concat_t = typename type_list_concat<left, right>::result;

template<typename list, typename type>
struct type_list_remove;

template<typename type>
struct type_list_remove<type_list<>, type> {
    using result = type_list<>;
};

template<typename first, typename... rest, typename type>
struct type_list_remove<type_list<first, rest...>, type> {
private:
    using tail = typename type_list_remove<type_list<rest...>, type>::result;

public:
    using result = std::conditional_t<
        std::is_same_v<first, type>,
        tail,
        type_list_prepend_t<tail, first>>;
};

template<typename list, typename type>
using type_list_remove_t = typename type_list_remove<list, type>::result;

template<typename source, typename result = type_list<>>
struct type_list_unique;

template<typename result>
struct type_list_unique<type_list<>, result> {
    using type = result;
};

template<typename first, typename... rest, typename result>
struct type_list_unique<type_list<first, rest...>, result> {
private:
    using next = std::conditional_t<
        type_list_contains_v<result, first>,
        result,
        type_list_append_t<result, first>>;

public:
    using type = typename type_list_unique<type_list<rest...>, next>::type;
};

template<typename list>
using type_list_unique_t = typename type_list_unique<list>::type;

template<typename list, typename type, std::size_t index = 0>
struct type_list_index;

template<typename type, typename... rest, std::size_t index>
struct type_list_index<type_list<type, rest...>, type, index>
    : std::integral_constant<std::size_t, index> {};

template<typename first, typename... rest, typename type, std::size_t index>
struct type_list_index<type_list<first, rest...>, type, index>
    : type_list_index<type_list<rest...>, type, index + 1> {};

template<typename list, typename type>
inline constexpr std::size_t type_list_index_v = type_list_index<list, type>::value;

template<typename source, template<typename> class predicate, typename result = type_list<>>
struct type_list_filter;

template<template<typename> class predicate, typename result>
struct type_list_filter<type_list<>, predicate, result> {
    using type = result;
};

template<typename first, typename... rest, template<typename> class predicate, typename result>
struct type_list_filter<type_list<first, rest...>, predicate, result> {
private:
    using next = std::conditional_t<
        predicate<first>::value,
        type_list_append_t<result, first>,
        result>;

public:
    using type = typename type_list_filter<type_list<rest...>, predicate, next>::type;
};

template<typename list, template<typename> class predicate>
using type_list_filter_t = typename type_list_filter<list, predicate>::type;

template<typename subset, typename superset>
struct type_list_is_subset;

template<typename superset>
struct type_list_is_subset<type_list<>, superset> : std::true_type {};

template<typename first, typename... rest, typename superset>
struct type_list_is_subset<type_list<first, rest...>, superset>
    : std::bool_constant<
          type_list_contains_v<superset, first> &&
          type_list_is_subset<type_list<rest...>, superset>::value> {};

template<typename subset, typename superset>
inline constexpr bool type_list_is_subset_v =
    type_list_is_subset<subset, superset>::value;

template<typename left, typename right>
inline constexpr bool type_list_set_equal_v =
    type_list_is_subset_v<left, right> && type_list_is_subset_v<right, left>;

}
