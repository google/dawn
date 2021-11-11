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

/// Convience type definition for std::decay<T>::type
template <typename T>
using Decay = typename std::decay<T>::type;

/// NthTypeOf returns the `N`th type in `Types`
template <int N, typename... Types>
using NthTypeOf = typename std::tuple_element<N, std::tuple<Types...>>::type;

/// Signature describes the signature of a function.
template <typename RETURN, typename... PARAMETERS>
struct Signature {
  /// The return type of the function signature
  using ret = RETURN;
  /// The parameters of the function signature held in a std::tuple
  using parameters = std::tuple<PARAMETERS...>;
  /// The type of the Nth parameter of function signature
  template <std::size_t N>
  using parameter = NthTypeOf<N, PARAMETERS...>;
  /// The total number of parameters
  static constexpr std::size_t parameter_count = sizeof...(PARAMETERS);
};

/// SignatureOf is a traits helper that infers the signature of the function,
/// method, static method, lambda, or function-like object `F`.
template <typename F>
struct SignatureOf {
  /// The signature of the function-like object `F`
  using type = typename SignatureOf<decltype(&F::operator())>::type;
};

/// SignatureOf specialization for a regular function or static method.
template <typename R, typename... ARGS>
struct SignatureOf<R (*)(ARGS...)> {
  /// The signature of the function-like object `F`
  using type = Signature<typename std::decay<R>::type,
                         typename std::decay<ARGS>::type...>;
};

/// SignatureOf specialization for a non-static method.
template <typename R, typename C, typename... ARGS>
struct SignatureOf<R (C::*)(ARGS...)> {
  /// The signature of the function-like object `F`
  using type = Signature<typename std::decay<R>::type,
                         typename std::decay<ARGS>::type...>;
};

/// SignatureOf specialization for a non-static, const method.
template <typename R, typename C, typename... ARGS>
struct SignatureOf<R (C::*)(ARGS...) const> {
  /// The signature of the function-like object `F`
  using type = Signature<typename std::decay<R>::type,
                         typename std::decay<ARGS>::type...>;
};

/// SignatureOfT is an alias to `typename SignatureOf<F>::type`.
template <typename F>
using SignatureOfT = typename SignatureOf<F>::type;

/// ParameterType is an alias to `typename SignatureOf<F>::type::parameter<N>`.
template <typename F, std::size_t N>
using ParameterType = typename SignatureOfT<F>::template parameter<N>;

/// ReturnType is an alias to `typename SignatureOf<F>::type::ret`.
template <typename F>
using ReturnType = typename SignatureOfT<F>::ret;

/// `IsTypeOrDerived<T, BASE>::value` is true iff `T` is of type `BASE`, or
/// derives from `BASE`.
template <typename T, typename BASE>
using IsTypeOrDerived =
    std::integral_constant<bool,
                           std::is_base_of<BASE, Decay<T>>::value ||
                               std::is_same<BASE, Decay<T>>::value>;

/// If `CONDITION` is true then EnableIf resolves to type T, otherwise an
/// invalid type.
template <bool CONDITION, typename T>
using EnableIf = typename std::enable_if<CONDITION, T>::type;

/// If `T` is of type `BASE`, or derives from `BASE`, then EnableIfIsType
/// resolves to type `T`, otherwise an invalid type.
template <typename T, typename BASE>
using EnableIfIsType = EnableIf<IsTypeOrDerived<T, BASE>::value, T>;

/// If `T` is not of type `BASE`, or does not derive from `BASE`, then
/// EnableIfIsNotType resolves to type `T`, otherwise an invalid type.
template <typename T, typename BASE>
using EnableIfIsNotType = EnableIf<!IsTypeOrDerived<T, BASE>::value, T>;

}  // namespace traits
}  // namespace tint

#endif  // SRC_TRAITS_H_
