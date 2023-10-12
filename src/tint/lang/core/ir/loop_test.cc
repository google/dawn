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

#include "src/tint/lang/core/ir/loop.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_LoopTest = IRTestHelper;

TEST_F(IR_LoopTest, Parent) {
    auto* loop = b.Loop();
    EXPECT_EQ(loop->Initializer()->Parent(), loop);
    EXPECT_EQ(loop->Body()->Parent(), loop);
    EXPECT_EQ(loop->Continuing()->Parent(), loop);
}

TEST_F(IR_LoopTest, Result) {
    auto* loop = b.Loop();
    EXPECT_FALSE(loop->HasResults());
    EXPECT_FALSE(loop->HasMultiResults());
}

TEST_F(IR_LoopTest, Fail_NullInitializerBlock) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            Loop loop(nullptr, b.MultiInBlock(), b.MultiInBlock());
        },
        "");
}

TEST_F(IR_LoopTest, Fail_NullBodyBlock) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            Loop loop(b.Block(), nullptr, b.MultiInBlock());
        },
        "");
}

TEST_F(IR_LoopTest, Fail_NullContinuingBlock) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            Loop loop(b.Block(), b.MultiInBlock(), nullptr);
        },
        "");
}

TEST_F(IR_LoopTest, Clone) {
    auto* loop = b.Loop();
    auto* new_loop = clone_ctx.Clone(loop);

    EXPECT_NE(loop, new_loop);
    EXPECT_FALSE(new_loop->HasResults());
    EXPECT_EQ(0u, new_loop->Exits().Count());
    EXPECT_NE(nullptr, new_loop->Initializer());
    EXPECT_NE(loop->Initializer(), new_loop->Initializer());

    EXPECT_NE(nullptr, new_loop->Body());
    EXPECT_NE(loop->Body(), new_loop->Body());

    EXPECT_NE(nullptr, new_loop->Continuing());
    EXPECT_NE(loop->Continuing(), new_loop->Continuing());
}

TEST_F(IR_LoopTest, CloneWithExits) {
    Loop* new_loop = nullptr;
    {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* if_ = b.If(true);
            b.Append(if_->True(), [&] { b.Continue(loop); });
            b.Append(if_->False(), [&] { b.ExitLoop(loop); });
            b.Append(loop->Continuing(), [&] { b.BreakIf(loop, false); });

            b.NextIteration(loop);
        });
        new_loop = clone_ctx.Clone(loop);
    }

    ASSERT_EQ(2u, new_loop->Body()->Length());
    EXPECT_TRUE(new_loop->Body()->Front()->Is<If>());

    auto* new_if = new_loop->Body()->Front()->As<If>();
    ASSERT_EQ(1u, new_if->True()->Length());
    EXPECT_TRUE(new_if->True()->Front()->Is<Continue>());
    EXPECT_EQ(new_loop, new_if->True()->Front()->As<Continue>()->Loop());

    ASSERT_EQ(1u, new_if->False()->Length());
    EXPECT_TRUE(new_if->False()->Front()->Is<ExitLoop>());
    EXPECT_EQ(new_loop, new_if->False()->Front()->As<ExitLoop>()->Loop());

    ASSERT_EQ(1u, new_loop->Continuing()->Length());
    EXPECT_TRUE(new_loop->Continuing()->Front()->Is<BreakIf>());
    EXPECT_EQ(new_loop, new_loop->Continuing()->Front()->As<BreakIf>()->Loop());

    EXPECT_TRUE(new_loop->Body()->Back()->Is<NextIteration>());
    EXPECT_EQ(new_loop, new_loop->Body()->Back()->As<NextIteration>()->Loop());
}

TEST_F(IR_LoopTest, CloneWithResults) {
    Loop* new_loop = nullptr;
    auto* r0 = b.InstructionResult(ty.i32());
    auto* r1 = b.InstructionResult(ty.f32());
    {
        auto* loop = b.Loop();
        loop->SetResults(Vector{r0, r1});
        b.Append(loop->Body(), [&] { b.ExitLoop(loop, b.Constant(42_i), b.Constant(42_f)); });
        new_loop = clone_ctx.Clone(loop);
    }

    ASSERT_EQ(2u, new_loop->Results().Length());
    auto* new_r0 = new_loop->Results()[0]->As<InstructionResult>();
    ASSERT_NE(new_r0, nullptr);
    ASSERT_NE(new_r0, r0);
    EXPECT_EQ(new_r0->Type(), ty.i32());
    auto* new_r1 = new_loop->Results()[1]->As<InstructionResult>();
    ASSERT_NE(new_r1, nullptr);
    ASSERT_NE(new_r1, r1);
    EXPECT_EQ(new_r1->Type(), ty.f32());
}

}  // namespace
}  // namespace tint::core::ir
