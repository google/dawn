// Copyright 2021 The Tint Authors.
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

#ifndef SRC_TINT_SEM_CONSTANT_H_
#define SRC_TINT_SEM_CONSTANT_H_

#include <variant>

#include "src/tint/number.h"

// Forward declarations
namespace tint::sem {
class Type;
}

namespace tint::sem {

/// Constant is the interface to a compile-time evaluated expression value.
class Constant {
  public:
    /// Constructor
    Constant();

    /// Destructor
    virtual ~Constant();

    /// @returns the type of the constant
    virtual const sem::Type* Type() const = 0;

    /// @returns the value of this Constant, if this constant is of a scalar value or abstract
    /// numeric, otherwise std::monostate.
    virtual std::variant<std::monostate, AInt, AFloat> Value() const = 0;

    /// @returns the child constant element with the given index, or nullptr if the constant has no
    /// children, or the index is out of bounds.
    /// For arrays, this returns the i'th element of the array.
    /// For vectors, this returns the i'th element of the vector.
    /// For matrices, this returns the i'th column vector of the matrix.
    /// For structures, this returns the i'th member field of the structure.
    virtual const Constant* Index(size_t) const = 0;

    /// @returns true if child elements of this constant are positive-zero valued.
    virtual bool AllZero() const = 0;

    /// @returns true if any child elements of this constant are positive-zero valued.
    virtual bool AnyZero() const = 0;

    /// @returns true if all child elements of this constant have the same value and type.
    virtual bool AllEqual() const = 0;

    /// @returns a hash of the constant.
    virtual size_t Hash() const = 0;

    /// @returns the value of the constant as the given scalar or abstract value.
    template <typename T>
    T As() const {
        return std::visit(
            [](auto v) {
                if constexpr (std::is_same_v<decltype(v), std::monostate>) {
                    return T(0);
                } else {
                    return static_cast<T>(v);
                }
            },
            Value());
    }
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_CONSTANT_H_
