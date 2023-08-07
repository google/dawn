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

#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/glsl/writer/ast_printer/helper_test.h"

#include "gmock/gmock.h"

namespace tint::glsl::writer {
namespace {

using ::testing::HasSubstr;
using namespace tint::core::fluent_types;  // NOLINT
using namespace tint::number_suffixes;     // NOLINT

using GlslASTPrinterTest_VariableDecl = TestHelper;

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement) {
    auto* var = Var("a", ty.f32());
    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    ASTPrinter& gen = Build();
    gen.IncrementIndent();
    gen.EmitStatement(stmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), "  float a = 0.0f;\n");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Let) {
    auto* var = Let("a", ty.f32(), Call<f32>());
    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    ASTPrinter& gen = Build();
    gen.IncrementIndent();
    gen.EmitStatement(stmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), "  float a = 0.0f;\n");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Const) {
    auto* var = Const("a", ty.f32(), Call<f32>());
    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    ASTPrinter& gen = Build();
    gen.IncrementIndent();
    gen.EmitStatement(stmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), "");  // Not a mistake - 'const' is inlined
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Const_AInt) {
    auto* C = Const("C", Expr(1_a));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

void f() {
  int l = 1;
}

)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Const_AFloat) {
    auto* C = Const("C", Expr(1._a));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

void f() {
  float l = 1.0f;
}

)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Const_i32) {
    auto* C = Const("C", Expr(1_i));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

void f() {
  int l = 1;
}

)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Const_u32) {
    auto* C = Const("C", Expr(1_u));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

void f() {
  uint l = 1u;
}

)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Const_f32) {
    auto* C = Const("C", Expr(1_f));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

void f() {
  float l = 1.0f;
}

)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Const_f16) {
    Enable(core::Extension::kF16);

    auto* C = Const("C", Expr(1_h));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

void f() {
  float16_t l = 1.0hf;
}

)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Const_vec3_AInt) {
    auto* C = Const("C", Call<vec3<Infer>>(1_a, 2_a, 3_a));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

void f() {
  ivec3 l = ivec3(1, 2, 3);
}

)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Const_vec3_AFloat) {
    auto* C = Const("C", Call<vec3<Infer>>(1._a, 2._a, 3._a));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

void f() {
  vec3 l = vec3(1.0f, 2.0f, 3.0f);
}

)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Const_vec3_f32) {
    auto* C = Const("C", Call<vec3<f32>>(1_f, 2_f, 3_f));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

void f() {
  vec3 l = vec3(1.0f, 2.0f, 3.0f);
}

)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Const_vec3_f16) {
    Enable(core::Extension::kF16);

    auto* C = Const("C", Call<vec3<f16>>(1_h, 2_h, 3_h));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

void f() {
  f16vec3 l = f16vec3(1.0hf, 2.0hf, 3.0hf);
}

)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Const_mat2x3_AFloat) {
    auto* C = Const("C", Call<mat2x3<Infer>>(1._a, 2._a, 3._a, 4._a, 5._a, 6._a));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

void f() {
  mat2x3 l = mat2x3(vec3(1.0f, 2.0f, 3.0f), vec3(4.0f, 5.0f, 6.0f));
}

)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Const_mat2x3_f32) {
    auto* C = Const("C", Call<mat2x3<f32>>(1_f, 2_f, 3_f, 4_f, 5_f, 6_f));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

void f() {
  mat2x3 l = mat2x3(vec3(1.0f, 2.0f, 3.0f), vec3(4.0f, 5.0f, 6.0f));
}

)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Const_mat2x3_f16) {
    Enable(core::Extension::kF16);

    auto* C = Const("C", Call<mat2x3<f16>>(1_h, 2_h, 3_h, 4_h, 5_h, 6_h));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

void f() {
  f16mat2x3 l = f16mat2x3(f16vec3(1.0hf, 2.0hf, 3.0hf), f16vec3(4.0hf, 5.0hf, 6.0hf));
}

)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Const_arr_f32) {
    auto* C = Const("C", Call<array<f32, 3>>(1_f, 2_f, 3_f));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

void f() {
  float l[3] = float[3](1.0f, 2.0f, 3.0f);
}

)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Const_arr_f32_zero) {
    auto* C = Const("C", Call<array<f32, 2>>());
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

void f() {
  float l[2] = float[2](0.0f, 0.0f);
}

)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Const_arr_arr_f32_zero) {
    auto* C = Const("C", Call<array<array<f32, 2>, 3>>());
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

void f() {
  float l[3][2] = float[3][2](float[2](0.0f, 0.0f), float[2](0.0f, 0.0f), float[2](0.0f, 0.0f));
}

)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Const_arr_struct_zero) {
    Structure("S", Vector{Member("a", ty.i32()), Member("b", ty.f32())});
    auto* C = Const("C", Call(ty.array(ty("S"), 2_i)));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

struct S {
  int a;
  float b;
};

void f() {
  S l[2] = S[2](S(0, 0.0f), S(0, 0.0f));
}

)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Const_arr_vec2_bool) {
    auto* C = Const("C", Call<array<vec2<bool>, 3>>(         //
                             Call<vec2<bool>>(true, false),  //
                             Call<vec2<bool>>(false, true),  //
                             Call<vec2<bool>>(true, true)));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

void f() {
  bvec2 l[3] = bvec2[3](bvec2(true, false), bvec2(false, true), bvec2(true));
}

)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Array) {
    auto* var = Var("a", ty.array<f32, 5>());

    WrapInFunction(var, Expr("a"));

    ASTPrinter& gen = Build();
    gen.IncrementIndent();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(),
                HasSubstr("  float a[5] = float[5](0.0f, 0.0f, 0.0f, 0.0f, 0.0f);\n"));
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Private) {
    GlobalVar("a", ty.f32(), core::AddressSpace::kPrivate);

    WrapInFunction(Expr("a"));

    ASTPrinter& gen = Build();
    gen.IncrementIndent();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("  float a = 0.0f;\n"));
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Initializer_ZeroVec_f32) {
    auto* var = Var("a", ty.vec3<f32>(), Call<vec3<f32>>());

    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    ASTPrinter& gen = Build();
    gen.EmitStatement(stmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(vec3 a = vec3(0.0f);
)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Initializer_ZeroVec_f16) {
    Enable(core::Extension::kF16);

    auto* var = Var("a", ty.vec3<f16>(), Call<vec3<f16>>());

    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    ASTPrinter& gen = Build();
    gen.EmitStatement(stmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(f16vec3 a = f16vec3(0.0hf);
)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Initializer_ZeroMat_f32) {
    auto* var = Var("a", ty.mat2x3<f32>(), Call<mat2x3<f32>>());

    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    ASTPrinter& gen = Build();
    gen.EmitStatement(stmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(),
              R"(mat2x3 a = mat2x3(vec3(0.0f), vec3(0.0f));
)");
}

TEST_F(GlslASTPrinterTest_VariableDecl, Emit_VariableDeclStatement_Initializer_ZeroMat_f16) {
    Enable(core::Extension::kF16);

    auto* var = Var("a", ty.mat2x3<f16>(), Call<mat2x3<f16>>());

    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    ASTPrinter& gen = Build();
    gen.EmitStatement(stmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(),
              R"(f16mat2x3 a = f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf));
)");
}

}  // namespace
}  // namespace tint::glsl::writer
