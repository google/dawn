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

#include "src/ast/struct_block_decoration.h"
#include "src/type/depth_texture_type.h"
#include "src/type/multisampled_texture_type.h"
#include "src/type/sampled_texture_type.h"
#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest_Type = TestHelper;

TEST_F(BuilderTest_Type, GenerateAlias) {
  auto* alias_type = ty.alias("my_type", ty.f32());

  spirv::Builder& b = Build();

  auto id = b.GenerateTypeIfNeeded(alias_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedAlias) {
  auto* i32 = ty.i32();
  auto* f32 = ty.f32();
  auto* alias_type = ty.alias("my_type", f32);

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(alias_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(alias_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(f32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateRuntimeArray) {
  auto* ary = ty.array(ty.i32(), 0);

  spirv::Builder& b = Build();

  auto id = b.GenerateTypeIfNeeded(ary);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeRuntimeArray %2
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedRuntimeArray) {
  auto* ary = ty.array(ty.i32(), 0);

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(ary), 1u);
  EXPECT_EQ(b.GenerateTypeIfNeeded(ary), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeRuntimeArray %2
)");
}

TEST_F(BuilderTest_Type, GenerateArray) {
  auto* ary = ty.array(ty.i32(), 4);

  spirv::Builder& b = Build();

  auto id = b.GenerateTypeIfNeeded(ary);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeInt 32 0
%4 = OpConstant %3 4
%1 = OpTypeArray %2 %4
)");
}

TEST_F(BuilderTest_Type, GenerateArray_WithStride) {
  auto* ary = ty.array(ty.i32(), 4, 16u);

  spirv::Builder& b = Build();

  auto id = b.GenerateTypeIfNeeded(ary);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 ArrayStride 16
)");

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeInt 32 0
%4 = OpConstant %3 4
%1 = OpTypeArray %2 %4
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedArray) {
  auto* ary = ty.array(ty.i32(), 4);

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(ary), 1u);
  EXPECT_EQ(b.GenerateTypeIfNeeded(ary), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeInt 32 0
%4 = OpConstant %3 4
%1 = OpTypeArray %2 %4
)");
}

TEST_F(BuilderTest_Type, GenerateBool) {
  auto* bool_ = ty.bool_();

  spirv::Builder& b = Build();

  auto id = b.GenerateTypeIfNeeded(bool_);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  ASSERT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeBool
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedBool) {
  auto* bool_ = ty.bool_();
  auto* i32 = ty.i32();

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(bool_), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(bool_), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateF32) {
  auto* f32 = ty.f32();

  spirv::Builder& b = Build();

  auto id = b.GenerateTypeIfNeeded(f32);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  ASSERT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeFloat 32
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedF32) {
  auto* f32 = ty.f32();
  auto* i32 = ty.i32();

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(f32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(f32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateI32) {
  auto* i32 = ty.i32();

  spirv::Builder& b = Build();

  auto id = b.GenerateTypeIfNeeded(i32);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  ASSERT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeInt 32 1
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedI32) {
  auto* f32 = ty.f32();
  auto* i32 = ty.i32();

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(f32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateMatrix) {
  auto* mat2x3 = ty.mat2x3<f32>();

  spirv::Builder& b = Build();

  auto id = b.GenerateTypeIfNeeded(mat2x3);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(b.types().size(), 3u);
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 2
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedMatrix) {
  auto* mat = ty.mat4x3<i32>();
  auto* i32 = ty.i32();

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(mat), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 3u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(mat), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GeneratePtr) {
  type::Pointer ptr(ty.i32(), ast::StorageClass::kOutput);

  spirv::Builder& b = Build();

  auto id = b.GenerateTypeIfNeeded(&ptr);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypePointer Output %2
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedPtr) {
  type::Pointer ptr(ty.i32(), ast::StorageClass::kOutput);

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(&ptr), 1u);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&ptr), 1u);
}

TEST_F(BuilderTest_Type, GenerateStruct_Empty) {
  auto* s = Structure("S", {});

  spirv::Builder& b = Build();

  auto id = b.GenerateTypeIfNeeded(s);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "S"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeStruct
)");
}

TEST_F(BuilderTest_Type, GenerateStruct) {
  auto* s = Structure("my_struct", {Member("a", ty.f32())});

  spirv::Builder& b = Build();

  auto id = b.GenerateTypeIfNeeded(s);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeStruct %2
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "my_struct"
OpMemberName %1 0 "a"
)");
}

