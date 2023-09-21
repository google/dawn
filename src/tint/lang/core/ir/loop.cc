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

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::Loop);

namespace tint::core::ir {

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

Loop* Loop::Clone(CloneContext& ctx) {
    auto* new_init = ctx.ir.blocks.Create<MultiInBlock>();
    auto* new_body = ctx.ir.blocks.Create<MultiInBlock>();
    auto* new_continuing = ctx.ir.blocks.Create<MultiInBlock>();

    auto* new_loop = ctx.ir.instructions.Create<Loop>(new_init, new_body, new_continuing);
    ctx.Replace(this, new_loop);

    initializer_->CloneInto(ctx, new_init);
    body_->CloneInto(ctx, new_body);
    continuing_->CloneInto(ctx, new_continuing);

    return new_loop;
}

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

}  // namespace tint::core::ir
