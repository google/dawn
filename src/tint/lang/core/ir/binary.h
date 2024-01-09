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

#ifndef SRC_TINT_LANG_CORE_IR_BINARY_H_
#define SRC_TINT_LANG_CORE_IR_BINARY_H_

#include <string>

#include "src/tint/lang/core/binary_op.h"
#include "src/tint/lang/core/ir/operand_instruction.h"

// Forward declarations
namespace tint::core::intrinsic {
struct TableData;
}

namespace tint::core::ir {

/// The abstract base class for dialect-specific binary-op instructions in the IR.
class Binary : public Castable<Binary, OperandInstruction<2, 1>> {
  public:
    /// The offset in Operands() for the LHS
    static constexpr size_t kLhsOperandOffset = 0;

    /// The offset in Operands() for the RHS
    static constexpr size_t kRhsOperandOffset = 1;

    /// Constructor (no results, no operands)
    Binary();

    /// Constructor
    /// @param result the result value
    /// @param op the binary operator
    /// @param lhs the lhs of the instruction
    /// @param rhs the rhs of the instruction
    Binary(InstructionResult* result, BinaryOp op, Value* lhs, Value* rhs);
    ~Binary() override;

    /// @returns the binary operator
    BinaryOp Op() const { return op_; }

    /// @param op the new binary operator
    void SetOp(BinaryOp op) { op_ = op; }

    /// @returns the left-hand-side value for the instruction
    Value* LHS() { return operands_[kLhsOperandOffset]; }

    /// @returns the left-hand-side value for the instruction
    const Value* LHS() const { return operands_[kLhsOperandOffset]; }

    /// @returns the right-hand-side value for the instruction
    Value* RHS() { return operands_[kRhsOperandOffset]; }

    /// @returns the right-hand-side value for the instruction
    const Value* RHS() const { return operands_[kRhsOperandOffset]; }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() const override { return "binary"; }

    /// @returns the table data to validate this builtin
    virtual const core::intrinsic::TableData& TableData() const = 0;

  private:
    BinaryOp op_ = BinaryOp::kAdd;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_BINARY_H_
