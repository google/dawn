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

#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/writer/ast_printer/helper_test.h"

#include "gmock/gmock.h"

namespace tint::wgsl::writer {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

using WgslASTPrinterTest = TestHelper;

TEST_F(WgslASTPrinterTest, Emit_VariableDeclStatement) {
    auto* var = Var("a", ty.f32());

    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(stmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), "  var a : f32;\n");
}

TEST_F(WgslASTPrinterTest, Emit_VariableDeclStatement_InferredType) {
    auto* var = Var("a", Expr(123_i));

    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(stmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), "  var a = 123i;\n");
}

TEST_F(WgslASTPrinterTest, Emit_VariableDeclStatement_Const_AInt) {
    auto* C = Const("C", Expr(1_a));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(fn f() {
  const C = 1;
  let l = C;
}
)");
}

TEST_F(WgslASTPrinterTest, Emit_VariableDeclStatement_Const_AFloat) {
    auto* C = Const("C", Expr(1._a));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(fn f() {
  const C = 1.0;
  let l = C;
}
)");
}

TEST_F(WgslASTPrinterTest, Emit_VariableDeclStatement_Const_i32) {
    auto* C = Const("C", Expr(1_i));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(fn f() {
  const C = 1i;
  let l = C;
}
)");
}

TEST_F(WgslASTPrinterTest, Emit_VariableDeclStatement_Const_u32) {
    auto* C = Const("C", Expr(1_u));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(fn f() {
  const C = 1u;
  let l = C;
}
)");
}

TEST_F(WgslASTPrinterTest, Emit_VariableDeclStatement_Const_f32) {
    auto* C = Const("C", Expr(1_f));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(fn f() {
  const C = 1.0f;
  let l = C;
}
)");
}

TEST_F(WgslASTPrinterTest, Emit_VariableDeclStatement_Const_f16) {
    Enable(builtin::Extension::kF16);

    auto* C = Const("C", Expr(1_h));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(enable f16;

fn f() {
  const C = 1.0h;
  let l = C;
}
)");
}

TEST_F(WgslASTPrinterTest, Emit_VariableDeclStatement_Const_vec3_AInt) {
    auto* C = Const("C", Call<vec3<Infer>>(1_a, 2_a, 3_a));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(fn f() {
  const C = vec3(1, 2, 3);
  let l = C;
}
)");
}

TEST_F(WgslASTPrinterTest, Emit_VariableDeclStatement_Const_vec3_AFloat) {
    auto* C = Const("C", Call<vec3<Infer>>(1._a, 2._a, 3._a));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(fn f() {
  const C = vec3(1.0, 2.0, 3.0);
  let l = C;
}
)");
}

TEST_F(WgslASTPrinterTest, Emit_VariableDeclStatement_Const_vec3_f32) {
    auto* C = Const("C", Call<vec3<f32>>(1_f, 2_f, 3_f));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(fn f() {
  const C = vec3<f32>(1.0f, 2.0f, 3.0f);
  let l = C;
}
)");
}

TEST_F(WgslASTPrinterTest, Emit_VariableDeclStatement_Const_vec3_f16) {
    Enable(builtin::Extension::kF16);

    auto* C = Const("C", Call<vec3<f16>>(1_h, 2_h, 3_h));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(enable f16;

fn f() {
  const C = vec3<f16>(1.0h, 2.0h, 3.0h);
  let l = C;
}
)");
}

TEST_F(WgslASTPrinterTest, Emit_VariableDeclStatement_Const_mat2x3_AFloat) {
    auto* C = Const("C", Call<mat2x3<Infer>>(1._a, 2._a, 3._a, 4._a, 5._a, 6._a));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(fn f() {
  const C = mat2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
  let l = C;
}
)");
}

TEST_F(WgslASTPrinterTest, Emit_VariableDeclStatement_Const_mat2x3_f32) {
    auto* C = Const("C", Call<mat2x3<f32>>(1_f, 2_f, 3_f, 4_f, 5_f, 6_f));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(fn f() {
  const C = mat2x3<f32>(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);
  let l = C;
}
)");
}

TEST_F(WgslASTPrinterTest, Emit_VariableDeclStatement_Const_mat2x3_f16) {
    Enable(builtin::Extension::kF16);

    auto* C = Const("C", Call<mat2x3<f16>>(1_h, 2_h, 3_h, 4_h, 5_h, 6_h));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(enable f16;

fn f() {
  const C = mat2x3<f16>(1.0h, 2.0h, 3.0h, 4.0h, 5.0h, 6.0h);
  let l = C;
}
)");
}

TEST_F(WgslASTPrinterTest, Emit_VariableDeclStatement_Const_arr_f32) {
    auto* C = Const("C", Call<array<f32, 3>>(1_f, 2_f, 3_f));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(fn f() {
  const C = array<f32, 3u>(1.0f, 2.0f, 3.0f);
  let l = C;
}
)");
}

TEST_F(WgslASTPrinterTest, Emit_VariableDeclStatement_Const_arr_vec2_bool) {
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
    EXPECT_EQ(gen.Result(), R"(fn f() {
  const C = array<vec2<bool>, 3u>(vec2<bool>(true, false), vec2<bool>(false, true), vec2<bool>(true, true));
  let l = C;
}
)");
}
}  // namespace
}  // namespace tint::wgsl::writer
