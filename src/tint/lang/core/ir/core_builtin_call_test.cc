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

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/block_param.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_CoreBuiltinCallTest = IRTestHelper;

TEST_F(IR_CoreBuiltinCallTest, Usage) {
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);
    auto* builtin = b.Call(mod.Types().f32(), core::BuiltinFn::kAbs, arg1, arg2);

    EXPECT_THAT(arg1->Usages(), testing::UnorderedElementsAre(Usage{builtin, 0u}));
    EXPECT_THAT(arg2->Usages(), testing::UnorderedElementsAre(Usage{builtin, 1u}));
}

TEST_F(IR_CoreBuiltinCallTest, Result) {
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);
    auto* builtin = b.Call(mod.Types().f32(), core::BuiltinFn::kAbs, arg1, arg2);

    EXPECT_TRUE(builtin->HasResults());
    EXPECT_FALSE(builtin->HasMultiResults());
    EXPECT_TRUE(builtin->Result()->Is<InstructionResult>());
    EXPECT_EQ(builtin->Result()->Source(), builtin);
}

TEST_F(IR_CoreBuiltinCallTest, Fail_NullType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Call(nullptr, core::BuiltinFn::kAbs);
        },
        "");
}

TEST_F(IR_CoreBuiltinCallTest, Fail_NoneFunction) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Call(mod.Types().f32(), core::BuiltinFn::kNone);
        },
        "");
}

TEST_F(IR_CoreBuiltinCallTest, Fail_TintMaterializeFunction) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Call(mod.Types().f32(), core::BuiltinFn::kTintMaterialize);
        },
        "");
}

TEST_F(IR_CoreBuiltinCallTest, Clone) {
    auto* builtin = b.Call(mod.Types().f32(), core::BuiltinFn::kAbs, 1_u, 2_u);

    auto* new_b = clone_ctx.Clone(builtin);

    EXPECT_NE(builtin, new_b);
    EXPECT_NE(builtin->Result(), new_b->Result());
    EXPECT_EQ(mod.Types().f32(), new_b->Result()->Type());

    EXPECT_EQ(core::BuiltinFn::kAbs, new_b->Func());

    auto args = new_b->Args();
    EXPECT_EQ(2u, args.Length());

    auto* val0 = args[0]->As<Constant>()->Value();
    EXPECT_EQ(1_u, val0->As<core::constant::Scalar<u32>>()->ValueAs<u32>());

    auto* val1 = args[1]->As<Constant>()->Value();
    EXPECT_EQ(2_u, val1->As<core::constant::Scalar<u32>>()->ValueAs<u32>());
}

TEST_F(IR_CoreBuiltinCallTest, CloneNoArgs) {
    auto* builtin = b.Call(mod.Types().f32(), core::BuiltinFn::kAbs);

    auto* new_b = clone_ctx.Clone(builtin);
    EXPECT_NE(builtin->Result(), new_b->Result());
    EXPECT_EQ(mod.Types().f32(), new_b->Result()->Type());

    EXPECT_EQ(core::BuiltinFn::kAbs, new_b->Func());

    auto args = new_b->Args();
    EXPECT_TRUE(args.IsEmpty());
}

}  // namespace
}  // namespace tint::core::ir
