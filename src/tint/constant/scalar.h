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

#ifndef SRC_TINT_CONSTANT_SCALAR_H_
#define SRC_TINT_CONSTANT_SCALAR_H_

#include "src/tint/constant/value.h"
#include "src/tint/number.h"
#include "src/tint/type/type.h"
#include "src/tint/utils/castable.h"
#include "src/tint/utils/hash.h"

namespace tint::constant {

/// Scalar holds a single scalar or abstract-numeric value.
template <typename T>
class Scalar : public utils::Castable<Scalar<T>, Value> {
  public:
    static_assert(!std::is_same_v<UnwrapNumber<T>, T> || std::is_same_v<T, bool>,
                  "T must be a Number or bool");

    /// Constructor
    /// @param t the scalar type
    /// @param v the scalar value
    Scalar(const type::Type* t, T v) : type(t), value(v) {
        if constexpr (IsFloatingPoint<T>) {
            TINT_ASSERT(Constant, std::isfinite(v.value));
        }
    }
    ~Scalar() override = default;

    /// @copydoc Value::Type()
    const type::Type* Type() const override { return type; }

    /// @return nullptr, as Scalar does not hold any elements.
    const Value* Index(size_t) const override { return nullptr; }

    /// @copydoc Value::NumElements()
    size_t NumElements() const override { return 1; }

    /// @copydoc Value::AllZero()
    bool AllZero() const override { return IsPositiveZero(); }

    /// @copydoc Value::AnyZero()
    bool AnyZero() const override { return IsPositiveZero(); }

    /// @copydoc Value::Hash()
    size_t Hash() const override { return utils::Hash(type, ValueOf()); }

    /// Clones the constant into the provided context
    /// @param ctx the clone context
    /// @returns the cloned node
    Scalar* Clone(CloneContext& ctx) const override {
        auto* ty = type->Clone(ctx.type_ctx);
        return ctx.dst.constants->Create<Scalar<T>>(ty, value);
    }

    /// @returns `value` if `T` is not a Number, otherwise ValueOf returns the inner value of the
    /// Number.
    inline auto ValueOf() const {
        if constexpr (std::is_same_v<UnwrapNumber<T>, T>) {
            return value;
        } else {
            return value.value;
        }
    }

    /// @returns true if `value` is a positive zero.
    inline bool IsPositiveZero() const {
        using N = UnwrapNumber<T>;
        return Number<N>(value) == Number<N>(0);  // Considers sign bit
    }

    /// The scalar type
    type::Type const* const type;
    /// The scalar value
    const T value;

  protected:
    /// @copydoc Value::InternalValue()
    std::variant<std::monostate, AInt, AFloat> InternalValue() const override {
        if constexpr (IsFloatingPoint<UnwrapNumber<T>>) {
            return static_cast<AFloat>(value);
        } else {
            return static_cast<AInt>(value);
        }
    }
};

}  // namespace tint::constant

#endif  // SRC_TINT_CONSTANT_SCALAR_H_
