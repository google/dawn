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

#ifndef SRC_TINT_RESOLVER_CONST_EVAL_TEST_H_
#define SRC_TINT_RESOLVER_CONST_EVAL_TEST_H_

#include <limits>
#include <string>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/test_helper.h"

namespace tint::resolver {

template <typename T>
inline const auto kPiOver2 = T(UnwrapNumber<T>(1.57079632679489661923));

template <typename T>
inline const auto kPiOver4 = T(UnwrapNumber<T>(0.785398163397448309616));

template <typename T>
inline const auto k3PiOver4 = T(UnwrapNumber<T>(2.356194490192344928846));

/// Walks the sem::Constant @p c, accumulating all the inner-most scalar values into @p args
inline void CollectScalarArgs(const sem::Constant* c, builder::ScalarArgs& args) {
    Switch(
        c->Type(),  //
        [&](const sem::AbstractInt*) { args.values.Push(c->As<AInt>()); },
        [&](const sem::AbstractFloat*) { args.values.Push(c->As<AFloat>()); },
        [&](const sem::Bool*) { args.values.Push(c->As<bool>()); },
        [&](const sem::I32*) { args.values.Push(c->As<i32>()); },
        [&](const sem::U32*) { args.values.Push(c->As<u32>()); },
        [&](const sem::F32*) { args.values.Push(c->As<f32>()); },
        [&](const sem::F16*) { args.values.Push(c->As<f16>()); },
        [&](Default) {
            size_t i = 0;
            while (auto* child = c->Index(i++)) {
                CollectScalarArgs(child, args);
            }
        });
}

/// Walks the sem::Constant @p c, returning all the inner-most scalar values.
inline builder::ScalarArgs ScalarArgsFrom(const sem::Constant* c) {
    builder::ScalarArgs out;
    CollectScalarArgs(c, out);
    return out;
}

template <typename T>
inline constexpr auto Negate(const Number<T>& v) {
    if constexpr (std::is_integral_v<T>) {
        if constexpr (std::is_signed_v<T>) {
            // For signed integrals, avoid C++ UB by not negating the smallest negative number. In
            // WGSL, this operation is well defined to return the same value, see:
            // https://gpuweb.github.io/gpuweb/wgsl/#arithmetic-expr.
            if (v == std::numeric_limits<T>::min()) {
                return v;
            }
            return -v;

        } else {
            // Allow negating unsigned values
            using ST = std::make_signed_t<T>;
            auto as_signed = Number<ST>{static_cast<ST>(v)};
            return Number<T>{static_cast<T>(Negate(as_signed))};
        }
    } else {
        // float case
        return -v;
    }
}

template <typename T>
inline auto Abs(const Number<T>& v) {
    if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
        return v;
    } else {
        return Number<T>(std::abs(v));
    }
}

TINT_BEGIN_DISABLE_WARNING(CONSTANT_OVERFLOW);
template <typename T>
inline constexpr Number<T> Mul(Number<T> v1, Number<T> v2) {
    if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
        // For signed integrals, avoid C++ UB by multiplying as unsigned
        using UT = std::make_unsigned_t<T>;
        return static_cast<Number<T>>(static_cast<UT>(v1) * static_cast<UT>(v2));
    } else {
        return static_cast<Number<T>>(v1 * v2);
    }
}
TINT_END_DISABLE_WARNING(CONSTANT_OVERFLOW);

TINT_BEGIN_DISABLE_WARNING(CONSTANT_OVERFLOW);
template <typename T>
inline constexpr Number<T> Add(Number<T> v1, Number<T> v2) {
    if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
        // For signed integrals, avoid C++ UB by adding as unsigned
        using UT = std::make_unsigned_t<T>;
        return static_cast<Number<T>>(static_cast<UT>(v1) + static_cast<UT>(v2));
    } else {
        return static_cast<Number<T>>(v1 + v2);
    }
}
TINT_END_DISABLE_WARNING(CONSTANT_OVERFLOW);

