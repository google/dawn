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

#include <cstddef>
#include <tuple>
#include <type_traits>

#include "src/tint/utils/macros/concat.h"
#include "src/tint/utils/macros/foreach.h"
#include "src/tint/utils/math/math.h"

/// Forward declarations
namespace tint {
class CastableBase;
}

namespace tint {

namespace detail {

/// Helper for detecting whether the type T contains a nested Reflection class.
template <typename T, typename ENABLE = void>
struct HasReflection : std::false_type {};

/// Specialization for types that have a nested Reflection class.
template <typename T>
struct HasReflection<T, std::void_t<typename T::Reflection>> : std::true_type {};

/// Helper for inferring the base class size of T.
template <typename T, typename ENABLE = void>
struct BaseClassSize {
    /// Zero as T::Base does not exist
    static constexpr size_t value = 0;
};

/// Specialization for types that contain a 'Base' type alias.
template <typename T>
struct BaseClassSize<T, std::void_t<typename T::Base>> {
    /// The size of T::Base, or zero if T::Base is not a base of T
    static constexpr size_t value =
        std::is_base_of_v<typename T::Base, T> ? sizeof(typename T::Base) : 0;
};

/// A helper to check at compile-time that all the fields of a class are passed to TINT_REFLECT().
template <typename CLASS, size_t INDEX, size_t OFFSET, typename FIELDS, bool ASSERT = true>
struct CheckAllFieldsReflected;

/// CheckAllFieldsReflected specialization that the final computed offset matches the size of the
/// class.
template <typename CLASS, size_t INDEX, size_t OFFSET, bool ASSERT>
struct CheckAllFieldsReflected<CLASS, INDEX, OFFSET, std::tuple<void>, ASSERT> {
    /// True iff the calculated size of class from all the fields matches the actual class size.
    static constexpr bool value =
        tint::RoundUp(alignof(CLASS), BaseClassSize<CLASS>::value + OFFSET) == sizeof(CLASS);
    static_assert(value || (ASSERT == false),
                  "TINT_REFLECT() was not passed all the fields of the class, or the fields were "
                  "not passed in the same order they're declared in the class");
};

/// CheckAllFieldsReflected specialization that the field with index INDEX is at the expected offset
/// in the class.
template <typename CLASS,
          size_t INDEX,
          size_t OFFSET,
          typename FIELD,
          bool ASSERT,
          typename... OTHERS>
struct CheckAllFieldsReflected<CLASS, INDEX, OFFSET, std::tuple<FIELD, OTHERS...>, ASSERT> {
    /// True iff the calculated size of class from all the fields matches the actual class size.
    static constexpr bool value =
        CheckAllFieldsReflected<CLASS,
                                INDEX + 1,
                                tint::RoundUp(alignof(FIELD), OFFSET) + sizeof(FIELD),
                                std::tuple<OTHERS...>,
                                ASSERT>::value;
};

/// Evaluates to true if type `T` can be used with TINT_ASSERT_ALL_FIELDS_REFLECTED().
template <typename T>
static constexpr bool CanUseTintAssertAllFieldsReflected =
    !std::has_virtual_destructor_v<T> || std::is_base_of_v<CastableBase, T>;

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
    T::Reflection::ForeachField(object, callback);
}

/// Macro used by TINT_FOREACH() in TINT_REFLECT() to generate the T::Reflection::Fields tuple.
#define TINT_REFLECT_FIELD_TYPE(FIELD) decltype(Class::FIELD),

/// Macro used by TINT_FOREACH() in TINT_REFLECT() to call the callback function with each field in
/// the variadic.
#define TINT_REFLECT_CALLBACK_FIELD(FIELD) callback(object.FIELD);

// TINT_REFLECT(CLASS, ...) reflects each of the fields arguments of CLASS so that the types can be
// used with tint::ForeachField().
#define TINT_REFLECT(CLASS, ...)                                                            \
    struct Reflection {                                                                     \
        using Class = CLASS;                                                                \
        using Fields = std::tuple<TINT_FOREACH(TINT_REFLECT_FIELD_TYPE, __VA_ARGS__) void>; \
        template <typename OBJECT, typename CB>                                             \
        [[maybe_unused]] static void ForeachField(OBJECT&& object, CB&& callback) {         \
            TINT_FOREACH(TINT_REFLECT_CALLBACK_FIELD, __VA_ARGS__)                          \
        }                                                                                   \
    }

/// TINT_ASSERT_ALL_FIELDS_REFLECTED(...) performs a compile-time assertion that all the fields of
/// CLASS have been reflected with TINT_REFLECT().
/// @note The order in which the fields are passed to TINT_REFLECT must match the declaration order
/// in the class.
#define TINT_ASSERT_ALL_FIELDS_REFLECTED(CLASS)                                                   \
    static_assert(::tint::detail::CanUseTintAssertAllFieldsReflected<CLASS>,                      \
                  "TINT_ASSERT_ALL_FIELDS_REFLECTED() cannot be used on virtual classes, except " \
                  "for types using the tint::Castable framework");                                \
    static_assert(                                                                                \
        ::tint::detail::CheckAllFieldsReflected<CLASS, 0, 0, CLASS::Reflection::Fields>::value)

/// A template that can be specialized to reflect the valid range of an enum
/// Use TINT_REFLECT_ENUM_RANGE to specialize this class
template <typename T>
struct EnumRange;

/// Declares a specialization of EnumRange for the enum ENUM with the lowest enum value MIN and
/// largest enum value MAX. Must only be used in the `tint` namespace.
#define TINT_REFLECT_ENUM_RANGE(ENUM, MIN, MAX) \
    template <>                                 \
    struct EnumRange<ENUM> {                    \
        static constexpr ENUM kMin = ENUM::MIN; \
        static constexpr ENUM kMax = ENUM::MAX; \
    }

}  // namespace tint

#endif  // SRC_TINT_UTILS_REFLECTION_REFLECTION_H_
