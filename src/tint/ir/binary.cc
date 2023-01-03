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

#include "src/tint/ir/binary.h"
#include "src/tint/debug.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Binary);

namespace tint::ir {

Binary::Binary(Kind kind, Value* result, Value* lhs, Value* rhs)
    : Base(result), kind_(kind), lhs_(lhs), rhs_(rhs) {
    TINT_ASSERT(IR, lhs_);
    TINT_ASSERT(IR, rhs_);
    lhs_->AddUsage(this);
    rhs_->AddUsage(this);
}

Binary::~Binary() = default;

std::ostream& Binary::ToString(std::ostream& out, const SymbolTable& st) const {
    Result()->ToString(out, st) << " = ";
    lhs_->ToString(out, st) << " ";

    switch (GetKind()) {
        case Binary::Kind::kAdd:
            out << "+";
            break;
        case Binary::Kind::kSubtract:
            out << "-";
            break;
        case Binary::Kind::kMultiply:
            out << "*";
            break;
        case Binary::Kind::kDivide:
            out << "/";
            break;
        case Binary::Kind::kModulo:
            out << "%";
            break;
        case Binary::Kind::kAnd:
            out << "&";
            break;
        case Binary::Kind::kOr:
            out << "|";
            break;
        case Binary::Kind::kXor:
            out << "^";
            break;
        case Binary::Kind::kLogicalAnd:
            out << "&&";
            break;
        case Binary::Kind::kLogicalOr:
            out << "||";
            break;
        case Binary::Kind::kEqual:
            out << "==";
            break;
        case Binary::Kind::kNotEqual:
            out << "!=";
            break;
        case Binary::Kind::kLessThan:
            out << "<";
            break;
        case Binary::Kind::kGreaterThan:
            out << ">";
            break;
        case Binary::Kind::kLessThanEqual:
            out << "<=";
            break;
        case Binary::Kind::kGreaterThanEqual:
            out << ">=";
            break;
        case Binary::Kind::kShiftLeft:
            out << "<<";
            break;
        case Binary::Kind::kShiftRight:
            out << ">>";
            break;
    }
    out << " ";
    rhs_->ToString(out, st);

    return out;
}

}  // namespace tint::ir
