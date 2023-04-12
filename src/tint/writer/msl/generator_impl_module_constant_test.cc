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
#include "src/tint/writer/msl/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Emit_GlobalConst_AInt) {
    auto* var = GlobalConst("G", Expr(1_a));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  int const l = 1;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_GlobalConst_AFloat) {
    auto* var = GlobalConst("G", Expr(1._a));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  float const l = 1.0f;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_GlobalConst_i32) {
    auto* var = GlobalConst("G", Expr(1_i));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  int const l = 1;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_GlobalConst_u32) {
    auto* var = GlobalConst("G", Expr(1_u));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  uint const l = 1u;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_GlobalConst_f32) {
    auto* var = GlobalConst("G", Expr(1_f));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  float const l = 1.0f;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_GlobalConst_f16) {
    Enable(builtin::Extension::kF16);

    auto* var = GlobalConst("G", Expr(1_h));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  half const l = 1.0h;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_GlobalConst_vec3_AInt) {
    auto* var = GlobalConst("G", Call(ty.vec3<Infer>(), 1_a, 2_a, 3_a));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  int3 const l = int3(1, 2, 3);
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_GlobalConst_vec3_AFloat) {
    auto* var = GlobalConst("G", Call(ty.vec3<Infer>(), 1._a, 2._a, 3._a));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  float3 const l = float3(1.0f, 2.0f, 3.0f);
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_GlobalConst_vec3_f32) {
    auto* var = GlobalConst("G", vec3<f32>(1_f, 2_f, 3_f));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  float3 const l = float3(1.0f, 2.0f, 3.0f);
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_GlobalConst_vec3_f16) {
    Enable(builtin::Extension::kF16);

    auto* var = GlobalConst("G", vec3<f16>(1_h, 2_h, 3_h));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  half3 const l = half3(1.0h, 2.0h, 3.0h);
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_GlobalConst_mat2x3_AFloat) {
    auto* var = GlobalConst("G", Call(ty.mat2x3<Infer>(), 1._a, 2._a, 3._a, 4._a, 5._a, 6._a));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  float2x3 const l = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_GlobalConst_mat2x3_f32) {
    auto* var = GlobalConst("G", mat2x3<f32>(1_f, 2_f, 3_f, 4_f, 5_f, 6_f));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  float2x3 const l = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_GlobalConst_mat2x3_f16) {
    Enable(builtin::Extension::kF16);

    auto* var = GlobalConst("G", mat2x3<f16>(1_h, 2_h, 3_h, 4_h, 5_h, 6_h));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  half2x3 const l = half2x3(half3(1.0h, 2.0h, 3.0h), half3(4.0h, 5.0h, 6.0h));
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_GlobalConst_arr_f32) {
    auto* var = GlobalConst("G", Call(ty.array<f32, 3>(), 1_f, 2_f, 3_f));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;

template<typename T, size_t N>
struct tint_array {
    const constant T& operator[](size_t i) const constant { return elements[i]; }
    device T& operator[](size_t i) device { return elements[i]; }
    const device T& operator[](size_t i) const device { return elements[i]; }
    thread T& operator[](size_t i) thread { return elements[i]; }
    const thread T& operator[](size_t i) const thread { return elements[i]; }
    threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
    const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
    T elements[N];
};

void f() {
  tint_array<float, 3> const l = tint_array<float, 3>{1.0f, 2.0f, 3.0f};
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_GlobalConst_arr_vec2_bool) {
    auto* var = GlobalConst("G", Call(ty.array(ty.vec2<bool>(), 3_u),  //
                                      vec2<bool>(true, false),         //
                                      vec2<bool>(false, true),         //
                                      vec2<bool>(true, true)));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Decl(Let("l", Expr(var)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;

template<typename T, size_t N>
struct tint_array {
    const constant T& operator[](size_t i) const constant { return elements[i]; }
    device T& operator[](size_t i) device { return elements[i]; }
    const device T& operator[](size_t i) const device { return elements[i]; }
    thread T& operator[](size_t i) thread { return elements[i]; }
    const thread T& operator[](size_t i) const thread { return elements[i]; }
    threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
    const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
    T elements[N];
};

void f() {
  tint_array<bool2, 3> const l = tint_array<bool2, 3>{bool2(true, false), bool2(false, true), bool2(true)};
}

)");
}

}  // namespace
}  // namespace tint::writer::msl
