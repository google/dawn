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
    auto id = generator_.Type(ty.void_());
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeVoid\n");
}

TEST_F(SpvGeneratorImplTest, Type_Bool) {
    auto id = generator_.Type(ty.bool_());
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeBool\n");
}

TEST_F(SpvGeneratorImplTest, Type_I32) {
    auto id = generator_.Type(ty.i32());
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeInt 32 1\n");
}

TEST_F(SpvGeneratorImplTest, Type_U32) {
    auto id = generator_.Type(ty.u32());
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeInt 32 0\n");
}

TEST_F(SpvGeneratorImplTest, Type_F32) {
    auto id = generator_.Type(ty.f32());
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeFloat 32\n");
}

TEST_F(SpvGeneratorImplTest, Type_F16) {
    auto id = generator_.Type(ty.f16());
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeFloat 16\n");
    EXPECT_EQ(DumpInstructions(generator_.Module().Capabilities()),
              "OpCapability Float16\n"
              "OpCapability UniformAndStorageBuffer16BitAccess\n"
              "OpCapability StorageBuffer16BitAccess\n"
              "OpCapability StorageInputOutput16\n");
}

TEST_F(SpvGeneratorImplTest, Type_Vec2i) {
    auto id = generator_.Type(ty.vec2(ty.i32()));
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%2 = OpTypeInt 32 1\n"
              "%1 = OpTypeVector %2 2\n");
}

TEST_F(SpvGeneratorImplTest, Type_Vec3u) {
    auto id = generator_.Type(ty.vec3(ty.u32()));
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%2 = OpTypeInt 32 0\n"
              "%1 = OpTypeVector %2 3\n");
}

TEST_F(SpvGeneratorImplTest, Type_Vec4f) {
    auto id = generator_.Type(ty.vec4(ty.f32()));
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%2 = OpTypeFloat 32\n"
              "%1 = OpTypeVector %2 4\n");
}

TEST_F(SpvGeneratorImplTest, Type_Vec2h) {
    auto id = generator_.Type(ty.vec2(ty.f16()));
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%2 = OpTypeFloat 16\n"
              "%1 = OpTypeVector %2 2\n");
}

TEST_F(SpvGeneratorImplTest, Type_Vec4Bool) {
    auto id = generator_.Type(ty.vec4(ty.bool_()));
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%2 = OpTypeBool\n"
              "%1 = OpTypeVector %2 4\n");
}

TEST_F(SpvGeneratorImplTest, Type_Mat2x3f) {
    auto* vec = ty.mat2x3(ty.f32());
    auto id = generator_.Type(vec);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%3 = OpTypeFloat 32\n"
              "%2 = OpTypeVector %3 3\n"
              "%1 = OpTypeMatrix %2 2\n");
}

TEST_F(SpvGeneratorImplTest, Type_Mat4x2h) {
    auto* vec = ty.mat4x2(ty.f16());
    auto id = generator_.Type(vec);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%3 = OpTypeFloat 16\n"
              "%2 = OpTypeVector %3 2\n"
              "%1 = OpTypeMatrix %2 4\n");
}

TEST_F(SpvGeneratorImplTest, Type_Array_DefaultStride) {
    auto* arr = ty.array(ty.f32(), 4u);
    auto id = generator_.Type(arr);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%2 = OpTypeFloat 32\n"
              "%4 = OpTypeInt 32 0\n"
              "%3 = OpConstant %4 4\n"
              "%1 = OpTypeArray %2 %3\n");
    EXPECT_EQ(DumpInstructions(generator_.Module().Annots()), "OpDecorate %1 ArrayStride 4\n");
}

TEST_F(SpvGeneratorImplTest, Type_Array_ExplicitStride) {
    auto* arr = ty.array(ty.f32(), 4u, 16);
    auto id = generator_.Type(arr);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%2 = OpTypeFloat 32\n"
              "%4 = OpTypeInt 32 0\n"
              "%3 = OpConstant %4 4\n"
              "%1 = OpTypeArray %2 %3\n");
    EXPECT_EQ(DumpInstructions(generator_.Module().Annots()), "OpDecorate %1 ArrayStride 16\n");
}

TEST_F(SpvGeneratorImplTest, Type_Array_NestedArray) {
    auto* arr = ty.array(ty.array(ty.f32(), 64u), 4u);
    auto id = generator_.Type(arr);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%3 = OpTypeFloat 32\n"
              "%5 = OpTypeInt 32 0\n"
              "%4 = OpConstant %5 64\n"
              "%2 = OpTypeArray %3 %4\n"
              "%6 = OpConstant %5 4\n"
              "%1 = OpTypeArray %2 %6\n");
    EXPECT_EQ(DumpInstructions(generator_.Module().Annots()),
              "OpDecorate %2 ArrayStride 4\n"
              "OpDecorate %1 ArrayStride 256\n");
}

