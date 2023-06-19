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

#include "src/tint/ir/operand_instruction.h"
#include "src/tint/utils/castable.h"

namespace tint::ir {

/// A binary instruction in the IR.
class Binary : public utils::Castable<Binary, OperandInstruction<2, 1>> {
  public:
    /// The offset in Operands() for the LHS
    static constexpr size_t kLhsOperandOffset = 0;

    /// The offset in Operands() for the RHS
    static constexpr size_t kRhsOperandOffset = 1;

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
    /// @param result the result value
    /// @param kind the kind of binary instruction
    /// @param lhs the lhs of the instruction
    /// @param rhs the rhs of the instruction
    Binary(InstructionResult* result, enum Kind kind, Value* lhs, Value* rhs);
    ~Binary() override;

    /// @returns the kind of the binary instruction
    enum Kind Kind() { return kind_; }

    /// @returns the left-hand-side value for the instruction
    Value* LHS() { return operands_[kLhsOperandOffset]; }

    /// @returns the right-hand-side value for the instruction
    Value* RHS() { return operands_[kRhsOperandOffset]; }

  private:
    enum Kind kind_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_BINARY_H_
