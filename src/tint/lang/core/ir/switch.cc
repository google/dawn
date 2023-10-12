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

#include "src/tint/lang/core/ir/switch.h"

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::Switch);

namespace tint::core::ir {

Switch::Switch(Value* cond) {
    TINT_ASSERT(cond);

    AddOperand(Switch::kConditionOperandOffset, cond);
}

Switch::~Switch() = default;

void Switch::ForeachBlock(const std::function<void(ir::Block*)>& cb) {
    for (auto& c : cases_) {
        cb(c.Block());
    }
}

Switch* Switch::Clone(CloneContext& ctx) {
    auto* cond = ctx.Remap(Condition());
    auto* new_switch = ctx.ir.instructions.Create<Switch>(cond);
    ctx.Replace(this, new_switch);

    new_switch->cases_.Reserve(cases_.Length());
    for (const auto& cse : cases_) {
        Switch::Case new_case{};
        new_case.block = ctx.ir.blocks.Create<ir::Block>();
        cse.block->CloneInto(ctx, new_case.block);

        new_case.selectors.Reserve(cse.selectors.Length());
        for (const auto& sel : cse.selectors) {
            auto* new_val = sel.val ? ctx.Clone(sel.val) : nullptr;
            new_case.selectors.Push(Switch::CaseSelector{new_val});
        }
        new_switch->cases_.Push(new_case);
    }

    new_switch->SetResults(ctx.Clone(results_));

    return new_switch;
}

}  // namespace tint::core::ir
