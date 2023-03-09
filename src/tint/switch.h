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

#include "src/tint/castable.h"

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
using SwitchCaseType = std::remove_pointer_t<traits::ParameterType<std::remove_reference_t<FN>, 0>>;

/// Evaluates to true if the function `FN` has the signature of a Default case in a Switch().
/// @see Switch().
template <typename FN>
inline constexpr bool IsDefaultCase =
    std::is_same_v<traits::ParameterType<std::remove_reference_t<FN>, 0>, Default>;

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

/// The implementation of Switch() for non-Default cases.
/// Switch splits the cases into two a low and high block of cases, and quickly rules out blocks
/// that cannot match by comparing the HashCode of the object and the cases in the block. If a block
/// of cases may match the given object's type, then that block is split into two, and the process
/// recurses. When NonDefaultCases() is called with a single case, then As<> will be used to
/// dynamically cast to the case type and if the cast succeeds, then the case handler is called.
/// @returns true if a case handler was found, otherwise false.
template <typename T, typename RETURN_TYPE, typename... CASES>
inline bool NonDefaultCases([[maybe_unused]] T* object,
                            const TypeInfo* type,
                            [[maybe_unused]] RETURN_TYPE* result,
                            std::tuple<CASES...>&& cases) {
    using Cases = std::tuple<CASES...>;

    static constexpr bool kHasReturnType = !std::is_same_v<RETURN_TYPE, void>;
    static constexpr size_t kNumCases = sizeof...(CASES);

    if constexpr (kNumCases == 0) {
        // No cases. Nothing to do.
        return false;
    } else if constexpr (kNumCases == 1) {  // NOLINT: cpplint doesn't understand
                                            // `else if constexpr`
        // Single case.
        using CaseFunc = std::tuple_element_t<0, Cases>;
        static_assert(!IsDefaultCase<CaseFunc>, "NonDefaultCases called with a Default case");
        // Attempt to dynamically cast the object to the handler type. If that succeeds, call the
        // case handler with the cast object.
        using CaseType = SwitchCaseType<CaseFunc>;
        if (type->Is<CaseType>()) {
            auto* ptr = static_cast<CaseType*>(object);
            if constexpr (kHasReturnType) {
                new (result) RETURN_TYPE(static_cast<RETURN_TYPE>(std::get<0>(cases)(ptr)));
            } else {
                std::get<0>(cases)(ptr);
            }
            return true;
        }
        return false;
    } else {
        // Multiple cases.
        // Check the hashcode bits to see if there's any possibility of a case matching in these
        // cases. If there isn't, we can skip all these cases.
        if (MaybeAnyOf(TypeInfo::CombinedHashCodeOf<SwitchCaseType<CASES>...>(),
                       type->full_hashcode)) {
            // Split the cases into two, and recurse.
            constexpr size_t kMid = kNumCases / 2;
            return NonDefaultCases(object, type, result, traits::Slice<0, kMid>(cases)) ||
                   NonDefaultCases(object, type, result,
                                   traits::Slice<kMid, kNumCases - kMid>(cases));
        } else {
            return false;
        }
    }
}

