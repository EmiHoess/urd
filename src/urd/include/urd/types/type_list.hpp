// SPDX-FileCopyrightText: 2024 Emi Hoess <contact@emihoess.com>
// SPDX-License-Identifier: MIT

#pragma once

// -------------------------------------------------------------------------
// Declarations
// -------------------------------------------------------------------------
namespace urd {
    // A compile-time container for a list of types.
    // The static member `size` holds the number of types the list contains.
    template <typename... Ts>
    struct type_list {
        constexpr static size_t size = sizeof...(Ts);
    };

    // Returns the number of types contained in the given type_list.
    template <typename... Ts>
    constexpr int size(type_list<Ts...>) noexcept;

    // Returns true if the given type_list is empty (i.e. contains no types).
    template <typename... Ts>
    constexpr bool empty(type_list<Ts...>) noexcept;

    // Removes all occurrences of the provided element (type instance) from the given type_list.
    template <typename T, typename... Ts>
    constexpr auto remove(T, type_list<Ts...>) noexcept;

    // Returns the first element (constructed via default construction) from a non-empty type_list.
    template <typename T, typename... Ts>
    constexpr auto front(type_list<T, Ts...>) noexcept;

    // Checks whether the given type (constructed as T{}) exists in the type_list.
    template <typename T, typename... Ts>
    constexpr bool contains(T, type_list<Ts...>) noexcept;

    // Checks whether a type (from the first argument) is included in the type_list provided as L.
    template <typename T, typename L>
    constexpr bool contains(T, L) noexcept;

    // Removes duplicate occurrences from the type_list – if an element appears more than once,
    // only its first occurrence will be kept.
    template <typename T, typename... Ts>
    constexpr auto unique(type_list<T, Ts...>) noexcept;

    // Generates a compile-time list of all permutations of the elements in the given type_list.
    template <typename... Ts>
    constexpr auto permutations(type_list<Ts...>) noexcept;

    // Compares two type_lists and returns true if they consist of the same types in the same order.
    template <typename... Ts>
    constexpr bool operator==(type_list<Ts...>, type_list<Ts...>) noexcept;

    // Prepends a single type (wrapped as a value) to the beginning of a type_list.
    template <typename T, typename... Ts>
    constexpr auto operator+(T, type_list<Ts...>) noexcept;

    // Appends a single type (wrapped as a value) to the end of a type_list.
    template <typename... Ts, typename T>
    constexpr auto operator+(type_list<Ts...>, T) noexcept;

    // Concatenates two type_lists together into one, with the types of the second appended to the first.
    template <typename... Ts, typename... Us>
    constexpr auto operator+(type_list<Ts...>, type_list<Us...>) noexcept;

    // Subtracts (removes) from the first type_list every element that appears in the second type_list.
    template <typename... Ts, typename... Us>
    constexpr auto operator-(type_list<Ts...>, type_list<Us...>) noexcept;

    // Prepends a type T to each type_list contained within a type_list of type_lists.
    template <typename T, typename... Ts>
    constexpr auto operator*(T, type_list<Ts...>) noexcept;

    // Appends a type T to each type_list contained within a type_list of type_lists.
    template <typename... Ts, typename T>
    constexpr auto operator*(type_list<Ts...>, T) noexcept;

    // Computes the Cartesian product of two type_lists (each representing a list of type_lists).
    template <typename... Ts, typename... Us>
    constexpr auto operator*(type_list<Ts...>, type_list<Us...>) noexcept;
}

// -------------------------------------------------------------------------
// Function Definitions for type_list utilities
// -------------------------------------------------------------------------
namespace urd {

    // Returns the number of types contained in the type_list.
    template <typename... Ts>
    constexpr int size(type_list<Ts...>) noexcept {
        return sizeof...(Ts);
    }

    // Returns true if the type_list is empty; otherwise false.
    template <typename... Ts>
    constexpr bool empty(type_list<Ts...>) noexcept {
        return sizeof...(Ts) == 0;
    }

