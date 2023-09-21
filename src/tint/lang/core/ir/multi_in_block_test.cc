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

#include "src/tint/lang/core/ir/multi_in_block.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/block_param.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_MultiInBlockTest = IRTestHelper;

TEST_F(IR_MultiInBlockTest, Fail_NullInboundBranch) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* blk = b.MultiInBlock();
            blk->AddInboundSiblingBranch(nullptr);
        },
        "");
}

TEST_F(IR_MultiInBlockTest, CloneInto) {
    auto* loop = b.Loop();

    auto* blk = b.MultiInBlock();
    auto* add = b.Add(mod.Types().i32(), 1_i, 2_i);
    blk->Append(add);
    blk->SetParams({b.BlockParam(mod.Types().i32()), b.BlockParam(mod.Types().f32())});
    blk->SetParent(loop);

    auto* terminate = b.TerminateInvocation();
    blk->AddInboundSiblingBranch(terminate);

    auto* new_blk = b.MultiInBlock();
    blk->CloneInto(clone_ctx, new_blk);

    EXPECT_EQ(0u, new_blk->InboundSiblingBranches().Length());

    EXPECT_EQ(2u, new_blk->Params().Length());
    EXPECT_EQ(mod.Types().i32(), new_blk->Params()[0]->Type());
    EXPECT_EQ(mod.Types().f32(), new_blk->Params()[1]->Type());

    EXPECT_EQ(nullptr, new_blk->Parent());

    EXPECT_EQ(1u, new_blk->Length());
    EXPECT_NE(add, new_blk->Front());
    EXPECT_TRUE(new_blk->Front()->Is<Binary>());
    EXPECT_EQ(Binary::Kind::kAdd, new_blk->Front()->As<Binary>()->Kind());
}

TEST_F(IR_MultiInBlockTest, CloneEmpty) {
    auto* blk = b.MultiInBlock();
    auto* new_blk = b.MultiInBlock();
    blk->CloneInto(clone_ctx, new_blk);

    EXPECT_EQ(0u, new_blk->InboundSiblingBranches().Length());
    EXPECT_EQ(0u, new_blk->Params().Length());
}

}  // namespace
}  // namespace tint::core::ir
