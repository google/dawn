// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_SWITCH_H_
#define SRC_TINT_SWITCH_H_

#include <tuple>
#include <utility>

#include "src/tint/utils/bitcast.h"
#include "src/tint/utils/castable.h"
#include "src/tint/utils/defer.h"

namespace tint {

/// Default can be used as the default case for a Switch(), when all previous cases failed to match.
///
/// Example:
/// ```
/// Switch(object,
///     [&](TypeA*) { /* ... */ },
///     [&](TypeB*) { /* ... */ },
///     [&](Default) { /* If not TypeA or TypeB */ });
/// ```
struct Default {};

}  // namespace tint

namespace tint::detail {

/// Evaluates to the Switch case type being matched by the switch case function `FN`.
/// @note does not handle the Default case
/// @see Switch().
template <typename FN>
using SwitchCaseType =
    std::remove_pointer_t<utils::traits::ParameterType<std::remove_reference_t<FN>, 0>>;

/// Evaluates to true if the function `FN` has the signature of a Default case in a Switch().
/// @see Switch().
template <typename FN>
inline constexpr bool IsDefaultCase =
    std::is_same_v<utils::traits::ParameterType<std::remove_reference_t<FN>, 0>, Default>;

/// Searches the list of Switch cases for a Default case, returning the index of the Default case.
/// If the a Default case is not found in the tuple, then -1 is returned.
template <typename TUPLE, std::size_t START_IDX = 0>
constexpr int IndexOfDefaultCase() {
    if constexpr (START_IDX < std::tuple_size_v<TUPLE>) {
        return IsDefaultCase<std::tuple_element_t<START_IDX, TUPLE>>
                   ? static_cast<int>(START_IDX)
                   : IndexOfDefaultCase<TUPLE, START_IDX + 1>();
    } else {
        return -1;
    }
}

/// Resolves to T if T is not nullptr_t, otherwise resolves to Ignore.
template <typename T>
using NullptrToIgnore = std::conditional_t<std::is_same_v<T, std::nullptr_t>, utils::Ignore, T>;

/// Resolves to `const TYPE` if any of `CASE_RETURN_TYPES` are const or pointer-to-const, otherwise
/// resolves to TYPE.
template <typename TYPE, typename... CASE_RETURN_TYPES>
using PropagateReturnConst = std::conditional_t<
    // Are any of the pointer-stripped types const?
    (std::is_const_v<std::remove_pointer_t<CASE_RETURN_TYPES>> || ...),
    const TYPE,  // Yes: Apply const to TYPE
    TYPE>;       // No:  Passthrough

/// SwitchReturnTypeImpl is the implementation of SwitchReturnType
template <bool IS_CASTABLE, typename REQUESTED_TYPE, typename... CASE_RETURN_TYPES>
struct SwitchReturnTypeImpl;

/// SwitchReturnTypeImpl specialization for non-castable case types and an explicitly specified
/// return type.
template <typename REQUESTED_TYPE, typename... CASE_RETURN_TYPES>
struct SwitchReturnTypeImpl</*IS_CASTABLE*/ false, REQUESTED_TYPE, CASE_RETURN_TYPES...> {
    /// Resolves to `REQUESTED_TYPE`
    using type = REQUESTED_TYPE;
};

/// SwitchReturnTypeImpl specialization for non-castable case types and an inferred return type.
template <typename... CASE_RETURN_TYPES>
struct SwitchReturnTypeImpl</*IS_CASTABLE*/ false, utils::detail::Infer, CASE_RETURN_TYPES...> {
    /// Resolves to the common type for all the cases return types.
    using type = std::common_type_t<CASE_RETURN_TYPES...>;
};

/// SwitchReturnTypeImpl specialization for castable case types and an explicitly specified return
/// type.
template <typename REQUESTED_TYPE, typename... CASE_RETURN_TYPES>
struct SwitchReturnTypeImpl</*IS_CASTABLE*/ true, REQUESTED_TYPE, CASE_RETURN_TYPES...> {
  public:
    /// Resolves to `const REQUESTED_TYPE*` or `REQUESTED_TYPE*`
    using type = PropagateReturnConst<std::remove_pointer_t<REQUESTED_TYPE>, CASE_RETURN_TYPES...>*;
};

/// SwitchReturnTypeImpl specialization for castable case types and an inferred return type.
template <typename... CASE_RETURN_TYPES>
struct SwitchReturnTypeImpl</*IS_CASTABLE*/ true, utils::detail::Infer, CASE_RETURN_TYPES...> {
  private:
    using InferredType = utils::CastableCommonBase<
        detail::NullptrToIgnore<std::remove_pointer_t<CASE_RETURN_TYPES>>...>;

  public:
    /// `const T*` or `T*`, where T is the common base type for all the castable case types.
    using type = PropagateReturnConst<InferredType, CASE_RETURN_TYPES...>*;
};

/// Resolves to the return type for a Switch() with the requested return type `REQUESTED_TYPE` and
/// case statement return types. If `REQUESTED_TYPE` is Infer then the return type will be inferred
/// from the case return types.
template <typename REQUESTED_TYPE, typename... CASE_RETURN_TYPES>
using SwitchReturnType = typename SwitchReturnTypeImpl<
    utils::IsCastable<NullptrToIgnore<std::remove_pointer_t<CASE_RETURN_TYPES>>...>,
    REQUESTED_TYPE,
    CASE_RETURN_TYPES...>::type;

}  // namespace tint::detail

