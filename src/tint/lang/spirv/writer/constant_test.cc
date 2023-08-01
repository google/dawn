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
    auto const_bool = [&](bool val) { return mod.constant_values.Get(val); };
    auto* v = mod.constant_values.Composite(
        ty.vec4(ty.bool_()),
        Vector{const_bool(true), const_bool(false), const_bool(false), const_bool(true)});

    writer_.Constant(b.Constant(v));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpConstantComposite %v4bool %true %false %false %true");
}

TEST_F(SpirvWriterTest, Constant_Vec2i) {
    auto const_i32 = [&](float val) { return mod.constant_values.Get(i32(val)); };
    auto* v =
        mod.constant_values.Composite(ty.vec2(ty.i32()), Vector{const_i32(42), const_i32(-1)});
    writer_.Constant(b.Constant(v));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpConstantComposite %v2int %int_42 %int_n1");
}

TEST_F(SpirvWriterTest, Constant_Vec3u) {
    auto const_u32 = [&](float val) { return mod.constant_values.Get(u32(val)); };
    auto* v = mod.constant_values.Composite(
        ty.vec3(ty.u32()), Vector{const_u32(42), const_u32(0), const_u32(4000000000)});
    writer_.Constant(b.Constant(v));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpConstantComposite %v3uint %uint_42 %uint_0 %uint_4000000000");
}

TEST_F(SpirvWriterTest, Constant_Vec4f) {
    auto const_f32 = [&](float val) { return mod.constant_values.Get(f32(val)); };
    auto* v = mod.constant_values.Composite(
        ty.vec4(ty.f32()), Vector{const_f32(42), const_f32(0), const_f32(0.25), const_f32(-1)});
    writer_.Constant(b.Constant(v));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpConstantComposite %v4float %float_42 %float_0 %float_0_25 %float_n1");
}

TEST_F(SpirvWriterTest, Constant_Vec2h) {
    auto const_f16 = [&](float val) { return mod.constant_values.Get(f16(val)); };
    auto* v =
        mod.constant_values.Composite(ty.vec2(ty.f16()), Vector{const_f16(42), const_f16(0.25)});
    writer_.Constant(b.Constant(v));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpConstantComposite %v2half %half_0x1_5p_5 %half_0x1pn2");
}

TEST_F(SpirvWriterTest, Constant_Mat2x3f) {
    auto const_f32 = [&](float val) { return mod.constant_values.Get(f32(val)); };
    auto* f32 = ty.f32();
    auto* v = mod.constant_values.Composite(
        ty.mat2x3(f32),
        Vector{
            mod.constant_values.Composite(ty.vec3(f32),
                                          Vector{const_f32(42), const_f32(-1), const_f32(0.25)}),
            mod.constant_values.Composite(ty.vec3(f32),
                                          Vector{const_f32(-42), const_f32(0), const_f32(-0.25)}),
        });
    writer_.Constant(b.Constant(v));
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
    auto const_f16 = [&](float val) { return mod.constant_values.Get(f16(val)); };
    auto* f16 = ty.f16();
    auto* v = mod.constant_values.Composite(
        ty.mat4x2(f16),
        Vector{
            mod.constant_values.Composite(ty.vec2(f16), Vector{const_f16(42), const_f16(-1)}),
            mod.constant_values.Composite(ty.vec2(f16), Vector{const_f16(0), const_f16(0.25)}),
            mod.constant_values.Composite(ty.vec2(f16), Vector{const_f16(-42), const_f16(1)}),
            mod.constant_values.Composite(ty.vec2(f16), Vector{const_f16(0.5), const_f16(-0)}),
        });
    writer_.Constant(b.Constant(v));
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
    auto* arr =
        mod.constant_values.Composite(ty.array(ty.i32(), 4), Vector{
                                                                 mod.constant_values.Get(1_i),
                                                                 mod.constant_values.Get(2_i),
                                                                 mod.constant_values.Get(3_i),
                                                                 mod.constant_values.Get(4_i),
                                                             });
    writer_.Constant(b.Constant(arr));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpConstantComposite %_arr_int_uint_4 %int_1 %int_2 %int_3 %int_4");
}

TEST_F(SpirvWriterTest, Constant_Array_Array_I32) {
    auto* inner =
        mod.constant_values.Composite(ty.array(ty.i32(), 4), Vector{
                                                                 mod.constant_values.Get(1_i),
                                                                 mod.constant_values.Get(2_i),
                                                                 mod.constant_values.Get(3_i),
                                                                 mod.constant_values.Get(4_i),
                                                             });
    auto* arr = mod.constant_values.Composite(ty.array(ty.array(ty.i32(), 4), 4), Vector{
                                                                                      inner,
                                                                                      inner,
                                                                                      inner,
                                                                                      inner,
                                                                                  });
    writer_.Constant(b.Constant(arr));
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
    auto* str = mod.constant_values.Composite(str_ty, Vector{
                                                          mod.constant_values.Get(1_i),
                                                          mod.constant_values.Get(2_u),
                                                          mod.constant_values.Get(3_f),
                                                      });
    writer_.Constant(b.Constant(str));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpConstantComposite %MyStruct %int_1 %uint_2 %float_3");
}

// Test that we do not emit the same constant more than once.
TEST_F(SpirvWriterTest, Constant_Deduplicate) {
    writer_.Constant(b.Constant(i32(42)));
    writer_.Constant(b.Constant(i32(42)));
    writer_.Constant(b.Constant(i32(42)));
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%int_42 = OpConstant %int 42");
}

}  // namespace
}  // namespace tint::spirv::writer
