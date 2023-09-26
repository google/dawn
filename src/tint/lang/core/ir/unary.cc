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

#include "src/tint/lang/core/ir/unary.h"

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::Unary);

namespace tint::core::ir {

Unary::Unary(InstructionResult* result, enum Kind k, Value* val) : kind_(k) {
    AddOperand(Unary::kValueOperandOffset, val);
    AddResult(result);
}

Unary::~Unary() = default;

Unary* Unary::Clone(CloneContext& ctx) {
    auto* new_result = ctx.Clone(Result());
    auto* val = ctx.Remap(Val());
    return ctx.ir.instructions.Create<Unary>(new_result, kind_, val);
}

}  // namespace tint::core::ir