    // Recursively removes all occurrences of the specified type from the type_list.
    // [Case 1]: When the first element F is different from the type T to remove.
    template <typename T, typename F, typename... Ts>
    constexpr auto remove(T, type_list<F, Ts...>) noexcept {
        // Keep F and combine it with the recursive removal in the remainder.
        return F{} + remove(T{}, type_list<Ts...>{});
    }

    // Recursively removes an element when the first type in the list matches the type T to remove.
    template <typename T, typename... Ts>
    constexpr auto remove(T, type_list<T, Ts...>) noexcept {
        // Skip the current element T and continue processing the rest.
        return remove(T{}, type_list<Ts...>{});
    }

    // Base case for removal: Removing any type from an empty type_list returns an empty type_list.
    template <typename T>
    constexpr auto remove(T, type_list<>) noexcept {
        return type_list<>{};
    }

    // Returns the first element (the "front") of a non-empty type_list.
    template <typename T, typename... Ts>
    constexpr auto front(type_list<T, Ts...>) noexcept {
        return T{};
    }

    // Returns true if the first element of the type_list matches the type T.
    template <typename T, typename... Ts>
    constexpr bool contains(T, type_list<T, Ts...>) noexcept {
        return true;
    }

    // Recursively checks whether the type T exists in the type_list when the first type is not a match.
    template <typename T, typename F, typename... Ts>
    constexpr bool contains(T, type_list<F, Ts...>) noexcept {
        return contains(T{}, type_list<Ts...>{});
    }

    // Base case: An empty type_list does not contain the type, so returns false.
    template <typename T>
    constexpr bool contains(T, type_list<>) noexcept {
        return false;
    }

    // Checks if every type in the first type_list is included in the second type_list L.
    // It returns false immediately if any element is missing.
    template <typename T, typename ... Ts, typename L>
    constexpr bool include(type_list<T, Ts...>, L) noexcept {
        if constexpr (contains(T{}, L{})) {
            return include(type_list<Ts...>{}, L{});
        } else {
            return false;
        }
    }

    // Base case for include: An empty type_list is trivially included in any type_list.
    template <typename L>
    constexpr bool include(type_list<>, L) noexcept {
        return true;
    }

    // Removes duplicates from the type_list by excluding later occurrences
    // if the current element appears again later in the list.
    template <typename T, typename... Ts>
    constexpr auto unique(type_list<T, Ts...>) noexcept {
        constexpr bool found = contains(T{}, type_list<Ts...>{});
        if constexpr (found) {
            // Skip T if found later in the list.
            return unique(type_list<Ts...>{});
        } else {
            // Keep T and continue processing the rest.
            return T{} + unique(type_list<Ts...>{});
        }
    }

    // Base case for unique: An empty type_list remains empty.
    constexpr auto unique(type_list<>) noexcept {
        return type_list<>{};
    }

    // Starts generating all permutations for a given type_list by using an empty prefix.
    template<typename... Ts>
    constexpr auto permutations(type_list<Ts...>) noexcept {
        return permutations(type_list<>{}, type_list<Ts...>{});
    }

    // Base case for permutations: A single-element type_list has the one permutation itself.
    template<typename T>
    constexpr auto permutations(type_list<T>) noexcept {
        return type_list<type_list<T>>{};
    }

    // Generates permutations by splitting the type_list:
    // - 'center' handles the case when R is placed at the beginning of permutations of the rest.
    // - 'suffix' handles combining R into later positions.
    template<typename... Ls, typename R, typename... Rs>
    constexpr auto permutations(type_list<Ls...>, type_list<R, Rs...>) noexcept {
        constexpr auto center = R{} * permutations(type_list<Ls..., Rs...>{});
        constexpr auto suffix = permutations(type_list<Ls..., R>{}, type_list<Rs...>{});
        return center + suffix;
    }

    // Base case for the recursive permutation generation when the second type_list is empty.
    template<typename... Ls>
    constexpr auto permutations(type_list<Ls...>, type_list<>) noexcept {
        return type_list<>{};
    }

