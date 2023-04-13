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

#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/writer/wgsl/test_helper.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement) {
    auto* var = Var("a", ty.f32());

    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    gen.EmitStatement(stmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), "  var a : f32;\n");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_InferredType) {
    auto* var = Var("a", Expr(123_i));

    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    gen.EmitStatement(stmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), "  var a = 123i;\n");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Const_AInt) {
    auto* C = Const("C", Expr(1_a));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(fn f() {
  const C = 1;
  let l = C;
}
)");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Const_AFloat) {
    auto* C = Const("C", Expr(1._a));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(fn f() {
  const C = 1.0;
  let l = C;
}
)");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Const_i32) {
    auto* C = Const("C", Expr(1_i));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(fn f() {
  const C = 1i;
  let l = C;
}
)");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Const_u32) {
    auto* C = Const("C", Expr(1_u));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(fn f() {
  const C = 1u;
  let l = C;
}
)");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Const_f32) {
    auto* C = Const("C", Expr(1_f));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(fn f() {
  const C = 1.0f;
  let l = C;
}
)");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Const_f16) {
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
    EXPECT_EQ(gen.result(), R"(enable f16;

fn f() {
  const C = 1.0h;
  let l = C;
}
)");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Const_vec3_AInt) {
    auto* C = Const("C", Call(ty.vec3<Infer>(), 1_a, 2_a, 3_a));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(fn f() {
  const C = vec3(1, 2, 3);
  let l = C;
}
)");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Const_vec3_AFloat) {
    auto* C = Const("C", Call(ty.vec3<Infer>(), 1._a, 2._a, 3._a));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(fn f() {
  const C = vec3(1.0, 2.0, 3.0);
  let l = C;
}
)");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Const_vec3_f32) {
    auto* C = Const("C", vec3<f32>(1_f, 2_f, 3_f));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(fn f() {
  const C = vec3<f32>(1.0f, 2.0f, 3.0f);
  let l = C;
}
)");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Const_vec3_f16) {
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
    EXPECT_EQ(gen.result(), R"(enable f16;

fn f() {
  const C = vec3<f16>(1.0h, 2.0h, 3.0h);
  let l = C;
}
)");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Const_mat2x3_AFloat) {
    auto* C = Const("C", Call(ty.mat2x3<Infer>(), 1._a, 2._a, 3._a, 4._a, 5._a, 6._a));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(fn f() {
  const C = mat2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
  let l = C;
}
)");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Const_mat2x3_f32) {
    auto* C = Const("C", mat2x3<f32>(1_f, 2_f, 3_f, 4_f, 5_f, 6_f));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(fn f() {
  const C = mat2x3<f32>(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);
  let l = C;
}
)");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Const_mat2x3_f16) {
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
    EXPECT_EQ(gen.result(), R"(enable f16;

fn f() {
  const C = mat2x3<f16>(1.0h, 2.0h, 3.0h, 4.0h, 5.0h, 6.0h);
  let l = C;
}
)");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Const_arr_f32) {
    auto* C = Const("C", array<f32, 3>(1_f, 2_f, 3_f));
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(C),
             Decl(Let("l", Expr(C))),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(fn f() {
  const C = array<f32, 3u>(1.0f, 2.0f, 3.0f);
  let l = C;
}
)");
}

TEST_F(WgslGeneratorImplTest, Emit_VariableDeclStatement_Const_arr_vec2_bool) {
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
    EXPECT_EQ(gen.result(), R"(fn f() {
  const C = array<vec2<bool>, 3u>(vec2<bool>(true, false), vec2<bool>(false, true), vec2<bool>(true, true));
  let l = C;
}
)");
}
}  // namespace
}  // namespace tint::writer::wgsl
