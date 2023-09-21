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

#include "src/tint/lang/core/ir/bitcast.h"

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::Bitcast);

namespace tint::core::ir {

Bitcast::Bitcast(InstructionResult* result, Value* val) {
    AddOperand(Bitcast::kValueOperandOffset, val);
    AddResult(result);
}

Bitcast::~Bitcast() = default;

Bitcast* Bitcast::Clone(CloneContext& ctx) {
    auto* new_res = ctx.Clone(Result());
    auto* new_val = ctx.Clone(Val());
    return ctx.ir.instructions.Create<Bitcast>(new_res, new_val);
}

}  // namespace tint::core::ir
