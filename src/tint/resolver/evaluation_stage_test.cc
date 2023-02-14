// Copyright 2022 The Tint Authors.
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

#include "src/tint/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/tint/resolver/resolver_test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverEvaluationStageTest = ResolverTest;

TEST_F(ResolverEvaluationStageTest, Literal_i32) {
    auto* expr = Expr(123_i);
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Literal_f32) {
    auto* expr = Expr(123_f);
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Vector_Init) {
    auto* expr = vec3<f32>();
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Vector_Init_Const_Const) {
    // const f = 1.f;
    // vec2<f32>(f, f);
    auto* f = Const("f", Expr(1_f));
    auto* expr = vec2<f32>(f, f);
    WrapInFunction(f, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(f)->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Vector_Init_Runtime_Runtime) {
    // var f = 1.f;
    // vec2<f32>(f, f);
    auto* f = Var("f", Expr(1_f));
    auto* expr = vec2<f32>(f, f);
    WrapInFunction(f, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(f)->Stage(), sem::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, Vector_Conv_Const) {
    // const f = 1.f;
    // vec2<u32>(vec2<f32>(f));
    auto* f = Const("f", Expr(1_f));
    auto* expr = vec2<u32>(vec2<f32>(f));
    WrapInFunction(f, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(f)->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Vector_Conv_Runtime) {
    // var f = 1.f;
    // vec2<u32>(vec2<f32>(f));
    auto* f = Var("f", Expr(1_f));
    auto* expr = vec2<u32>(vec2<f32>(f));
    WrapInFunction(f, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(f)->Stage(), sem::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, Matrix_Init) {
    auto* expr = mat2x2<f32>();
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Array_Init) {
    auto* expr = array<f32, 3>();
    WrapInFunction(expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Array_Init_Const_Const) {
    // const f = 1.f;
    // array<f32, 2>(f, f);
    auto* f = Const("f", Expr(1_f));
    auto* expr = Call(ty.array<f32, 2>(), f, f);
    WrapInFunction(f, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(f)->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Array_Init_Const_Override) {
    // const f1 = 1.f;
    // override f2 = 2.f;
    // array<f32, 2>(f1, f2);
    auto* f1 = Const("f1", Expr(1_f));
    auto* f2 = Override("f2", Expr(2_f));
    auto* expr = Call(ty.array<f32, 2>(), f1, f2);
    WrapInFunction(f1, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(f1)->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(f2)->Stage(), sem::EvaluationStage::kOverride);
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kOverride);
}

TEST_F(ResolverEvaluationStageTest, Array_Init_Override_Runtime) {
    // override f1 = 1.f;
    // var f2 = 2.f;
    // array<f32, 2>(f1, f2);
    auto* f1 = Override("f1", Expr(1_f));
    auto* f2 = Var("f2", Expr(2_f));
    auto* expr = Call(ty.array<f32, 2>(), f1, f2);
    WrapInFunction(f2, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(f1)->Stage(), sem::EvaluationStage::kOverride);
    EXPECT_EQ(Sem().Get(f2)->Stage(), sem::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, Array_Init_Const_Runtime) {
    // const f1 = 1.f;
    // var f2 = 2.f;
    // array<f32, 2>(f1, f2);
    auto* f1 = Const("f1", Expr(1_f));
    auto* f2 = Var("f2", Expr(2_f));
    auto* expr = Call(ty.array<f32, 2>(), f1, f2);
    WrapInFunction(f1, f2, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(f1)->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(f2)->Stage(), sem::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, Array_Init_Runtime_Runtime) {
    // var f = 1.f;
    // array<f32, 2>(f, f);
    auto* f = Var("f", Expr(1_f));
    auto* expr = Call(ty.array<f32, 2>(), f, f);
    WrapInFunction(f, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(f)->Stage(), sem::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, IndexAccessor_Const_Const) {
    // const vec = vec4<f32>();
    // const idx = 1_i;
    // vec[idx]
    auto* vec = Const("vec", vec4<f32>());
    auto* idx = Const("idx", Expr(1_i));
    auto* expr = IndexAccessor(vec, idx);
    WrapInFunction(vec, idx, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(vec)->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(idx)->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, IndexAccessor_Runtime_Const) {
    // var vec = vec4<f32>();
    // const idx = 1_i;
    // vec[idx]
    auto* vec = Var("vec", vec4<f32>());
    auto* idx = Const("idx", Expr(1_i));
    auto* expr = IndexAccessor(vec, idx);
    WrapInFunction(vec, idx, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(vec)->Stage(), sem::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(idx)->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, IndexAccessor_Const_Override) {
    // const vec = vec4<f32>();
    // override idx = 1_i;
    // vec[idx]
    auto* vec = Const("vec", vec4<f32>());
    auto* idx = Override("idx", Expr(1_i));
    auto* expr = IndexAccessor(vec, idx);
    WrapInFunction(vec, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(vec)->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(idx)->Stage(), sem::EvaluationStage::kOverride);
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kOverride);
}

TEST_F(ResolverEvaluationStageTest, IndexAccessor_Const_Runtime) {
    // const vec = vec4<f32>();
    // let idx = 1_i;
    // vec[idx]
    auto* vec = Const("vec", vec4<f32>());
    auto* idx = Let("idx", Expr(1_i));
    auto* expr = IndexAccessor(vec, idx);
    WrapInFunction(vec, idx, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(vec)->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(idx)->Stage(), sem::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, Swizzle_Const) {
    // const vec = S();
    // vec.m
    auto* vec = Const("vec", vec4<f32>());
    auto* expr = MemberAccessor(vec, "xz");
    WrapInFunction(vec, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(vec)->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Swizzle_Runtime) {
    // var vec = S();
    // vec.m
    auto* vec = Var("vec", vec4<f32>());
    auto* expr = MemberAccessor(vec, "rg");
    WrapInFunction(vec, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(vec)->Stage(), sem::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, MemberAccessor_Const) {
    // struct S { m : i32 };
    // const str = S();
    // str.m
    Structure("S", utils::Vector{Member("m", ty.i32())});
    auto* str = Const("str", Call("S"));
    auto* expr = MemberAccessor(str, "m");
    WrapInFunction(str, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(str)->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, MemberAccessor_Runtime) {
    // struct S { m : i32 };
    // var str = S();
    // str.m
    Structure("S", utils::Vector{Member("m", ty.i32())});
    auto* str = Var("str", Call("S"));
    auto* expr = MemberAccessor(str, "m");
    WrapInFunction(str, expr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(str)->Stage(), sem::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(expr)->Stage(), sem::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, Binary_Runtime) {
    // let one = 1;
    // let result = (one == 1) && (one == 1);
    auto* one = Let("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal("one", 1_a);
    auto* binary = LogicalAnd(lhs, rhs);
    auto* result = Let("result", binary);
    WrapInFunction(one, result);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(lhs)->Stage(), sem::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(rhs)->Stage(), sem::EvaluationStage::kRuntime);
    EXPECT_EQ(Sem().Get(binary)->Stage(), sem::EvaluationStage::kRuntime);
}

TEST_F(ResolverEvaluationStageTest, Binary_Const) {
    // const one = 1;
    // const result = (one == 1) && (one == 1);
    auto* one = Const("one", Expr(1_a));
    auto* lhs = Equal("one", 1_a);
    auto* rhs = Equal("one", 1_a);
    auto* binary = LogicalAnd(lhs, rhs);
    auto* result = Const("result", binary);
    WrapInFunction(one, result);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(lhs)->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(rhs)->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(binary)->Stage(), sem::EvaluationStage::kConstant);
}

TEST_F(ResolverEvaluationStageTest, Binary_NotEvaluated) {
    // const one = 1;
    // const result = (one == 0) && (one == 1);
    auto* one = Const("one", Expr(1_a));
    auto* lhs = Equal("one", 0_a);
    auto* rhs = Equal("one", 1_a);
    auto* binary = LogicalAnd(lhs, rhs);
    auto* result = Const("result", binary);
    WrapInFunction(one, result);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(lhs)->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(Sem().Get(rhs)->Stage(), sem::EvaluationStage::kNotEvaluated);
    EXPECT_EQ(Sem().Get(binary)->Stage(), sem::EvaluationStage::kConstant);
}

}  // namespace
}  // namespace tint::resolver
