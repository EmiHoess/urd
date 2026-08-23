#include <type_traits>
#include <functional>

#include <urd/types/type_list.hpp>

namespace meh{

// Helper to extract function signature from callable objects
template<typename T>
struct function_traits;

// Specialization for function pointers
template<typename R, typename... Args>
struct function_traits<R(*)(Args...)> {
    using parameter_types = type_list<Args...>;
};

// Specialization for member function pointers
template<typename R, typename C, typename... Args>
struct function_traits<R(C::*)(Args...)> {
    using parameter_types = type_list<Args...>;
};

// Specialization for const member function pointers
template<typename R, typename C, typename... Args>
struct function_traits<R(C::*)(Args...) const> {
    using parameter_types = type_list<Args...>;
};

// Specialization for std::function
template<typename R, typename... Args>
struct function_traits<std::function<R(Args...)>> {
    using parameter_types = type_list<Args...>;
};

// Main template for callable objects (including lambdas)
template<typename T>
struct function_traits {
    using parameter_types = typename function_traits<decltype(&T::operator())>::parameter_types;
};

// Main function to get parameter types from any callable
template<typename Callable>
auto get_parameter_types(Callable&& callable) {
    using traits = function_traits<std::decay_t<Callable>>;
    return typename traits::parameter_types{};
}

// Helper to get parameter types as a type alias
template<typename Callable>
using parameter_types_t = typename function_traits<std::decay_t<Callable>>::parameter_types;

}