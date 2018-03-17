// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <tuple>
#include <type_traits>

namespace rms {
namespace util {

/**
 * Trait which allows to get callable result type.
 * @tparam T Callable to get info from.
 */
template <typename T>
struct FunctionTraits : public FunctionTraits<decltype(&T::operator())> {};

/**
 * Trait which allows to get callable result type and args count.
 * @tparam ClassType Callable to get info from.
 * @tparam ReturnType Return type.
 * @tparam Args Passed variadic arguments.
 */
template <typename ClassType, typename ReturnType, typename... Args>
struct FunctionTraits<ReturnType (ClassType::*)(Args...) const> {
  typedef ReturnType resultType;

  // cppcheck-suppress unusedStructMember
  constexpr static const std::size_t argsCount = sizeof...(Args);

  template <std::size_t N>
  struct arg {
    // The i-th argument is equivalent to the i-th tuple element of a tuple
    // composed of those arguments.
    typedef typename std::decay<typename std::tuple_element<N, std::tuple<Args...>>::type>::type type;
  };
};

}  // namespace util
}  // namespace rms
