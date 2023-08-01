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

#include "src/tint/lang/glsl/writer/ast_printer/helper_test.h"

#include "src/tint/utils/text/string_stream.h"

#include "gmock/gmock.h"

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

namespace tint::glsl::writer {
namespace {

using GlslASTPrinterTest_Bitcast = TestHelper;

TEST_F(GlslASTPrinterTest_Bitcast, EmitExpression_Bitcast_Float) {
    auto* a = Let("a", Expr(1_i));
    auto* bitcast = Bitcast<f32>(Expr("a"));
    WrapInFunction(a, bitcast);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, bitcast);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "intBitsToFloat(a)");
}

TEST_F(GlslASTPrinterTest_Bitcast, EmitExpression_Bitcast_Int) {
    auto* a = Let("a", Expr(1_u));
    auto* bitcast = Bitcast<i32>(Expr("a"));
    WrapInFunction(a, bitcast);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, bitcast);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "int(a)");
}

TEST_F(GlslASTPrinterTest_Bitcast, EmitExpression_Bitcast_Uint) {
    auto* a = Let("a", Expr(1_i));
    auto* bitcast = Bitcast<u32>(Expr("a"));
    WrapInFunction(a, bitcast);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, bitcast);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "uint(a)");
}

TEST_F(GlslASTPrinterTest_Bitcast, EmitExpression_Bitcast_F16_Vec2) {
    Enable(builtin::Extension::kF16);

    auto* a = Let("a", Call<vec2<f16>>(1_h, 2_h));
    auto* b = Let("b", Bitcast<i32>(Expr("a")));
    auto* c = Let("c", Bitcast<vec2<f16>>(Expr("b")));
    auto* d = Let("d", Bitcast<f32>(Expr("c")));
    auto* e = Let("e", Bitcast<vec2<f16>>(Expr("d")));
    auto* f = Let("f", Bitcast<u32>(Expr("e")));
    auto* g = Let("g", Bitcast<vec2<f16>>(Expr("f")));
    WrapInFunction(a, b, c, d, e, f, g);

    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

int tint_bitcast_from_f16(f16vec2 src) {
  uint r = packFloat2x16(src);
  return int(r);
}
f16vec2 tint_bitcast_to_f16(int src) {
  uint r = uint(src);
  return unpackFloat2x16(r);
}
float tint_bitcast_from_f16_1(f16vec2 src) {
  uint r = packFloat2x16(src);
  return uintBitsToFloat(r);
}
f16vec2 tint_bitcast_to_f16_1(float src) {
  uint r = floatBitsToUint(src);
  return unpackFloat2x16(r);
}
uint tint_bitcast_from_f16_2(f16vec2 src) {
  uint r = packFloat2x16(src);
  return uint(r);
}
f16vec2 tint_bitcast_to_f16_2(uint src) {
  uint r = uint(src);
  return unpackFloat2x16(r);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  f16vec2 a = f16vec2(1.0hf, 2.0hf);
  int b = tint_bitcast_from_f16(a);
  f16vec2 c = tint_bitcast_to_f16(b);
  float d = tint_bitcast_from_f16_1(c);
  f16vec2 e = tint_bitcast_to_f16_1(d);
  uint f = tint_bitcast_from_f16_2(e);
  f16vec2 g = tint_bitcast_to_f16_2(f);
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Bitcast, EmitExpression_Bitcast_F16_Vec4) {
    Enable(builtin::Extension::kF16);

    auto* a = Let("a", Call<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
    auto* b = Let("b", Bitcast<vec2<i32>>(Expr("a")));
    auto* c = Let("c", Bitcast<vec4<f16>>(Expr("b")));
    auto* d = Let("d", Bitcast<vec2<f32>>(Expr("c")));
    auto* e = Let("e", Bitcast<vec4<f16>>(Expr("d")));
    auto* f = Let("f", Bitcast<vec2<u32>>(Expr("e")));
    auto* g = Let("g", Bitcast<vec4<f16>>(Expr("f")));
    WrapInFunction(a, b, c, d, e, f, g);

    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

ivec2 tint_bitcast_from_f16(f16vec4 src) {
  uvec2 r = uvec2(packFloat2x16(src.xy), packFloat2x16(src.zw));
  return ivec2(r);
}
f16vec4 tint_bitcast_to_f16(ivec2 src) {
  uvec2 r = uvec2(src);
  f16vec2 v_xy = unpackFloat2x16(r.x);
  f16vec2 v_zw = unpackFloat2x16(r.y);
  return f16vec4(v_xy.x, v_xy.y, v_zw.x, v_zw.y);
}
vec2 tint_bitcast_from_f16_1(f16vec4 src) {
  uvec2 r = uvec2(packFloat2x16(src.xy), packFloat2x16(src.zw));
  return uintBitsToFloat(r);
}
f16vec4 tint_bitcast_to_f16_1(vec2 src) {
  uvec2 r = floatBitsToUint(src);
  f16vec2 v_xy = unpackFloat2x16(r.x);
  f16vec2 v_zw = unpackFloat2x16(r.y);
  return f16vec4(v_xy.x, v_xy.y, v_zw.x, v_zw.y);
}
uvec2 tint_bitcast_from_f16_2(f16vec4 src) {
  uvec2 r = uvec2(packFloat2x16(src.xy), packFloat2x16(src.zw));
  return uvec2(r);
}
f16vec4 tint_bitcast_to_f16_2(uvec2 src) {
  uvec2 r = uvec2(src);
  f16vec2 v_xy = unpackFloat2x16(r.x);
  f16vec2 v_zw = unpackFloat2x16(r.y);
  return f16vec4(v_xy.x, v_xy.y, v_zw.x, v_zw.y);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  f16vec4 a = f16vec4(1.0hf, 2.0hf, 3.0hf, 4.0hf);
  ivec2 b = tint_bitcast_from_f16(a);
  f16vec4 c = tint_bitcast_to_f16(b);
  vec2 d = tint_bitcast_from_f16_1(c);
  f16vec4 e = tint_bitcast_to_f16_1(d);
  uvec2 f = tint_bitcast_from_f16_2(e);
  f16vec4 g = tint_bitcast_to_f16_2(f);
  return;
}
)");
}

}  // namespace
}  // namespace tint::glsl::writer
