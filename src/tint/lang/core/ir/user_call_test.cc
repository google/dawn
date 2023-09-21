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

#include "src/tint/lang/core/ir/user_call.h"

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_UserCallTest = IRTestHelper;

TEST_F(IR_UserCallTest, Usage) {
    auto* func = b.Function("myfunc", mod.Types().void_());
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);
    auto* e = b.Call(mod.Types().void_(), func, Vector{arg1, arg2});
    EXPECT_THAT(func->Usages(), testing::UnorderedElementsAre(Usage{e, 0u}));
    EXPECT_THAT(arg1->Usages(), testing::UnorderedElementsAre(Usage{e, 1u}));
    EXPECT_THAT(arg2->Usages(), testing::UnorderedElementsAre(Usage{e, 2u}));
}

TEST_F(IR_UserCallTest, Results) {
    auto* func = b.Function("myfunc", mod.Types().void_());
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);
    auto* e = b.Call(mod.Types().void_(), func, Vector{arg1, arg2});

    EXPECT_TRUE(e->HasResults());
    EXPECT_FALSE(e->HasMultiResults());
    EXPECT_TRUE(e->Result()->Is<InstructionResult>());
    EXPECT_EQ(e->Result()->Source(), e);
}

TEST_F(IR_UserCallTest, Fail_NullType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Call(nullptr, b.Function("myfunc", mod.Types().void_()));
        },
        "");
}

TEST_F(IR_UserCallTest, Clone) {
    auto* func = b.Function("myfunc", mod.Types().void_());
    auto* e = b.Call(mod.Types().void_(), func, Vector{b.Constant(1_u), b.Constant(2_u)});

    auto* new_func = clone_ctx.Clone(func);
    auto* new_e = clone_ctx.Clone(e);

    EXPECT_NE(e, new_e);
    EXPECT_NE(nullptr, new_e->Result());
    EXPECT_NE(e->Result(), new_e->Result());

    EXPECT_EQ(new_func, new_e->Target());

    auto args = new_e->Args();
    EXPECT_EQ(2u, args.Length());

    auto new_arg1 = args[0]->As<Constant>()->Value();
    ASSERT_TRUE(new_arg1->Is<core::constant::Scalar<u32>>());
    EXPECT_EQ(1_u, new_arg1->As<core::constant::Scalar<u32>>()->ValueAs<u32>());

    auto new_arg2 = args[1]->As<Constant>()->Value();
    ASSERT_TRUE(new_arg2->Is<core::constant::Scalar<u32>>());
    EXPECT_EQ(2_u, new_arg2->As<core::constant::Scalar<u32>>()->ValueAs<u32>());
}

TEST_F(IR_UserCallTest, CloneWithoutArgs) {
    auto* func = b.Function("myfunc", mod.Types().void_());
    auto* e = b.Call(mod.Types().void_(), func);

    auto* new_e = clone_ctx.Clone(e);

    EXPECT_EQ(0u, new_e->Args().Length());
}

}  // namespace
}  // namespace tint::core::ir