TEST_F(BuilderTest_Type, GenerateStruct_Decorated) {
  auto* s = Structure("my_struct", {Member("a", ty.f32())},
                      {create<ast::StructBlockDecoration>()});

  spirv::Builder& b = Build();

  auto id = b.GenerateTypeIfNeeded(s);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeStruct %2
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "my_struct"
OpMemberName %1 0 "a"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 Block
OpMemberDecorate %1 0 Offset 0
)");
}

TEST_F(BuilderTest_Type, GenerateStruct_DecoratedMembers) {
  auto* s = Structure("S", {
                               Member("a", ty.f32()),
                               Member("b", ty.f32(), {MemberAlign(8)}),
                           });

  spirv::Builder& b = Build();

  auto id = b.GenerateTypeIfNeeded(s);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeStruct %2 %2
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "S"
OpMemberName %1 0 "a"
OpMemberName %1 1 "b"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpMemberDecorate %1 0 Offset 0
OpMemberDecorate %1 1 Offset 8
)");
}

TEST_F(BuilderTest_Type, GenerateStruct_NonLayout_Matrix) {
  auto* s = Structure("S", {
                               Member("a", ty.mat2x2<f32>()),
                               Member("b", ty.mat2x3<f32>()),
                               Member("c", ty.mat4x4<f32>()),
                           });

  spirv::Builder& b = Build();

  auto id = b.GenerateTypeIfNeeded(s);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 2
%2 = OpTypeMatrix %3 2
%6 = OpTypeVector %4 3
%5 = OpTypeMatrix %6 2
%8 = OpTypeVector %4 4
%7 = OpTypeMatrix %8 4
%1 = OpTypeStruct %2 %5 %7
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "S"
OpMemberName %1 0 "a"
OpMemberName %1 1 "b"
OpMemberName %1 2 "c"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpMemberDecorate %1 0 Offset 0
OpMemberDecorate %1 0 ColMajor
OpMemberDecorate %1 0 MatrixStride 8
OpMemberDecorate %1 1 Offset 16
OpMemberDecorate %1 1 ColMajor
OpMemberDecorate %1 1 MatrixStride 16
OpMemberDecorate %1 2 Offset 48
OpMemberDecorate %1 2 ColMajor
OpMemberDecorate %1 2 MatrixStride 16
)");
}

TEST_F(BuilderTest_Type, GenerateStruct_DecoratedMembers_LayoutMatrix) {
  // We have to infer layout for matrix when it also has an offset.
  auto* s = Structure("S", {
                               Member("a", ty.mat2x2<f32>()),
                               Member("b", ty.mat2x3<f32>()),
                               Member("c", ty.mat4x4<f32>()),
                           });

  spirv::Builder& b = Build();

  auto id = b.GenerateTypeIfNeeded(s);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 2
%2 = OpTypeMatrix %3 2
%6 = OpTypeVector %4 3
%5 = OpTypeMatrix %6 2
%8 = OpTypeVector %4 4
%7 = OpTypeMatrix %8 4
%1 = OpTypeStruct %2 %5 %7
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "S"
OpMemberName %1 0 "a"
OpMemberName %1 1 "b"
OpMemberName %1 2 "c"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpMemberDecorate %1 0 Offset 0
OpMemberDecorate %1 0 ColMajor
OpMemberDecorate %1 0 MatrixStride 8
OpMemberDecorate %1 1 Offset 16
OpMemberDecorate %1 1 ColMajor
OpMemberDecorate %1 1 MatrixStride 16
OpMemberDecorate %1 2 Offset 48
OpMemberDecorate %1 2 ColMajor
OpMemberDecorate %1 2 MatrixStride 16
)");
}

TEST_F(BuilderTest_Type, GenerateStruct_DecoratedMembers_LayoutArraysOfMatrix) {
  // We have to infer layout for matrix when it also has an offset.
  // The decoration goes on the struct member, even if the matrix is buried
  // in levels of arrays.
  auto* arr_mat2x2 = ty.array(ty.mat2x2<f32>(), 1);      // Singly nested array
  auto* arr_arr_mat2x3 = ty.array(ty.mat2x3<f32>(), 1);  // Doubly nested array
  auto* rtarr_mat4x4 = ty.array(ty.mat4x4<f32>(), 0);    // Runtime array

  auto* s = Structure("S", {
                               Member("a", arr_mat2x2),
                               Member("b", arr_arr_mat2x3),
                               Member("c", rtarr_mat4x4),
                           });

  spirv::Builder& b = Build();

  auto id = b.GenerateTypeIfNeeded(s);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 2
%3 = OpTypeMatrix %4 2
%6 = OpTypeInt 32 0
%7 = OpConstant %6 1
%2 = OpTypeArray %3 %7
%10 = OpTypeVector %5 3
%9 = OpTypeMatrix %10 2
%8 = OpTypeArray %9 %7
%13 = OpTypeVector %5 4
%12 = OpTypeMatrix %13 4
%11 = OpTypeRuntimeArray %12
%1 = OpTypeStruct %2 %8 %11
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "S"
OpMemberName %1 0 "a"
OpMemberName %1 1 "b"
OpMemberName %1 2 "c"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpMemberDecorate %1 0 Offset 0
OpMemberDecorate %1 0 ColMajor
OpMemberDecorate %1 0 MatrixStride 8
OpDecorate %2 ArrayStride 16
OpMemberDecorate %1 1 Offset 16
OpMemberDecorate %1 1 ColMajor
OpMemberDecorate %1 1 MatrixStride 16
OpDecorate %8 ArrayStride 32
OpMemberDecorate %1 2 Offset 48
OpMemberDecorate %1 2 ColMajor
OpMemberDecorate %1 2 MatrixStride 16
OpDecorate %11 ArrayStride 64
)");
}

