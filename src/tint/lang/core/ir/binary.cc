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

#include "src/tint/lang/core/ir/binary.h"

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::Binary);

namespace tint::core::ir {

Binary::Binary(InstructionResult* result, BinaryOp op, Value* lhs, Value* rhs) : op_(op) {
    AddOperand(Binary::kLhsOperandOffset, lhs);
    AddOperand(Binary::kRhsOperandOffset, rhs);
    AddResult(result);
}

Binary::~Binary() = default;

Binary* Binary::Clone(CloneContext& ctx) {
    auto* new_result = ctx.Clone(Result());
    auto* lhs = ctx.Remap(LHS());
    auto* rhs = ctx.Remap(RHS());
    return ctx.ir.instructions.Create<Binary>(new_result, op_, lhs, rhs);
}

std::string_view ToString(enum BinaryOp op) {
    switch (op) {
        case BinaryOp::kAdd:
            return "add";
        case BinaryOp::kSubtract:
            return "subtract";
        case BinaryOp::kMultiply:
            return "multiply";
        case BinaryOp::kDivide:
            return "divide";
        case BinaryOp::kModulo:
            return "modulo";
        case BinaryOp::kAnd:
            return "and";
        case BinaryOp::kOr:
            return "or";
        case BinaryOp::kXor:
            return "xor";
        case BinaryOp::kEqual:
            return "equal";
        case BinaryOp::kNotEqual:
            return "not equal";
        case BinaryOp::kLessThan:
            return "less than";
        case BinaryOp::kGreaterThan:
            return "greater than";
        case BinaryOp::kLessThanEqual:
            return "less than equal";
        case BinaryOp::kGreaterThanEqual:
            return "greater than equal";
        case BinaryOp::kShiftLeft:
            return "shift left";
        case BinaryOp::kShiftRight:
            return "shift right";
    }
    return "<unknown>";
}

}  // namespace tint::core::ir
