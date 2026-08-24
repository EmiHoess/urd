#pragma once
#include <urd/types/array.hpp>
#include <array>

namespace meh{
    template<class T>
    struct is_array : std::false_type{};
    
    template<class T, std::size_t N>
    struct is_array<std::array<T,N>> : std::true_type{};
    
    template<class T, std::size_t N>
    struct is_array<urd::array<T,N>> : std::true_type{};

    template<class T, std::size_t N, typename Layout>
    struct is_array<urd::array<T,N,Layout>> : std::true_type{};

    // optional:
    template<class T>
    struct is_array<T const>:is_array<T>{};
    template<class T>
    struct is_array<T volatile>:is_array<T>{};
    template<class T>
    struct is_array<T volatile const>:is_array<T>{};

    template<class T>
    struct is_urd_array : std::false_type{};
    
    template<class T, std::size_t N>
    struct is_urd_array<urd::array<T,N>> : std::true_type{};

    template<class T, std::size_t N, typename Layout>
    struct is_urd_array<urd::array<T,N,Layout>> : std::true_type{};
    

    template<typename T>
    constexpr auto instantiate_one_item()
    {
        if constexpr (std::is_array<T>::value)
        {
            return std::remove_all_extents<T>::type();
        }
        else if constexpr (meh::is_array<T>::value)
        {
            return typename T::value_type();
        }
        else
        {
            return T();
        }
    }

    template<typename Type>
    constexpr auto collection_size()
    {
        if constexpr (meh::is_array<Type>::value)
        {
            return std::tuple_size<typename std::remove_reference_t<Type>>::value;
        }
        else
        {
            return 1;
        }
    }

    template <class T>
    using collection_item_type = decltype(instantiate_one_item<T>());


    // Primary template - will fail for non-array types
    template<typename Array>
    struct element_type_of_array;

    // Specialization for std::array
    template<typename T, std::size_t N>
    struct element_type_of_array<std::array<T, N>> {
        using type = T;
    };

    // Convenience alias
    template<typename Array>
    using element_type_of_array_t = typename element_type_of_array<Array>::type;
}