// Concats any number of std::vectors
template <typename Vec, typename... Vecs>
[[nodiscard]] inline auto Concat(Vec&& v1, Vecs&&... vs) {
    auto total_size = v1.size() + (vs.size() + ...);
    v1.reserve(total_size);
    (std::move(vs.begin(), vs.end(), std::back_inserter(v1)), ...);
    return std::move(v1);
}

// Concats vectors `vs` into `v1`
template <typename Vec, typename... Vecs>
inline void ConcatInto(Vec& v1, Vecs&&... vs) {
    auto total_size = v1.size() + (vs.size() + ...);
    v1.reserve(total_size);
    (std::move(vs.begin(), vs.end(), std::back_inserter(v1)), ...);
}

// Concats vectors `vs` into `v1` iff `condition` is true
template <bool condition, typename Vec, typename... Vecs>
inline void ConcatIntoIf([[maybe_unused]] Vec& v1, [[maybe_unused]] Vecs&&... vs) {
    if constexpr (condition) {
        ConcatInto(v1, std::forward<Vecs>(vs)...);
    }
}

/// Returns the overflow error message for binary ops
template <typename NumberT>
inline std::string OverflowErrorMessage(NumberT lhs, const char* op, NumberT rhs) {
    std::stringstream ss;
    ss << std::setprecision(20);
    ss << "'" << lhs.value << " " << op << " " << rhs.value << "' cannot be represented as '"
       << FriendlyName<NumberT>() << "'";
    return ss.str();
}

/// Returns the overflow error message for converions
template <typename VALUE_TY>
std::string OverflowErrorMessage(VALUE_TY value, std::string_view target_ty) {
    std::stringstream ss;
    ss << std::setprecision(20);
    ss << "value " << value << " cannot be represented as "
       << "'" << target_ty << "'";
    return ss.str();
}

using builder::IsValue;
using builder::Mat;
using builder::Val;
using builder::Value;
using builder::ValueBase;
using builder::Vec;

using Types = std::variant<  //
    Value<AInt>,
    Value<AFloat>,
    Value<u32>,
    Value<i32>,
    Value<f32>,
    Value<f16>,
    Value<bool>,

    Value<builder::vec2<AInt>>,
    Value<builder::vec2<AFloat>>,
    Value<builder::vec2<u32>>,
    Value<builder::vec2<i32>>,
    Value<builder::vec2<f32>>,
    Value<builder::vec2<f16>>,
    Value<builder::vec2<bool>>,

    Value<builder::vec3<AInt>>,
    Value<builder::vec3<AFloat>>,
    Value<builder::vec3<u32>>,
    Value<builder::vec3<i32>>,
    Value<builder::vec3<f32>>,
    Value<builder::vec3<f16>>,
    Value<builder::vec3<bool>>,

    Value<builder::vec4<AInt>>,
    Value<builder::vec4<AFloat>>,
    Value<builder::vec4<u32>>,
    Value<builder::vec4<i32>>,
    Value<builder::vec4<f32>>,
    Value<builder::vec4<f16>>,
    Value<builder::vec4<bool>>,

    Value<builder::mat2x2<AInt>>,
    Value<builder::mat2x2<AFloat>>,
    Value<builder::mat2x2<f32>>,
    Value<builder::mat2x2<f16>>,

    Value<builder::mat2x3<AInt>>,
    Value<builder::mat2x3<AFloat>>,
    Value<builder::mat2x3<f32>>,
    Value<builder::mat2x3<f16>>,

    Value<builder::mat3x2<AInt>>,
    Value<builder::mat3x2<AFloat>>,
    Value<builder::mat3x2<f32>>,
    Value<builder::mat3x2<f16>>
    //
    >;

/// Returns the current Value<T> in the `types` variant as a `ValueBase` pointer to use the
/// polymorphic API. This trades longer compile times using std::variant for longer runtime via
/// virtual function calls.
template <typename ValueVariant>
inline const ValueBase* ToValueBase(const ValueVariant& types) {
    return std::visit(
        [](auto&& t) -> const ValueBase* { return static_cast<const ValueBase*>(&t); }, types);
}