    // Compares two type_lists and returns true if their contained types (and their order) are identical.
    template <typename... Ts>
    constexpr bool operator==(type_list<Ts...>, type_list<Ts...>) noexcept {
        return true;
    }

    // If two type_lists do not have the same parameter pack, they are not equal.
    template <typename... Ts, typename... Us>
    constexpr bool operator==(type_list<Ts...>, type_list<Us...>) noexcept {
        return false;
    }

    // Prepends a type T to the front of the given type_list.
    template <typename T, typename... Ts>
    constexpr auto operator+(T, type_list<Ts...>) noexcept {
        return type_list<T>{} + type_list<Ts...>{};
    }

    // Appends a type T to the end of the given type_list.
    template <typename... Ts, typename T>
    constexpr auto operator+(type_list<Ts...>, T) noexcept {
        return type_list<Ts...>{} + type_list<T>{};
    }

    // Concatenates two type_lists by combining their contained types.
    template <typename... Ts, typename... Us>
    constexpr auto operator+(type_list<Ts...>, type_list<Us...>) noexcept {
        return type_list<Ts..., Us...>{};
    }

    // Subtracting an empty type_list from another returns the original type_list.
    template <typename... Ts>
    constexpr auto operator-(type_list<Ts...>, type_list<>) noexcept {
        return type_list<Ts...>{};
    }

    // Recursively subtracts each element of the second type_list from the first.
    template <typename... Ts, typename U, typename... Us>
    constexpr auto operator-(type_list<Ts...>, type_list<U, Us...>) noexcept {
        return remove(U{}, type_list<Ts...>{} - type_list<Us...>{});
    }

    // Prepends an element T to every individual type_list within a type_list of type_lists.
    template <typename T, typename... Ts>
    constexpr auto operator*(T, type_list<Ts...>) noexcept {
        return type_list<type_list<T>>{} * type_list<Ts...>{};
    }

    // Appends an element T to every individual type_list within a type_list of type_lists.
    template <typename... Ts, typename T>
    constexpr auto operator*(type_list<Ts...>, T) noexcept {
        return type_list<Ts...>{} * type_list<type_list<T>>{};
    }

    // Base case for distribute:
    // Distributing types from a type_list into an empty list of type_lists yields an empty result.
    template <typename... T>
    constexpr auto distribute(type_list<T...>, type_list<>) noexcept {
        return type_list<>{};
    }

    // Distributes the types from the first type_list into each type_list element
    // within the second argument and concatenates the results.
    template <typename... T, typename... U, typename... Us>
    constexpr auto distribute(type_list<T...>, type_list<type_list<U...>, Us...>) noexcept {
        constexpr auto head = type_list<type_list<T..., U...>>{};
        constexpr auto tail = distribute(type_list<T...>{}, type_list<Us...>{});
        return head + tail;
    }

    // When multiplying (Cartesian product) an empty list of type_lists with another type_list,
    // the result is empty.
    template <typename... Us>
    constexpr auto operator*(type_list<>, type_list<Us...>) noexcept {
        return type_list<>{};
    }

    // Computes the Cartesian product of a type_list of type_lists with another type_list.
    // It distributes the second type_list across each inner type_list within the first.
    template <typename... T, typename... Ts, typename... Us>
    constexpr auto operator*(type_list<type_list<T...>, Ts...>, type_list<Us...>) noexcept {
        constexpr auto head = distribute(type_list<T...>{}, type_list<Us...>{});
        constexpr auto tail = type_list<Ts...>{} * type_list<Us...>{};
        return head + tail;
    }
// -------------------------------------------------------------------------
// Type-level aliases
// -------------------------------------------------------------------------

    // Appends T to the end of List, yielding a new type_list.
    template<typename List, typename T> struct append_impl;
    template<typename... Ts, typename T>
    struct append_impl<type_list<Ts...>, T> { using type = type_list<Ts..., T>; };
    template<typename List, typename T>
    using append_t = typename append_impl<List, T>::type;

    // Prepends T to the front of List, yielding a new type_list.
    template<typename List, typename T> struct prepend_impl;
    template<typename... Ts, typename T>
    struct prepend_impl<type_list<Ts...>, T> { using type = type_list<T, Ts...>; };
    template<typename List, typename T>
    using prepend_t = typename prepend_impl<List, T>::type;

