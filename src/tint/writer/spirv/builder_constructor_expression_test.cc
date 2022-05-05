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
    auto* c = Expr(42.2f);
    auto* g = Global("g", ty.f32(), c, ast::StorageClass::kPrivate);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateConstructorExpression(g, c), 2u);
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 42.2000008
)");
}

TEST_F(SpvBuilderConstructorTest, Type_WithCasts_OutsideFunction_IsError) {
    auto* t = Construct<f32>(Construct<u32>(1_i));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateExpression(t), 0u);
    EXPECT_TRUE(b.has_error()) << b.error();
    EXPECT_EQ(b.error(),
              "Internal error: trying to add SPIR-V instruction 124 outside a "
              "function");
}

TEST_F(SpvBuilderConstructorTest, Type) {
    auto* t = vec3<f32>(1.0f, 1.0f, 3.0f);
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateConstructorExpression(nullptr, t), 5u);
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
)");
}

TEST_F(SpvBuilderConstructorTest, Type_WithCasts) {
    auto* t = vec2<f32>(Construct<f32>(1_i), Construct<f32>(1_i));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateExpression(t), 7u);
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%4 = OpTypeInt 32 1
%5 = OpConstant %4 1
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%3 = OpConvertSToF %2 %5
%6 = OpConvertSToF %2 %5
%7 = OpCompositeConstruct %1 %3 %6
)");
}

TEST_F(SpvBuilderConstructorTest, Type_WithAlias) {
    // type Int = i32
    // cast<Int>(2.3f)

    auto* alias = Alias("Int", ty.i32());
    auto* cast = Construct(ty.Of(alias), 2.3f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 1u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeFloat 32
%4 = OpConstant %3 2.29999995
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%1 = OpConvertFToS %2 %4
)");
}

TEST_F(SpvBuilderConstructorTest, Type_IdentifierExpression_Param) {
    auto* var = Var("ident", ty.f32());

    auto* t = vec2<f32>(1.0f, "ident");
    WrapInFunction(var, t);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();

    EXPECT_EQ(b.GenerateExpression(t), 8u);
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Function %3
%4 = OpConstantNull %3
%5 = OpTypeVector %3 2
%6 = OpConstant %3 1
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
              R"(%1 = OpVariable %2 Function %4
)");

    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%7 = OpLoad %3 %1
%8 = OpCompositeConstruct %5 %6 %7
)");
}

TEST_F(SpvBuilderConstructorTest, Vector_Bitcast_Params) {
    auto* t = vec2<u32>(Construct<u32>(1_i), Construct<u32>(1_i));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateExpression(t), 7u);
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 0
%1 = OpTypeVector %2 2
%4 = OpTypeInt 32 1
%5 = OpConstant %4 1
)");

    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%3 = OpBitcast %2 %5
