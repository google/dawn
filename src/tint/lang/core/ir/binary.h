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

#include "src/tint/lang/core/ir/operand_instruction.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::ir {

/// A binary operator.
enum class BinaryOp {
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

/// A binary instruction in the IR.
class Binary final : public Castable<Binary, OperandInstruction<2, 1>> {
  public:
    /// The offset in Operands() for the LHS
    static constexpr size_t kLhsOperandOffset = 0;

    /// The offset in Operands() for the RHS
    static constexpr size_t kRhsOperandOffset = 1;

    /// Constructor
    /// @param result the result value
    /// @param op the binary operator
    /// @param lhs the lhs of the instruction
    /// @param rhs the rhs of the instruction
    Binary(InstructionResult* result, BinaryOp op, Value* lhs, Value* rhs);
    ~Binary() override;

    /// @copydoc Instruction::Clone()
    Binary* Clone(CloneContext& ctx) override;

    /// @returns the binary operator
    BinaryOp Op() { return op_; }

    /// @returns the left-hand-side value for the instruction
    Value* LHS() { return operands_[kLhsOperandOffset]; }

    /// @returns the right-hand-side value for the instruction
    Value* RHS() { return operands_[kRhsOperandOffset]; }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() override { return "binary"; }

  private:
    BinaryOp op_;
};

/// @param kind the enum value
/// @returns the string for the given enum value
std::string_view ToString(BinaryOp kind);

/// Emits the name of the intrinsic type.
template <typename STREAM, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& out, BinaryOp kind) {
    return out << ToString(kind);
}

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_BINARY_H_
