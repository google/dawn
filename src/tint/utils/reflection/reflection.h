// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_UTILS_REFLECTION_REFLECTION_H_
#define SRC_TINT_UTILS_REFLECTION_REFLECTION_H_

#include <type_traits>

#include "src/tint/utils/macros/concat.h"
#include "src/tint/utils/macros/foreach.h"

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
static constexpr bool HasReflection = tint::detail::HasReflection<T>::value;

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

#endif  // SRC_TINT_UTILS_REFLECTION_REFLECTION_H_
