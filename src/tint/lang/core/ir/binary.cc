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
// WITHOUT WARRANTIES OR CONDITIONS OF ANY op, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
