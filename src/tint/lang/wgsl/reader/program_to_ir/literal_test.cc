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
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/wgsl/ast/case_selector.h"
#include "src/tint/lang/wgsl/ast/int_literal_expression.h"
#include "src/tint/lang/wgsl/helpers/ir_program_test.h"

namespace tint::wgsl::reader {
namespace {

ir::Value* GlobalVarInitializer(ir::Module& m) {
    if (m.root_block->Length() == 0u) {
        ADD_FAILURE() << "m.root_block has no instruction";
        return nullptr;
    }

    auto instr = m.root_block->Instructions();
    auto* var = instr->As<ir::Var>();
    if (!var) {
        ADD_FAILURE() << "m.root_block.instructions[0] was not a var";
        return nullptr;
    }
    return var->Initializer();
}

using namespace tint::number_suffixes;  // NOLINT

using ProgramToIRLiteralTest = helpers::IRProgramTest;

TEST_F(ProgramToIRLiteralTest, EmitLiteral_Bool_True) {
    auto* expr = Expr(true);
    GlobalVar("a", ty.bool_(), core::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* init = GlobalVarInitializer(m.Get());
    ASSERT_TRUE(Is<ir::Constant>(init));
    auto* val = init->As<ir::Constant>()->Value();
    EXPECT_TRUE(val->Is<constant::Scalar<bool>>());
    EXPECT_TRUE(val->As<constant::Scalar<bool>>()->ValueAs<bool>());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_Bool_False) {
    auto* expr = Expr(false);
    GlobalVar("a", ty.bool_(), core::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* init = GlobalVarInitializer(m.Get());
    ASSERT_TRUE(Is<ir::Constant>(init));
    auto* val = init->As<ir::Constant>()->Value();
    EXPECT_TRUE(val->Is<constant::Scalar<bool>>());
    EXPECT_FALSE(val->As<constant::Scalar<bool>>()->ValueAs<bool>());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_Bool_Deduped) {
    GlobalVar("a", ty.bool_(), core::AddressSpace::kPrivate, Expr(true));
    GlobalVar("b", ty.bool_(), core::AddressSpace::kPrivate, Expr(false));
    GlobalVar("c", ty.bool_(), core::AddressSpace::kPrivate, Expr(true));
    GlobalVar("d", ty.bool_(), core::AddressSpace::kPrivate, Expr(false));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto itr = m.Get().root_block->begin();
    auto* var_a = (*itr)->As<ir::Var>();
    ++itr;

    ASSERT_NE(var_a, nullptr);
    auto* var_b = (*itr)->As<ir::Var>();
    ++itr;

    ASSERT_NE(var_b, nullptr);
    auto* var_c = (*itr)->As<ir::Var>();
    ++itr;

    ASSERT_NE(var_c, nullptr);
    auto* var_d = (*itr)->As<ir::Var>();
    ASSERT_NE(var_d, nullptr);

    ASSERT_EQ(var_a->Initializer(), var_c->Initializer());
    ASSERT_EQ(var_b->Initializer(), var_d->Initializer());
    ASSERT_NE(var_a->Initializer(), var_b->Initializer());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_F32) {
    auto* expr = Expr(1.2_f);
    GlobalVar("a", ty.f32(), core::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* init = GlobalVarInitializer(m.Get());
    ASSERT_TRUE(Is<ir::Constant>(init));
    auto* val = init->As<ir::Constant>()->Value();
    EXPECT_TRUE(val->Is<constant::Scalar<f32>>());
    EXPECT_EQ(1.2_f, val->As<constant::Scalar<f32>>()->ValueAs<f32>());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_F32_Deduped) {
    GlobalVar("a", ty.f32(), core::AddressSpace::kPrivate, Expr(1.2_f));
    GlobalVar("b", ty.f32(), core::AddressSpace::kPrivate, Expr(1.25_f));
    GlobalVar("c", ty.f32(), core::AddressSpace::kPrivate, Expr(1.2_f));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto itr = m.Get().root_block->begin();
    auto* var_a = (*itr)->As<ir::Var>();
    ASSERT_NE(var_a, nullptr);
    ++itr;

    auto* var_b = (*itr)->As<ir::Var>();
    ASSERT_NE(var_b, nullptr);
    ++itr;

    auto* var_c = (*itr)->As<ir::Var>();
    ASSERT_NE(var_c, nullptr);

    ASSERT_EQ(var_a->Initializer(), var_c->Initializer());
    ASSERT_NE(var_a->Initializer(), var_b->Initializer());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_F16) {
    Enable(core::Extension::kF16);
    auto* expr = Expr(1.2_h);
    GlobalVar("a", ty.f16(), core::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* init = GlobalVarInitializer(m.Get());
    ASSERT_TRUE(Is<ir::Constant>(init));
    auto* val = init->As<ir::Constant>()->Value();
    EXPECT_TRUE(val->Is<constant::Scalar<f16>>());
    EXPECT_EQ(1.2_h, val->As<constant::Scalar<f16>>()->ValueAs<f32>());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_F16_Deduped) {
    Enable(core::Extension::kF16);
    GlobalVar("a", ty.f16(), core::AddressSpace::kPrivate, Expr(1.2_h));
    GlobalVar("b", ty.f16(), core::AddressSpace::kPrivate, Expr(1.25_h));
    GlobalVar("c", ty.f16(), core::AddressSpace::kPrivate, Expr(1.2_h));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto itr = m.Get().root_block->begin();
    auto* var_a = (*itr)->As<ir::Var>();
    ASSERT_NE(var_a, nullptr);
    ++itr;

    auto* var_b = (*itr)->As<ir::Var>();
    ASSERT_NE(var_b, nullptr);
    ++itr;

    auto* var_c = (*itr)->As<ir::Var>();
    ASSERT_NE(var_c, nullptr);

    ASSERT_EQ(var_a->Initializer(), var_c->Initializer());
    ASSERT_NE(var_a->Initializer(), var_b->Initializer());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_I32) {
    auto* expr = Expr(-2_i);
    GlobalVar("a", ty.i32(), core::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* init = GlobalVarInitializer(m.Get());
    ASSERT_TRUE(Is<ir::Constant>(init));
    auto* val = init->As<ir::Constant>()->Value();
    EXPECT_TRUE(val->Is<constant::Scalar<i32>>());
    EXPECT_EQ(-2_i, val->As<constant::Scalar<i32>>()->ValueAs<f32>());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_I32_Deduped) {
    GlobalVar("a", ty.i32(), core::AddressSpace::kPrivate, Expr(-2_i));
    GlobalVar("b", ty.i32(), core::AddressSpace::kPrivate, Expr(2_i));
    GlobalVar("c", ty.i32(), core::AddressSpace::kPrivate, Expr(-2_i));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto itr = m.Get().root_block->begin();
    auto* var_a = (*itr)->As<ir::Var>();
    ASSERT_NE(var_a, nullptr);
    ++itr;

    auto* var_b = (*itr)->As<ir::Var>();
    ASSERT_NE(var_b, nullptr);
    ++itr;

    auto* var_c = (*itr)->As<ir::Var>();
    ASSERT_NE(var_c, nullptr);

    ASSERT_EQ(var_a->Initializer(), var_c->Initializer());
    ASSERT_NE(var_a->Initializer(), var_b->Initializer());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_U32) {
    auto* expr = Expr(2_u);
    GlobalVar("a", ty.u32(), core::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* init = GlobalVarInitializer(m.Get());
    ASSERT_TRUE(Is<ir::Constant>(init));
    auto* val = init->As<ir::Constant>()->Value();
    EXPECT_TRUE(val->Is<constant::Scalar<u32>>());
    EXPECT_EQ(2_u, val->As<constant::Scalar<u32>>()->ValueAs<f32>());
}

TEST_F(ProgramToIRLiteralTest, EmitLiteral_U32_Deduped) {
    GlobalVar("a", ty.u32(), core::AddressSpace::kPrivate, Expr(2_u));
    GlobalVar("b", ty.u32(), core::AddressSpace::kPrivate, Expr(3_u));
    GlobalVar("c", ty.u32(), core::AddressSpace::kPrivate, Expr(2_u));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto itr = m.Get().root_block->begin();
    auto* var_a = (*itr)->As<ir::Var>();
    ASSERT_NE(var_a, nullptr);
    ++itr;

    auto* var_b = (*itr)->As<ir::Var>();
    ASSERT_NE(var_b, nullptr);
    ++itr;

    auto* var_c = (*itr)->As<ir::Var>();
    ASSERT_NE(var_c, nullptr);

    ASSERT_EQ(var_a->Initializer(), var_c->Initializer());
    ASSERT_NE(var_a->Initializer(), var_b->Initializer());
}

}  // namespace
}  // namespace tint::wgsl::reader
