// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/core/ir/loop.h"

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

    new_loop->SetResults(ctx.Clone(results_));

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
