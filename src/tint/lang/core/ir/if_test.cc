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

#include "src/tint/lang/core/ir/if.h"
#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_IfTest = IRTestHelper;

TEST_F(IR_IfTest, Usage) {
    auto* cond = b.Constant(true);
    auto* if_ = b.If(cond);
    EXPECT_THAT(cond->Usages(), testing::UnorderedElementsAre(Usage{if_, 0u}));
}

TEST_F(IR_IfTest, Result) {
    auto* if_ = b.If(b.Constant(true));

    EXPECT_FALSE(if_->HasResults());
    EXPECT_FALSE(if_->HasMultiResults());
}

TEST_F(IR_IfTest, Parent) {
    auto* cond = b.Constant(true);
    auto* if_ = b.If(cond);
    EXPECT_EQ(if_->True()->Parent(), if_);
    EXPECT_EQ(if_->False()->Parent(), if_);
}

TEST_F(IR_IfTest, Fail_NullTrueBlock) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            If if_(b.Constant(false), nullptr, b.Block());
        },
        "");
}

TEST_F(IR_IfTest, Fail_NullFalseBlock) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            If if_(b.Constant(false), b.Block(), nullptr);
        },
        "");
}

TEST_F(IR_IfTest, Clone) {
    auto* if_ = b.If(b.Constant(true));
    auto* new_if = clone_ctx.Clone(if_);

    EXPECT_NE(if_, new_if);

    auto new_cond = new_if->Condition()->As<Constant>()->Value();
    ASSERT_TRUE(new_cond->Is<core::constant::Scalar<bool>>());
    EXPECT_TRUE(new_cond->As<core::constant::Scalar<bool>>()->ValueAs<bool>());

    EXPECT_NE(nullptr, new_if->True());
    EXPECT_NE(nullptr, new_if->False());
    EXPECT_NE(if_->True(), new_if->True());
    EXPECT_NE(if_->False(), new_if->False());
}

TEST_F(IR_IfTest, CloneWithExits) {
    If* new_if = nullptr;
    {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.ExitIf(if_); });
        new_if = clone_ctx.Clone(if_);
    }

    ASSERT_EQ(1u, new_if->True()->Length());
    EXPECT_TRUE(new_if->True()->Front()->Is<ExitIf>());
    EXPECT_EQ(new_if, new_if->True()->Front()->As<ExitIf>()->If());
}

}  // namespace
}  // namespace tint::core::ir
