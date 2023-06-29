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

#include "src/tint/type/bool.h"
#include "src/tint/type/f16.h"
#include "src/tint/type/f32.h"
#include "src/tint/type/i32.h"
#include "src/tint/type/type.h"
#include "src/tint/type/u32.h"
#include "src/tint/type/void.h"
#include "src/tint/writer/spirv/ir/test_helper_ir.h"

namespace tint::writer::spirv {
namespace {

TEST_F(SpvGeneratorImplTest, Type_Void) {
    generator_.Type(ty.void_());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%void = OpTypeVoid");
}

TEST_F(SpvGeneratorImplTest, Type_Bool) {
    generator_.Type(ty.bool_());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%bool = OpTypeBool");
}

TEST_F(SpvGeneratorImplTest, Type_I32) {
    generator_.Type(ty.i32());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%int = OpTypeInt 32 1");
}

TEST_F(SpvGeneratorImplTest, Type_U32) {
    generator_.Type(ty.u32());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%uint = OpTypeInt 32 0");
}

TEST_F(SpvGeneratorImplTest, Type_F32) {
    generator_.Type(ty.f32());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%float = OpTypeFloat 32");
}

TEST_F(SpvGeneratorImplTest, Type_F16) {
    generator_.Type(ty.f16());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpCapability Float16");
    EXPECT_INST("OpCapability UniformAndStorageBuffer16BitAccess");
    EXPECT_INST("OpCapability StorageBuffer16BitAccess");
    EXPECT_INST("OpCapability StorageInputOutput16");
    EXPECT_INST("%half = OpTypeFloat 16");
}

TEST_F(SpvGeneratorImplTest, Type_Vec2i) {
    generator_.Type(ty.vec2<i32>());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v2int = OpTypeVector %int 2");
}

TEST_F(SpvGeneratorImplTest, Type_Vec3u) {
    generator_.Type(ty.vec3<u32>());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v3uint = OpTypeVector %uint 3");
}

TEST_F(SpvGeneratorImplTest, Type_Vec4f) {
    generator_.Type(ty.vec4<f32>());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v4float = OpTypeVector %float 4");
}

TEST_F(SpvGeneratorImplTest, Type_Vec2h) {
    generator_.Type(ty.vec2<f16>());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v2half = OpTypeVector %half 2");
}

TEST_F(SpvGeneratorImplTest, Type_Vec4Bool) {
    generator_.Type(ty.vec4<bool>());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v4bool = OpTypeVector %bool 4");
}

TEST_F(SpvGeneratorImplTest, Type_Mat2x3f) {
    generator_.Type(ty.mat2x3(ty.f32()));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%mat2v3float = OpTypeMatrix %v3float 2");
}

TEST_F(SpvGeneratorImplTest, Type_Mat4x2h) {
    generator_.Type(ty.mat4x2(ty.f16()));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%mat4v2half = OpTypeMatrix %v2half 4");
}

TEST_F(SpvGeneratorImplTest, Type_Array_DefaultStride) {
    generator_.Type(ty.array<f32, 4>());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpDecorate %_arr_float_uint_4 ArrayStride 4");
    EXPECT_INST("%_arr_float_uint_4 = OpTypeArray %float %uint_4");
}

TEST_F(SpvGeneratorImplTest, Type_Array_ExplicitStride) {
    generator_.Type(ty.array<f32, 4>(16));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpDecorate %_arr_float_uint_4 ArrayStride 16");
    EXPECT_INST("%_arr_float_uint_4 = OpTypeArray %float %uint_4");
}

TEST_F(SpvGeneratorImplTest, Type_Array_NestedArray) {
    generator_.Type(ty.array(ty.array<f32, 64u>(), 4u));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpDecorate %_arr_float_uint_64 ArrayStride 4");
    EXPECT_INST("OpDecorate %_arr__arr_float_uint_64_uint_4 ArrayStride 256");
    EXPECT_INST("%_arr_float_uint_64 = OpTypeArray %float %uint_64");
    EXPECT_INST("%_arr__arr_float_uint_64_uint_4 = OpTypeArray %_arr_float_uint_64 %uint_4");
}

TEST_F(SpvGeneratorImplTest, Type_RuntimeArray_DefaultStride) {
    generator_.Type(ty.array<f32>());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpDecorate %_runtimearr_float ArrayStride 4");
    EXPECT_INST("%_runtimearr_float = OpTypeRuntimeArray %float");
}

TEST_F(SpvGeneratorImplTest, Type_RuntimeArray_ExplicitStride) {
    generator_.Type(ty.array<f32>(16));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpDecorate %_runtimearr_float ArrayStride 16");
    EXPECT_INST("%_runtimearr_float = OpTypeRuntimeArray %float");
}

TEST_F(SpvGeneratorImplTest, Type_Struct) {
    auto* str =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.Register("a"), ty.f32()},
                                                   {mod.symbols.Register("b"), ty.vec4<i32>()},
                                               });
    generator_.Type(str);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpMemberName %MyStruct 0 \"a\"");
    EXPECT_INST("OpMemberName %MyStruct 1 \"b\"");
    EXPECT_INST("OpName %MyStruct \"MyStruct\"");
    EXPECT_INST("OpMemberDecorate %MyStruct 0 Offset 0");
    EXPECT_INST("OpMemberDecorate %MyStruct 1 Offset 16");
    EXPECT_INST("%MyStruct = OpTypeStruct %float %v4int");
}

TEST_F(SpvGeneratorImplTest, Type_Struct_MatrixLayout) {
    auto* str = ty.Struct(
        mod.symbols.New("MyStruct"),
        {
            {mod.symbols.Register("m"), ty.mat3x3<f32>()},
            // Matrices nested inside arrays need layout decorations on the struct member too.
            {mod.symbols.Register("arr"), ty.array(ty.array(ty.mat2x4<f16>(), 4), 4)},
        });
    generator_.Type(str);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpMemberDecorate %MyStruct 0 ColMajor");
    EXPECT_INST("OpMemberDecorate %MyStruct 0 MatrixStride 16");
    EXPECT_INST("OpMemberDecorate %MyStruct 1 ColMajor");
    EXPECT_INST("OpMemberDecorate %MyStruct 1 MatrixStride 8");
    EXPECT_INST("%MyStruct = OpTypeStruct %mat3v3float %_arr__arr_mat2v4half_uint_4_uint_4");
}

// Test that we can emit multiple types.
// Includes types with the same opcode but different parameters.
TEST_F(SpvGeneratorImplTest, Type_Multiple) {
    generator_.Type(ty.i32());
    generator_.Type(ty.u32());
    generator_.Type(ty.f32());
    generator_.Type(ty.f16());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
       %half = OpTypeFloat 16
)");
}

// Test that we do not emit the same type more than once.
TEST_F(SpvGeneratorImplTest, Type_Deduplicate) {
    auto id = generator_.Type(ty.i32());
    EXPECT_EQ(generator_.Type(ty.i32()), id);
    EXPECT_EQ(generator_.Type(ty.i32()), id);

    ASSERT_TRUE(Generate()) << Error() << output_;
}

}  // namespace
}  // namespace tint::writer::spirv
