// Copyright 2020 The Tint Authors.
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

#include "src/tint/writer/spirv/spv_dump.h"
#include "src/tint/writer/spirv/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

using SpvBuilderConstructorTest = TestHelper;

TEST_F(SpvBuilderConstructorTest, Const) {
    auto* c = Expr(42.2_f);
    auto* g = GlobalVar("g", ty.f32(), c, builtin::AddressSpace::kPrivate);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateConstructorExpression(g, c), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 42.2000008
)");
}

TEST_F(SpvBuilderConstructorTest, Type) {
    auto* t = vec3<f32>(1_f, 1_f, 3_f);
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateConstructorExpression(nullptr, t), 5u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
)");
}

TEST_F(SpvBuilderConstructorTest, Type_WithCasts) {
    auto* t = vec2<f32>(Call<f32>(1_i), Call<f32>(1_i));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateExpression(t), 4u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_WithAlias) {
    // type Int = i32
    // cast<Int>(2.3f)

    auto* alias = Alias("Int", ty.i32());
    auto* cast = Call(ty.Of(alias), 2.3_f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 2u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 2
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_IdentifierExpression_Param) {
    auto* var = Var("ident", ty.f32());

    auto* t = vec2<f32>(1_f, "ident");
    WrapInFunction(var, t);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.Diagnostics();

    EXPECT_EQ(b.GenerateExpression(t), 8u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Function %3
%4 = OpConstantNull %3
%5 = OpTypeVector %3 2
%6 = OpConstant %3 1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().variables()),
              R"(%1 = OpVariable %2 Function %4
)");

    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%7 = OpLoad %3 %1
%8 = OpCompositeConstruct %5 %6 %7
)");
}

