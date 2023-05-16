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

#include "src/tint/ir/test_helper.h"

#include "gmock/gmock.h"
#include "src/tint/ast/case_selector.h"
#include "src/tint/ast/int_literal_expression.h"
#include "src/tint/constant/scalar.h"
#include "src/tint/ir/block.h"
#include "src/tint/ir/constant.h"
#include "src/tint/ir/var.h"

namespace tint::ir {
namespace {

Value* GlobalVarInitializer(const Module& m) {
    if (m.root_block->instructions.Length() == 0u) {
        ADD_FAILURE() << "m.root_block has no instruction";
        return nullptr;
    }
    auto* var = m.root_block->instructions[0]->As<ir::Var>();
    if (!var) {
        ADD_FAILURE() << "m.root_block.instructions[0] was not a var";
        return nullptr;
    }
    return var->initializer;
}

using namespace tint::number_suffixes;  // NOLINT

using IR_BuilderImplTest = TestHelper;

TEST_F(IR_BuilderImplTest, EmitLiteral_Bool_True) {
    auto* expr = Expr(true);
    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* init = GlobalVarInitializer(m.Get());
    ASSERT_TRUE(Is<Constant>(init));
    auto* val = init->As<Constant>()->value;
    EXPECT_TRUE(val->Is<constant::Scalar<bool>>());
    EXPECT_TRUE(val->As<constant::Scalar<bool>>()->ValueAs<bool>());
}

TEST_F(IR_BuilderImplTest, EmitLiteral_Bool_False) {
    auto* expr = Expr(false);
    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* init = GlobalVarInitializer(m.Get());
    ASSERT_TRUE(Is<Constant>(init));
    auto* val = init->As<Constant>()->value;
    EXPECT_TRUE(val->Is<constant::Scalar<bool>>());
    EXPECT_FALSE(val->As<constant::Scalar<bool>>()->ValueAs<bool>());
}

TEST_F(IR_BuilderImplTest, EmitLiteral_F32) {
    auto* expr = Expr(1.2_f);
    GlobalVar("a", ty.f32(), builtin::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* init = GlobalVarInitializer(m.Get());
    ASSERT_TRUE(Is<Constant>(init));
    auto* val = init->As<Constant>()->value;
    EXPECT_TRUE(val->Is<constant::Scalar<f32>>());
    EXPECT_EQ(1.2_f, val->As<constant::Scalar<f32>>()->ValueAs<f32>());
}

TEST_F(IR_BuilderImplTest, EmitLiteral_F16) {
    Enable(builtin::Extension::kF16);
    auto* expr = Expr(1.2_h);
    GlobalVar("a", ty.f16(), builtin::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* init = GlobalVarInitializer(m.Get());
    ASSERT_TRUE(Is<Constant>(init));
    auto* val = init->As<Constant>()->value;
    EXPECT_TRUE(val->Is<constant::Scalar<f16>>());
    EXPECT_EQ(1.2_h, val->As<constant::Scalar<f16>>()->ValueAs<f32>());
}

TEST_F(IR_BuilderImplTest, EmitLiteral_I32) {
    auto* expr = Expr(-2_i);
    GlobalVar("a", ty.i32(), builtin::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* init = GlobalVarInitializer(m.Get());
    ASSERT_TRUE(Is<Constant>(init));
    auto* val = init->As<Constant>()->value;
    EXPECT_TRUE(val->Is<constant::Scalar<i32>>());
    EXPECT_EQ(-2_i, val->As<constant::Scalar<i32>>()->ValueAs<f32>());
}

TEST_F(IR_BuilderImplTest, EmitLiteral_U32) {
    auto* expr = Expr(2_u);
    GlobalVar("a", ty.u32(), builtin::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* init = GlobalVarInitializer(m.Get());
    ASSERT_TRUE(Is<Constant>(init));
    auto* val = init->As<Constant>()->value;
    EXPECT_TRUE(val->Is<constant::Scalar<u32>>());
    EXPECT_EQ(2_u, val->As<constant::Scalar<u32>>()->ValueAs<f32>());
}

}  // namespace
}  // namespace tint::ir
