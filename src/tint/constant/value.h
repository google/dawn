// Copyright 2022 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#ifndef SRC_TINT_CONSTANT_VALUE_H_
#define SRC_TINT_CONSTANT_VALUE_H_

#include <variant>

#include "src/tint/constant/clone_context.h"
#include "src/tint/constant/node.h"
#include "src/tint/number.h"
#include "src/tint/type/type.h"
#include "src/tint/utils/castable.h"

namespace tint::constant {

/// Value is the interface to a compile-time evaluated expression value.
class Value : public utils::Castable<Value, Node> {
  public:
    /// Constructor
    Value();

    /// Destructor
    ~Value() override;

    /// @returns the type of the value
    virtual const type::Type* Type() const = 0;

    /// @param i the index of the element
    /// @returns the child element with the given index, or nullptr if there are no children, or
    /// the index is out of bounds.
    ///
    /// For arrays, this returns the i'th element of the array.
    /// For vectors, this returns the i'th element of the vector.
    /// For matrices, this returns the i'th column vector of the matrix.
    /// For structures, this returns the i'th member field of the structure.
    virtual const Value* Index(size_t i) const = 0;

    /// @return the number of elements held by this Value
    virtual size_t NumElements() const = 0;

    /// @returns true if child elements are positive-zero valued.
    virtual bool AllZero() const = 0;

    /// @returns true if any child elements are positive-zero valued.
    virtual bool AnyZero() const = 0;

    /// @returns a hash of the value.
    virtual size_t Hash() const = 0;

    /// @returns the value as the given scalar or abstract value.
    template <typename T>
    T ValueAs() const {
        return std::visit(
            [](auto v) {
                if constexpr (std::is_same_v<decltype(v), std::monostate>) {
                    return T(0);
                } else {
                    return static_cast<T>(v);
                }
            },
            InternalValue());
    }

    /// @param b the value to compare too
    /// @returns true if this value is equal to @p b
    bool Equal(const Value* b) const;

    /// Clones the constant into the provided context
    /// @param ctx the clone context
    /// @returns the cloned node
    virtual Value* Clone(CloneContext& ctx) const = 0;

  protected:
    /// @returns the value, if this is of a scalar value or abstract numeric, otherwise
    /// std::monostate.
    virtual std::variant<std::monostate, AInt, AFloat> InternalValue() const = 0;
};

}  // namespace tint::constant

#endif  // SRC_TINT_CONSTANT_VALUE_H_