TEST_F(SpvBuilderConstructorTest, Vector_Bitcast_Params) {
    auto* var = Var("v", vec3<f32>(1_f, 2_f, 3_f));
    auto* cast = Bitcast(ty.vec3<u32>(), var);
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.Diagnostics();
    ASSERT_EQ(b.GenerateExpression(cast), 10u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstant %2 3
%6 = OpConstantComposite %1 %3 %4 %5
%8 = OpTypePointer Function %1
%9 = OpConstantNull %1
%12 = OpTypeInt 32 0
%11 = OpTypeVector %12 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %7 %6
%13 = OpLoad %1 %7
%10 = OpBitcast %11 %13
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Bool_With_Bool) {
    auto* cast = Call<bool>(true);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateExpression(cast), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_I32_With_I32) {
    auto* cast = Call<i32>(2_i);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 2u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 2
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_U32_With_U32) {
    auto* cast = Call<u32>(2_u);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 2u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 2
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_F32_With_F32) {
    auto* cast = Call<f32>(2_f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 2u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 2
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_F16_With_F16) {
    Enable(builtin::Extension::kF16);

    auto* cast = Call<f16>(2_h);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 2u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 16
%2 = OpConstant %1 0x1p+1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec2_With_Bool_Literal) {
    auto* cast = vec2<bool>(true);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeBool
%1 = OpTypeVector %2 2
%3 = OpConstantTrue %2
%4 = OpConstantComposite %1 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec2_With_Bool_Var) {
    auto* var = Var("v", Expr(true));
    auto* cast = vec2<bool>(var);
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.Diagnostics();
    ASSERT_EQ(b.GenerateExpression(cast), 8u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%6 = OpTypeVector %1 2
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%7 = OpLoad %1 %3
%8 = OpCompositeConstruct %6 %7 %7
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec2_With_F32_Literal) {
    auto* cast = vec2<f32>(2_f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec2_With_F16_Literal) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec2<f16>(2_h);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 2
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec2_With_F32_F32) {
    auto* var = Decl(Var("x", ty.f32(), Expr(2_f)));
    auto* cast = vec2<f32>("x", "x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 9u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 2
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%6 = OpTypeVector %1 2
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%7 = OpLoad %1 %3
%8 = OpLoad %1 %3
%9 = OpCompositeConstruct %6 %7 %8
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec2_With_F16_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.f16(), Expr(2_h)));
    auto* cast = vec2<f16>("x", "x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 9u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 16
%2 = OpConstant %1 0x1p+1
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%6 = OpTypeVector %1 2
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%7 = OpLoad %1 %3
%8 = OpLoad %1 %3
%9 = OpCompositeConstruct %6 %7 %8
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec2_With_F32_F32_Const) {
    auto* cast = vec2<f32>(1_f, 2_f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 5u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstantComposite %1 %3 %4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec2_With_F16_F16_Const) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec2<f16>(1_h, 2_h);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 5u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 2
%3 = OpConstant %2 0x1p+0
%4 = OpConstant %2 0x1p+1
%5 = OpConstantComposite %1 %3 %4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec2_F32_With_Vec2) {
    auto* var = Decl(Var("x", ty.vec2<f32>(), vec2<f32>(1_f, 2_f)));
    auto* cast = vec2<f32>("x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 10u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstantComposite %1 %3 %4
%7 = OpTypePointer Function %1
%8 = OpConstantNull %1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %6 %5
%10 = OpLoad %1 %6
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec2_F16_With_Vec2) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.vec2<f16>(), vec2<f16>(1_h, 2_h)));
    auto* cast = vec2<f16>("x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 10u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 2
%3 = OpConstant %2 0x1p+0
%4 = OpConstant %2 0x1p+1
%5 = OpConstantComposite %1 %3 %4
%7 = OpTypePointer Function %1
%8 = OpConstantNull %1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %6 %5
%10 = OpLoad %1 %6
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec2_F32_With_Vec2_Const) {
    auto* cast = vec2<f32>(vec2<f32>(1_f, 2_f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 5u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstantComposite %1 %3 %4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec2_F16_With_Vec2_Const) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec2<f16>(vec2<f16>(1_h, 2_h));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 5u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 2
%3 = OpConstant %2 0x1p+0
%4 = OpConstant %2 0x1p+1
%5 = OpConstantComposite %1 %3 %4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_F32) {
    auto* var = Decl(Var("x", ty.f32(), Expr(2_f)));
    auto* cast = vec3<f32>("x", "x", "x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 10u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 2
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%6 = OpTypeVector %1 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%7 = OpLoad %1 %3
%8 = OpLoad %1 %3
%9 = OpLoad %1 %3
%10 = OpCompositeConstruct %6 %7 %8 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.f16(), Expr(2_h)));
    auto* cast = vec3<f16>("x", "x", "x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 10u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 16
%2 = OpConstant %1 0x1p+1
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%6 = OpTypeVector %1 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%7 = OpLoad %1 %3
%8 = OpLoad %1 %3
%9 = OpLoad %1 %3
%10 = OpCompositeConstruct %6 %7 %8 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_F32_Const) {
    auto* cast = vec3<f32>(1_f, 2_f, 3_f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstant %2 3
%6 = OpConstantComposite %1 %3 %4 %5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_F16_Const) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec3<f16>(1_h, 2_h, 3_h);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 3
%3 = OpConstant %2 0x1p+0
%4 = OpConstant %2 0x1p+1
%5 = OpConstant %2 0x1.8p+1
%6 = OpConstantComposite %1 %3 %4 %5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_Bool) {
    auto* var = Decl(Var("x", ty.bool_(), Expr(true)));
    auto* cast = vec3<bool>("x", "x", "x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 10u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%6 = OpTypeVector %1 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%7 = OpLoad %1 %3
%8 = OpLoad %1 %3
%9 = OpLoad %1 %3
%10 = OpCompositeConstruct %6 %7 %8 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_Bool_Const) {
    auto* cast = vec3<bool>(true, false, true);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 5u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeBool
%1 = OpTypeVector %2 3
%3 = OpConstantTrue %2
%4 = OpConstantNull %2
%5 = OpConstantComposite %1 %3 %4 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_F32_F32_F32) {
    auto* var = Decl(Var("x", ty.f32(), Expr(2_f)));
    auto* cast = vec3<f32>("x", "x", "x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 10u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 2
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%6 = OpTypeVector %1 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%7 = OpLoad %1 %3
%8 = OpLoad %1 %3
%9 = OpLoad %1 %3
%10 = OpCompositeConstruct %6 %7 %8 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_F16_F16_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.f16(), Expr(2_h)));
    auto* cast = vec3<f16>("x", "x", "x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 10u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 16
%2 = OpConstant %1 0x1p+1
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%6 = OpTypeVector %1 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%7 = OpLoad %1 %3
%8 = OpLoad %1 %3
%9 = OpLoad %1 %3
%10 = OpCompositeConstruct %6 %7 %8 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_F32_F32_F32_Const) {
    auto* cast = vec3<f32>(1_f, 2_f, 3_f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstant %2 3
%6 = OpConstantComposite %1 %3 %4 %5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_F16_F16_F16_Const) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec3<f16>(1_h, 2_h, 3_h);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 3
%3 = OpConstant %2 0x1p+0
%4 = OpConstant %2 0x1p+1
%5 = OpConstant %2 0x1.8p+1
%6 = OpConstantComposite %1 %3 %4 %5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_F32_Vec2) {
    auto* var = Decl(Var("x", ty.vec2<f32>(), vec2<f32>(2_f, 3_f)));
    auto* cast = vec3<f32>(1_f, "x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 14u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 2
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %4
%7 = OpTypePointer Function %1
%8 = OpConstantNull %1
%9 = OpTypeVector %2 3
%10 = OpConstant %2 1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %6 %5
%11 = OpLoad %1 %6
%12 = OpCompositeExtract %2 %11 0
%13 = OpCompositeExtract %2 %11 1
%14 = OpCompositeConstruct %9 %10 %12 %13
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_F16_Vec2) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.vec2<f16>(), vec2<f16>(2_h, 3_h)));
    auto* cast = vec3<f16>(1_h, "x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 14u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 2
%3 = OpConstant %2 0x1p+1
%4 = OpConstant %2 0x1.8p+1
%5 = OpConstantComposite %1 %3 %4
%7 = OpTypePointer Function %1
%8 = OpConstantNull %1
%9 = OpTypeVector %2 3
%10 = OpConstant %2 0x1p+0
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %6 %5
%11 = OpLoad %1 %6
%12 = OpCompositeExtract %2 %11 0
%13 = OpCompositeExtract %2 %11 1
%14 = OpCompositeConstruct %9 %10 %12 %13
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_F32_Vec2_Const) {
    auto* cast = vec3<f32>(1_f, vec2<f32>(2_f, 3_f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstant %2 3
%6 = OpConstantComposite %1 %3 %4 %5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_F16_Vec2_Const) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec3<f16>(1_h, vec2<f16>(2_h, 3_h));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 3
%3 = OpConstant %2 0x1p+0
%4 = OpConstant %2 0x1p+1
%5 = OpConstant %2 0x1.8p+1
%6 = OpConstantComposite %1 %3 %4 %5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_Vec2_F32) {
    auto* var = Decl(Var("x", ty.vec2<f32>(), vec2<f32>(1_f, 2_f)));
    auto* cast = vec3<f32>("x", 3_f);
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 14u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstantComposite %1 %3 %4
%7 = OpTypePointer Function %1
%8 = OpConstantNull %1
%9 = OpTypeVector %2 3
%13 = OpConstant %2 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %6 %5
%10 = OpLoad %1 %6
%11 = OpCompositeExtract %2 %10 0
%12 = OpCompositeExtract %2 %10 1
%14 = OpCompositeConstruct %9 %11 %12 %13
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_Vec2_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.vec2<f16>(), vec2<f16>(1_h, 2_h)));
    auto* cast = vec3<f16>("x", 3_h);
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 14u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 2
%3 = OpConstant %2 0x1p+0
%4 = OpConstant %2 0x1p+1
%5 = OpConstantComposite %1 %3 %4
%7 = OpTypePointer Function %1
%8 = OpConstantNull %1
%9 = OpTypeVector %2 3
%13 = OpConstant %2 0x1.8p+1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %6 %5
%10 = OpLoad %1 %6
%11 = OpCompositeExtract %2 %10 0
%12 = OpCompositeExtract %2 %10 1
%14 = OpCompositeConstruct %9 %11 %12 %13
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_Vec2_F32_Const) {
    auto* cast = vec3<f32>(vec2<f32>(1_f, 2_f), 3_f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstant %2 3
%6 = OpConstantComposite %1 %3 %4 %5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_Vec2_F16_Const) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec3<f16>(vec2<f16>(1_h, 2_h), 3_h);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 3
%3 = OpConstant %2 0x1p+0
%4 = OpConstant %2 0x1p+1
%5 = OpConstant %2 0x1.8p+1
%6 = OpConstantComposite %1 %3 %4 %5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_F32_With_Vec3) {
    auto* var = Decl(Var("x", ty.vec3<f32>(), vec3<f32>(1_f, 2_f, 3_f)));
    auto* cast = vec3<f32>("x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 11u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstant %2 3
%6 = OpConstantComposite %1 %3 %4 %5
%8 = OpTypePointer Function %1
%9 = OpConstantNull %1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %7 %6
%11 = OpLoad %1 %7
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_F16_With_Vec3) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.vec3<f16>(), vec3<f16>(1_h, 2_h, 3_h)));
    auto* cast = vec3<f16>("x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 11u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 3
%3 = OpConstant %2 0x1p+0
%4 = OpConstant %2 0x1p+1
%5 = OpConstant %2 0x1.8p+1
%6 = OpConstantComposite %1 %3 %4 %5
%8 = OpTypePointer Function %1
%9 = OpConstantNull %1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %7 %6
%11 = OpLoad %1 %7
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_F32_With_Vec3_Const) {
    auto* cast = vec3<f32>(vec3<f32>(1_f, 2_f, 3_f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstant %2 3
%6 = OpConstantComposite %1 %3 %4 %5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_F16_With_Vec3_Const) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec3<f16>(vec3<f16>(1_h, 2_h, 3_h));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 3
%3 = OpConstant %2 0x1p+0
%4 = OpConstant %2 0x1p+1
%5 = OpConstant %2 0x1.8p+1
%6 = OpConstantComposite %1 %3 %4 %5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_Bool) {
    auto* var = Decl(Var("x", ty.bool_(), Expr(true)));
    auto* cast = vec4<bool>("x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 8u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%6 = OpTypeVector %1 4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%7 = OpLoad %1 %3
%8 = OpCompositeConstruct %6 %7 %7 %7 %7
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_Bool_Const) {
    auto* cast = vec4<bool>(true);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeBool
%1 = OpTypeVector %2 4
%3 = OpConstantTrue %2
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F32) {
    auto* var = Decl(Var("x", ty.f32(), Expr(2_f)));
    auto* cast = vec4<f32>("x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 8u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 2
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%6 = OpTypeVector %1 4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%7 = OpLoad %1 %3
%8 = OpCompositeConstruct %6 %7 %7 %7 %7
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.f16(), Expr(2_h)));
    auto* cast = vec4<f16>("x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 8u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 16
%2 = OpConstant %1 0x1p+1
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%6 = OpTypeVector %1 4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%7 = OpLoad %1 %3
%8 = OpCompositeConstruct %6 %7 %7 %7 %7
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F32_Const) {
    auto* cast = vec4<f32>(2_f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F16_Const) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(2_h);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 4
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F32_F32_F32_F32) {
    auto* var = Decl(Var("x", ty.f32(), Expr(2_f)));
    auto* cast = vec4<f32>("x", "x", "x", "x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 11u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 2
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%6 = OpTypeVector %1 4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%7 = OpLoad %1 %3
%8 = OpLoad %1 %3
%9 = OpLoad %1 %3
%10 = OpLoad %1 %3
%11 = OpCompositeConstruct %6 %7 %8 %9 %10
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F16_F16_F16_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.f16(), Expr(2_h)));
    auto* cast = vec4<f16>("x", "x", "x", "x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 11u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 16
%2 = OpConstant %1 0x1p+1
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%6 = OpTypeVector %1 4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%7 = OpLoad %1 %3
%8 = OpLoad %1 %3
%9 = OpLoad %1 %3
%10 = OpLoad %1 %3
%11 = OpCompositeConstruct %6 %7 %8 %9 %10
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F32_F32_F32_F32_Const) {
    auto* cast = vec4<f32>(1_f, 2_f, 3_f, 4_f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 7u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstant %2 3
%6 = OpConstant %2 4
%7 = OpConstantComposite %1 %3 %4 %5 %6
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F16_F16_F16_F16_Const) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(1_h, 2_h, 3_h, 4_h);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 7u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 4
%3 = OpConstant %2 0x1p+0
%4 = OpConstant %2 0x1p+1
%5 = OpConstant %2 0x1.8p+1
%6 = OpConstant %2 0x1p+2
%7 = OpConstantComposite %1 %3 %4 %5 %6
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F32_F32_Vec2) {
    auto* var = Decl(Var("x", ty.vec2<f32>(), vec2<f32>(1_f, 2_f)));
    auto* cast = vec4<f32>(1_f, 2_f, "x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 13u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstantComposite %1 %3 %4
%7 = OpTypePointer Function %1
%8 = OpConstantNull %1
%9 = OpTypeVector %2 4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %6 %5
%10 = OpLoad %1 %6
%11 = OpCompositeExtract %2 %10 0
%12 = OpCompositeExtract %2 %10 1
%13 = OpCompositeConstruct %9 %3 %4 %11 %12
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F16_F16_Vec2) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.vec2<f16>(), vec2<f16>(1_h, 2_h)));
    auto* cast = vec4<f16>(1_h, 2_h, "x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 13u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 2
%3 = OpConstant %2 0x1p+0
%4 = OpConstant %2 0x1p+1
%5 = OpConstantComposite %1 %3 %4
%7 = OpTypePointer Function %1
%8 = OpConstantNull %1
%9 = OpTypeVector %2 4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %6 %5
%10 = OpLoad %1 %6
%11 = OpCompositeExtract %2 %10 0
%12 = OpCompositeExtract %2 %10 1
%13 = OpCompositeConstruct %9 %3 %4 %11 %12
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F32_F32_Vec2_Const) {
    auto* cast = vec4<f32>(1_f, 2_f, vec2<f32>(3_f, 4_f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 7u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstant %2 3
%6 = OpConstant %2 4
%7 = OpConstantComposite %1 %3 %4 %5 %6
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F16_F16_Vec2_Const) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(1_h, 2_h, vec2<f16>(3_h, 4_h));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 7u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 4
%3 = OpConstant %2 0x1p+0
%4 = OpConstant %2 0x1p+1
%5 = OpConstant %2 0x1.8p+1
%6 = OpConstant %2 0x1p+2
%7 = OpConstantComposite %1 %3 %4 %5 %6
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F32_Vec2_F32) {
    auto* var = Decl(Var("x", ty.vec2<f32>(), vec2<f32>(2_f, 3_f)));
    auto* cast = vec4<f32>(1_f, "x", 4_f);
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 15u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 2
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %4
%7 = OpTypePointer Function %1
%8 = OpConstantNull %1
%9 = OpTypeVector %2 4
%10 = OpConstant %2 1
%14 = OpConstant %2 4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %6 %5
%11 = OpLoad %1 %6
%12 = OpCompositeExtract %2 %11 0
%13 = OpCompositeExtract %2 %11 1
%15 = OpCompositeConstruct %9 %10 %12 %13 %14
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F16_Vec2_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.vec2<f16>(), vec2<f16>(2_h, 3_h)));
    auto* cast = vec4<f16>(1_h, "x", 4_h);
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 15u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 2
%3 = OpConstant %2 0x1p+1
%4 = OpConstant %2 0x1.8p+1
%5 = OpConstantComposite %1 %3 %4
%7 = OpTypePointer Function %1
%8 = OpConstantNull %1
%9 = OpTypeVector %2 4
%10 = OpConstant %2 0x1p+0
%14 = OpConstant %2 0x1p+2
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %6 %5
%11 = OpLoad %1 %6
%12 = OpCompositeExtract %2 %11 0
%13 = OpCompositeExtract %2 %11 1
%15 = OpCompositeConstruct %9 %10 %12 %13 %14
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F32_Vec2_F32_Const) {
    auto* cast = vec4<f32>(1_f, vec2<f32>(2_f, 3_f), 4_f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 7u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstant %2 3
%6 = OpConstant %2 4
%7 = OpConstantComposite %1 %3 %4 %5 %6
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F16_Vec2_F16_Const) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(1_h, vec2<f16>(2_h, 3_h), 4_h);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 7u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 4
%3 = OpConstant %2 0x1p+0
%4 = OpConstant %2 0x1p+1
%5 = OpConstant %2 0x1.8p+1
%6 = OpConstant %2 0x1p+2
%7 = OpConstantComposite %1 %3 %4 %5 %6
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_Vec2_F32_F32) {
    auto* var = Decl(Var("x", ty.vec2<f32>(), vec2<f32>(1_f, 2_f)));
    auto* cast = vec4<f32>("x", 3_f, 4_f);
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 15u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstantComposite %1 %3 %4
%7 = OpTypePointer Function %1
%8 = OpConstantNull %1
%9 = OpTypeVector %2 4
%13 = OpConstant %2 3
%14 = OpConstant %2 4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %6 %5
%10 = OpLoad %1 %6
%11 = OpCompositeExtract %2 %10 0
%12 = OpCompositeExtract %2 %10 1
%15 = OpCompositeConstruct %9 %11 %12 %13 %14
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_Vec2_F16_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.vec2<f16>(), vec2<f16>(1_h, 2_h)));
    auto* cast = vec4<f16>("x", 3_h, 4_h);
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 15u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 2
%3 = OpConstant %2 0x1p+0
%4 = OpConstant %2 0x1p+1
%5 = OpConstantComposite %1 %3 %4
%7 = OpTypePointer Function %1
%8 = OpConstantNull %1
%9 = OpTypeVector %2 4
%13 = OpConstant %2 0x1.8p+1
%14 = OpConstant %2 0x1p+2
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %6 %5
%10 = OpLoad %1 %6
%11 = OpCompositeExtract %2 %10 0
%12 = OpCompositeExtract %2 %10 1
%15 = OpCompositeConstruct %9 %11 %12 %13 %14
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_Vec2_F32_F32_Const) {
    auto* cast = vec4<f32>(vec2<f32>(1_f, 2_f), 3_f, 4_f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 7u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstant %2 3
%6 = OpConstant %2 4
%7 = OpConstantComposite %1 %3 %4 %5 %6
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_Vec2_F16_F16_Const) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(vec2<f16>(1_h, 2_h), 3_h, 4_h);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 7u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 4
%3 = OpConstant %2 0x1p+0
%4 = OpConstant %2 0x1p+1
%5 = OpConstant %2 0x1.8p+1
%6 = OpConstant %2 0x1p+2
%7 = OpConstantComposite %1 %3 %4 %5 %6
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_F32_With_Vec2_Vec2) {
    auto* var = Decl(Var("x", ty.vec2<f32>(), vec2<f32>(1_f, 2_f)));
    auto* cast = vec4<f32>("x", "x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 16u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstantComposite %1 %3 %4
%7 = OpTypePointer Function %1
%8 = OpConstantNull %1
%9 = OpTypeVector %2 4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %6 %5
%10 = OpLoad %1 %6
%11 = OpCompositeExtract %2 %10 0
%12 = OpCompositeExtract %2 %10 1
%13 = OpLoad %1 %6
%14 = OpCompositeExtract %2 %13 0
%15 = OpCompositeExtract %2 %13 1
%16 = OpCompositeConstruct %9 %11 %12 %14 %15
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_F16_With_Vec2_Vec2) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.vec2<f16>(), vec2<f16>(1_h, 2_h)));
    auto* cast = vec4<f16>("x", "x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 16u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 2
%3 = OpConstant %2 0x1p+0
%4 = OpConstant %2 0x1p+1
%5 = OpConstantComposite %1 %3 %4
%7 = OpTypePointer Function %1
%8 = OpConstantNull %1
%9 = OpTypeVector %2 4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %6 %5
%10 = OpLoad %1 %6
%11 = OpCompositeExtract %2 %10 0
%12 = OpCompositeExtract %2 %10 1
%13 = OpLoad %1 %6
%14 = OpCompositeExtract %2 %13 0
%15 = OpCompositeExtract %2 %13 1
%16 = OpCompositeConstruct %9 %11 %12 %14 %15
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_F32_With_Vec2_Vec2_Const) {
    auto* cast = vec4<f32>(vec2<f32>(1_f, 2_f), vec2<f32>(1_f, 2_f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 5u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstantComposite %1 %3 %4 %3 %4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_F16_With_Vec2_Vec2_Const) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(vec2<f16>(1_h, 2_h), vec2<f16>(1_h, 2_h));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 5u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 4
%3 = OpConstant %2 0x1p+0
%4 = OpConstant %2 0x1p+1
%5 = OpConstantComposite %1 %3 %4 %3 %4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F32_Vec3) {
    auto* var = Decl(Var("x", ty.vec3<f32>(), vec3<f32>(2_f, 2_f, 2_f)));
    auto* cast = vec4<f32>(2_f, "x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 13u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3
%6 = OpTypePointer Function %1
%7 = OpConstantNull %1
%8 = OpTypeVector %2 4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %5 %4
%9 = OpLoad %1 %5
%10 = OpCompositeExtract %2 %9 0
%11 = OpCompositeExtract %2 %9 1
%12 = OpCompositeExtract %2 %9 2
%13 = OpCompositeConstruct %8 %3 %10 %11 %12
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F16_Vec3) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.vec3<f16>(), vec3<f16>(2_h, 2_h, 2_h)));
    auto* cast = vec4<f16>(2_h, "x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 13u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 3
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3 %3
%6 = OpTypePointer Function %1
%7 = OpConstantNull %1
%8 = OpTypeVector %2 4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %5 %4
%9 = OpLoad %1 %5
%10 = OpCompositeExtract %2 %9 0
%11 = OpCompositeExtract %2 %9 1
%12 = OpCompositeExtract %2 %9 2
%13 = OpCompositeConstruct %8 %3 %10 %11 %12
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F32_Vec3_Const) {
    auto* cast = vec4<f32>(2_f, vec3<f32>(2_f, 2_f, 2_f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F16_Vec3_Const) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(2_h, vec3<f16>(2_h, 2_h, 2_h));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 4
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_Vec3_F32) {
    auto* var = Decl(Var("x", ty.vec3<f32>(), vec3<f32>(2_f, 2_f, 2_f)));
    auto* cast = vec4<f32>("x", 2_f);
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 13u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3
%6 = OpTypePointer Function %1
%7 = OpConstantNull %1
%8 = OpTypeVector %2 4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %5 %4
%9 = OpLoad %1 %5
%10 = OpCompositeExtract %2 %9 0
%11 = OpCompositeExtract %2 %9 1
%12 = OpCompositeExtract %2 %9 2
%13 = OpCompositeConstruct %8 %10 %11 %12 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_Vec3_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.vec3<f16>(), vec3<f16>(2_h, 2_h, 2_h)));
    auto* cast = vec4<f16>("x", 2_h);
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var));
    EXPECT_EQ(b.GenerateExpression(cast), 13u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 3
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3 %3
%6 = OpTypePointer Function %1
%7 = OpConstantNull %1
%8 = OpTypeVector %2 4
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %5 %4
%9 = OpLoad %1 %5
%10 = OpCompositeExtract %2 %9 0
%11 = OpCompositeExtract %2 %9 1
%12 = OpCompositeExtract %2 %9 2
%13 = OpCompositeConstruct %8 %10 %11 %12 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_Vec3_F32_Const) {
    auto* cast = vec4<f32>(vec3<f32>(2_f, 2_f, 2_f), 2_f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_Vec3_F16_Const) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(vec3<f16>(2_h, 2_h, 2_h), 2_h);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 4
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_F32_With_Vec4) {
    auto* value = vec4<f32>(2_f, 2_f, 2_f, 2_f);
    auto* cast = vec4<f32>(value);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_F16_With_Vec4) {
    Enable(builtin::Extension::kF16);

    auto* value = vec4<f16>(2_h, 2_h, 2_h, 2_h);
    auto* cast = vec4<f16>(value);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 4
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_F32_With_F32) {
    auto* ctor = Call<f32>(2_f);
    GlobalConst("g", ty.f32(), ctor);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeFloat 32
%6 = OpConstant %5 2
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %7 %6
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_F16_With_F16) {
    Enable(builtin::Extension::kF16);

    auto* ctor = Call<f16>(2_h);
    GlobalConst("g", ty.f16(), ctor);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeFloat 16
%6 = OpConstant %5 0x1p+1
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %7 %6
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_F32_With_F32) {
    auto* ctor = Call<f32>(2_f);
    GlobalVar("g", ty.f32(), builtin::AddressSpace::kPrivate, ctor);

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 2
%4 = OpTypePointer Private %1
%3 = OpVariable %4 Private %2
%6 = OpTypeVoid
%5 = OpTypeFunction %6
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_F16_With_F16) {
    Enable(builtin::Extension::kF16);

    auto* ctor = Call<f16>(2_h);
    GlobalVar("g", ty.f16(), builtin::AddressSpace::kPrivate, ctor);

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 16
%2 = OpConstant %1 0x1p+1
%4 = OpTypePointer Private %1
%3 = OpVariable %4 Private %2
%6 = OpTypeVoid
%5 = OpTypeFunction %6
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_U32_With_F32) {
    auto* ctor = Call<u32>(1.5_f);
    GlobalConst("g", ty.u32(), ctor);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeInt 32 0
%6 = OpConstant %5 1
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %7 %6
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_U32_With_F16) {
    Enable(builtin::Extension::kF16);

    auto* ctor = Call<u32>(1.5_h);
    GlobalConst("g", ty.u32(), ctor);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeInt 32 0
%6 = OpConstant %5 1
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %7 %6
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_U32_With_F32) {
    auto* ctor = Call<u32>(1.5_f);
    GlobalVar("g", ty.u32(), builtin::AddressSpace::kPrivate, ctor);

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 1
%4 = OpTypePointer Private %1
%3 = OpVariable %4 Private %2
%6 = OpTypeVoid
%5 = OpTypeFunction %6
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_U32_With_F16) {
    Enable(builtin::Extension::kF16);

    auto* ctor = Call<u32>(1.5_h);
    GlobalVar("g", ty.u32(), builtin::AddressSpace::kPrivate, ctor);

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 1
%4 = OpTypePointer Private %1
%3 = OpVariable %4 Private %2
%6 = OpTypeVoid
%5 = OpTypeFunction %6
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec2_With_F32) {
    auto* cast = vec2<f32>(2_f);
    GlobalConst("g", ty.vec2<f32>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 2
%7 = OpConstant %6 2
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec2_With_F16) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec2<f16>(2_h);
    GlobalConst("g", ty.vec2<f16>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 2
%7 = OpConstant %6 0x1p+1
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec2_With_F32) {
    auto* cast = vec2<f32>(2_f);
    auto* g = GlobalVar("g", ty.vec2<f32>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec2_With_F16) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec2<f16>(2_h);
    auto* g = GlobalVar("g", ty.vec2<f16>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 2
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec2_F32_With_Vec2) {
    auto* cast = vec2<f32>(vec2<f32>(2_f, 2_f));
    GlobalConst("g", ty.vec2<f32>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 2
%7 = OpConstant %6 2
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec2_F16_With_Vec2) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec2<f16>(vec2<f16>(2_h, 2_h));
    GlobalConst("g", ty.vec2<f16>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 2
%7 = OpConstant %6 0x1p+1
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec2_F32_With_Vec2) {
    auto* cast = vec2<f32>(vec2<f32>(2_f, 2_f));
    GlobalVar("a", ty.vec2<f32>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3
%6 = OpTypePointer Private %1
%5 = OpVariable %6 Private %4
%8 = OpTypeVoid
%7 = OpTypeFunction %8
)");

    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec2_F16_With_Vec2) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec2<f16>(vec2<f16>(2_h, 2_h));
    GlobalVar("a", ty.vec2<f16>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 2
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3
%6 = OpTypePointer Private %1
%5 = OpVariable %6 Private %4
%8 = OpTypeVoid
%7 = OpTypeFunction %8
)");

    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec3_F32_With_Vec3) {
    auto* cast = vec3<f32>(vec3<f32>(2_f, 2_f, 2_f));
    GlobalConst("g", ty.vec3<f32>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 3
%7 = OpConstant %6 2
%8 = OpConstantComposite %5 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec3_F16_With_Vec3) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec3<f16>(vec3<f16>(2_h, 2_h, 2_h));
    GlobalConst("g", ty.vec3<f16>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 3
%7 = OpConstant %6 0x1p+1
%8 = OpConstantComposite %5 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec3_F32_With_Vec3) {
    auto* cast = vec3<f32>(vec3<f32>(2_f, 2_f, 2_f));
    GlobalVar("a", ty.vec3<f32>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3
%6 = OpTypePointer Private %1
%5 = OpVariable %6 Private %4
%8 = OpTypeVoid
%7 = OpTypeFunction %8
)");

    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec3_F16_With_Vec3) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec3<f16>(vec3<f16>(2_h, 2_h, 2_h));
    GlobalVar("a", ty.vec3<f16>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 3
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3 %3
%6 = OpTypePointer Private %1
%5 = OpVariable %6 Private %4
%8 = OpTypeVoid
%7 = OpTypeFunction %8
)");

    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec4_F32_With_Vec4) {
    auto* cast = vec4<f32>(vec4<f32>(2_f, 2_f, 2_f, 2_f));
    GlobalConst("g", ty.vec4<f32>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 4
%7 = OpConstant %6 2
%8 = OpConstantComposite %5 %7 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec4_F16_With_Vec4) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(vec4<f16>(2_h, 2_h, 2_h, 2_h));
    GlobalConst("g", ty.vec4<f16>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 4
%7 = OpConstant %6 0x1p+1
%8 = OpConstantComposite %5 %7 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec4_F32_With_Vec4) {
    auto* cast = vec4<f32>(vec4<f32>(2_f, 2_f, 2_f, 2_f));
    GlobalVar("a", ty.vec4<f32>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3 %3
%6 = OpTypePointer Private %1
%5 = OpVariable %6 Private %4
%8 = OpTypeVoid
%7 = OpTypeFunction %8
)");

    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec4_F16_With_Vec4) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(vec4<f16>(2_h, 2_h, 2_h, 2_h));
    GlobalVar("a", ty.vec4<f16>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 4
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3 %3 %3
%6 = OpTypePointer Private %1
%5 = OpVariable %6 Private %4
%8 = OpTypeVoid
%7 = OpTypeFunction %8
)");

    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec3_With_F32) {
    auto* cast = vec3<f32>(2_f);
    GlobalConst("g", ty.vec3<f32>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 3
%7 = OpConstant %6 2
%8 = OpConstantComposite %5 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec3_With_F16) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec3<f16>(2_h);
    GlobalConst("g", ty.vec3<f16>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 3
%7 = OpConstant %6 0x1p+1
%8 = OpConstantComposite %5 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec3_With_F32) {
    auto* cast = vec3<f32>(2_f);
    auto* g = GlobalVar("g", ty.vec3<f32>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec3_With_F16) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec3<f16>(2_h);
    auto* g = GlobalVar("g", ty.vec3<f16>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 3
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec3_With_F32_Vec2) {
    auto* cast = vec3<f32>(2_f, vec2<f32>(2_f, 2_f));
    GlobalConst("g", ty.vec3<f32>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 3
%7 = OpConstant %6 2
%8 = OpConstantComposite %5 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec3_With_F16_Vec2) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec3<f16>(2_h, vec2<f16>(2_h, 2_h));
    GlobalConst("g", ty.vec3<f16>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 3
%7 = OpConstant %6 0x1p+1
%8 = OpConstantComposite %5 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec3_With_F32_Vec2) {
    auto* cast = vec3<f32>(2_f, vec2<f32>(2_f, 2_f));
    auto* g = GlobalVar("g", ty.vec3<f32>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec3_With_F16_Vec2) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec3<f16>(2_h, vec2<f16>(2_h, 2_h));
    auto* g = GlobalVar("g", ty.vec3<f16>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 3
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec3_With_Vec2_F32) {
    auto* cast = vec3<f32>(vec2<f32>(2_f, 2_f), 2_f);
    GlobalConst("g", ty.vec3<f32>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 3
%7 = OpConstant %6 2
%8 = OpConstantComposite %5 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec3_With_Vec2_F16) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec3<f16>(vec2<f16>(2_h, 2_h), 2_h);
    GlobalConst("g", ty.vec3<f16>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 3
%7 = OpConstant %6 0x1p+1
%8 = OpConstantComposite %5 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec3_With_Vec2_F32) {
    auto* cast = vec3<f32>(vec2<f32>(2_f, 2_f), 2_f);
    auto* g = GlobalVar("g", ty.vec3<f32>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec3_With_Vec2_F16) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec3<f16>(vec2<f16>(2_h, 2_h), 2_h);
    auto* g = GlobalVar("g", ty.vec3<f16>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 3
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec4_With_F32) {
    auto* cast = vec4<f32>(2_f);
    GlobalConst("g", ty.vec4<f32>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 4
%7 = OpConstant %6 2
%8 = OpConstantComposite %5 %7 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec4_With_F16) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(2_h);
    GlobalConst("g", ty.vec4<f16>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 4
%7 = OpConstant %6 0x1p+1
%8 = OpConstantComposite %5 %7 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec4_With_F32) {
    auto* cast = vec4<f32>(2_f);
    auto* g = GlobalVar("g", ty.vec4<f32>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec4_With_F16) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(2_h);
    auto* g = GlobalVar("g", ty.vec4<f16>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 4
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec4_With_F32_F32_Vec2) {
    auto* cast = vec4<f32>(2_f, 2_f, vec2<f32>(2_f, 2_f));
    GlobalConst("g", ty.vec4<f32>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 4
%7 = OpConstant %6 2
%8 = OpConstantComposite %5 %7 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec4_With_F16_F16_Vec2) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(2_h, 2_h, vec2<f16>(2_h, 2_h));
    GlobalConst("g", ty.vec4<f16>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 4
%7 = OpConstant %6 0x1p+1
%8 = OpConstantComposite %5 %7 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec4_With_F32_F32_Vec2) {
    auto* cast = vec4<f32>(2_f, 2_f, vec2<f32>(2_f, 2_f));
    auto* g = GlobalVar("g", ty.vec4<f32>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec4_With_F16_F16_Vec2) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(2_h, 2_h, vec2<f16>(2_h, 2_h));
    auto* g = GlobalVar("g", ty.vec4<f16>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 4
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec4_With_F32_Vec2_F32) {
    auto* cast = vec4<f32>(2_f, vec2<f32>(2_f, 2_f), 2_f);
    GlobalConst("g", ty.vec4<f32>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 4
%7 = OpConstant %6 2
%8 = OpConstantComposite %5 %7 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec4_With_F16_Vec2_F16) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(2_h, vec2<f16>(2_h, 2_h), 2_h);
    GlobalConst("g", ty.vec4<f16>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 4
%7 = OpConstant %6 0x1p+1
%8 = OpConstantComposite %5 %7 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec4_With_F32_Vec2_F32) {
    auto* cast = vec4<f32>(2_f, vec2<f32>(2_f, 2_f), 2_f);
    auto* g = GlobalVar("g", ty.vec4<f32>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec4_With_F16_Vec2_F16) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(2_h, vec2<f16>(2_h, 2_h), 2_h);
    auto* g = GlobalVar("g", ty.vec4<f16>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 4
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec4_With_Vec2_F32_F32) {
    auto* cast = vec4<f32>(vec2<f32>(2_f, 2_f), 2_f, 2_f);
    GlobalConst("g", ty.vec4<f32>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 4
%7 = OpConstant %6 2
%8 = OpConstantComposite %5 %7 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec4_With_Vec2_F16_F16) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(vec2<f16>(2_h, 2_h), 2_h, 2_h);
    GlobalConst("g", ty.vec4<f16>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 4
%7 = OpConstant %6 0x1p+1
%8 = OpConstantComposite %5 %7 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec4_With_Vec2_F32_F32) {
    auto* cast = vec4<f32>(vec2<f32>(2_f, 2_f), 2_f, 2_f);
    auto* g = GlobalVar("g", ty.vec4<f32>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec4_With_Vec2_F16_F16) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(vec2<f16>(2_h, 2_h), 2_h, 2_h);
    auto* g = GlobalVar("g", ty.vec4<f16>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 4
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec4_F32_With_Vec2_Vec2) {
    auto* cast = vec4<f32>(vec2<f32>(2_f, 2_f), vec2<f32>(2_f, 2_f));
    GlobalConst("g", ty.vec4<f32>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 4
%7 = OpConstant %6 2
%8 = OpConstantComposite %5 %7 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec4_F16_With_Vec2_Vec2) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(vec2<f16>(2_h, 2_h), vec2<f16>(2_h, 2_h));
    GlobalConst("g", ty.vec4<f16>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 4
%7 = OpConstant %6 0x1p+1
%8 = OpConstantComposite %5 %7 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec4_F32_With_Vec2_Vec2) {
    auto* cast = vec4<f32>(vec2<f32>(2_f, 2_f), vec2<f32>(2_f, 2_f));
    auto* g = GlobalVar("g", ty.vec4<f32>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec4_F16_With_Vec2_Vec2) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(vec2<f16>(2_h, 2_h), vec2<f16>(2_h, 2_h));
    auto* g = GlobalVar("g", ty.vec4<f16>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 4
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec4_With_F32_Vec3) {
    auto* cast = vec4<f32>(2_f, vec3<f32>(2_f, 2_f, 2_f));
    GlobalConst("g", ty.vec4<f32>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 4
%7 = OpConstant %6 2
%8 = OpConstantComposite %5 %7 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec4_With_F32_Vec3) {
    auto* cast = vec4<f32>(2_f, vec3<f32>(2_f, 2_f, 2_f));
    auto* g = GlobalVar("g", ty.vec4<f32>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec4_With_F16_Vec3) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(2_h, vec3<f16>(2_h, 2_h, 2_h));
    auto* g = GlobalVar("g", ty.vec4<f16>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 4
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec4_With_Vec3_F32) {
    auto* cast = vec4<f32>(vec3<f32>(2_f, 2_f, 2_f), 2_f);
    GlobalConst("g", ty.vec4<f32>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 4
%7 = OpConstant %6 2
%8 = OpConstantComposite %5 %7 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalConst_Vec4_With_Vec3_F16) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(vec3<f16>(2_h, 2_h, 2_h), 2_h);
    GlobalConst("g", ty.vec4<f16>(), cast);
    WrapInFunction(Decl(Var("l", Expr("g"))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 4
%7 = OpConstant %6 0x1p+1
%8 = OpConstantComposite %5 %7 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpStore %9 %8
OpReturn
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec4_With_Vec3_F32) {
    auto* cast = vec4<f32>(vec3<f32>(2_f, 2_f, 2_f), 2_f);
    auto* g = GlobalVar("g", ty.vec4<f32>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_GlobalVar_Vec4_With_Vec3_F16) {
    Enable(builtin::Extension::kF16);

    auto* cast = vec4<f16>(vec3<f16>(2_h, 2_h, 2_h), 2_h);
    auto* g = GlobalVar("g", ty.vec4<f16>(), builtin::AddressSpace::kPrivate, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 4
%3 = OpConstant %2 0x1p+1
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat2x2_F32_With_Vec2_Vec2) {
    auto* cast = mat2x2<f32>(vec2<f32>(2_f, 2_f), vec2<f32>(2_f, 2_f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 2
%1 = OpTypeMatrix %2 2
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4
%6 = OpConstantComposite %1 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat2x2_F16_With_Vec2_Vec2) {
    Enable(builtin::Extension::kF16);

    auto* cast = mat2x2<f16>(vec2<f16>(2_h, 2_h), vec2<f16>(2_h, 2_h));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 16
%2 = OpTypeVector %3 2
%1 = OpTypeMatrix %2 2
%4 = OpConstant %3 0x1p+1
%5 = OpConstantComposite %2 %4 %4
%6 = OpConstantComposite %1 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat3x2_F32_With_Vec2_Vec2_Vec2) {
    auto* cast = mat3x2<f32>(vec2<f32>(2_f, 2_f), vec2<f32>(2_f, 2_f), vec2<f32>(2_f, 2_f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 2
%1 = OpTypeMatrix %2 3
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat3x2_F16_With_Vec2_Vec2_Vec2) {
    Enable(builtin::Extension::kF16);

    auto* cast = mat3x2<f16>(vec2<f16>(2_h, 2_h), vec2<f16>(2_h, 2_h), vec2<f16>(2_h, 2_h));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 16
%2 = OpTypeVector %3 2
%1 = OpTypeMatrix %2 3
%4 = OpConstant %3 0x1p+1
%5 = OpConstantComposite %2 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat4x2_F32_With_Vec2_Vec2_Vec2_Vec2) {
    auto* cast = mat4x2<f32>(vec2<f32>(2_f, 2_f), vec2<f32>(2_f, 2_f), vec2<f32>(2_f, 2_f),
                             vec2<f32>(2_f, 2_f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 2
%1 = OpTypeMatrix %2 4
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat4x2_F16_With_Vec2_Vec2_Vec2_Vec2) {
    Enable(builtin::Extension::kF16);

    auto* cast = mat4x2<f16>(vec2<f16>(2_h, 2_h), vec2<f16>(2_h, 2_h), vec2<f16>(2_h, 2_h),
                             vec2<f16>(2_h, 2_h));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 16
%2 = OpTypeVector %3 2
%1 = OpTypeMatrix %2 4
%4 = OpConstant %3 0x1p+1
%5 = OpConstantComposite %2 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat2x3_F32_With_Vec3_Vec3) {
    auto* cast = mat2x3<f32>(vec3<f32>(2_f, 2_f, 2_f), vec3<f32>(2_f, 2_f, 2_f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 2
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat2x3_F16_With_Vec3_Vec3) {
    Enable(builtin::Extension::kF16);

    auto* cast = mat2x3<f16>(vec3<f16>(2_h, 2_h, 2_h), vec3<f16>(2_h, 2_h, 2_h));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 16
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 2
%4 = OpConstant %3 0x1p+1
%5 = OpConstantComposite %2 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat3x3_F32_With_Vec3_Vec3_Vec3) {
    auto* cast =
        mat3x3<f32>(vec3<f32>(2_f, 2_f, 2_f), vec3<f32>(2_f, 2_f, 2_f), vec3<f32>(2_f, 2_f, 2_f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 3
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat3x3_F16_With_Vec3_Vec3_Vec3) {
    Enable(builtin::Extension::kF16);

    auto* cast =
        mat3x3<f16>(vec3<f16>(2_h, 2_h, 2_h), vec3<f16>(2_h, 2_h, 2_h), vec3<f16>(2_h, 2_h, 2_h));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 16
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 3
%4 = OpConstant %3 0x1p+1
%5 = OpConstantComposite %2 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat4x3_F32_With_Vec3_Vec3_Vec3_Vec3) {
    auto* cast = mat4x3<f32>(vec3<f32>(2_f, 2_f, 2_f), vec3<f32>(2_f, 2_f, 2_f),
                             vec3<f32>(2_f, 2_f, 2_f), vec3<f32>(2_f, 2_f, 2_f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 4
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat4x3_F16_With_Vec3_Vec3_Vec3_Vec3) {
    Enable(builtin::Extension::kF16);

    auto* cast = mat4x3<f16>(vec3<f16>(2_h, 2_h, 2_h), vec3<f16>(2_h, 2_h, 2_h),
                             vec3<f16>(2_h, 2_h, 2_h), vec3<f16>(2_h, 2_h, 2_h));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 16
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 4
%4 = OpConstant %3 0x1p+1
%5 = OpConstantComposite %2 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat2x4_F32_With_Vec4_Vec4) {
    auto* cast = mat2x4<f32>(vec4<f32>(2_f, 2_f, 2_f, 2_f), vec4<f32>(2_f, 2_f, 2_f, 2_f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 4
%1 = OpTypeMatrix %2 2
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat2x4_F16_With_Vec4_Vec4) {
    Enable(builtin::Extension::kF16);

    auto* cast = mat2x4<f16>(vec4<f16>(2_h, 2_h, 2_h, 2_h), vec4<f16>(2_h, 2_h, 2_h, 2_h));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 16
%2 = OpTypeVector %3 4
%1 = OpTypeMatrix %2 2
%4 = OpConstant %3 0x1p+1
%5 = OpConstantComposite %2 %4 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat3x4_F32_With_Vec4_Vec4_Vec4) {
    auto* cast = mat3x4<f32>(vec4<f32>(2_f, 2_f, 2_f, 2_f), vec4<f32>(2_f, 2_f, 2_f, 2_f),
                             vec4<f32>(2_f, 2_f, 2_f, 2_f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 4
%1 = OpTypeMatrix %2 3
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat3x4_F16_With_Vec4_Vec4_Vec4) {
    Enable(builtin::Extension::kF16);

    auto* cast = mat3x4<f16>(vec4<f16>(2_h, 2_h, 2_h, 2_h), vec4<f16>(2_h, 2_h, 2_h, 2_h),
                             vec4<f16>(2_h, 2_h, 2_h, 2_h));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 16
%2 = OpTypeVector %3 4
%1 = OpTypeMatrix %2 3
%4 = OpConstant %3 0x1p+1
%5 = OpConstantComposite %2 %4 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat4x4_F32_With_Vec4_Vec4_Vec4_Vec4) {
    auto* cast = mat4x4<f32>(vec4<f32>(2_f, 2_f, 2_f, 2_f), vec4<f32>(2_f, 2_f, 2_f, 2_f),
                             vec4<f32>(2_f, 2_f, 2_f, 2_f), vec4<f32>(2_f, 2_f, 2_f, 2_f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 4
%1 = OpTypeMatrix %2 4
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat4x4_F16_With_Vec4_Vec4_Vec4_Vec4) {
    Enable(builtin::Extension::kF16);

    auto* cast = mat4x4<f16>(vec4<f16>(2_h, 2_h, 2_h, 2_h), vec4<f16>(2_h, 2_h, 2_h, 2_h),
                             vec4<f16>(2_h, 2_h, 2_h, 2_h), vec4<f16>(2_h, 2_h, 2_h, 2_h));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 16
%2 = OpTypeVector %3 4
%1 = OpTypeMatrix %2 4
%4 = OpConstant %3 0x1p+1
%5 = OpConstantComposite %2 %4 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Array_5_F32) {
    auto* cast = array<f32, 5>(2_f, 2_f, 2_f, 2_f, 2_f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%3 = OpTypeInt 32 0
%4 = OpConstant %3 5
%1 = OpTypeArray %2 %4
%5 = OpConstant %2 2
%6 = OpConstantComposite %1 %5 %5 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Array_5_F16) {
    Enable(builtin::Extension::kF16);

    auto* cast = array<f16, 5>(2_h, 2_h, 2_h, 2_h, 2_h);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%3 = OpTypeInt 32 0
%4 = OpConstant %3 5
%1 = OpTypeArray %2 %4
%5 = OpConstant %2 0x1p+1
%6 = OpConstantComposite %1 %5 %5 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Array_2_Vec3_F32) {
    auto* first = vec3<f32>(1_f, 2_f, 3_f);
    auto* second = vec3<f32>(1_f, 2_f, 3_f);
    auto* t = Call(ty.array(ty.vec3<f32>(), 2_u), first, second);
    WrapInFunction(t);
    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(t), 10u);
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 3
%4 = OpTypeInt 32 0
%5 = OpConstant %4 2
%1 = OpTypeArray %2 %5
%6 = OpConstant %3 1
%7 = OpConstant %3 2
%8 = OpConstant %3 3
%9 = OpConstantComposite %2 %6 %7 %8
%10 = OpConstantComposite %1 %9 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Array_2_Vec3_F16) {
    Enable(builtin::Extension::kF16);

    auto* first = vec3<f16>(1_h, 2_h, 3_h);
    auto* second = vec3<f16>(1_h, 2_h, 3_h);
    auto* t = Call(ty.array(ty.vec3<f16>(), 2_u), first, second);
    WrapInFunction(t);
    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(t), 10u);
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 16
%2 = OpTypeVector %3 3
%4 = OpTypeInt 32 0
%5 = OpConstant %4 2
%1 = OpTypeArray %2 %5
%6 = OpConstant %3 0x1p+0
%7 = OpConstant %3 0x1p+1
%8 = OpConstant %3 0x1.8p+1
%9 = OpConstantComposite %2 %6 %7 %8
%10 = OpConstantComposite %1 %9 %9
)");
}

TEST_F(SpvBuilderConstructorTest, CommonInitializer_TwoVectors) {
    auto* v1 = vec3<f32>(2_f, 2_f, 2_f);
    auto* v2 = vec3<f32>(2_f, 2_f, 2_f);
    WrapInFunction(WrapInStatement(v1), WrapInStatement(v2));

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(v1), 4u);
    EXPECT_EQ(b.GenerateExpression(v2), 4u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, CommonInitializer_TwoArrays) {
    auto* a1 = array<f32, 3>(2_f, 2_f, 2_f);
    auto* a2 = array<f32, 3>(2_f, 2_f, 2_f);
    WrapInFunction(WrapInStatement(a1), WrapInStatement(a2));

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(a1), 6u);
    EXPECT_EQ(b.GenerateExpression(a2), 6u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%3 = OpTypeInt 32 0
%4 = OpConstant %3 3
%1 = OpTypeArray %2 %4
%5 = OpConstant %2 2
%6 = OpConstantComposite %1 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, CommonInitializer_Array_VecArray) {
    // Test that initializers of different types with the same values produce
    // different OpConstantComposite instructions.
    // crbug.com/tint/777
    auto* a1 = array<f32, 2>(1_f, 2_f);
    auto* a2 = vec2<f32>(1_f, 2_f);
    WrapInFunction(WrapInStatement(a1), WrapInStatement(a2));
    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_EQ(b.GenerateExpression(a1), 7u);
    EXPECT_EQ(b.GenerateExpression(a2), 9u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%3 = OpTypeInt 32 0
%4 = OpConstant %3 2
%1 = OpTypeArray %2 %4
%5 = OpConstant %2 1
%6 = OpConstant %2 2
%7 = OpConstantComposite %1 %5 %6
%8 = OpTypeVector %2 2
%9 = OpConstantComposite %8 %5 %6
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Struct) {
    auto* s = Structure("my_struct", utils::Vector{
                                         Member("a", ty.f32()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    auto* t = Call(ty.Of(s), 2_f, vec3<f32>(2_f, 2_f, 2_f));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateExpression(t), 6u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%3 = OpTypeVector %2 3
%1 = OpTypeStruct %2 %3
%4 = OpConstant %2 2
%5 = OpConstantComposite %3 %4 %4 %4
%6 = OpConstantComposite %1 %4 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ZeroInit_F32) {
    auto* t = Call<f32>();

    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateExpression(t), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 32
%2 = OpConstantNull %1
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ZeroInit_F16) {
    Enable(builtin::Extension::kF16);

    auto* t = Call<f16>();

    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateExpression(t), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 16
%2 = OpConstantNull %1
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ZeroInit_I32) {
    auto* t = Call<i32>();

    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateExpression(t), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstantNull %1
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ZeroInit_U32) {
    auto* t = Call<u32>();

    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateExpression(t), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstantNull %1
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ZeroInit_Bool) {
    auto* t = Call<bool>();

    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateExpression(t), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeBool
%2 = OpConstantNull %1
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ZeroInit_Vector) {
    auto* t = vec2<i32>();

    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateExpression(t), 3u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeVector %2 2
%3 = OpConstantNull %1
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ZeroInit_Matrix_F32) {
    auto* t = mat4x2<f32>();

    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateExpression(t), 4u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 2
%1 = OpTypeMatrix %2 4
%4 = OpConstantNull %1
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ZeroInit_Matrix_F16) {
    Enable(builtin::Extension::kF16);

    auto* t = mat4x2<f16>();

    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateExpression(t), 4u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 16
%2 = OpTypeVector %3 2
%1 = OpTypeMatrix %2 4
%4 = OpConstantNull %1
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ZeroInit_Array) {
    auto* t = array<i32, 2>();

    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateExpression(t), 5u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeInt 32 0
%4 = OpConstant %3 2
%1 = OpTypeArray %2 %4
%5 = OpConstantNull %1
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ZeroInit_Struct) {
    auto* s = Structure("my_struct", utils::Vector{Member("a", ty.f32())});
    auto* t = Call(ty.Of(s));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_EQ(b.GenerateExpression(t), 3u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeStruct %2
%3 = OpConstantNull %1
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_U32_To_I32) {
    auto* var = Decl(Var("x", ty.u32(), Expr(2_u)));
    auto* cast = Call<i32>("x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 2
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%7 = OpTypeInt 32 1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%8 = OpLoad %1 %3
%6 = OpBitcast %7 %8
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_F32_To_I32) {
    auto* var = Decl(Var("x", ty.f32(), Expr(2.4_f)));
    auto* cast = Call<i32>("x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 2.4000001
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%7 = OpTypeInt 32 1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%8 = OpLoad %1 %3
%6 = OpConvertFToS %7 %8
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_F16_To_I32) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.f16(), Expr(2.4_h)));
    auto* cast = Call<i32>("x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 16
%2 = OpConstant %1 0x1.33p+1
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%7 = OpTypeInt 32 1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%8 = OpLoad %1 %3
%6 = OpConvertFToS %7 %8
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_I32_To_U32) {
    auto* var = Decl(Var("x", ty.i32(), Expr(2_i)));
    auto* cast = Call<u32>("x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 2
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%7 = OpTypeInt 32 0
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%8 = OpLoad %1 %3
%6 = OpBitcast %7 %8
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_F32_To_U32) {
    auto* var = Decl(Var("x", ty.f32(), Expr(2.4_f)));
    auto* cast = Call<u32>("x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 2.4000001
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%7 = OpTypeInt 32 0
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%8 = OpLoad %1 %3
%6 = OpConvertFToU %7 %8
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_F16_To_U32) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.f16(), Expr(2.4_h)));
    auto* cast = Call<u32>("x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 16
%2 = OpConstant %1 0x1.33p+1
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%7 = OpTypeInt 32 0
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%8 = OpLoad %1 %3
%6 = OpConvertFToU %7 %8
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_I32_To_F32) {
    auto* var = Decl(Var("x", ty.i32(), Expr(2_i)));
    auto* cast = Call<f32>("x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 2
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%7 = OpTypeFloat 32
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%8 = OpLoad %1 %3
%6 = OpConvertSToF %7 %8
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_U32_To_F32) {
    auto* var = Decl(Var("x", ty.u32(), Expr(2_u)));
    auto* cast = Call<f32>("x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 2
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%7 = OpTypeFloat 32
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%8 = OpLoad %1 %3
%6 = OpConvertUToF %7 %8
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_F16_To_F32) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.f16(), Expr(2_h)));
    auto* cast = Call<f32>("x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 16
%2 = OpConstant %1 0x1p+1
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%7 = OpTypeFloat 32
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%8 = OpLoad %1 %3
%6 = OpFConvert %7 %8
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_I32_To_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.i32(), Expr(2_i)));
    auto* cast = Call<f16>("x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 2
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%7 = OpTypeFloat 16
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%8 = OpLoad %1 %3
%6 = OpConvertSToF %7 %8
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_U32_To_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.u32(), Expr(2_u)));
    auto* cast = Call<f16>("x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 2
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%7 = OpTypeFloat 16
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%8 = OpLoad %1 %3
%6 = OpConvertUToF %7 %8
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_F32_To_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = Decl(Var("x", ty.f32(), Expr(2_f)));
    auto* cast = Call<f16>("x");
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 2
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%7 = OpTypeFloat 16
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %3 %2
%8 = OpLoad %1 %3
%6 = OpFConvert %7 %8
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_Vectors_U32_to_I32) {
    auto* var = GlobalVar("i", ty.vec3<u32>(), builtin::AddressSpace::kPrivate);

    auto* cast = vec3<i32>("i");
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeInt 32 1
%7 = OpTypeVector %8 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%9 = OpLoad %3 %1
%6 = OpBitcast %7 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_Vectors_F32_to_I32) {
    auto* var = GlobalVar("i", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* cast = vec3<i32>("i");
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeInt 32 1
%7 = OpTypeVector %8 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%9 = OpLoad %3 %1
%6 = OpConvertFToS %7 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_Vectors_F16_to_I32) {
    Enable(builtin::Extension::kF16);

    auto* var = GlobalVar("i", ty.vec3<f16>(), builtin::AddressSpace::kPrivate);

    auto* cast = vec3<i32>("i");
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 16
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeInt 32 1
%7 = OpTypeVector %8 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%9 = OpLoad %3 %1
%6 = OpConvertFToS %7 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_Vectors_I32_to_U32) {
    auto* var = GlobalVar("i", ty.vec3<i32>(), builtin::AddressSpace::kPrivate);

    auto* cast = vec3<u32>("i");
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeInt 32 0
%7 = OpTypeVector %8 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%9 = OpLoad %3 %1
%6 = OpBitcast %7 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_Vectors_F32_to_U32) {
    auto* var = GlobalVar("i", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* cast = vec3<u32>("i");
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeInt 32 0
%7 = OpTypeVector %8 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%9 = OpLoad %3 %1
%6 = OpConvertFToU %7 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_Vectors_F16_to_U32) {
    Enable(builtin::Extension::kF16);

    auto* var = GlobalVar("i", ty.vec3<f16>(), builtin::AddressSpace::kPrivate);

    auto* cast = vec3<u32>("i");
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 16
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeInt 32 0
%7 = OpTypeVector %8 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%9 = OpLoad %3 %1
%6 = OpConvertFToU %7 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_Vectors_I32_to_F32) {
    auto* var = GlobalVar("i", ty.vec3<i32>(), builtin::AddressSpace::kPrivate);

    auto* cast = vec3<f32>("i");
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeFloat 32
%7 = OpTypeVector %8 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%9 = OpLoad %3 %1
%6 = OpConvertSToF %7 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_Vectors_U32_to_F32) {
    auto* var = GlobalVar("i", ty.vec3<u32>(), builtin::AddressSpace::kPrivate);

    auto* cast = vec3<f32>("i");
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeFloat 32
%7 = OpTypeVector %8 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%9 = OpLoad %3 %1
%6 = OpConvertUToF %7 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_Vectors_F16_to_F32) {
    Enable(builtin::Extension::kF16);

    auto* var = GlobalVar("i", ty.vec3<f16>(), builtin::AddressSpace::kPrivate);

    auto* cast = vec3<f32>("i");
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 16
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeFloat 32
%7 = OpTypeVector %8 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%9 = OpLoad %3 %1
%6 = OpFConvert %7 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_Vectors_I32_to_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = GlobalVar("i", ty.vec3<i32>(), builtin::AddressSpace::kPrivate);

    auto* cast = vec3<f16>("i");
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeFloat 16
%7 = OpTypeVector %8 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%9 = OpLoad %3 %1
%6 = OpConvertSToF %7 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_Vectors_U32_to_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = GlobalVar("i", ty.vec3<u32>(), builtin::AddressSpace::kPrivate);

    auto* cast = vec3<f16>("i");
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeFloat 16
%7 = OpTypeVector %8 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%9 = OpLoad %3 %1
%6 = OpConvertUToF %7 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_Vectors_F32_to_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = GlobalVar("i", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* cast = vec3<f16>("i");
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeFloat 16
%7 = OpTypeVector %8 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(%9 = OpLoad %3 %1
%6 = OpFConvert %7 %9
)");
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_GlobalVectorWithAllConstInitializers) {
    // vec3<f32>(1.0, 2.0, 3.0)  -> true
    auto* t = vec3<f32>(1_f, 2_f, 3_f);
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_GlobalArrayWithAllConstInitializers) {
    // array<vec3<f32>, 2u>(vec3<f32>(1.0, 2.0, 3.0), vec3<f32>(1.0, 2.0, 3.0))
    //   -> true
    auto* t =
        Call(ty.array(ty.vec3<f32>(), 2_u), vec3<f32>(1_f, 2_f, 3_f), vec3<f32>(1_f, 2_f, 3_f));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_GlobalVectorWithMatchingTypeInitializers) {
    // vec2<f32>(f32(1.0), f32(2.0))  -> false

    auto* t = vec2<f32>(Call<f32>(1_f), Call<f32>(2_f));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_GlobalWithTypeConversionInitializer) {
    // vec2<f32>(f32(1), f32(2)) -> false

    auto* t = vec2<f32>(Call<f32>(1_i), Call<f32>(2_i));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_FALSE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_VectorWithAllConstInitializers) {
    // vec3<f32>(1.0, 2.0, 3.0)  -> true

    auto* t = vec3<f32>(1_f, 2_f, 3_f);
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_Vector_WithIdent) {
    // vec3<f32>(a, b, c)  -> false

    GlobalVar("a", ty.f32(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.f32(), builtin::AddressSpace::kPrivate);
    GlobalVar("c", ty.f32(), builtin::AddressSpace::kPrivate);

    auto* t = vec3<f32>("a", "b", "c");
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_FALSE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_ArrayWithAllConstInitializers) {
    // array<vec3<f32>, 2u>(vec3<f32>(1.0, 2.0, 3.0), vec3<f32>(1.0, 2.0, 3.0))
    //   -> true

    auto* first = vec3<f32>(1_f, 2_f, 3_f);
    auto* second = vec3<f32>(1_f, 2_f, 3_f);

    auto* t = Call(ty.array(ty.vec3<f32>(), 2_u), first, second);
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_VectorWithTypeConversionConstInitializers) {
    // vec2<f32>(f32(1), f32(2))  -> false

    auto* t = vec2<f32>(Call<f32>(1_i), Call<f32>(2_i));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_FALSE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_BitCastScalars) {
    auto* t = vec2<u32>(Call<u32>(1_i), Call<u32>(1_i));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_FALSE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_Struct) {
    auto* s = Structure("my_struct", utils::Vector{
                                         Member("a", ty.f32()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    auto* t = Call(ty.Of(s), 2_f, vec3<f32>(2_f, 2_f, 2_f));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_Struct_WithIdentSubExpression) {
    auto* s = Structure("my_struct", utils::Vector{
                                         Member("a", ty.f32()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    GlobalVar("a", ty.f32(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* t = Call(ty.Of(s), "a", "b");
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_FALSE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, ConstantCompositeScoping) {
    // if (true) {
    //    let x = vec3<f32>(1.0, 2.0, 3.0);
    // }
    // let y = vec3<f32>(1.0, 2.0, 3.0); // Reuses the ID 'x'

    WrapInFunction(If(true, Block(Decl(Let("x", vec3<f32>(1_f, 2_f, 3_f))))),
                   Decl(Let("y", vec3<f32>(1_f, 2_f, 3_f))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %3 "test_function"
OpExecutionMode %3 LocalSize 1 1 1
OpName %3 "test_function"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeBool
%6 = OpConstantTrue %5
%10 = OpTypeFloat 32
%9 = OpTypeVector %10 3
%11 = OpConstant %10 1
%12 = OpConstant %10 2
%13 = OpConstant %10 3
%14 = OpConstantComposite %9 %11 %12 %13
%3 = OpFunction %2 None %1
%4 = OpLabel
OpSelectionMerge %7 None
OpBranchConditional %6 %8 %7
%8 = OpLabel
OpBranch %7
%7 = OpLabel
OpReturn
OpFunctionEnd
)");
    Validate(b);
}

// TODO(crbug.com/tint/1155) Implement when overrides are fully implemented.
// TEST_F(SpvBuilderConstructorTest, SpecConstantCompositeScoping)

TEST_F(SpvBuilderConstructorTest, CompositeConstructScoping) {
    // var one = 1.0;
    // if (true) {
    //    let x = vec3<f32>(one, 2.0, 3.0);
    // }
    // let y = vec3<f32>(one, 2.0, 3.0); // Mustn't reuse the ID 'x'

    WrapInFunction(Decl(Var("one", Expr(1_f))),
                   If(true, Block(Decl(Let("x", vec3<f32>("one", 2_f, 3_f))))),
                   Decl(Let("y", vec3<f32>("one", 2_f, 3_f))));

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %3 "test_function"
OpExecutionMode %3 LocalSize 1 1 1
OpName %3 "test_function"
OpName %7 "one"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeFloat 32
%6 = OpConstant %5 1
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
%10 = OpTypeBool
%11 = OpConstantTrue %10
%14 = OpTypeVector %5 3
%16 = OpConstant %5 2
%17 = OpConstant %5 3
%3 = OpFunction %2 None %1
%4 = OpLabel
%7 = OpVariable %8 Function %9
OpStore %7 %6
OpSelectionMerge %12 None
OpBranchConditional %11 %13 %12
%13 = OpLabel
%15 = OpLoad %5 %7
%18 = OpCompositeConstruct %14 %15 %16 %17
OpBranch %12
%12 = OpLabel
%19 = OpLoad %5 %7
%20 = OpCompositeConstruct %14 %19 %16 %17
OpReturn
OpFunctionEnd
)");
    Validate(b);
}
}  // namespace
}  // namespace tint::writer::spirv