    // True if T appears in List.
    template<typename List, typename T> struct contains_type : std::false_type {};
    template<typename T, typename... Rest>
    struct contains_type<type_list<T, Rest...>, T> : std::true_type {};
    template<typename Head, typename... Rest, typename T>
    struct contains_type<type_list<Head, Rest...>, T> : contains_type<type_list<Rest...>, T> {};
    template<typename List, typename T>
    inline constexpr bool contains_v = contains_type<List, T>::value;

    // Appends T to List only if T is not already present.
    template<typename List, typename T>
    using insert_if_new_t = std::conditional_t<contains_v<List, T>, List, append_t<List, T>>;

    // Index of T in List. Hard error (static_assert) if T is not found.
    template<typename T, typename List, std::size_t I = 0> struct index_of_impl;
    template<typename T, std::size_t I>
    struct index_of_impl<T, type_list<>, I> {
        static_assert(sizeof(T) == 0, "index_of_v: type not found in list");
    };
    template<typename T, typename... Rest, std::size_t I>
    struct index_of_impl<T, type_list<T, Rest...>, I> {
        static constexpr std::size_t value = I;
    };
    template<typename T, typename Head, typename... Rest, std::size_t I>
    struct index_of_impl<T, type_list<Head, Rest...>, I>
        : index_of_impl<T, type_list<Rest...>, I + 1> {};
    template<typename T, typename List>
    inline constexpr std::size_t index_of_v = index_of_impl<T, List>::value;

    // Concatenation of two type_lists with deduplication (union).
    // Elements of B not already in A are appended; order within each list is preserved.
    template<typename Dest, typename Source> struct union_impl;
    template<typename Dest>
    struct union_impl<Dest, type_list<>> { using type = Dest; };
    template<typename Dest, typename First, typename... Rest>
    struct union_impl<Dest, type_list<First, Rest...>> {
        using type = typename union_impl<insert_if_new_t<Dest, First>, type_list<Rest...>>::type;
    };
    template<typename A, typename B>
    using union_t = typename union_impl<A, B>::type;

    // Keeps only types in List for which Pred<T>::value is true.
    template<typename Acc, typename List, template<typename> class Pred> struct filter_impl;
    template<typename Acc, template<typename> class Pred>
    struct filter_impl<Acc, type_list<>, Pred> { using type = Acc; };
    template<typename Acc, typename First, typename... Rest, template<typename> class Pred>
    struct filter_impl<Acc, type_list<First, Rest...>, Pred> {
        using new_acc = std::conditional_t<Pred<First>::value, append_t<Acc, First>, Acc>;
        using type    = typename filter_impl<new_acc, type_list<Rest...>, Pred>::type;
    };
    template<typename List, template<typename> class Pred>
    using filter_t = typename filter_impl<type_list<>, List, Pred>::type;

    // True if every type in Sub also appears in Super.
    template<typename Sub, typename Super> struct is_subset_of : std::false_type {};
    template<typename Super>
    struct is_subset_of<type_list<>, Super> : std::true_type {};
    template<typename First, typename... Rest, typename Super>
    struct is_subset_of<type_list<First, Rest...>, Super>
        : std::bool_constant<contains_v<Super, First> && is_subset_of<type_list<Rest...>, Super>::value> {};
    template<typename Sub, typename Super>
    inline constexpr bool is_subset_of_v = is_subset_of<Sub, Super>::value;

    // True if L1 and L2 contain exactly the same types (order-independent).
    template<typename L1, typename L2>
    inline constexpr bool sets_equal_v = is_subset_of_v<L1, L2> && is_subset_of_v<L2, L1>;

} // namespace urd

// -------------------------------------------------------------------------
// Convenient Type Definitions
// -------------------------------------------------------------------------
// Alias that brings `urd::type_list` into the current scope as simply `type_list`
template<typename... Ts>
using type_list = urd::type_list<Ts...>;
