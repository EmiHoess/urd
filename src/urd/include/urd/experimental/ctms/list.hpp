// SPDX-FileCopyrightText: 2024 Emi Hoess <contact@emihoess.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <tuple>
#include <type_traits>

#include <urd/experimental/ctms/ticket.hpp>
#include <urd/types/type_list.hpp>

namespace urd
{
    struct __nty_datadeps_ {};

    template <typename... Args>
    using lists = urd::type_list<Args...>;

    template <typename ID, typename T = decltype([]{}), size_t N = 0>
    using current_list = decltype(get_last_ticket<ID, T, lists<>, N>())::type;

    template <typename ID, typename T = decltype([]{}), size_t N = 0>
    inline constexpr decltype(auto) last_ticked = get_last_ticket<ID, T, lists<>, N>();

    template <typename ID, typename T, typename U, size_t N = 0>
    consteval decltype(auto) push()
    {
        constexpr size_t number = next_ticket_number<ID>(); 
        if constexpr(number == 0)
        {
            return stamp<ID, lists<T>, number>().ticket;
        }
        else
        {
            auto l = lists<T>();
            auto r = current_list<ID, U, number - 1>();

            return stamp<ID, decltype(r + l), number>().ticket;
        }
    }

    template <typename ID, typename T, auto s = push<ID, T, decltype([]{})>()>
    constexpr decltype(auto) push_back = []{ return s; };

    template <typename ID, typename T = decltype([]{}) >
    constexpr decltype(auto) clear = []{ 
        if constexpr ( urd::current_list<ID>::size == 0 )
        {
            return stamp<ID, lists<>, 0>().ticket;
        }else
        {
            return stamp<ID, lists<>, last_ticked<ID, T>.value + 1>().ticket;
        }
    };

}