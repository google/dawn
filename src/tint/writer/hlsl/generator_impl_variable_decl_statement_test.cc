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

#include "gmock/gmock.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/writer/hlsl/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::hlsl {
namespace {

using ::testing::HasSubstr;

using HlslGeneratorImplTest_VariableDecl = TestHelper;

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement) {
    auto* var = Var("a", ty.f32());
    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), "  float a = 0.0f;\n");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Let) {
    auto* var = Let("a", ty.f32(), Call<f32>());
    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), "  const float a = 0.0f;\n");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const) {
    auto* var = Const("a", ty.f32(), Call<f32>());
    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), "");  // Not a mistake - 'const' is inlined
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_AInt) {
    auto* C = Const("C", Expr(1_a));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const int l = 1;
}
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_AFloat) {
    auto* C = Const("C", Expr(1._a));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const float l = 1.0f;
}
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_i32) {
    auto* C = Const("C", Expr(1_i));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const int l = 1;
}
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_u32) {
    auto* C = Const("C", Expr(1_u));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const uint l = 1u;
}
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_f32) {
    auto* C = Const("C", Expr(1_f));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const float l = 1.0f;
}
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_f16) {
    Enable(builtin::Extension::kF16);

    auto* C = Const("C", Expr(1_h));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const float16_t l = float16_t(1.0h);
}
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_vec3_AInt) {
    auto* C = Const("C", Call(ty.vec3<Infer>(), 1_a, 2_a, 3_a));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const int3 l = int3(1, 2, 3);
}
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_vec3_AFloat) {
    auto* C = Const("C", Call(ty.vec3<Infer>(), 1._a, 2._a, 3._a));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const float3 l = float3(1.0f, 2.0f, 3.0f);
}
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_vec3_f32) {
    auto* C = Const("C", vec3<f32>(1_f, 2_f, 3_f));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const float3 l = float3(1.0f, 2.0f, 3.0f);
}
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_vec3_f16) {
    Enable(builtin::Extension::kF16);

    auto* C = Const("C", vec3<f16>(1_h, 2_h, 3_h));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const vector<float16_t, 3> l = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
}
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_mat2x3_AFloat) {
    auto* C = Const("C", Call(ty.mat2x3<Infer>(), 1._a, 2._a, 3._a, 4._a, 5._a, 6._a));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const float2x3 l = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
}
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_mat2x3_f32) {
    auto* C = Const("C", mat2x3<f32>(1_f, 2_f, 3_f, 4_f, 5_f, 6_f));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const float2x3 l = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
}
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_mat2x3_f16) {
    Enable(builtin::Extension::kF16);

    auto* C = Const("C", mat2x3<f16>(1_h, 2_h, 3_h, 4_h, 5_h, 6_h));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const matrix<float16_t, 2, 3> l = matrix<float16_t, 2, 3>(vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h)), vector<float16_t, 3>(float16_t(4.0h), float16_t(5.0h), float16_t(6.0h)));
}
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_arr_f32) {
    auto* C = Const("C", Call(ty.array<f32, 3>(), 1_f, 2_f, 3_f));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const float l[3] = {1.0f, 2.0f, 3.0f};
}
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_arr_vec2_bool) {
    auto* C = Const("C", Call(ty.array(ty.vec2<bool>(), 3_u),  //
                              vec2<bool>(true, false),         //
                              vec2<bool>(false, true),         //
                              vec2<bool>(true, true)));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const bool2 l[3] = {bool2(true, false), bool2(false, true), (true).xx};
}
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Array) {
    auto* var = Var("a", ty.array<f32, 5>());

    WrapInFunction(var, Expr("a"));

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("  float a[5] = (float[5])0;\n"));
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Private) {
    GlobalVar("a", ty.f32(), builtin::AddressSpace::kPrivate);

    WrapInFunction(Expr("a"));

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("  static float a = 0.0f;\n"));
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Initializer_ZeroVec_F32) {
    auto* var = Var("a", ty.vec3<f32>(), vec3<f32>());

    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(float3 a = (0.0f).xxx;
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Initializer_ZeroVec_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = Var("a", ty.vec3<f16>(), vec3<f16>());

    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(vector<float16_t, 3> a = (float16_t(0.0h)).xxx;
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Initializer_ZeroMat_F32) {
    auto* var = Var("a", ty.mat2x3<f32>(), mat2x3<f32>());

    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(),
              R"(float2x3 a = float2x3((0.0f).xxx, (0.0f).xxx);
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Initializer_ZeroMat_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = Var("a", ty.mat2x3<f16>(), mat2x3<f16>());

    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.Diagnostics();
    EXPECT_EQ(
        gen.result(),
        R"(matrix<float16_t, 2, 3> a = matrix<float16_t, 2, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx);
)");
}

}  // namespace
}  // namespace tint::writer::hlsl
