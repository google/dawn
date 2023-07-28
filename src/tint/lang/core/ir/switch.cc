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

#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Switch);

namespace tint::ir {

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

}  // namespace tint::ir
