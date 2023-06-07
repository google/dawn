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

#include "src/tint/ir/if.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::If);

#include "src/tint/ir/block.h"

namespace tint::ir {

If::If(Value* cond, ir::Block* t, ir::Block* f, ir::Block* m) : true_(t), false_(f), merge_(m) {
    TINT_ASSERT(IR, cond);
    TINT_ASSERT(IR, true_);
    TINT_ASSERT(IR, false_);
    TINT_ASSERT(IR, merge_);

    AddOperand(cond);
    if (true_) {
        true_->AddInboundBranch(this);
        true_->SetParent(this);
    }
    if (false_) {
        false_->AddInboundBranch(this);
        false_->SetParent(this);
    }
    if (merge_) {
        merge_->SetParent(this);
    }
}

If::~If() = default;

}  // namespace tint::ir
