// Copyright 2021 The Tint Authors.
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

#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/writer/glsl/test_helper.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::glsl {
namespace {

using ::testing::HasSubstr;

using GlslGeneratorImplTest_VariableDecl = TestHelper;

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement) {
    auto* var = Var("a", ty.f32());
    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();
    gen.increment_indent();
    gen.EmitStatement(stmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), "  float a = 0.0f;\n");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Let) {
    auto* var = Let("a", ty.f32(), Call<f32>());
    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();
    gen.increment_indent();
    gen.EmitStatement(stmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), "  float a = 0.0f;\n");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const) {
    auto* var = Const("a", ty.f32(), Call<f32>());
    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();
    gen.increment_indent();
    gen.EmitStatement(stmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), "");  // Not a mistake - 'const' is inlined
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_AInt) {
    auto* C = Const("C", Expr(1_a));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

void f() {
  int l = 1;
}

)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_AFloat) {
    auto* C = Const("C", Expr(1._a));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

void f() {
  float l = 1.0f;
}

)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_i32) {
    auto* C = Const("C", Expr(1_i));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

void f() {
  int l = 1;
}

)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_u32) {
    auto* C = Const("C", Expr(1_u));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

void f() {
  uint l = 1u;
}

)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_f32) {
    auto* C = Const("C", Expr(1_f));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

void f() {
  float l = 1.0f;
}

)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_f16) {
    Enable(builtin::Extension::kF16);

    auto* C = Const("C", Expr(1_h));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

void f() {
  float16_t l = 1.0hf;
}

)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_vec3_AInt) {
    auto* C = Const("C", Call(ty.vec3<Infer>(), 1_a, 2_a, 3_a));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

void f() {
  ivec3 l = ivec3(1, 2, 3);
}

)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_vec3_AFloat) {
    auto* C = Const("C", Call(ty.vec3<Infer>(), 1._a, 2._a, 3._a));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

void f() {
  vec3 l = vec3(1.0f, 2.0f, 3.0f);
}

)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_vec3_f32) {
    auto* C = Const("C", vec3<f32>(1_f, 2_f, 3_f));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

void f() {
  vec3 l = vec3(1.0f, 2.0f, 3.0f);
}

)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_vec3_f16) {
    Enable(builtin::Extension::kF16);

    auto* C = Const("C", vec3<f16>(1_h, 2_h, 3_h));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

void f() {
  f16vec3 l = f16vec3(1.0hf, 2.0hf, 3.0hf);
}

)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_mat2x3_AFloat) {
    auto* C = Const("C", Call(ty.mat2x3<Infer>(), 1._a, 2._a, 3._a, 4._a, 5._a, 6._a));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

void f() {
  mat2x3 l = mat2x3(vec3(1.0f, 2.0f, 3.0f), vec3(4.0f, 5.0f, 6.0f));
}

)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_mat2x3_f32) {
    auto* C = Const("C", mat2x3<f32>(1_f, 2_f, 3_f, 4_f, 5_f, 6_f));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

void f() {
  mat2x3 l = mat2x3(vec3(1.0f, 2.0f, 3.0f), vec3(4.0f, 5.0f, 6.0f));
}

)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_mat2x3_f16) {
    Enable(builtin::Extension::kF16);

    auto* C = Const("C", mat2x3<f16>(1_h, 2_h, 3_h, 4_h, 5_h, 6_h));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

void f() {
  f16mat2x3 l = f16mat2x3(f16vec3(1.0hf, 2.0hf, 3.0hf), f16vec3(4.0hf, 5.0hf, 6.0hf));
}

)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_arr_f32) {
    auto* C = Const("C", Call(ty.array<f32, 3>(), 1_f, 2_f, 3_f));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

void f() {
  float l[3] = float[3](1.0f, 2.0f, 3.0f);
}

)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_arr_f32_zero) {
    auto* C = Const("C", Call(ty.array<f32, 2>()));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

void f() {
  float l[2] = float[2](0.0f, 0.0f);
}

)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_arr_arr_f32_zero) {
    auto* C = Const("C", Call(ty.array(ty.array<f32, 2>(), 3_i)));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

void f() {
  float l[3][2] = float[3][2](float[2](0.0f, 0.0f), float[2](0.0f, 0.0f), float[2](0.0f, 0.0f));
}

)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_arr_struct_zero) {
    Structure("S", utils::Vector{Member("a", ty.i32()), Member("b", ty.f32())});
    auto* C = Const("C", Call(ty.array(ty("S"), 2_i)));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

struct S {
  int a;
  float b;
};

void f() {
  S l[2] = S[2](S(0, 0.0f), S(0, 0.0f));
}

)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const_arr_vec2_bool) {
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
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

void f() {
  bvec2 l[3] = bvec2[3](bvec2(true, false), bvec2(false, true), bvec2(true));
}

)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Array) {
    auto* var = Var("a", ty.array<f32, 5>());

    WrapInFunction(var, Expr("a"));

    GeneratorImpl& gen = Build();
    gen.increment_indent();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(),
                HasSubstr("  float a[5] = float[5](0.0f, 0.0f, 0.0f, 0.0f, 0.0f);\n"));
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Private) {
    GlobalVar("a", ty.f32(), builtin::AddressSpace::kPrivate);

    WrapInFunction(Expr("a"));

    GeneratorImpl& gen = Build();
    gen.increment_indent();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr("  float a = 0.0f;\n"));
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Initializer_ZeroVec_f32) {
    auto* var = Var("a", ty.vec3<f32>(), vec3<f32>());

    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();
    gen.EmitStatement(stmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(vec3 a = vec3(0.0f);
)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Initializer_ZeroVec_f16) {
    Enable(builtin::Extension::kF16);

    auto* var = Var("a", ty.vec3<f16>(), vec3<f16>());

    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();
    gen.EmitStatement(stmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(f16vec3 a = f16vec3(0.0hf);
)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Initializer_ZeroMat_f32) {
    auto* var = Var("a", ty.mat2x3<f32>(), mat2x3<f32>());

    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();
    gen.EmitStatement(stmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(),
              R"(mat2x3 a = mat2x3(vec3(0.0f), vec3(0.0f));
)");
}

TEST_F(GlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Initializer_ZeroMat_f16) {
    Enable(builtin::Extension::kF16);

    auto* var = Var("a", ty.mat2x3<f16>(), mat2x3<f16>());

    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();
    gen.EmitStatement(stmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(),
              R"(f16mat2x3 a = f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf));
)");
}

}  // namespace
}  // namespace tint::writer::glsl
