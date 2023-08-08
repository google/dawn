// Copyright 2023 The Tint Authors.
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

#include "src/tint/lang/core/binary_op.h"

namespace tint::core {

std::string_view ToString(BinaryOp value) {
    switch (value) {
        case BinaryOp::kAnd:
            return "&";
        case BinaryOp::kOr:
            return "|";
        case BinaryOp::kXor:
            return "^";
        case BinaryOp::kLogicalAnd:
            return "&&";
        case BinaryOp::kLogicalOr:
            return "||";
        case BinaryOp::kEqual:
            return "==";
        case BinaryOp::kNotEqual:
            return "!=";
        case BinaryOp::kLessThan:
            return "<";
        case BinaryOp::kGreaterThan:
            return ">";
        case BinaryOp::kLessThanEqual:
            return "<=";
        case BinaryOp::kGreaterThanEqual:
            return ">=";
        case BinaryOp::kShiftLeft:
            return "<<";
        case BinaryOp::kShiftRight:
            return ">>";
        case BinaryOp::kAdd:
            return "+";
        case BinaryOp::kSubtract:
            return "-";
        case BinaryOp::kMultiply:
            return "*";
        case BinaryOp::kDivide:
            return "/";
        case BinaryOp::kModulo:
            return "%";
    }
    return "<unknown>";
}

}  // namespace tint::core