/// The implementation of Switch() for all cases.
/// @see NonDefaultCases
template <typename T, typename RETURN_TYPE, typename... CASES>
inline void SwitchCases(T* object, RETURN_TYPE* result, std::tuple<CASES...>&& cases) {
    using Cases = std::tuple<CASES...>;

    static constexpr int kDefaultIndex = detail::IndexOfDefaultCase<Cases>();
    static constexpr bool kHasDefaultCase = kDefaultIndex >= 0;
    static constexpr bool kHasReturnType = !std::is_same_v<RETURN_TYPE, void>;

    // Static assertions
    static constexpr bool kDefaultIsOK =
        kDefaultIndex == -1 || kDefaultIndex == static_cast<int>(std::tuple_size_v<Cases> - 1);
    static constexpr bool kReturnIsOK =
        kHasDefaultCase || !kHasReturnType || std::is_constructible_v<RETURN_TYPE>;
    static_assert(kDefaultIsOK, "Default case must be last in Switch()");
    static_assert(kReturnIsOK,
                  "Switch() requires either a Default case or a return type that is either void or "
                  "default-constructable");

    // If the static asserts have fired, don't bother spewing more errors below
    static constexpr bool kAllOK = kDefaultIsOK && kReturnIsOK;
    if constexpr (kAllOK) {
        if (object) {
            auto* type = &object->TypeInfo();
            if constexpr (kHasDefaultCase) {
                // Evaluate non-default cases.
                if (!detail::NonDefaultCases<T>(object, type, result,
                                                traits::Slice<0, kDefaultIndex>(cases))) {
                    // Nothing matched. Evaluate default case.
                    if constexpr (kHasReturnType) {
                        new (result) RETURN_TYPE(
                            static_cast<RETURN_TYPE>(std::get<kDefaultIndex>(cases)({})));
                    } else {
                        std::get<kDefaultIndex>(cases)({});
                    }
                }
            } else {
                if (!detail::NonDefaultCases<T>(object, type, result, std::move(cases))) {
                    // Nothing matched. No default case.
                    if constexpr (kHasReturnType) {
                        new (result) RETURN_TYPE();
                    }
                }
            }
        } else {
            // Object is nullptr, so no cases can match
            if constexpr (kHasDefaultCase) {
                // Evaluate default case.
                if constexpr (kHasReturnType) {
                    new (result)
                        RETURN_TYPE(static_cast<RETURN_TYPE>(std::get<kDefaultIndex>(cases)({})));
                } else {
                    std::get<kDefaultIndex>(cases)({});
                }
            } else {
                // No default case, no case can match.
                if constexpr (kHasReturnType) {
                    new (result) RETURN_TYPE();
                }
            }
        }
    }
}

/// Resolves to T if T is not nullptr_t, otherwise resolves to Ignore.
template <typename T>
using NullptrToIgnore = std::conditional_t<std::is_same_v<T, std::nullptr_t>, Ignore, T>;

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
struct SwitchReturnTypeImpl</*IS_CASTABLE*/ false, Infer, CASE_RETURN_TYPES...> {
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
struct SwitchReturnTypeImpl</*IS_CASTABLE*/ true, Infer, CASE_RETURN_TYPES...> {
  private:
    using InferredType =
        CastableCommonBase<detail::NullptrToIgnore<std::remove_pointer_t<CASE_RETURN_TYPES>>...>;

  public:
    /// `const T*` or `T*`, where T is the common base type for all the castable case types.
    using type = PropagateReturnConst<InferredType, CASE_RETURN_TYPES...>*;
};

/// Resolves to the return type for a Switch() with the requested return type `REQUESTED_TYPE` and
/// case statement return types. If `REQUESTED_TYPE` is Infer then the return type will be inferred
/// from the case return types.
template <typename REQUESTED_TYPE, typename... CASE_RETURN_TYPES>
using SwitchReturnType = typename SwitchReturnTypeImpl<
    IsCastable<NullptrToIgnore<std::remove_pointer_t<CASE_RETURN_TYPES>>...>,
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
template <typename RETURN_TYPE = detail::Infer, typename T = CastableBase, typename... CASES>
inline auto Switch(T* object, CASES&&... cases) {
    using ReturnType = detail::SwitchReturnType<RETURN_TYPE, traits::ReturnType<CASES>...>;
    static constexpr bool kHasReturnType = !std::is_same_v<ReturnType, void>;

    if constexpr (kHasReturnType) {
        // Replacement for std::aligned_storage as this is broken on earlier versions of MSVC.
        struct alignas(alignof(ReturnType)) ReturnStorage {
            uint8_t data[sizeof(ReturnType)];
        };
        ReturnStorage storage;
        auto* res = utils::Bitcast<ReturnType*>(&storage);
        TINT_DEFER(res->~ReturnType());
        detail::SwitchCases(object, res, std::forward_as_tuple(std::forward<CASES>(cases)...));
        return *res;
    } else {
        detail::SwitchCases<T, void>(object, nullptr,
                                     std::forward_as_tuple(std::forward<CASES>(cases)...));
    }
}

}  // namespace tint

#endif  // SRC_TINT_SWITCH_H_
