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

#include "src/tint/lang/core/ir/if.h"

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::If);

namespace tint::core::ir {

If::If(Value* cond, ir::Block* t, ir::Block* f) : true_(t), false_(f) {
    TINT_ASSERT(true_);
    TINT_ASSERT(false_);

    AddOperand(If::kConditionOperandOffset, cond);

    if (true_) {
        true_->SetParent(this);
    }
    if (false_) {
        false_->SetParent(this);
    }
}

If::~If() = default;

void If::ForeachBlock(const std::function<void(ir::Block*)>& cb) {
    if (true_) {
        cb(true_);
    }
    if (false_) {
        cb(false_);
    }
}

If* If::Clone(CloneContext& ctx) {
    auto* cond = ctx.Remap(Condition());
    auto* new_true = ctx.ir.blocks.Create<ir::Block>();
    auto* new_false = ctx.ir.blocks.Create<ir::Block>();

    auto* new_if = ctx.ir.instructions.Create<If>(cond, new_true, new_false);
    ctx.Replace(this, new_if);

    true_->CloneInto(ctx, new_true);
    false_->CloneInto(ctx, new_false);

    new_if->SetResults(ctx.Clone(results_));

    return new_if;
}

}  // namespace tint::core::ir
