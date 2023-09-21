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

#include "src/tint/lang/core/ir/break_if.h"

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_BreakIfTest = IRTestHelper;

TEST_F(IR_BreakIfTest, Usage) {
    auto* loop = b.Loop();
    auto* cond = b.Constant(true);
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);

    auto* brk = b.BreakIf(loop, cond, arg1, arg2);

    EXPECT_THAT(cond->Usages(), testing::UnorderedElementsAre(Usage{brk, 0u}));
    EXPECT_THAT(arg1->Usages(), testing::UnorderedElementsAre(Usage{brk, 1u}));
    EXPECT_THAT(arg2->Usages(), testing::UnorderedElementsAre(Usage{brk, 2u}));
}

TEST_F(IR_BreakIfTest, Results) {
    auto* loop = b.Loop();
    auto* cond = b.Constant(true);
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);

    auto* brk = b.BreakIf(loop, cond, arg1, arg2);
    EXPECT_FALSE(brk->HasResults());
    EXPECT_FALSE(brk->HasMultiResults());
}

TEST_F(IR_BreakIfTest, Fail_NullLoop) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.BreakIf(nullptr, true);
        },
        "");
}

TEST_F(IR_BreakIfTest, Clone) {
    auto* loop = b.Loop();
    auto* cond = b.Constant(true);
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);

    auto* brk = b.BreakIf(loop, cond, arg1, arg2);

    auto* new_loop = clone_ctx.Clone(loop);
    clone_ctx.Replace(loop, new_loop);

    auto* new_brk = clone_ctx.Clone(brk);

    EXPECT_NE(brk, new_brk);

    EXPECT_EQ(new_loop, new_brk->Loop());

    auto args = new_brk->Args();
    EXPECT_EQ(2u, args.Length());

    auto new_cond = new_brk->Condition()->As<Constant>()->Value();
    ASSERT_TRUE(new_cond->Is<core::constant::Scalar<bool>>());
    EXPECT_TRUE(new_cond->As<core::constant::Scalar<bool>>()->ValueAs<bool>());

    auto new_arg0 = args[0]->As<Constant>()->Value();
    ASSERT_TRUE(new_arg0->Is<core::constant::Scalar<u32>>());
    EXPECT_EQ(1_u, new_arg0->As<core::constant::Scalar<u32>>()->ValueAs<u32>());

    auto new_arg1 = args[1]->As<Constant>()->Value();
    ASSERT_TRUE(new_arg1->Is<core::constant::Scalar<u32>>());
    EXPECT_EQ(2_u, new_arg1->As<core::constant::Scalar<u32>>()->ValueAs<u32>());
}

TEST_F(IR_BreakIfTest, CloneNoArgs) {
    auto* loop = b.Loop();
    auto* cond = b.Constant(true);

    auto* brk = b.BreakIf(loop, cond);
    auto* new_brk = clone_ctx.Clone(brk);

    auto args = new_brk->Args();
    EXPECT_EQ(0u, args.Length());
}

}  // namespace
}  // namespace tint::core::ir