TEST_F(BuilderTest_Type, GenerateU32) {
  auto* u32 = ty.u32();

  spirv::Builder& b = Build();

  auto id = b.GenerateTypeIfNeeded(u32);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  ASSERT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeInt 32 0
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedU32) {
  auto* u32 = ty.u32();
  auto* f32 = ty.f32();

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(u32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(f32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(u32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateVector) {
  auto* vec = ty.vec3<f32>();

  spirv::Builder& b = Build();

  auto id = b.GenerateTypeIfNeeded(vec);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(b.types().size(), 2u);
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedVector) {
  auto* vec = ty.vec3<i32>();
  auto* i32 = ty.i32();

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(vec), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(vec), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateVoid) {
  auto* void_ = ty.void_();

  spirv::Builder& b = Build();

  auto id = b.GenerateTypeIfNeeded(void_);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  ASSERT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeVoid
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedVoid) {
  auto* void_ = ty.void_();
  auto* i32 = ty.i32();

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(void_), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(void_), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

struct PtrData {
  ast::StorageClass ast_class;
  SpvStorageClass result;
};
inline std::ostream& operator<<(std::ostream& out, PtrData data) {
  out << data.ast_class;
  return out;
}
using PtrDataTest = TestParamHelper<PtrData>;
TEST_P(PtrDataTest, ConvertStorageClass) {
  auto params = GetParam();

  spirv::Builder& b = Build();

  EXPECT_EQ(b.ConvertStorageClass(params.ast_class), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest_Type,
    PtrDataTest,
    testing::Values(
        PtrData{ast::StorageClass::kNone, SpvStorageClassMax},
        PtrData{ast::StorageClass::kInput, SpvStorageClassInput},
        PtrData{ast::StorageClass::kOutput, SpvStorageClassOutput},
        PtrData{ast::StorageClass::kUniform, SpvStorageClassUniform},
        PtrData{ast::StorageClass::kWorkgroup, SpvStorageClassWorkgroup},
        PtrData{ast::StorageClass::kUniformConstant,
                SpvStorageClassUniformConstant},
        PtrData{ast::StorageClass::kStorage, SpvStorageClassStorageBuffer},
        PtrData{ast::StorageClass::kImage, SpvStorageClassImage},
        PtrData{ast::StorageClass::kPrivate, SpvStorageClassPrivate},
        PtrData{ast::StorageClass::kFunction, SpvStorageClassFunction}));

TEST_F(BuilderTest_Type, DepthTexture_Generate_2d) {
  type::DepthTexture two_d(type::TextureDimension::k2d);

  spirv::Builder& b = Build();

  auto id_two_d = b.GenerateTypeIfNeeded(&two_d);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id_two_d);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 1 0 0 1 Unknown
)");
}

TEST_F(BuilderTest_Type, DepthTexture_Generate_2dArray) {
  type::DepthTexture two_d_array(type::TextureDimension::k2dArray);

  spirv::Builder& b = Build();

  auto id_two_d_array = b.GenerateTypeIfNeeded(&two_d_array);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id_two_d_array);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 1 1 0 1 Unknown
)");
}

TEST_F(BuilderTest_Type, DepthTexture_Generate_Cube) {
  type::DepthTexture cube(type::TextureDimension::kCube);

  spirv::Builder& b = Build();

  auto id_cube = b.GenerateTypeIfNeeded(&cube);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id_cube);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 Cube 1 0 0 1 Unknown
)");
  EXPECT_EQ(DumpInstructions(b.capabilities()), "");
}

