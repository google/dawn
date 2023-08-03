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

#include "src/tint/lang/core/ir/binary.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Binary);

namespace tint::ir {

Binary::Binary(InstructionResult* result, enum Kind kind, Value* lhs, Value* rhs) : kind_(kind) {
    AddOperand(Binary::kLhsOperandOffset, lhs);
    AddOperand(Binary::kRhsOperandOffset, rhs);
    AddResult(result);
}

Binary::~Binary() = default;

std::string_view ToString(enum Binary::Kind kind) {
    switch (kind) {
        case Binary::Kind::kAdd:
            return "add";
        case Binary::Kind::kSubtract:
            return "subtract";
        case Binary::Kind::kMultiply:
            return "multiply";
        case Binary::Kind::kDivide:
            return "divide";
        case Binary::Kind::kModulo:
            return "modulo";
        case Binary::Kind::kAnd:
            return "and";
        case Binary::Kind::kOr:
            return "or";
        case Binary::Kind::kXor:
            return "xor";
        case Binary::Kind::kEqual:
            return "equal";
        case Binary::Kind::kNotEqual:
            return "not equal";
        case Binary::Kind::kLessThan:
            return "less than";
        case Binary::Kind::kGreaterThan:
            return "greater than";
        case Binary::Kind::kLessThanEqual:
            return "less than equal";
        case Binary::Kind::kGreaterThanEqual:
            return "greater than equal";
        case Binary::Kind::kShiftLeft:
            return "shift left";
        case Binary::Kind::kShiftRight:
            return "shift right";
    }
    return "<unknown>";
}

}  // namespace tint::ir
