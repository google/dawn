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

#include "src/tint/ir/op.h"

namespace tint::ir {

Op::Op() {}

Op::Op(Kind kind, Register result, Register lhs, Register rhs)
    : kind_(kind), result_(result), args_({lhs, rhs}) {}

Op::Op(const Op&) = default;

Op::Op(Op&& o) = default;

Op::~Op() = default;

Op& Op::operator=(const Op& o) = default;

Op& Op::operator=(Op&& o) = default;

std::ostream& operator<<(std::ostream& out, const Op& op) {
    out << op.Result() << " = ";
    if (op.HasLHS()) {
        out << op.LHS();
    }
    out << " ";

    switch (op.GetKind()) {
        case Op::Kind::kAdd:
            out << "+";
            break;
        case Op::Kind::kSubtract:
            out << "-";
            break;
        case Op::Kind::kMultiply:
            out << "*";
            break;
        case Op::Kind::kDivide:
            out << "/";
            break;
        case Op::Kind::kModulo:
            out << "%";
            break;
        case Op::Kind::kAnd:
            out << "&";
            break;
        case Op::Kind::kOr:
            out << "|";
            break;
        case Op::Kind::kXor:
            out << "^";
            break;
        case Op::Kind::kLogicalAnd:
            out << "&&";
            break;
        case Op::Kind::kLogicalOr:
            out << "||";
            break;
        case Op::Kind::kEqual:
            out << "==";
            break;
        case Op::Kind::kNotEqual:
            out << "!=";
            break;
        case Op::Kind::kLessThan:
            out << "<";
            break;
        case Op::Kind::kGreaterThan:
            out << ">";
            break;
        case Op::Kind::kLessThanEqual:
            out << "<=";
            break;
        case Op::Kind::kGreaterThanEqual:
            out << ">=";
            break;
        case Op::Kind::kShiftLeft:
            out << "<<";
            break;
        case Op::Kind::kShiftRight:
            out << ">>";
            break;
    }

    if (op.HasRHS()) {
        out << " " << op.RHS();
    }

    return out;
}

}  // namespace tint::ir