TEST_F(SpvGeneratorImplTest, Type_RuntimeArray_DefaultStride) {
    auto* arr = ty.runtime_array(ty.f32());
    auto id = generator_.Type(arr);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%2 = OpTypeFloat 32\n"
              "%1 = OpTypeRuntimeArray %2\n");
    EXPECT_EQ(DumpInstructions(generator_.Module().Annots()), "OpDecorate %1 ArrayStride 4\n");
}

TEST_F(SpvGeneratorImplTest, Type_RuntimeArray_ExplicitStride) {
    auto* arr = ty.runtime_array(ty.f32(), 16);
    auto id = generator_.Type(arr);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%2 = OpTypeFloat 32\n"
              "%1 = OpTypeRuntimeArray %2\n");
    EXPECT_EQ(DumpInstructions(generator_.Module().Annots()), "OpDecorate %1 ArrayStride 16\n");
}

TEST_F(SpvGeneratorImplTest, Type_Struct) {
    auto* str =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.Register("a"), ty.f32()},
                                                   {mod.symbols.Register("b"), ty.vec4<i32>()},
                                               });
    auto id = generator_.Type(str);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), R"(%2 = OpTypeFloat 32
%4 = OpTypeInt 32 1
%3 = OpTypeVector %4 4
%1 = OpTypeStruct %2 %3
)");
    EXPECT_EQ(DumpInstructions(generator_.Module().Annots()), R"(OpMemberDecorate %1 0 Offset 0
OpMemberDecorate %1 1 Offset 16
)");
    EXPECT_EQ(DumpInstructions(generator_.Module().Debug()), R"(OpMemberName %1 0 "a"
OpMemberName %1 1 "b"
OpName %1 "MyStruct"
)");
}

TEST_F(SpvGeneratorImplTest, Type_Struct_MatrixLayout) {
    auto* str = ty.Struct(
        mod.symbols.New("MyStruct"),
        {
            {mod.symbols.Register("m"), ty.mat3x3<f32>()},
            // Matrices nested inside arrays need layout decorations on the struct member too.
            {mod.symbols.Register("arr"), ty.array(ty.array(ty.mat2x4<f16>(), 4), 4)},
        });
    auto id = generator_.Type(str);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypeMatrix %3 3
%9 = OpTypeFloat 16
%8 = OpTypeVector %9 4
%7 = OpTypeMatrix %8 2
%11 = OpTypeInt 32 0
%10 = OpConstant %11 4
%6 = OpTypeArray %7 %10
%5 = OpTypeArray %6 %10
%1 = OpTypeStruct %2 %5
)");
    EXPECT_EQ(DumpInstructions(generator_.Module().Annots()), R"(OpMemberDecorate %1 0 Offset 0
OpMemberDecorate %1 0 ColMajor
OpMemberDecorate %1 0 MatrixStride 16
OpDecorate %6 ArrayStride 16
OpDecorate %5 ArrayStride 64
OpMemberDecorate %1 1 Offset 48
OpMemberDecorate %1 1 ColMajor
OpMemberDecorate %1 1 MatrixStride 8
)");
    EXPECT_EQ(DumpInstructions(generator_.Module().Debug()), R"(OpMemberName %1 0 "m"
OpMemberName %1 1 "arr"
OpName %1 "MyStruct"
)");
}

// Test that we can emit multiple types.
// Includes types with the same opcode but different parameters.
TEST_F(SpvGeneratorImplTest, Type_Multiple) {
    EXPECT_EQ(generator_.Type(ty.i32()), 1u);
    EXPECT_EQ(generator_.Type(ty.u32()), 2u);
    EXPECT_EQ(generator_.Type(ty.f32()), 3u);
    EXPECT_EQ(generator_.Type(ty.f16()), 4u);
    EXPECT_EQ(DumpTypes(), R"(%1 = OpTypeInt 32 1
%2 = OpTypeInt 32 0
%3 = OpTypeFloat 32
%4 = OpTypeFloat 16
)");
}

// Test that we do not emit the same type more than once.
TEST_F(SpvGeneratorImplTest, Type_Deduplicate) {
    auto* i32 = ty.i32();
    EXPECT_EQ(generator_.Type(i32), 1u);
    EXPECT_EQ(generator_.Type(i32), 1u);
    EXPECT_EQ(generator_.Type(i32), 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeInt 32 1\n");
}

}  // namespace
}  // namespace tint::writer::spirv
