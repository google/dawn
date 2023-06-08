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

#include "src/tint/ir/loop.h"

#include <utility>

#include "src/tint/ir/multi_in_block.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Loop);

namespace tint::ir {

Loop::Loop(ir::Block* i, ir::MultiInBlock* b, ir::MultiInBlock* c, ir::MultiInBlock* m)
    : initializer_(i), body_(b), continuing_(c), merge_(m) {
    TINT_ASSERT(IR, initializer_);
    TINT_ASSERT(IR, body_);
    TINT_ASSERT(IR, continuing_);
    TINT_ASSERT(IR, merge_);

    if (initializer_) {
        initializer_->SetParent(this);
    }
    if (body_) {
        body_->SetParent(this);
    }
    if (continuing_) {
        continuing_->SetParent(this);
    }
    if (merge_) {
        merge_->SetParent(this);
    }
}

Loop::~Loop() = default;

bool Loop::HasInitializer() {
    return initializer_->HasBranchTarget();
}

}  // namespace tint::ir
