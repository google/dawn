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

#include "src/tint/lang/core/ir/loop.h"

#include <utility>

#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Loop);

namespace tint::ir {

Loop::Loop(ir::Block* i, ir::MultiInBlock* b, ir::MultiInBlock* c)
    : initializer_(i), body_(b), continuing_(c) {
    TINT_ASSERT(initializer_);
    TINT_ASSERT(body_);
    TINT_ASSERT(continuing_);

    if (initializer_) {
        initializer_->SetParent(this);
    }
    if (body_) {
        body_->SetParent(this);
    }
    if (continuing_) {
        continuing_->SetParent(this);
    }
}

Loop::~Loop() = default;

void Loop::ForeachBlock(const std::function<void(ir::Block*)>& cb) {
    if (initializer_) {
        cb(initializer_);
    }
    if (body_) {
        cb(body_);
    }
    if (continuing_) {
        cb(continuing_);
    }
}

bool Loop::HasInitializer() {
    return initializer_->HasTerminator();
}

}  // namespace tint::ir