%6 = OpBitcast %2 %5
%7 = OpCompositeConstruct %1 %3 %6
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Bool_With_Bool) {
    auto* cast = Construct<bool>(true);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateExpression(cast), 3u);
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeBool
%3 = OpConstantTrue %2
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_I32_With_I32) {
    auto* cast = Construct<i32>(2_i);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 3u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpConstant %2 2
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_U32_With_U32) {
    auto* cast = Construct<u32>(2_u);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 3u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 0
%3 = OpConstant %2 2
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_F32_With_F32) {
    auto* cast = Construct<f32>(2.0f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 3u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%3 = OpConstant %2 2
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec2_With_Bool_Literal) {
    auto* cast = vec2<bool>(true);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeBool
%1 = OpTypeVector %2 2
%3 = OpConstantTrue %2
%4 = OpConstantComposite %1 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec2_With_Bool_Var) {
    auto* var = Var("v", nullptr, Expr(true));
    auto* cast = vec2<bool>(var);
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();
    ASSERT_EQ(b.GenerateExpression(cast), 8u) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%6 = OpTypeVector %1 2
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(OpStore %3 %2
%7 = OpLoad %1 %3
%8 = OpCompositeConstruct %6 %7 %7
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec2_With_F32_Literal) {
    auto* cast = vec2<f32>(2.0f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec2_With_F32_Var) {
    auto* var = Var("v", nullptr, Expr(2.0f));
    auto* cast = vec2<f32>(var);
    WrapInFunction(var, cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();
    ASSERT_EQ(b.GenerateExpression(cast), 8u) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 2
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
%6 = OpTypeVector %1 2
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"(OpStore %3 %2
%7 = OpLoad %1 %3
%8 = OpCompositeConstruct %6 %7 %7
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec2_With_F32_F32) {
    auto* cast = vec2<f32>(2.0f, 2.0f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec2_With_Vec2) {
    auto* value = vec2<f32>(2.0f, 2.0f);
    auto* cast = vec2<f32>(value);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 5u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 2
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_F32) {
    auto* cast = vec3<f32>(2.0f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_Bool) {
    auto* cast = vec3<bool>(true);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeBool
%1 = OpTypeVector %2 3
%3 = OpConstantTrue %2
%4 = OpConstantComposite %1 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_F32_F32_F32) {
    auto* cast = vec3<f32>(2.0f, 2.0f, 2.0f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_F32_Vec2) {
    auto* cast = vec3<f32>(2.0f, vec2<f32>(2.0f, 2.0f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 8u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 2
%4 = OpTypeVector %2 2
%5 = OpConstantComposite %4 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%6 = OpCompositeExtract %2 %5 0
%7 = OpCompositeExtract %2 %5 1
%8 = OpCompositeConstruct %1 %3 %6 %7
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_Vec2_F32) {
    auto* cast = vec3<f32>(vec2<f32>(2.0f, 2.0f), 2.0f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 8u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpTypeVector %2 2
%4 = OpConstant %2 2
%5 = OpConstantComposite %3 %4 %4
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%6 = OpCompositeExtract %2 %5 0
%7 = OpCompositeExtract %2 %5 1
%8 = OpCompositeConstruct %1 %6 %7 %4
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec3_With_Vec3) {
    auto* value = vec3<f32>(2.0f, 2.0f, 2.0f);
    auto* cast = vec3<f32>(value);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 5u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 3
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_Bool) {
    auto* cast = vec4<bool>(true);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeBool
%1 = OpTypeVector %2 4
%3 = OpConstantTrue %2
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F32) {
    auto* cast = vec4<f32>(2.0f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F32_F32_F32_F32) {
    auto* cast = vec4<f32>(2.0f, 2.0f, 2.0f, 2.0f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 4u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F32_F32_Vec2) {
    auto* cast = vec4<f32>(2.0f, 2.0f, vec2<f32>(2.0f, 2.0f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 8u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpTypeVector %2 2
%5 = OpConstantComposite %4 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%6 = OpCompositeExtract %2 %5 0
%7 = OpCompositeExtract %2 %5 1
%8 = OpCompositeConstruct %1 %3 %3 %6 %7
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F32_Vec2_F32) {
    auto* cast = vec4<f32>(2.0f, vec2<f32>(2.0f, 2.0f), 2.0f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 8u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpTypeVector %2 2
%5 = OpConstantComposite %4 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%6 = OpCompositeExtract %2 %5 0
%7 = OpCompositeExtract %2 %5 1
%8 = OpCompositeConstruct %1 %3 %6 %7 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_Vec2_F32_F32) {
    auto* cast = vec4<f32>(vec2<f32>(2.0f, 2.0f), 2.0f, 2.0f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 8u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpTypeVector %2 2
%4 = OpConstant %2 2
%5 = OpConstantComposite %3 %4 %4
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%6 = OpCompositeExtract %2 %5 0
%7 = OpCompositeExtract %2 %5 1
%8 = OpCompositeConstruct %1 %6 %7 %4 %4
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_Vec2_Vec2) {
    auto* cast = vec4<f32>(vec2<f32>(2.0f, 2.0f), vec2<f32>(2.0f, 2.0f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 10u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpTypeVector %2 2
%4 = OpConstant %2 2
%5 = OpConstantComposite %3 %4 %4
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%6 = OpCompositeExtract %2 %5 0
%7 = OpCompositeExtract %2 %5 1
%8 = OpCompositeExtract %2 %5 0
%9 = OpCompositeExtract %2 %5 1
%10 = OpCompositeConstruct %1 %6 %7 %8 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_F32_Vec3) {
    auto* cast = vec4<f32>(2.0f, vec3<f32>(2.0f, 2.0f, 2.0f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 9u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpTypeVector %2 3
%5 = OpConstantComposite %4 %3 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%6 = OpCompositeExtract %2 %5 0
%7 = OpCompositeExtract %2 %5 1
%8 = OpCompositeExtract %2 %5 2
%9 = OpCompositeConstruct %1 %3 %6 %7 %8
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_Vec3_F32) {
    auto* cast = vec4<f32>(vec3<f32>(2.0f, 2.0f, 2.0f), 2.0f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 9u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpTypeVector %2 3
%4 = OpConstant %2 2
%5 = OpConstantComposite %3 %4 %4 %4
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%6 = OpCompositeExtract %2 %5 0
%7 = OpCompositeExtract %2 %5 1
%8 = OpCompositeExtract %2 %5 2
%9 = OpCompositeConstruct %1 %6 %7 %8 %4
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Vec4_With_Vec4) {
    auto* value = vec4<f32>(2.0f, 2.0f, 2.0f, 2.0f);
    auto* cast = vec4<f32>(value);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 5u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 4
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4 %4
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"()");
}

TEST_F(SpvBuilderConstructorTest, Type_ModuleScope_F32_With_F32) {
    auto* ctor = Construct<f32>(2.0f);
    GlobalConst("g", ty.f32(), ctor);

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 2
%4 = OpTypeVoid
%3 = OpTypeFunction %4
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_ModuleScope_U32_With_F32) {
    auto* ctor = Construct<u32>(1.5f);
    GlobalConst("g", ty.u32(), ctor);

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 1
%4 = OpTypeVoid
%3 = OpTypeFunction %4
)");
    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_ModuleScope_Vec2_With_F32) {
    auto* cast = vec2<f32>(2.0f);
    auto* g = Global("g", ty.vec2<f32>(), cast, ast::StorageClass::kPrivate);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ModuleScope_Vec2_With_Vec2) {
    auto* cast = vec2<f32>(vec2<f32>(2.0f, 2.0f));
    GlobalConst("a", ty.vec2<f32>(), cast);

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3
%6 = OpTypeVoid
%5 = OpTypeFunction %6
)");

    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_ModuleScope_Vec3_With_Vec3) {
    auto* cast = vec3<f32>(vec3<f32>(2.0f, 2.0f, 2.0f));
    GlobalConst("a", ty.vec3<f32>(), cast);

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3
%6 = OpTypeVoid
%5 = OpTypeFunction %6
)");

    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_ModuleScope_Vec4_With_Vec4) {
    auto* cast = vec4<f32>(vec4<f32>(2.0f, 2.0f, 2.0f, 2.0f));
    GlobalConst("a", ty.vec4<f32>(), cast);

    spirv::Builder& b = SanitizeAndBuild();
    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3 %3
%6 = OpTypeVoid
%5 = OpTypeFunction %6
)");

    Validate(b);
}

TEST_F(SpvBuilderConstructorTest, Type_ModuleScope_Vec3_With_F32) {
    auto* cast = vec3<f32>(2.0f);
    auto* g = Global("g", ty.vec3<f32>(), cast, ast::StorageClass::kPrivate);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ModuleScope_Vec3_With_F32_Vec2) {
    auto* cast = vec3<f32>(2.0f, vec2<f32>(2.0f, 2.0f));
    auto* g = Global("g", ty.vec3<f32>(), cast, ast::StorageClass::kPrivate);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 11u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 2
%4 = OpTypeVector %2 2
%5 = OpConstantComposite %4 %3 %3
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%6 = OpSpecConstantOp %2 CompositeExtract %5 8
%10 = OpConstant %7 1
%9 = OpSpecConstantOp %2 CompositeExtract %5 10
%11 = OpSpecConstantComposite %1 %3 %6 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ModuleScope_Vec3_With_Vec2_F32) {
    auto* cast = vec3<f32>(vec2<f32>(2.0f, 2.0f), 2.0f);
    auto* g = Global("g", ty.vec3<f32>(), cast, ast::StorageClass::kPrivate);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 11u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpTypeVector %2 2
%4 = OpConstant %2 2
%5 = OpConstantComposite %3 %4 %4
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%6 = OpSpecConstantOp %2 CompositeExtract %5 8
%10 = OpConstant %7 1
%9 = OpSpecConstantOp %2 CompositeExtract %5 10
%11 = OpSpecConstantComposite %1 %6 %9 %4
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ModuleScope_Vec4_With_F32) {
    auto* cast = vec4<f32>(2.0f);
    auto* g = Global("g", ty.vec4<f32>(), cast, ast::StorageClass::kPrivate);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 4u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ModuleScope_Vec4_With_F32_F32_Vec2) {
    auto* cast = vec4<f32>(2.0f, 2.0f, vec2<f32>(2.0f, 2.0f));
    auto* g = Global("g", ty.vec4<f32>(), cast, ast::StorageClass::kPrivate);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 11u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpTypeVector %2 2
%5 = OpConstantComposite %4 %3 %3
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%6 = OpSpecConstantOp %2 CompositeExtract %5 8
%10 = OpConstant %7 1
%9 = OpSpecConstantOp %2 CompositeExtract %5 10
%11 = OpSpecConstantComposite %1 %3 %3 %6 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ModuleScope_Vec4_With_F32_Vec2_F32) {
    auto* cast = vec4<f32>(2.0f, vec2<f32>(2.0f, 2.0f), 2.0f);
    auto* g = Global("g", ty.vec4<f32>(), cast, ast::StorageClass::kPrivate);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 11u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpTypeVector %2 2
%5 = OpConstantComposite %4 %3 %3
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%6 = OpSpecConstantOp %2 CompositeExtract %5 8
%10 = OpConstant %7 1
%9 = OpSpecConstantOp %2 CompositeExtract %5 10
%11 = OpSpecConstantComposite %1 %3 %6 %9 %3
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ModuleScope_Vec4_With_Vec2_F32_F32) {
    auto* cast = vec4<f32>(vec2<f32>(2.0f, 2.0f), 2.0f, 2.0f);
    auto* g = Global("g", ty.vec4<f32>(), cast, ast::StorageClass::kPrivate);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 11u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpTypeVector %2 2
%4 = OpConstant %2 2
%5 = OpConstantComposite %3 %4 %4
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%6 = OpSpecConstantOp %2 CompositeExtract %5 8
%10 = OpConstant %7 1
%9 = OpSpecConstantOp %2 CompositeExtract %5 10
%11 = OpSpecConstantComposite %1 %6 %9 %4 %4
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ModuleScope_Vec4_With_Vec2_Vec2) {
    auto* cast = vec4<f32>(vec2<f32>(2.0f, 2.0f), vec2<f32>(2.0f, 2.0f));
    auto* g = Global("g", ty.vec4<f32>(), cast, ast::StorageClass::kPrivate);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 13u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpTypeVector %2 2
%4 = OpConstant %2 2
%5 = OpConstantComposite %3 %4 %4
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%6 = OpSpecConstantOp %2 CompositeExtract %5 8
%10 = OpConstant %7 1
%9 = OpSpecConstantOp %2 CompositeExtract %5 10
%11 = OpSpecConstantOp %2 CompositeExtract %5 8
%12 = OpSpecConstantOp %2 CompositeExtract %5 10
%13 = OpSpecConstantComposite %1 %6 %9 %11 %12
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ModuleScope_Vec4_With_F32_Vec3) {
    auto* cast = vec4<f32>(2.0f, vec3<f32>(2.0f, 2.0f, 2.0f));
    auto* g = Global("g", ty.vec4<f32>(), cast, ast::StorageClass::kPrivate);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 13u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpTypeVector %2 3
%5 = OpConstantComposite %4 %3 %3 %3
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%6 = OpSpecConstantOp %2 CompositeExtract %5 8
%10 = OpConstant %7 1
%9 = OpSpecConstantOp %2 CompositeExtract %5 10
%12 = OpConstant %7 2
%11 = OpSpecConstantOp %2 CompositeExtract %5 12
%13 = OpSpecConstantComposite %1 %3 %6 %9 %11
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ModuleScope_Vec4_With_Vec3_F32) {
    auto* cast = vec4<f32>(vec3<f32>(2.0f, 2.0f, 2.0f), 2.0f);
    auto* g = Global("g", ty.vec4<f32>(), cast, ast::StorageClass::kPrivate);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateConstructorExpression(g, cast), 13u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpTypeVector %2 3
%4 = OpConstant %2 2
%5 = OpConstantComposite %3 %4 %4 %4
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%6 = OpSpecConstantOp %2 CompositeExtract %5 8
%10 = OpConstant %7 1
%9 = OpSpecConstantOp %2 CompositeExtract %5 10
%12 = OpConstant %7 2
%11 = OpSpecConstantOp %2 CompositeExtract %5 12
%13 = OpSpecConstantComposite %1 %6 %9 %11 %4
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat2x2_With_Vec2_Vec2) {
    auto* cast = mat2x2<f32>(vec2<f32>(2.0f, 2.0f), vec2<f32>(2.0f, 2.0f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 2
%1 = OpTypeMatrix %2 2
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4
%6 = OpConstantComposite %1 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat3x2_With_Vec2_Vec2_Vec2) {
    auto* cast = mat3x2<f32>(vec2<f32>(2.0f, 2.0f), vec2<f32>(2.0f, 2.0f), vec2<f32>(2.0f, 2.0f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 2
%1 = OpTypeMatrix %2 3
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat4x2_With_Vec2_Vec2_Vec2_Vec2) {
    auto* cast = mat4x2<f32>(vec2<f32>(2.0f, 2.0f), vec2<f32>(2.0f, 2.0f), vec2<f32>(2.0f, 2.0f),
                             vec2<f32>(2.0f, 2.0f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 2
%1 = OpTypeMatrix %2 4
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat2x3_With_Vec3_Vec3) {
    auto* cast = mat2x3<f32>(vec3<f32>(2.0f, 2.0f, 2.0f), vec3<f32>(2.0f, 2.0f, 2.0f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 2
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat3x3_With_Vec3_Vec3_Vec3) {
    auto* cast = mat3x3<f32>(vec3<f32>(2.0f, 2.0f, 2.0f), vec3<f32>(2.0f, 2.0f, 2.0f),
                             vec3<f32>(2.0f, 2.0f, 2.0f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 3
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat4x3_With_Vec3_Vec3_Vec3_Vec3) {
    auto* cast = mat4x3<f32>(vec3<f32>(2.0f, 2.0f, 2.0f), vec3<f32>(2.0f, 2.0f, 2.0f),
                             vec3<f32>(2.0f, 2.0f, 2.0f), vec3<f32>(2.0f, 2.0f, 2.0f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 4
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat2x4_With_Vec4_Vec4) {
    auto* cast = mat2x4<f32>(vec4<f32>(2.0f, 2.0f, 2.0f, 2.0f), vec4<f32>(2.0f, 2.0f, 2.0f, 2.0f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 4
%1 = OpTypeMatrix %2 2
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat3x4_With_Vec4_Vec4_Vec4) {
    auto* cast = mat3x4<f32>(vec4<f32>(2.0f, 2.0f, 2.0f, 2.0f), vec4<f32>(2.0f, 2.0f, 2.0f, 2.0f),
                             vec4<f32>(2.0f, 2.0f, 2.0f, 2.0f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 4
%1 = OpTypeMatrix %2 3
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Mat4x4_With_Vec4_Vec4_Vec4_Vec4) {
    auto* cast = mat4x4<f32>(vec4<f32>(2.0f, 2.0f, 2.0f, 2.0f), vec4<f32>(2.0f, 2.0f, 2.0f, 2.0f),
                             vec4<f32>(2.0f, 2.0f, 2.0f, 2.0f), vec4<f32>(2.0f, 2.0f, 2.0f, 2.0f));
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 4
%1 = OpTypeMatrix %2 4
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Array_5_F32) {
    auto* cast = array<f32, 5>(2.0f, 2.0f, 2.0f, 2.0f, 2.0f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 6u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%3 = OpTypeInt 32 0
%4 = OpConstant %3 5
%1 = OpTypeArray %2 %4
%5 = OpConstant %2 2
%6 = OpConstantComposite %1 %5 %5 %5 %5 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Array_2_Vec3) {
    auto* first = vec3<f32>(1.f, 2.f, 3.f);
    auto* second = vec3<f32>(1.f, 2.f, 3.f);
    auto* t = Construct(ty.array(ty.vec3<f32>(), 2_u), first, second);
    WrapInFunction(t);
    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(t), 10u);
    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
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

TEST_F(SpvBuilderConstructorTest, CommonInitializer_TwoVectors) {
    auto* v1 = vec3<f32>(2.0f, 2.0f, 2.0f);
    auto* v2 = vec3<f32>(2.0f, 2.0f, 2.0f);
    ast::StatementList stmts = {
        WrapInStatement(v1),
        WrapInStatement(v2),
    };
    WrapInFunction(stmts);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(v1), 4u);
    EXPECT_EQ(b.GenerateExpression(v2), 4u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3
)");
}

TEST_F(SpvBuilderConstructorTest, CommonInitializer_TwoArrays) {
    auto* a1 = array<f32, 3>(2.0f, 2.0f, 2.0f);
    auto* a2 = array<f32, 3>(2.0f, 2.0f, 2.0f);
    ast::StatementList stmts = {
        WrapInStatement(a1),
        WrapInStatement(a2),
    };
    WrapInFunction(stmts);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(a1), 6u);
    EXPECT_EQ(b.GenerateExpression(a2), 6u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
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
    auto* a1 = array<f32, 2>(1.0f, 2.0f);
    auto* a2 = vec2<f32>(1.0f, 2.0f);
    ast::StatementList stmts = {
        WrapInStatement(a1),
        WrapInStatement(a2),
    };
    WrapInFunction(stmts);
    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(a1), 7u);
    EXPECT_EQ(b.GenerateExpression(a2), 9u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
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
    auto* s = Structure("my_struct", {
                                         Member("a", ty.f32()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    auto* t = Construct(ty.Of(s), 2.0f, vec3<f32>(2.0f, 2.0f, 2.0f));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateExpression(t), 6u);
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%3 = OpTypeVector %2 3
%1 = OpTypeStruct %2 %3
%4 = OpConstant %2 2
%5 = OpConstantComposite %3 %4 %4 %4
%6 = OpConstantComposite %1 %4 %5
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ZeroInit_F32) {
    auto* t = Construct<f32>();

    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateExpression(t), 2u);
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstantNull %1
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ZeroInit_I32) {
    auto* t = Construct<i32>();

    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateExpression(t), 2u);
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstantNull %1
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ZeroInit_U32) {
    auto* t = Construct<u32>();

    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateExpression(t), 2u);
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstantNull %1
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ZeroInit_Bool) {
    auto* t = Construct<bool>();

    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateExpression(t), 2u);
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeBool
%2 = OpConstantNull %1
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ZeroInit_Vector) {
    auto* t = vec2<i32>();

    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateExpression(t), 3u);
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeVector %2 2
%3 = OpConstantNull %1
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ZeroInit_Matrix) {
    auto* t = mat4x2<f32>();

    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateExpression(t), 4u);
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 2
%1 = OpTypeMatrix %2 4
%4 = OpConstantNull %1
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ZeroInit_Array) {
    auto* t = array<i32, 2>();

    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateExpression(t), 5u);
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeInt 32 0
%4 = OpConstant %3 2
%1 = OpTypeArray %2 %4
%5 = OpConstantNull %1
)");
}

TEST_F(SpvBuilderConstructorTest, Type_ZeroInit_Struct) {
    auto* s = Structure("my_struct", {Member("a", ty.f32())});
    auto* t = Construct(ty.Of(s));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_EQ(b.GenerateExpression(t), 3u);
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeStruct %2
%3 = OpConstantNull %1
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_U32_To_I32) {
    auto* cast = Construct<i32>(2_u);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 1u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeInt 32 0
%4 = OpConstant %3 2
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%1 = OpBitcast %2 %4
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_I32_To_U32) {
    auto* cast = Construct<u32>(2_i);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 1u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 0
%3 = OpTypeInt 32 1
%4 = OpConstant %3 2
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%1 = OpBitcast %2 %4
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_F32_To_I32) {
    auto* cast = Construct<i32>(2.4f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 1u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeFloat 32
%4 = OpConstant %3 2.4000001
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%1 = OpConvertFToS %2 %4
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_F32_To_U32) {
    auto* cast = Construct<u32>(2.4f);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 1u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 0
%3 = OpTypeFloat 32
%4 = OpConstant %3 2.4000001
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%1 = OpConvertFToU %2 %4
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_I32_To_F32) {
    auto* cast = Construct<f32>(2_i);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 1u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%3 = OpTypeInt 32 1
%4 = OpConstant %3 2
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%1 = OpConvertSToF %2 %4
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_U32_To_F32) {
    auto* cast = Construct<f32>(2_u);
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_EQ(b.GenerateExpression(cast), 1u);

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%3 = OpTypeInt 32 0
%4 = OpConstant %3 2
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%1 = OpConvertUToF %2 %4
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_Vectors_U32_to_I32) {
    auto* var = Global("i", ty.vec3<u32>(), ast::StorageClass::kPrivate);

    auto* cast = vec3<i32>("i");
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeInt 32 1
%7 = OpTypeVector %8 3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%9 = OpLoad %3 %1
%6 = OpBitcast %7 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_Vectors_F32_to_I32) {
    auto* var = Global("i", ty.vec3<f32>(), ast::StorageClass::kPrivate);

    auto* cast = vec3<i32>("i");
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeInt 32 1
%7 = OpTypeVector %8 3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%9 = OpLoad %3 %1
%6 = OpConvertFToS %7 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_Vectors_I32_to_U32) {
    auto* var = Global("i", ty.vec3<i32>(), ast::StorageClass::kPrivate);

    auto* cast = vec3<u32>("i");
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeInt 32 0
%7 = OpTypeVector %8 3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%9 = OpLoad %3 %1
%6 = OpBitcast %7 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_Vectors_F32_to_U32) {
    auto* var = Global("i", ty.vec3<f32>(), ast::StorageClass::kPrivate);

    auto* cast = vec3<u32>("i");
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeInt 32 0
%7 = OpTypeVector %8 3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%9 = OpLoad %3 %1
%6 = OpConvertFToU %7 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_Vectors_I32_to_F32) {
    auto* var = Global("i", ty.vec3<i32>(), ast::StorageClass::kPrivate);

    auto* cast = vec3<f32>("i");
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeFloat 32
%7 = OpTypeVector %8 3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%9 = OpLoad %3 %1
%6 = OpConvertSToF %7 %9
)");
}

TEST_F(SpvBuilderConstructorTest, Type_Convert_Vectors_U32_to_F32) {
    auto* var = Global("i", ty.vec3<u32>(), ast::StorageClass::kPrivate);

    auto* cast = vec3<f32>("i");
    WrapInFunction(cast);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
    EXPECT_EQ(b.GenerateExpression(cast), 6u) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeFloat 32
%7 = OpTypeVector %8 3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(%9 = OpLoad %3 %1
%6 = OpConvertUToF %7 %9
)");
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_GlobalVectorWithAllConstConstructors) {
    // vec3<f32>(1.0, 2.0, 3.0)  -> true
    auto* t = vec3<f32>(1.f, 2.f, 3.f);
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_GlobalArrayWithAllConstConstructors) {
    // array<vec3<f32>, 2u>(vec3<f32>(1.0, 2.0, 3.0), vec3<f32>(1.0, 2.0, 3.0))
    //   -> true
    auto* t = Construct(ty.array(ty.vec3<f32>(), 2_u), vec3<f32>(1.f, 2.f, 3.f),
                        vec3<f32>(1.f, 2.f, 3.f));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_GlobalVectorWithMatchingTypeConstructors) {
    // vec2<f32>(f32(1.0), f32(2.0))  -> false

    auto* t = vec2<f32>(Construct<f32>(1.f), Construct<f32>(2.f));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_GlobalWithTypeConversionConstructor) {
    // vec2<f32>(f32(1), f32(2)) -> false

    auto* t = vec2<f32>(Construct<f32>(1_i), Construct<f32>(2_i));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_FALSE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_VectorWithAllConstConstructors) {
    // vec3<f32>(1.0, 2.0, 3.0)  -> true

    auto* t = vec3<f32>(1.f, 2.f, 3.f);
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_Vector_WithIdent) {
    // vec3<f32>(a, b, c)  -> false

    Global("a", ty.f32(), ast::StorageClass::kPrivate);
    Global("b", ty.f32(), ast::StorageClass::kPrivate);
    Global("c", ty.f32(), ast::StorageClass::kPrivate);

    auto* t = vec3<f32>("a", "b", "c");
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_FALSE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_ArrayWithAllConstConstructors) {
    // array<vec3<f32>, 2u>(vec3<f32>(1.0, 2.0, 3.0), vec3<f32>(1.0, 2.0, 3.0))
    //   -> true

    auto* first = vec3<f32>(1.f, 2.f, 3.f);
    auto* second = vec3<f32>(1.f, 2.f, 3.f);

    auto* t = Construct(ty.array(ty.vec3<f32>(), 2_u), first, second);
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_VectorWithTypeConversionConstConstructors) {
    // vec2<f32>(f32(1), f32(2))  -> false

    auto* t = vec2<f32>(Construct<f32>(1_i), Construct<f32>(2_i));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_FALSE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_BitCastScalars) {
    auto* t = vec2<u32>(Construct<u32>(1_i), Construct<u32>(1_i));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_FALSE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_Struct) {
    auto* s = Structure("my_struct", {
                                         Member("a", ty.f32()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    auto* t = Construct(ty.Of(s), 2.f, vec3<f32>(2.f, 2.f, 2.f));
    WrapInFunction(t);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.IsConstructorConst(t));
    EXPECT_FALSE(b.has_error());
}

TEST_F(SpvBuilderConstructorTest, IsConstructorConst_Struct_WithIdentSubExpression) {
    auto* s = Structure("my_struct", {
                                         Member("a", ty.f32()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    Global("a", ty.f32(), ast::StorageClass::kPrivate);
    Global("b", ty.vec3<f32>(), ast::StorageClass::kPrivate);

    auto* t = Construct(ty.Of(s), "a", "b");
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

    WrapInFunction(If(true, Block(Decl(Let("x", nullptr, vec3<f32>(1.f, 2.f, 3.f))))),
                   Decl(Let("y", nullptr, vec3<f32>(1.f, 2.f, 3.f))));

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

    WrapInFunction(Decl(Var("one", nullptr, Expr(1.f))),
                   If(true, Block(Decl(Let("x", nullptr, vec3<f32>("one", 2.f, 3.f))))),
                   Decl(Let("y", nullptr, vec3<f32>("one", 2.f, 3.f))));

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
