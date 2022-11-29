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

#include "src/tint/ir/instruction.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Instruction);

namespace tint::ir {

Instruction::Instruction() {}

Instruction::Instruction(Kind kind, const Value* result, const Value* lhs, const Value* rhs)
    : kind_(kind), result_(result), args_({lhs, rhs}) {}

Instruction::Instruction(const Instruction&) = default;

Instruction::Instruction(Instruction&& instr) = default;

Instruction::~Instruction() = default;

Instruction& Instruction::operator=(const Instruction& instr) = default;

Instruction& Instruction::operator=(Instruction&& instr) = default;

std::ostream& operator<<(std::ostream& out, const Instruction& instr) {
    out << *(instr.Result()) << " = ";
    if (instr.HasLHS()) {
        out << *(instr.LHS());
    }
    out << " ";

    switch (instr.GetKind()) {
        case Instruction::Kind::kAdd:
            out << "+";
            break;
        case Instruction::Kind::kSubtract:
            out << "-";
            break;
        case Instruction::Kind::kMultiply:
            out << "*";
            break;
        case Instruction::Kind::kDivide:
            out << "/";
            break;
        case Instruction::Kind::kModulo:
            out << "%";
            break;
        case Instruction::Kind::kAnd:
            out << "&";
            break;
        case Instruction::Kind::kOr:
            out << "|";
            break;
        case Instruction::Kind::kXor:
            out << "^";
            break;
        case Instruction::Kind::kLogicalAnd:
            out << "&&";
            break;
        case Instruction::Kind::kLogicalOr:
            out << "||";
            break;
        case Instruction::Kind::kEqual:
            out << "==";
            break;
        case Instruction::Kind::kNotEqual:
            out << "!=";
            break;
        case Instruction::Kind::kLessThan:
            out << "<";
            break;
        case Instruction::Kind::kGreaterThan:
            out << ">";
            break;
        case Instruction::Kind::kLessThanEqual:
            out << "<=";
            break;
        case Instruction::Kind::kGreaterThanEqual:
            out << ">=";
            break;
        case Instruction::Kind::kShiftLeft:
            out << "<<";
            break;
        case Instruction::Kind::kShiftRight:
            out << ">>";
            break;
    }

    if (instr.HasRHS()) {
        out << " " << *(instr.RHS());
    }

    return out;
}

}  // namespace tint::ir
