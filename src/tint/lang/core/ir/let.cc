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

#include "src/tint/lang/core/ir/let.h"

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/store.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::Let);

namespace tint::core::ir {

Let::Let(InstructionResult* result, ir::Value* value) {
    AddOperand(Let::kValueOperandOffset, value);
    AddResult(result);
}

Let::~Let() = default;

Let* Let::Clone(CloneContext& ctx) {
    auto* new_result = ctx.Clone(Result());
    auto* new_val = ctx.Clone(Value());
    auto* new_let = ctx.ir.instructions.Create<Let>(new_result, new_val);

    auto name = ctx.ir.NameOf(this);
    ctx.ir.SetName(new_let, name.Name());

    return new_let;
}

}  // namespace tint::core::ir
