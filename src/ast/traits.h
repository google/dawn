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

#ifndef SRC_AST_TRAITS_H_
#define SRC_AST_TRAITS_H_

#include <type_traits>

namespace tint {
namespace ast {
namespace traits {

/// FirstParamType is a traits helper that infers the type of the first
/// parameter of the function, method, static method, lambda, or function-like
/// object `F`.
template <typename F>
struct FirstParamType {
  /// The type of the first parameter of the function-like object `F`
  using type = typename FirstParamType<decltype(&F::operator())>::type;
};

/// FirstParamType specialization for a regular function or static method.
template <typename R, typename Arg>
struct FirstParamType<R (*)(Arg)> {
  /// The type of the first parameter of the function
  using type = typename std::decay<Arg>::type;
};

/// FirstParamType specialization for a non-static method.
template <typename R, typename C, typename Arg>
struct FirstParamType<R (C::*)(Arg)> {
  /// The type of the first parameter of the function
  using type = typename std::decay<Arg>::type;
};

/// FirstParamType specialization for a non-static, const method.
template <typename R, typename C, typename Arg>
struct FirstParamType<R (C::*)(Arg) const> {
  /// The type of the first parameter of the function
  using type = typename std::decay<Arg>::type;
};

/// FirstParamTypeT is an alias to `typename FirstParamType<F>::type`.
template <typename F>
using FirstParamTypeT = typename FirstParamType<F>::type;

/// If T is a base of BASE then EnableIfIsType resolves to type T, otherwise an
/// invalid type.
template <typename T, typename BASE>
using EnableIfIsType =
    typename std::enable_if<std::is_base_of<BASE, T>::value, T>::type;

}  // namespace traits
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TRAITS_H_