namespace tint {

/// Switch is used to dispatch one of the provided callback case handler functions based on the type
/// of `object` and the parameter type of the case handlers. Switch will sequentially check the type
/// of `object` against each of the switch case handler functions, and will invoke the first case
/// handler function which has a parameter type that matches the object type. When a case handler is
/// matched, it will be called with the single argument of `object` cast to the case handler's
/// parameter type. Switch will invoke at most one case handler. Each of the case functions must
/// have the signature `R(T*)` or `R(const T*)`, where `T` is the type matched by that case and `R`
/// is the return type, consistent across all case handlers.
///
/// An optional default case function with the signature `R(Default)` can be used as the last case.
/// This default case will be called if all previous cases failed to match.
///
/// If `object` is nullptr and a default case is provided, then the default case will be called. If
/// `object` is nullptr and no default case is provided, then no cases will be called.
///
/// Example:
/// ```
/// Switch(object,
///     [&](TypeA*) { /* ... */ },
///     [&](TypeB*) { /* ... */ });
///
/// Switch(object,
///     [&](TypeA*) { /* ... */ },
///     [&](TypeB*) { /* ... */ },
///     [&](Default) { /* Called if object is not TypeA or TypeB */ });
/// ```
///
/// @param object the object who's type is used to
/// @param cases the switch cases
/// @return the value returned by the called case. If no cases matched, then the zero value for the
/// consistent case type.
template <typename RETURN_TYPE = utils::detail::Infer,
          typename T = utils::CastableBase,
          typename... CASES>
inline auto Switch(T* object, CASES&&... cases) {
    using ReturnType = detail::SwitchReturnType<RETURN_TYPE, utils::traits::ReturnType<CASES>...>;
    static constexpr int kDefaultIndex = detail::IndexOfDefaultCase<std::tuple<CASES...>>();
    static constexpr bool kHasDefaultCase = kDefaultIndex >= 0;
    static constexpr bool kHasReturnType = !std::is_same_v<ReturnType, void>;

    // Static assertions
    static constexpr bool kDefaultIsOK =
        kDefaultIndex == -1 || kDefaultIndex == static_cast<int>(sizeof...(CASES) - 1);
    static constexpr bool kReturnIsOK =
        kHasDefaultCase || !kHasReturnType || std::is_constructible_v<ReturnType>;
    static_assert(kDefaultIsOK, "Default case must be last in Switch()");
    static_assert(kReturnIsOK,
                  "Switch() requires either a Default case or a return type that is either void or "
                  "default-constructable");

    if (!object) {  // Object is nullptr, so no cases can match
        if constexpr (kHasDefaultCase) {
            // Evaluate default case.
            auto&& default_case =
                std::get<kDefaultIndex>(std::forward_as_tuple(std::forward<CASES>(cases)...));
            return static_cast<ReturnType>(default_case(Default{}));
        } else {
            // No default case, no case can match.
            if constexpr (kHasReturnType) {
                return ReturnType{};
            } else {
                return;
            }
        }
    }

    // Replacement for std::aligned_storage as this is broken on earlier versions of MSVC.
    using ReturnTypeOrU8 = std::conditional_t<kHasReturnType, ReturnType, uint8_t>;
    struct alignas(alignof(ReturnTypeOrU8)) ReturnStorage {
        uint8_t data[sizeof(ReturnTypeOrU8)];
    };
    ReturnStorage storage;
    auto* result = utils::Bitcast<ReturnTypeOrU8*>(&storage);

    const utils::TypeInfo& type_info = object->TypeInfo();

    // Examines the parameter type of the case function.
    // If the parameter is a pointer type that `object` is of, or derives from, then that case
    // function is called with `object` cast to that type, and `try_case` returns true.
    // If the parameter is of type `Default`, then that case function is called and `try_case`
    // returns true.
    // Otherwise `try_case` returns false.
    // If the case function is called and it returns a value, then this is copy constructed to the
    // `result` pointer.
    auto try_case = [&](auto&& case_fn) {
        using CaseFunc = std::decay_t<decltype(case_fn)>;
        using CaseType = detail::SwitchCaseType<CaseFunc>;
        bool success = false;
        if constexpr (std::is_same_v<CaseType, Default>) {
            if constexpr (kHasReturnType) {
                new (result) ReturnType(static_cast<ReturnType>(case_fn(Default{})));
            } else {
                case_fn(Default{});
            }
            success = true;
        } else {
            if (type_info.Is<CaseType>()) {
                auto* v = static_cast<CaseType*>(object);
                if constexpr (kHasReturnType) {
                    new (result) ReturnType(static_cast<ReturnType>(case_fn(v)));
                } else {
                    case_fn(v);
                }
                success = true;
            }
        }
        return success;
    };

    // Use a logical-or fold expression to try each of the cases in turn, until one matches the
    // object type or a Default is reached. `handled` is true if a case function was called.
    bool handled = ((try_case(std::forward<CASES>(cases)) || ...));

    if constexpr (kHasReturnType) {
        if constexpr (kHasDefaultCase) {
            // Default case means there must be a returned value.
            // No need to check handled, no requirement for a zero-initializer of ReturnType.
            TINT_DEFER(result->~ReturnType());
            return *result;
        } else {
            if (handled) {
                TINT_DEFER(result->~ReturnType());
                return *result;
            }
            return ReturnType{};
        }
    }
}

}  // namespace tint

#endif  // SRC_TINT_SWITCH_H_
