// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/msl/writer/ast_printer/helper_test.h"
#include "src/tint/lang/wgsl/ast/id_attribute.h"

namespace tint::msl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using MslASTPrinterTest = TestHelper;

TEST_F(MslASTPrinterTest, Emit_GlobalConst_AInt) {
    auto* var = GlobalConst("G", Expr(1_a));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  int const l = 1;
}

)");
}

TEST_F(MslASTPrinterTest, Emit_GlobalConst_AFloat) {
    auto* var = GlobalConst("G", Expr(1._a));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  float const l = 1.0f;
}

)");
}

TEST_F(MslASTPrinterTest, Emit_GlobalConst_i32) {
    auto* var = GlobalConst("G", Expr(1_i));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  int const l = 1;
}

)");
}

TEST_F(MslASTPrinterTest, Emit_GlobalConst_u32) {
    auto* var = GlobalConst("G", Expr(1_u));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  uint const l = 1u;
}

)");
}

TEST_F(MslASTPrinterTest, Emit_GlobalConst_f32) {
    auto* var = GlobalConst("G", Expr(1_f));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  float const l = 1.0f;
}

)");
}

TEST_F(MslASTPrinterTest, Emit_GlobalConst_f16) {
    Enable(wgsl::Extension::kF16);

    auto* var = GlobalConst("G", Expr(1_h));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  half const l = 1.0h;
}

)");
}

TEST_F(MslASTPrinterTest, Emit_GlobalConst_vec3_AInt) {
    auto* var = GlobalConst("G", Call<vec3<Infer>>(1_a, 2_a, 3_a));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  int3 const l = int3(1, 2, 3);
}

)");
}

TEST_F(MslASTPrinterTest, Emit_GlobalConst_vec3_AFloat) {
    auto* var = GlobalConst("G", Call<vec3<Infer>>(1._a, 2._a, 3._a));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  float3 const l = float3(1.0f, 2.0f, 3.0f);
}

)");
}

TEST_F(MslASTPrinterTest, Emit_GlobalConst_vec3_f32) {
    auto* var = GlobalConst("G", Call<vec3<f32>>(1_f, 2_f, 3_f));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  float3 const l = float3(1.0f, 2.0f, 3.0f);
}

)");
}

TEST_F(MslASTPrinterTest, Emit_GlobalConst_vec3_f16) {
    Enable(wgsl::Extension::kF16);

    auto* var = GlobalConst("G", Call<vec3<f16>>(1_h, 2_h, 3_h));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  half3 const l = half3(1.0h, 2.0h, 3.0h);
}

)");
}

TEST_F(MslASTPrinterTest, Emit_GlobalConst_mat2x3_AFloat) {
    auto* var = GlobalConst("G", Call<mat2x3<Infer>>(1._a, 2._a, 3._a, 4._a, 5._a, 6._a));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  float2x3 const l = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
}

)");
}

TEST_F(MslASTPrinterTest, Emit_GlobalConst_mat2x3_f32) {
    auto* var = GlobalConst("G", Call<mat2x3<f32>>(1_f, 2_f, 3_f, 4_f, 5_f, 6_f));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  float2x3 const l = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
}

)");
}

TEST_F(MslASTPrinterTest, Emit_GlobalConst_mat2x3_f16) {
    Enable(wgsl::Extension::kF16);

    auto* var = GlobalConst("G", Call<mat2x3<f16>>(1_h, 2_h, 3_h, 4_h, 5_h, 6_h));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  half2x3 const l = half2x3(half3(1.0h, 2.0h, 3.0h), half3(4.0h, 5.0h, 6.0h));
}

)");
}

TEST_F(MslASTPrinterTest, Emit_GlobalConst_arr_f32) {
    auto* var = GlobalConst("G", Call(ty.array<f32, 3>(), 1_f, 2_f, 3_f));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

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

TEST_F(MslASTPrinterTest, Emit_GlobalConst_arr_vec2_bool) {
    auto* var = GlobalConst("G", Call<array<vec2<bool>, 3>>(Call<vec2<bool>>(true, false),  //
                                                            Call<vec2<bool>>(false, true),  //
                                                            Call<vec2<bool>>(true, true)));
    Func("f", tint::Empty, ty.void_(), Vector{Decl(Let("l", Expr(var)))});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

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
}  // namespace tint::msl::writer
