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

#include "src/tint/lang/core/ir/store_vector_element.h"

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::StoreVectorElement);

namespace tint::core::ir {

StoreVectorElement::StoreVectorElement(ir::Value* to, ir::Value* index, ir::Value* value) {
    flags_.Add(Flag::kSequenced);

    AddOperand(StoreVectorElement::kToOperandOffset, to);
    AddOperand(StoreVectorElement::kIndexOperandOffset, index);
    AddOperand(StoreVectorElement::kValueOperandOffset, value);
}

StoreVectorElement::~StoreVectorElement() = default;

StoreVectorElement* StoreVectorElement::Clone(CloneContext& ctx) {
    auto* new_to = ctx.Clone(To());
    auto* new_idx = ctx.Clone(Index());
    auto* new_val = ctx.Clone(Value());
    return ctx.ir.instructions.Create<StoreVectorElement>(new_to, new_idx, new_val);
}

}  // namespace tint::core::ir