/// Prints Types to ostream
inline std::ostream& operator<<(std::ostream& o, const Types& types) {
    return ToValueBase(types)->Print(o);
}

// Calls `f` on deepest elements of both `a` and `b`. If function returns Action::kStop, it stops
// traversing, and return Action::kStop; if the function returns Action::kContinue, it continues and
// returns Action::kContinue when done.
// TODO(amaiorano): Move to Constant.h?
enum class Action { kStop, kContinue };
template <typename Func>
inline Action ForEachElemPair(const sem::Constant* a, const sem::Constant* b, Func&& f) {
    EXPECT_EQ(a->Type(), b->Type());
    size_t i = 0;
    while (true) {
        auto* a_elem = a->Index(i);
        if (!a_elem) {
            break;
        }
        auto* b_elem = b->Index(i);
        if (ForEachElemPair(a_elem, b_elem, f) == Action::kStop) {
            return Action::kStop;
        }
        i++;
    }
    if (i == 0) {
        return f(a, b);
    }
    return Action::kContinue;
}

/// Defines common bit value patterns for the input `NumberT` type used for testing.
template <typename NumberT>
struct BitValues {
    /// The unwrapped number type
    using T = UnwrapNumber<NumberT>;
    /// The unsigned unwrapped number type
    using UT = std::make_unsigned_t<T>;
    /// Details
    struct detail {
        /// Unsigned type of `T`
        using UT = std::make_unsigned_t<T>;
        /// Size in bits of type T
        static constexpr size_t NumBits = sizeof(T) * 8;
        /// All bits set 1
        static constexpr T All = T{~T{0}};
        /// Only left-most bits set to 1, rest set to 0
        static constexpr T LeftMost = static_cast<T>(UT{1} << (NumBits - 1u));
        /// Only left-most bits set to 0, rest set to 1
        static constexpr T AllButLeftMost = T{~LeftMost};
        /// Only two left-most bits set to 1, rest set to 0
        static constexpr T TwoLeftMost = static_cast<T>(UT{0b11} << (NumBits - 2u));
        /// Only two left-most bits set to 0, rest set to 1
        static constexpr T AllButTwoLeftMost = T{~TwoLeftMost};
        /// Only right-most bit set to 1, rest set to 0
        static constexpr T RightMost = T{1};
        /// Only right-most bit set to 0, rest set to 1
        static constexpr T AllButRightMost = T{~RightMost};
    };

    /// Size in bits of type NumberT
    static inline const size_t NumBits = detail::NumBits;
    /// All bits set 1
    static inline const NumberT All = NumberT{detail::All};
    /// Only left-most bits set to 1, rest set to 0
    static inline const NumberT LeftMost = NumberT{detail::LeftMost};
    /// Only left-most bits set to 0, rest set to 1
    static inline const NumberT AllButLeftMost = NumberT{detail::AllButLeftMost};
    /// Only two left-most bits set to 1, rest set to 0
    static inline const NumberT TwoLeftMost = NumberT{detail::TwoLeftMost};
    /// Only two left-most bits set to 0, rest set to 1
    static inline const NumberT AllButTwoLeftMost = NumberT{detail::AllButTwoLeftMost};
    /// Only right-most bit set to 1, rest set to 0
    static inline const NumberT RightMost = NumberT{detail::RightMost};
    /// Only right-most bit set to 0, rest set to 1
    static inline const NumberT AllButRightMost = NumberT{detail::AllButRightMost};

    /// Performs a left-shift of `val` by `shiftBy`, both of varying type cast to `T`.
    /// @param val value to shift left
    /// @param shiftBy number of bits to shift left by
    /// @returns the shifted value
    template <typename U, typename V>
    static constexpr NumberT Lsh(U val, V shiftBy) {
        return NumberT{static_cast<T>(static_cast<UT>(val) << static_cast<UT>(shiftBy))};
    }
};

using ResolverConstEvalTest = ResolverTest;

}  // namespace tint::resolver

#endif  // SRC_TINT_RESOLVER_CONST_EVAL_TEST_H_
