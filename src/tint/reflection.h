// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_REFLECTION_H_
#define SRC_TINT_REFLECTION_H_

#include <type_traits>

#include "src/tint/utils/concat.h"
#include "src/tint/utils/foreach_macro.h"

namespace tint {

namespace detail {

/// Helper for detecting whether the type T contains a nested Reflection class.
template <typename T, typename ENABLE = void>
struct HasReflection : std::false_type {};

/// Specialization for types that have a nested Reflection class.
template <typename T>
struct HasReflection<T, std::void_t<typename T::Reflection>> : std::true_type {};

}  // namespace detail

/// Is true if the class T has reflected its fields with TINT_REFLECT()
template <typename T>
static constexpr bool HasReflection = detail::HasReflection<T>::value;

/// Calls @p callback with each field of @p object
/// @param object the object
/// @param callback a function that is called for each field of @p object.
/// @tparam CB a function with the signature `void(FIELD)`
template <typename OBJECT, typename CB>
void ForeachField(OBJECT&& object, CB&& callback) {
    using T = std::decay_t<OBJECT>;
    static_assert(HasReflection<T>, "object type requires a tint::Reflect<> specialization");
    T::Reflection::Fields(object, callback);
}

/// Macro used by TINT_FOREACH() in TINT_REFLECT() to call the callback function with each field in
/// the variadic.
#define TINT_REFLECT_CALLBACK_FIELD(field) callback(object.field);

// TINT_REFLECT(...) reflects each of the fields arguments so that the types can be used with
// tint::ForeachField().
#define TINT_REFLECT(...)                                          \
    struct Reflection {                                            \
        template <typename OBJECT, typename CB>                    \
        static void Fields(OBJECT&& object, CB&& callback) {       \
            TINT_FOREACH(TINT_REFLECT_CALLBACK_FIELD, __VA_ARGS__) \
        }                                                          \
    }

}  // namespace tint

#endif  // SRC_TINT_REFLECTION_H_
