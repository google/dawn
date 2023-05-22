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

#ifndef SRC_TINT_IR_BINARY_H_
#define SRC_TINT_IR_BINARY_H_

#include "src/tint/ir/instruction.h"
#include "src/tint/utils/castable.h"

namespace tint::ir {

/// An instruction in the IR.
class Binary : public utils::Castable<Binary, Instruction> {
  public:
    /// The kind of instruction.
    enum class Kind {
        kAdd,
        kSubtract,
        kMultiply,
        kDivide,
        kModulo,

        kAnd,
        kOr,
        kXor,

        kEqual,
        kNotEqual,
        kLessThan,
        kGreaterThan,
        kLessThanEqual,
        kGreaterThanEqual,

        kShiftLeft,
        kShiftRight
    };

    /// Constructor
    /// @param kind the kind of binary instruction
    /// @param type the result type
    /// @param lhs the lhs of the instruction
    /// @param rhs the rhs of the instruction
    Binary(enum Kind kind, const type::Type* type, Value* lhs, Value* rhs);
    ~Binary() override;

    /// @returns the kind of the binary instruction
    enum Kind Kind() const { return kind_; }

    /// @returns the type of the value
    const type::Type* Type() const override { return result_type_; }

    /// @returns the left-hand-side value for the instruction
    const Value* LHS() const { return lhs_; }

    /// @returns the right-hand-side value for the instruction
    const Value* RHS() const { return rhs_; }

  private:
    enum Kind kind_;
    const type::Type* result_type_;
    Value* lhs_;
    Value* rhs_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_BINARY_H_
