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

#include "src/tint/lang/core/ir/switch.h"

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

using IR_SwitchTest = IRTestHelper;

TEST_F(IR_SwitchTest, Usage) {
    auto* cond = b.Constant(true);
    auto* switch_ = b.Switch(cond);
    EXPECT_THAT(cond->Usages(), testing::UnorderedElementsAre(Usage{switch_, 0u}));
}

TEST_F(IR_SwitchTest, Results) {
    auto* cond = b.Constant(true);
    auto* switch_ = b.Switch(cond);
    EXPECT_FALSE(switch_->HasResults());
    EXPECT_FALSE(switch_->HasMultiResults());
}

TEST_F(IR_SwitchTest, Parent) {
    auto* switch_ = b.Switch(1_i);
    b.Case(switch_, {Switch::CaseSelector{nullptr}});
    EXPECT_THAT(switch_->Cases().Front().Block()->Parent(), switch_);
}

TEST_F(IR_SwitchTest, Clone) {
    auto* switch_ = b.Switch(1_i);
    switch_->Cases().Push(
        Switch::Case{{Switch::CaseSelector{}, Switch::CaseSelector{b.Constant(2_i)}}, b.Block()});
    switch_->Cases().Push(Switch::Case{{Switch::CaseSelector{b.Constant(3_i)}}, b.Block()});

    auto* new_switch = clone_ctx.Clone(switch_);

    EXPECT_NE(switch_, new_switch);

    auto new_cond = new_switch->Condition()->As<Constant>()->Value();
    ASSERT_TRUE(new_cond->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(1_i, new_cond->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    auto& cases = new_switch->Cases();
    ASSERT_EQ(2u, cases.Length());

    {
        auto& case1 = cases[0];
        EXPECT_NE(nullptr, case1.block);
        EXPECT_NE(switch_->Cases()[0].block, case1.block);

        ASSERT_EQ(2u, case1.selectors.Length());
        EXPECT_EQ(nullptr, case1.selectors[0].val);
        auto val = case1.selectors[1].val->Value();
        ASSERT_TRUE(val->Is<core::constant::Scalar<i32>>());
        EXPECT_EQ(2_i, val->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
    }

    {
        auto& case2 = cases[1];
        EXPECT_NE(nullptr, case2.block);
        EXPECT_NE(switch_->Cases()[1].block, case2.block);

        ASSERT_EQ(1u, case2.selectors.Length());
        auto val = case2.selectors[0].val->Value();
        ASSERT_TRUE(val->Is<core::constant::Scalar<i32>>());
        EXPECT_EQ(3_i, val->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
    }
}

TEST_F(IR_SwitchTest, CloneWithExits) {
    Switch* new_switch = nullptr;
    {
        auto* switch_ = b.Switch(1_i);

        auto* blk = b.Block();
        b.Append(blk, [&] { b.ExitSwitch(switch_); });
        switch_->Cases().Push(Switch::Case{{Switch::CaseSelector{b.Constant(3_i)}}, blk});
        new_switch = clone_ctx.Clone(switch_);
    }

    auto& case_ = new_switch->Cases().Front();
    ASSERT_TRUE(case_.block->Front()->Is<ExitSwitch>());
    EXPECT_EQ(new_switch, case_.block->Front()->As<ExitSwitch>()->Switch());
}

TEST_F(IR_SwitchTest, CloneWithResults) {
    Switch* new_switch = nullptr;
    auto* r0 = b.InstructionResult(ty.i32());
    auto* r1 = b.InstructionResult(ty.f32());
    {
        auto* switch_ = b.Switch(1_i);
        switch_->SetResults(Vector{r0, r1});

        auto* blk = b.Block();
        b.Append(blk, [&] { b.ExitSwitch(switch_, b.Constant(42_i), b.Constant(42_f)); });
        switch_->Cases().Push(Switch::Case{{Switch::CaseSelector{b.Constant(3_i)}}, blk});
        new_switch = clone_ctx.Clone(switch_);
    }

    ASSERT_EQ(2u, new_switch->Results().Length());
    auto* new_r0 = new_switch->Results()[0]->As<InstructionResult>();
    ASSERT_NE(new_r0, nullptr);
    ASSERT_NE(new_r0, r0);
    EXPECT_EQ(new_r0->Type(), ty.i32());
    auto* new_r1 = new_switch->Results()[1]->As<InstructionResult>();
    ASSERT_NE(new_r1, nullptr);
    ASSERT_NE(new_r1, r1);
    EXPECT_EQ(new_r1->Type(), ty.f32());
}

}  // namespace
}  // namespace tint::core::ir
