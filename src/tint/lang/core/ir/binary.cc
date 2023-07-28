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

}  // namespace tint::ir
