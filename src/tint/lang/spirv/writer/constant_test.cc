// Copyright 2023 The Tint Authors.
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

#include "src/tint/lang/spirv/writer/common/helper_test.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::number_suffixes;  // NOLINT

TEST_F(SpirvWriterTest, Constant_Bool) {
    writer_.Constant(b.Constant(true));
    writer_.Constant(b.Constant(false));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%true = OpConstantTrue %bool");
    EXPECT_INST("%false = OpConstantFalse %bool");
}

TEST_F(SpirvWriterTest, Constant_I32) {
    writer_.Constant(b.Constant(i32(42)));
    writer_.Constant(b.Constant(i32(-1)));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%int_42 = OpConstant %int 42");
    EXPECT_INST("%int_n1 = OpConstant %int -1");
}

TEST_F(SpirvWriterTest, Constant_U32) {
    writer_.Constant(b.Constant(u32(42)));
    writer_.Constant(b.Constant(u32(4000000000)));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%uint_42 = OpConstant %uint 42");
    EXPECT_INST("%uint_4000000000 = OpConstant %uint 4000000000");
}

TEST_F(SpirvWriterTest, Constant_F32) {
    writer_.Constant(b.Constant(f32(42)));
    writer_.Constant(b.Constant(f32(-1)));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%float_42 = OpConstant %float 42");
    EXPECT_INST("%float_n1 = OpConstant %float -1");
}

TEST_F(SpirvWriterTest, Constant_F16) {
    writer_.Constant(b.Constant(f16(42)));
    writer_.Constant(b.Constant(f16(-1)));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%half_0x1_5p_5 = OpConstant %half 0x1.5p+5");
    EXPECT_INST("%half_n0x1p_0 = OpConstant %half -0x1p+0");
}

TEST_F(SpirvWriterTest, Constant_Vec4Bool) {
    writer_.Constant(b.Composite(ty.vec4<bool>(), true, false, false, true));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpConstantComposite %v4bool %true %false %false %true");
}

TEST_F(SpirvWriterTest, Constant_Vec2i) {
    writer_.Constant(b.Composite(ty.vec2<i32>(), 42_i, -1_i));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpConstantComposite %v2int %int_42 %int_n1");
}

TEST_F(SpirvWriterTest, Constant_Vec3u) {
    writer_.Constant(b.Composite(ty.vec3<u32>(), 42_u, 0_u, 4000000000_u));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpConstantComposite %v3uint %uint_42 %uint_0 %uint_4000000000");
}

TEST_F(SpirvWriterTest, Constant_Vec4f) {
    writer_.Constant(b.Composite(ty.vec4<f32>(), 42_f, 0_f, 0.25_f, -1_f));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpConstantComposite %v4float %float_42 %float_0 %float_0_25 %float_n1");
}

TEST_F(SpirvWriterTest, Constant_Vec2h) {
    writer_.Constant(b.Composite(ty.vec2<f16>(), 42_h, 0.25_h));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpConstantComposite %v2half %half_0x1_5p_5 %half_0x1pn2");
}

TEST_F(SpirvWriterTest, Constant_Mat2x3f) {
    writer_.Constant(b.Composite(ty.mat2x3<f32>(),  //
                                 b.Composite(ty.vec3<f32>(), 42_f, -1_f, 0.25_f),
                                 b.Composite(ty.vec3<f32>(), -42_f, 0_f, -0.25_f)));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
   %float_42 = OpConstant %float 42
   %float_n1 = OpConstant %float -1
 %float_0_25 = OpConstant %float 0.25
          %5 = OpConstantComposite %v3float %float_42 %float_n1 %float_0_25
  %float_n42 = OpConstant %float -42
    %float_0 = OpConstant %float 0
%float_n0_25 = OpConstant %float -0.25
          %9 = OpConstantComposite %v3float %float_n42 %float_0 %float_n0_25
          %1 = OpConstantComposite %mat2v3float %5 %9
)");
}

TEST_F(SpirvWriterTest, Constant_Mat4x2h) {
    writer_.Constant(b.Composite(ty.mat4x2<f16>(),                          //
                                 b.Composite(ty.vec2<f16>(), 42_h, -1_h),   //
                                 b.Composite(ty.vec2<f16>(), 0_h, 0.25_h),  //
                                 b.Composite(ty.vec2<f16>(), -42_h, 1_h),   //
                                 b.Composite(ty.vec2<f16>(), 0.5_h, f16(-0))));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
%half_0x1_5p_5 = OpConstant %half 0x1.5p+5
%half_n0x1p_0 = OpConstant %half -0x1p+0
          %5 = OpConstantComposite %v2half %half_0x1_5p_5 %half_n0x1p_0
%half_0x0p_0 = OpConstant %half 0x0p+0
%half_0x1pn2 = OpConstant %half 0x1p-2
          %8 = OpConstantComposite %v2half %half_0x0p_0 %half_0x1pn2
%half_n0x1_5p_5 = OpConstant %half -0x1.5p+5
%half_0x1p_0 = OpConstant %half 0x1p+0
         %11 = OpConstantComposite %v2half %half_n0x1_5p_5 %half_0x1p_0
%half_0x1pn1 = OpConstant %half 0x1p-1
         %14 = OpConstantComposite %v2half %half_0x1pn1 %half_0x0p_0
          %1 = OpConstantComposite %mat4v2half %5 %8 %11 %14
)");
}

TEST_F(SpirvWriterTest, Constant_Array_I32) {
    writer_.Constant(b.Composite(ty.array<i32, 4>(), 1_i, 2_i, 3_i, 4_i));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpConstantComposite %_arr_int_uint_4 %int_1 %int_2 %int_3 %int_4");
}

TEST_F(SpirvWriterTest, Constant_Array_Array_I32) {
    auto* inner = b.Composite(ty.array<i32, 4>(), 1_i, 2_i, 3_i, 4_i);
    writer_.Constant(b.Composite(ty.array(ty.array<i32, 4>(), 4), inner, inner, inner, inner));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %7 = OpConstantComposite %_arr_int_uint_4 %int_1 %int_2 %int_3 %int_4
          %1 = OpConstantComposite %_arr__arr_int_uint_4_uint_4 %7 %7 %7 %7
)");
}

TEST_F(SpirvWriterTest, Constant_Struct) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.u32()},
                                                              {mod.symbols.New("c"), ty.f32()},
                                                          });
    writer_.Constant(b.Composite(str_ty, 1_i, 2_u, 3_f));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpConstantComposite %MyStruct %int_1 %uint_2 %float_3");
}

// Test that we do not emit the same constant more than once.
TEST_F(SpirvWriterTest, Constant_Deduplicate) {
    writer_.Constant(b.Constant(42_i));
    writer_.Constant(b.Constant(42_i));
    writer_.Constant(b.Constant(42_i));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%int_42 = OpConstant %int 42");
}

}  // namespace
}  // namespace tint::spirv::writer
