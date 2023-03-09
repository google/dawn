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
#include <optional>
#include <string>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/switch.h"
#include "src/tint/type/test_helper.h"
#include "src/tint/utils/string_stream.h"

namespace tint::resolver {

template <typename T>
inline const auto kPiOver2 = T(UnwrapNumber<T>(1.57079632679489661923));

template <typename T>
inline const auto kPiOver4 = T(UnwrapNumber<T>(0.785398163397448309616));

template <typename T>
inline const auto k3PiOver4 = T(UnwrapNumber<T>(2.356194490192344928846));

/// Walks the constant::Value @p c, accumulating all the inner-most scalar values into @p args
template <size_t N>
inline void CollectScalars(const constant::Value* c, utils::Vector<builder::Scalar, N>& scalars) {
    Switch(
        c->Type(),  //
        [&](const type::AbstractInt*) { scalars.Push(c->ValueAs<AInt>()); },
        [&](const type::AbstractFloat*) { scalars.Push(c->ValueAs<AFloat>()); },
        [&](const type::Bool*) { scalars.Push(c->ValueAs<bool>()); },
        [&](const type::I32*) { scalars.Push(c->ValueAs<i32>()); },
        [&](const type::U32*) { scalars.Push(c->ValueAs<u32>()); },
        [&](const type::F32*) { scalars.Push(c->ValueAs<f32>()); },
        [&](const type::F16*) { scalars.Push(c->ValueAs<f16>()); },
        [&](Default) {
            size_t i = 0;
            while (auto* child = c->Index(i++)) {
                CollectScalars(child, scalars);
            }
        });
}

/// Walks the constant::Value @p c, returning all the inner-most scalar values.
inline utils::Vector<builder::Scalar, 16> ScalarsFrom(const constant::Value* c) {
    utils::Vector<builder::Scalar, 16> out;
    CollectScalars(c, out);
    return out;
}

template <typename T>
inline auto Abs(const Number<T>& v) {
    if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
        return v;
    } else {
        return Number<T>(std::abs(v));
    }
}

/// Flags that can be passed to CheckConstant()
struct CheckConstantFlags {
    /// Expected value may be positive or negative
    bool pos_or_neg = false;
    /// Expected value should be compared using EXPECT_FLOAT_EQ instead of EQ, or EXPECT_NEAR if
    /// float_compare_epsilon is set.
    bool float_compare = false;
    /// Expected value should be compared using EXPECT_NEAR if float_compare is set.
    std::optional<double> float_compare_epsilon;
};

/// CheckConstant checks that @p got_constant, the result value of
/// constant-evaluation is equal to @p expected_value.
/// @param got_constant the constant value evaluated by the resolver
/// @param expected_value the expected value for the test
/// @param flags optional flags for controlling the comparisons
inline void CheckConstant(const constant::Value* got_constant,
                          const builder::Value& expected_value,
                          CheckConstantFlags flags = {}) {
    auto values_flat = ScalarsFrom(got_constant);
    auto expected_values_flat = expected_value.args;
    ASSERT_EQ(values_flat.Length(), expected_values_flat.Length());
    for (size_t i = 0; i < values_flat.Length(); ++i) {
        auto& got_scalar = values_flat[i];
        auto& expected_scalar = expected_values_flat[i];
        std::visit(
            [&](const auto& expected) {
                using T = std::decay_t<decltype(expected)>;

                ASSERT_TRUE(std::holds_alternative<T>(got_scalar))
                    << "Scalar variant index: " << got_scalar.index();
                auto got = std::get<T>(got_scalar);

                if constexpr (std::is_same_v<bool, T>) {
                    EXPECT_EQ(got, expected) << "index: " << i;
                } else if constexpr (IsFloatingPoint<T>) {
                    if (std::isnan(expected)) {
                        EXPECT_TRUE(std::isnan(got)) << "index: " << i;
                    } else {
                        if (flags.pos_or_neg) {
                            got = Abs(got);
                        }
                        if (flags.float_compare) {
                            if (flags.float_compare_epsilon) {
                                EXPECT_NEAR(got, expected, *flags.float_compare_epsilon)
                                    << "index: " << i;
                            } else {
                                EXPECT_FLOAT_EQ(got, expected) << "index: " << i;
                            }
                        } else {
                            EXPECT_EQ(got, expected) << "index: " << i;
                        }
                    }
                } else {
                    if (flags.pos_or_neg) {
                        got = Abs(got);
                    }
                    EXPECT_EQ(got, expected) << "index: " << i;

                    // Check that the constant's integer doesn't contain unexpected
                    // data in the MSBs that are outside of the bit-width of T.
                    EXPECT_EQ(AInt(got), AInt(expected)) << "index: " << i;
                }
            },
            expected_scalar);
    }
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
    utils::StringStream ss;
    ss << "'" << lhs.value << " " << op << " " << rhs.value << "' cannot be represented as '"
       << FriendlyName<NumberT>() << "'";
    return ss.str();
}

/// Returns the overflow error message for conversions
template <typename VALUE_TY>
std::string OverflowErrorMessage(VALUE_TY value, std::string_view target_ty) {
    utils::StringStream ss;
    ss << "value " << value << " cannot be represented as "
       << "'" << target_ty << "'";
    return ss.str();
}

/// Returns the overflow error message for exponentiation
template <typename NumberT>
std::string OverflowExpErrorMessage(std::string_view base, NumberT exp) {
    utils::StringStream ss;
    ss << base << "^" << exp << " cannot be represented as "
       << "'" << FriendlyName<NumberT>() << "'";
    return ss.str();
}

using builder::Array;
using builder::IsValue;
using builder::Mat;
using builder::Val;
using builder::Value;
using builder::Vec;

// Calls `f` on deepest elements of both `a` and `b`. If function returns Action::kStop, it stops
// traversing, and return Action::kStop; if the function returns Action::kContinue, it continues and
// returns Action::kContinue when done.
// TODO(amaiorano): Move to Constant.h?
enum class Action { kStop, kContinue };
template <typename Func>
inline Action ForEachElemPair(const constant::Value* a, const constant::Value* b, Func&& f) {
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
