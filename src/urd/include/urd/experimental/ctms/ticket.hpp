#pragma once

namespace urd
{
    template <typename ID, size_t N, typename T>
    struct ticket
    {
        using type = T;
        static constexpr size_t value = N;
        using identity_type = ID;
    };

    template <typename ID, size_t N>
    struct print
    {
        friend constexpr decltype(auto) ticket_details(print<ID, N>);
    };

    template <typename ID, typename R, size_t N>
    struct stamp
    {
        friend constexpr decltype(auto) ticket_details(print<ID, N>)
        {
            return R();
        }

        static constexpr ticket<ID, N, R> ticket{};
    };

    template <typename ID, size_t N, auto = []{}>
    using is_printed = print<ID, N>;

    template <typename ID, size_t N, auto = []{}>
    inline constexpr bool is_stamped = requires(is_printed<ID, N> t){ ticket_details(t); };

    template <typename ID, typename U = decltype([]{}), size_t N = 0>
    consteval size_t next_ticket_number()
    {
        if constexpr(is_stamped<ID, N>)
        {
            constexpr size_t M = N + 1;

            constexpr size_t D = 2 * N;
            constexpr size_t H = N / 2;

            if constexpr(is_stamped<ID, D>)
                return next_ticket_number<ID, U, D + 1>();
            else if constexpr(is_stamped<ID, N + H>)
                return next_ticket_number<ID, U, H + M>();
            else
                return next_ticket_number<ID, U, M>();
        } else return N;
    }


    template <typename ID, size_t N = 0>
    consteval decltype(auto) size()
    {
        constexpr size_t number = next_ticket_number<ID>();
        return decltype(ticket_details(print<ID, number - 1>()))::size;
    }

    template <typename ID, typename T, typename base_type, size_t N = 0>
    consteval decltype(auto) get_last_ticket()
    {
        constexpr size_t number = next_ticket_number<ID>();  
        if constexpr(number == 0)
            return ticket<ID, number, base_type>();
        else
            return ticket<ID, number - 1, decltype(ticket_details(print<ID, number - 1>()))>();
    }

}