TEST_F(BuilderTest_Type, DepthTexture_Generate_CubeArray) {
  type::DepthTexture cube_array(type::TextureDimension::kCubeArray);

  spirv::Builder& b = Build();

  auto id_cube_array = b.GenerateTypeIfNeeded(&cube_array);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id_cube_array);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 Cube 1 1 0 1 Unknown
)");
  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability SampledCubeArray
)");
}

TEST_F(BuilderTest_Type, MultisampledTexture_Generate_2d_i32) {
  type::MultisampledTexture ms(type::TextureDimension::k2d, ty.i32());

  spirv::Builder& b = Build();

  EXPECT_EQ(1u, b.GenerateTypeIfNeeded(&ms));
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeImage %2 2D 0 0 1 1 Unknown
)");
}

TEST_F(BuilderTest_Type, MultisampledTexture_Generate_2d_u32) {
  type::MultisampledTexture ms(type::TextureDimension::k2d, ty.u32());

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(&ms), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeInt 32 0
%1 = OpTypeImage %2 2D 0 0 1 1 Unknown
)");
}

TEST_F(BuilderTest_Type, MultisampledTexture_Generate_2d_f32) {
  type::MultisampledTexture ms(type::TextureDimension::k2d, ty.f32());

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(&ms), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 0 1 1 Unknown
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_1d_i32) {
  auto* s = create<type::SampledTexture>(type::TextureDimension::k1d, ty.i32());

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeInt 32 1
%1 = OpTypeImage %2 1D 0 0 0 1 Unknown
)");

  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability Sampled1D
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_1d_u32) {
  auto* s = create<type::SampledTexture>(type::TextureDimension::k1d, ty.u32());

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeInt 32 0
%1 = OpTypeImage %2 1D 0 0 0 1 Unknown
)");

  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability Sampled1D
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_1d_f32) {
  auto* s = create<type::SampledTexture>(type::TextureDimension::k1d, ty.f32());

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 1D 0 0 0 1 Unknown
)");

  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability Sampled1D
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_2d) {
  auto* s = create<type::SampledTexture>(type::TextureDimension::k2d, ty.f32());

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 0 0 1 Unknown
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_2d_array) {
  auto* s =
      create<type::SampledTexture>(type::TextureDimension::k2dArray, ty.f32());

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 1 0 1 Unknown
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_3d) {
  auto* s = create<type::SampledTexture>(type::TextureDimension::k3d, ty.f32());

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 3D 0 0 0 1 Unknown
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_Cube) {
  auto* s =
      create<type::SampledTexture>(type::TextureDimension::kCube, ty.f32());

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 Cube 0 0 0 1 Unknown
)");
  EXPECT_EQ(DumpInstructions(b.capabilities()), "");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_CubeArray) {
  auto* s = create<type::SampledTexture>(type::TextureDimension::kCubeArray,
                                         ty.f32());

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 Cube 0 1 0 1 Unknown
)");
  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability SampledCubeArray
)");
}

TEST_F(BuilderTest_Type, StorageTexture_Generate_1d_R16Float) {
  auto* subtype =
      type::StorageTexture::SubtypeFor(type::ImageFormat::kR16Float, Types());
  auto* s = create<type::StorageTexture>(type::TextureDimension::k1d,
                                         type::ImageFormat::kR16Float, subtype);

  Global("test_var", s, ast::StorageClass::kNone);

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 1D 0 0 0 2 R16f
)");

  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability Image1D
OpCapability StorageImageExtendedFormats
)");
}

TEST_F(BuilderTest_Type, StorageTexture_Generate_1d_R8SNorm) {
  auto* subtype =
      type::StorageTexture::SubtypeFor(type::ImageFormat::kR8Snorm, Types());
  auto* s = create<type::StorageTexture>(type::TextureDimension::k1d,
                                         type::ImageFormat::kR8Snorm, subtype);

  Global("test_var", s, ast::StorageClass::kNone);

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 1D 0 0 0 2 R8Snorm
)");

  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability Image1D
OpCapability StorageImageExtendedFormats
)");
}

TEST_F(BuilderTest_Type, StorageTexture_Generate_1d_R8UNorm) {
  auto* subtype =
      type::StorageTexture::SubtypeFor(type::ImageFormat::kR8Unorm, Types());
  auto* s = create<type::StorageTexture>(type::TextureDimension::k1d,
                                         type::ImageFormat::kR8Unorm, subtype);

  Global("test_var", s, ast::StorageClass::kNone);

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 1D 0 0 0 2 R8
)");

  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability Image1D
OpCapability StorageImageExtendedFormats
)");
}

