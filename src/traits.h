// Copyright 2020 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_TRAITS_H_
#define SRC_TRAITS_H_

#include <tuple>

namespace tint {
namespace traits {

/// NthTypeOf returns the `N`th type in `Types`
template <int N, typename... Types>
using NthTypeOf = typename std::tuple_element<N, std::tuple<Types...>>::type;

/// ParamType is a traits helper that infers the type of the `N`th parameter
/// of the function, method, static method, lambda, or function-like object `F`.
template <typename F, int N>
struct ParamType {
  /// The type of the `N`th parameter of the function-like object `F`
  using type = typename ParamType<decltype(&F::operator()), N>::type;
};

/// ParamType specialization for a regular function or static method.
template <typename R, int N, typename... Args>
struct ParamType<R (*)(Args...), N> {
  /// Arg is the raw type of the `N`th parameter of the function
  using Arg = NthTypeOf<N, Args...>;
  /// The type of the `N`th parameter of the function
  using type = typename std::decay<Arg>::type;
};

/// ParamType specialization for a non-static method.
template <typename R, typename C, int N, typename... Args>
struct ParamType<R (C::*)(Args...), N> {
  /// Arg is the raw type of the `N`th parameter of the function
  using Arg = NthTypeOf<N, Args...>;
  /// The type of the `N`th parameter of the function
  using type = typename std::decay<Arg>::type;
};

/// ParamType specialization for a non-static, const method.
template <typename R, typename C, int N, typename... Args>
struct ParamType<R (C::*)(Args...) const, N> {
  /// Arg is the raw type of the `N`th parameter of the function
  using Arg = NthTypeOf<N, Args...>;
  /// The type of the `N`th parameter of the function
  using type = typename std::decay<Arg>::type;
};

/// ParamTypeT is an alias to `typename ParamType<F, N>::type`.
template <typename F, int N>
using ParamTypeT = typename ParamType<F, N>::type;

/// `IsTypeOrDerived<T, BASE>::value` is true iff `T` is of type `BASE`, or
/// derives from `BASE`.
template <typename T, typename BASE>
using IsTypeOrDerived = std::integral_constant<
    bool,
    std::is_base_of<BASE, typename std::decay<T>::type>::value ||
        std::is_same<BASE, typename std::decay<T>::type>::value>;

/// If `CONDITION` is true then EnableIf resolves to type T, otherwise an
/// invalid type.
template <bool CONDITION, typename T>
using EnableIf = typename std::enable_if<CONDITION, T>::type;

/// If T is a base of BASE then EnableIfIsType resolves to type T, otherwise an
/// invalid type.
template <typename T, typename BASE>
using EnableIfIsType = EnableIf<IsTypeOrDerived<T, BASE>::value, T>;

}  // namespace traits
}  // namespace tint

#endif  // SRC_TRAITS_H_
