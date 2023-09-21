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

#include "src/tint/lang/core/ir/load_vector_element.h"

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::LoadVectorElement);

namespace tint::core::ir {

LoadVectorElement::LoadVectorElement(InstructionResult* result, ir::Value* from, ir::Value* index) {
    flags_.Add(Flag::kSequenced);

    AddOperand(LoadVectorElement::kFromOperandOffset, from);
    AddOperand(LoadVectorElement::kIndexOperandOffset, index);
    AddResult(result);
}

LoadVectorElement::~LoadVectorElement() = default;

LoadVectorElement* LoadVectorElement::Clone(CloneContext& ctx) {
    auto* new_result = ctx.Clone(Result());
    auto* new_from = ctx.Clone(From());
    auto* new_index = ctx.Clone(Index());
    return ctx.ir.instructions.Create<LoadVectorElement>(new_result, new_from, new_index);
}

}  // namespace tint::core::ir