TEST_F(BuilderTest_Type, StorageTexture_Generate_1d_R8Uint) {
  auto* subtype =
      type::StorageTexture::SubtypeFor(type::ImageFormat::kR8Uint, Types());
  auto* s = create<type::StorageTexture>(type::TextureDimension::k1d,
                                         type::ImageFormat::kR8Uint, subtype);

  Global("test_var", s, ast::StorageClass::kNone);

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 0
%1 = OpTypeImage %2 1D 0 0 0 2 R8ui
)");
}

TEST_F(BuilderTest_Type, StorageTexture_Generate_1d_R8Sint) {
  auto* subtype =
      type::StorageTexture::SubtypeFor(type::ImageFormat::kR8Sint, Types());
  auto* s = create<type::StorageTexture>(type::TextureDimension::k1d,
                                         type::ImageFormat::kR8Sint, subtype);

  Global("test_var", s, ast::StorageClass::kNone);

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeImage %2 1D 0 0 0 2 R8i
)");
}

TEST_F(BuilderTest_Type, StorageTexture_Generate_2d) {
  auto* subtype =
      type::StorageTexture::SubtypeFor(type::ImageFormat::kR16Float, Types());
  auto* s = create<type::StorageTexture>(type::TextureDimension::k2d,
                                         type::ImageFormat::kR16Float, subtype);

  Global("test_var", s, ast::StorageClass::kNone);

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 0 0 2 R16f
)");
}

TEST_F(BuilderTest_Type, StorageTexture_Generate_2dArray) {
  auto* subtype =
      type::StorageTexture::SubtypeFor(type::ImageFormat::kR16Float, Types());
  auto* s = create<type::StorageTexture>(type::TextureDimension::k2dArray,
                                         type::ImageFormat::kR16Float, subtype);

  Global("test_var", s, ast::StorageClass::kNone);

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 1 0 2 R16f
)");
}

TEST_F(BuilderTest_Type, StorageTexture_Generate_3d) {
  auto* subtype =
      type::StorageTexture::SubtypeFor(type::ImageFormat::kR16Float, Types());
  auto* s = create<type::StorageTexture>(type::TextureDimension::k3d,
                                         type::ImageFormat::kR16Float, subtype);

  Global("test_var", s, ast::StorageClass::kNone);

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 3D 0 0 0 2 R16f
)");
}

TEST_F(BuilderTest_Type,
       StorageTexture_Generate_SampledTypeFloat_Format_r32float) {
  auto* subtype =
      type::StorageTexture::SubtypeFor(type::ImageFormat::kR32Float, Types());
  auto* s = create<type::StorageTexture>(type::TextureDimension::k2d,
                                         type::ImageFormat::kR32Float, subtype);

  Global("test_var", s, ast::StorageClass::kNone);

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 0 0 2 R32f
)");
}

TEST_F(BuilderTest_Type,
       StorageTexture_Generate_SampledTypeSint_Format_r32sint) {
  auto* subtype =
      type::StorageTexture::SubtypeFor(type::ImageFormat::kR32Sint, Types());
  auto* s = create<type::StorageTexture>(type::TextureDimension::k2d,
                                         type::ImageFormat::kR32Sint, subtype);

  Global("test_var", s, ast::StorageClass::kNone);

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeImage %2 2D 0 0 0 2 R32i
)");
}

TEST_F(BuilderTest_Type,
       StorageTexture_Generate_SampledTypeUint_Format_r32uint) {
  auto* subtype =
      type::StorageTexture::SubtypeFor(type::ImageFormat::kR32Uint, Types());
  auto* s = create<type::StorageTexture>(type::TextureDimension::k2d,
                                         type::ImageFormat::kR32Uint, subtype);

  Global("test_var", s, ast::StorageClass::kNone);

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 0
%1 = OpTypeImage %2 2D 0 0 0 2 R32ui
)");
}

TEST_F(BuilderTest_Type, Sampler) {
  type::Sampler sampler(type::SamplerKind::kSampler);

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(&sampler), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), "%1 = OpTypeSampler\n");
}

TEST_F(BuilderTest_Type, ComparisonSampler) {
  type::Sampler sampler(type::SamplerKind::kComparisonSampler);

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(&sampler), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), "%1 = OpTypeSampler\n");
}

TEST_F(BuilderTest_Type, Dedup_Sampler_And_ComparisonSampler) {
  type::Sampler comp_sampler(type::SamplerKind::kComparisonSampler);
  type::Sampler sampler(type::SamplerKind::kSampler);

  spirv::Builder& b = Build();

  EXPECT_EQ(b.GenerateTypeIfNeeded(&comp_sampler), 1u);

  EXPECT_EQ(b.GenerateTypeIfNeeded(&sampler), 1u);

  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), "%1 = OpTypeSampler\n");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
