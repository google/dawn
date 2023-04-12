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

#include "src/tint/ast/id_attribute.h"
#include "src/tint/writer/hlsl/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::hlsl {
namespace {

using HlslGeneratorImplTest_ModuleConstant = TestHelper;

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_GlobalConst_AInt) {
    auto* var = GlobalConst("G", Expr(1_a));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const int l = 1;
}
)");
}

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_GlobalConst_AFloat) {
    auto* var = GlobalConst("G", Expr(1._a));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const float l = 1.0f;
}
)");
}

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_GlobalConst_i32) {
    auto* var = GlobalConst("G", Expr(1_i));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const int l = 1;
}
)");
}

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_GlobalConst_u32) {
    auto* var = GlobalConst("G", Expr(1_u));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const uint l = 1u;
}
)");
}

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_GlobalConst_f32) {
    auto* var = GlobalConst("G", Expr(1_f));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const float l = 1.0f;
}
)");
}

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_GlobalConst_f16) {
    Enable(builtin::Extension::kF16);

    auto* var = GlobalConst("G", Expr(1_h));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const float16_t l = float16_t(1.0h);
}
)");
}

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_GlobalConst_vec3_AInt) {
    auto* var = GlobalConst("G", Call(ty.vec3<Infer>(), 1_a, 2_a, 3_a));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const int3 l = int3(1, 2, 3);
}
)");
}

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_GlobalConst_vec3_AFloat) {
    auto* var = GlobalConst("G", Call(ty.vec3<Infer>(), 1._a, 2._a, 3._a));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const float3 l = float3(1.0f, 2.0f, 3.0f);
}
)");
}

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_GlobalConst_vec3_f32) {
    auto* var = GlobalConst("G", vec3<f32>(1_f, 2_f, 3_f));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const float3 l = float3(1.0f, 2.0f, 3.0f);
}
)");
}

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_GlobalConst_vec3_f16) {
    Enable(builtin::Extension::kF16);

    auto* var = GlobalConst("G", vec3<f16>(1_h, 2_h, 3_h));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const vector<float16_t, 3> l = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
}
)");
}

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_GlobalConst_mat2x3_AFloat) {
    auto* var = GlobalConst("G", Call(ty.mat2x3<Infer>(), 1._a, 2._a, 3._a, 4._a, 5._a, 6._a));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const float2x3 l = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
}
)");
}

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_GlobalConst_mat2x3_f32) {
    auto* var = GlobalConst("G", mat2x3<f32>(1_f, 2_f, 3_f, 4_f, 5_f, 6_f));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const float2x3 l = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
}
)");
}

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_GlobalConst_mat2x3_f16) {
    Enable(builtin::Extension::kF16);

    auto* var = GlobalConst("G", mat2x3<f16>(1_h, 2_h, 3_h, 4_h, 5_h, 6_h));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const matrix<float16_t, 2, 3> l = matrix<float16_t, 2, 3>(vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h)), vector<float16_t, 3>(float16_t(4.0h), float16_t(5.0h), float16_t(6.0h)));
}
)");
}

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_GlobalConst_arr_f32) {
    auto* var = GlobalConst("G", Call(ty.array<f32, 3>(), 1_f, 2_f, 3_f));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const float l[3] = {1.0f, 2.0f, 3.0f};
}
)");
}

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_GlobalConst_arr_vec2_bool) {
    auto* var = GlobalConst("G", Call(ty.array(ty.vec2<bool>(), 3_u),  //
                                      vec2<bool>(true, false),         //
                                      vec2<bool>(false, true),         //
                                      vec2<bool>(true, true)));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(void f() {
  const bool2 l[3] = {bool2(true, false), bool2(false, true), (true).xx};
}
)");
}

}  // namespace
}  // namespace tint::writer::hlsl
