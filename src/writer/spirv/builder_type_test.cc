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

#include <memory>

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/depth_texture_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/sampled_texture_type.h"
#include "src/ast/type/sampler_type.h"
#include "src/ast/type/storage_texture_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/texture_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest_Type = testing::Test;

TEST_F(BuilderTest_Type, GenerateAlias) {
  ast::type::F32Type f32;
  ast::type::AliasType alias_type("my_type", &f32);

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateTypeIfNeeded(&alias_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedAlias) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  ast::type::AliasType alias_type("my_type", &f32);

  ast::Module mod;
  Builder b(&mod);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&alias_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&alias_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&f32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateRuntimeArray) {
  ast::type::I32Type i32;
  ast::type::ArrayType ary(&i32);

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateTypeIfNeeded(&ary);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeRuntimeArray %2
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedRuntimeArray) {
  ast::type::I32Type i32;
  ast::type::ArrayType ary(&i32);

  ast::Module mod;
  Builder b(&mod);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&ary), 1u);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&ary), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeRuntimeArray %2
)");
}

TEST_F(BuilderTest_Type, GenerateArray) {
  ast::type::I32Type i32;
  ast::type::ArrayType ary(&i32, 4);

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateTypeIfNeeded(&ary);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeInt 32 0
%4 = OpConstant %3 4
%1 = OpTypeArray %2 %4
)");
}

TEST_F(BuilderTest_Type, GenerateArray_WithStride) {
  ast::type::I32Type i32;
  ast::type::ArrayType ary(&i32, 4);
  ary.set_array_stride(16u);

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateTypeIfNeeded(&ary);
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
  ast::type::I32Type i32;
  ast::type::ArrayType ary(&i32, 4);

  ast::Module mod;
  Builder b(&mod);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&ary), 1u);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&ary), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeInt 32 0
%4 = OpConstant %3 4
%1 = OpTypeArray %2 %4
)");
}

TEST_F(BuilderTest_Type, GenerateBool) {
  ast::type::BoolType bool_type;

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateTypeIfNeeded(&bool_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  ASSERT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeBool
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedBool) {
  ast::type::I32Type i32;
  ast::type::BoolType bool_type;

  ast::Module mod;
  Builder b(&mod);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&bool_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&bool_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateF32) {
  ast::type::F32Type f32;

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateTypeIfNeeded(&f32);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  ASSERT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeFloat 32
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedF32) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::Module mod;
  Builder b(&mod);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&f32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&f32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateI32) {
  ast::type::I32Type i32;

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateTypeIfNeeded(&i32);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  ASSERT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeInt 32 1
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedI32) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::Module mod;
  Builder b(&mod);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&f32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateMatrix) {
  ast::type::F32Type f32;
  ast::type::MatrixType mat_type(&f32, 3, 2);

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateTypeIfNeeded(&mat_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(b.types().size(), 3u);
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 2
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedMatrix) {
  ast::type::I32Type i32;
  ast::type::MatrixType mat_type(&i32, 3, 4);

  ast::Module mod;
  Builder b(&mod);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&mat_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 3u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&mat_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GeneratePtr) {
  ast::type::I32Type i32;
  ast::type::PointerType ptr(&i32, ast::StorageClass::kOutput);

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateTypeIfNeeded(&ptr);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypePointer Output %2
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedPtr) {
  ast::type::I32Type i32;
  ast::type::PointerType ptr(&i32, ast::StorageClass::kOutput);

  ast::Module mod;
  Builder b(&mod);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&ptr), 1u);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&ptr), 1u);
}

TEST_F(BuilderTest_Type, GenerateStruct_Empty) {
  auto s = std::make_unique<ast::Struct>();
  ast::type::StructType s_type(std::move(s));

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateTypeIfNeeded(&s_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstructions(b.debug()), "");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeStruct
)");
}

TEST_F(BuilderTest_Type, GenerateStruct) {
  ast::type::F32Type f32;

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(
      std::make_unique<ast::StructMember>("a", &f32, std::move(decos)));

  auto s = std::make_unique<ast::Struct>(ast::StructDecoration::kNone,
                                         std::move(members));
  ast::type::StructType s_type(std::move(s));
  s_type.set_name("my_struct");

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateTypeIfNeeded(&s_type);
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
  ast::type::F32Type f32;

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(
      std::make_unique<ast::StructMember>("a", &f32, std::move(decos)));

  auto s = std::make_unique<ast::Struct>(ast::StructDecoration::kBlock,
                                         std::move(members));
  ast::type::StructType s_type(std::move(s));
  s_type.set_name("my_struct");

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateTypeIfNeeded(&s_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeStruct %2
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "my_struct"
OpMemberName %1 0 "a"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 Block
)");
}

TEST_F(BuilderTest_Type, GenerateStruct_DecoratedMembers) {
  ast::type::F32Type f32;

  ast::StructMemberDecorationList a_decos;
  a_decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  ast::StructMemberDecorationList b_decos;
  b_decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(8));

  ast::StructMemberList members;
  members.push_back(
      std::make_unique<ast::StructMember>("a", &f32, std::move(a_decos)));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(b_decos)));

  auto s = std::make_unique<ast::Struct>(ast::StructDecoration::kNone,
                                         std::move(members));
  ast::type::StructType s_type(std::move(s));

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateTypeIfNeeded(&s_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeStruct %2 %2
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpMemberName %1 0 "a"
OpMemberName %1 1 "b"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpMemberDecorate %1 0 Offset 0
OpMemberDecorate %1 1 Offset 8
)");
}

TEST_F(BuilderTest_Type, GenerateStruct_NonLayout_Matrix) {
  // Don't infer layout for matrix when there is no offset.
  ast::type::F32Type f32;
  ast::type::MatrixType glsl_mat2x2(&f32, 2, 2);
  ast::type::MatrixType glsl_mat2x3(&f32, 3, 2);  // 2 columns, 3 rows
  ast::type::MatrixType glsl_mat4x4(&f32, 4, 4);

  ast::StructMemberDecorationList empty_a;
  ast::StructMemberDecorationList empty_b;
  ast::StructMemberDecorationList empty_c;
  ast::StructMemberList members;
  members.push_back(std::make_unique<ast::StructMember>("a", &glsl_mat2x2,
                                                        std::move(empty_a)));
  members.push_back(std::make_unique<ast::StructMember>("b", &glsl_mat2x3,
                                                        std::move(empty_b)));
  members.push_back(std::make_unique<ast::StructMember>("c", &glsl_mat4x4,
                                                        std::move(empty_c)));

  auto s = std::make_unique<ast::Struct>(ast::StructDecoration::kNone,
                                         std::move(members));
  ast::type::StructType s_type(std::move(s));

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateTypeIfNeeded(&s_type);
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
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpMemberName %1 0 "a"
OpMemberName %1 1 "b"
OpMemberName %1 2 "c"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), "");
}

TEST_F(BuilderTest_Type, GenerateStruct_DecoratedMembers_LayoutMatrix) {
  // We have to infer layout for matrix when it also has an offset.
  ast::type::F32Type f32;
  ast::type::MatrixType glsl_mat2x2(&f32, 2, 2);
  ast::type::MatrixType glsl_mat2x3(&f32, 3, 2);  // 2 columns, 3 rows
  ast::type::MatrixType glsl_mat4x4(&f32, 4, 4);

  ast::StructMemberDecorationList a_decos;
  a_decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  ast::StructMemberDecorationList b_decos;
  b_decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(16));
  ast::StructMemberDecorationList c_decos;
  c_decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(48));

  ast::StructMemberList members;
  members.push_back(std::make_unique<ast::StructMember>("a", &glsl_mat2x2,
                                                        std::move(a_decos)));
  members.push_back(std::make_unique<ast::StructMember>("b", &glsl_mat2x3,
                                                        std::move(b_decos)));
  members.push_back(std::make_unique<ast::StructMember>("c", &glsl_mat4x4,
                                                        std::move(c_decos)));

  auto s = std::make_unique<ast::Struct>(ast::StructDecoration::kNone,
                                         std::move(members));
  ast::type::StructType s_type(std::move(s));

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateTypeIfNeeded(&s_type);
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
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpMemberName %1 0 "a"
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
  ast::type::F32Type f32;

  ast::type::MatrixType glsl_mat2x2(&f32, 2, 2);
  ast::type::ArrayType arr_mat2x2(&glsl_mat2x2, 1);  // Singly nested array

  ast::type::MatrixType glsl_mat2x3(&f32, 3, 2);  // 2 columns, 3 rows
  ast::type::ArrayType arr_mat2x3(&glsl_mat2x3, 1);
  ast::type::ArrayType arr_arr_mat2x2(&arr_mat2x3, 1);  // Doubly nested array

  ast::type::MatrixType glsl_mat4x4(&f32, 4, 4);
  ast::type::ArrayType rtarr_mat4x4(&glsl_mat4x4);  // Runtime array

  ast::StructMemberDecorationList a_decos;
  a_decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  ast::StructMemberDecorationList b_decos;
  b_decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(16));
  ast::StructMemberDecorationList c_decos;
  c_decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(48));

  ast::StructMemberList members;
  members.push_back(std::make_unique<ast::StructMember>("a", &glsl_mat2x2,
                                                        std::move(a_decos)));
  members.push_back(std::make_unique<ast::StructMember>("b", &glsl_mat2x3,
                                                        std::move(b_decos)));
  members.push_back(std::make_unique<ast::StructMember>("c", &glsl_mat4x4,
                                                        std::move(c_decos)));

  auto s = std::make_unique<ast::Struct>(ast::StructDecoration::kNone,
                                         std::move(members));
  ast::type::StructType s_type(std::move(s));

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateTypeIfNeeded(&s_type);
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
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpMemberName %1 0 "a"
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

TEST_F(BuilderTest_Type, GenerateU32) {
  ast::type::U32Type u32;

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateTypeIfNeeded(&u32);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  ASSERT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeInt 32 0
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedU32) {
  ast::type::U32Type u32;
  ast::type::F32Type f32;

  ast::Module mod;
  Builder b(&mod);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&u32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&f32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&u32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateVector) {
  ast::type::F32Type f32;
  ast::type::VectorType vec_type(&f32, 3);

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateTypeIfNeeded(&vec_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(b.types().size(), 2u);
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedVector) {
  ast::type::I32Type i32;
  ast::type::VectorType vec_type(&i32, 3);

  ast::Module mod;
  Builder b(&mod);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&vec_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&vec_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateVoid) {
  ast::type::VoidType void_type;

  ast::Module mod;
  Builder b(&mod);
  auto id = b.GenerateTypeIfNeeded(&void_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  ASSERT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeVoid
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedVoid) {
  ast::type::I32Type i32;
  ast::type::VoidType void_type;

  ast::Module mod;
  Builder b(&mod);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&void_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&void_type), 1u);
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
using PtrDataTest = testing::TestWithParam<PtrData>;
TEST_P(PtrDataTest, ConvertStorageClass) {
  auto params = GetParam();

  ast::Module mod;
  Builder b(&mod);
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
        PtrData{ast::StorageClass::kStorageBuffer,
                SpvStorageClassStorageBuffer},
        PtrData{ast::StorageClass::kImage, SpvStorageClassImage},
        PtrData{ast::StorageClass::kPrivate, SpvStorageClassPrivate},
        PtrData{ast::StorageClass::kFunction, SpvStorageClassFunction}));

using DepthTextureTypeTest = BuilderTest_Type;
TEST_F(DepthTextureTypeTest, Generate_2d) {
  ast::type::DepthTextureType two_d(ast::type::TextureDimension::k2d);

  ast::Module mod;
  Builder b(&mod);

  auto id_two_d = b.GenerateTypeIfNeeded(&two_d);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id_two_d);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 1 0 0 1 Unknown
)");
}

TEST_F(DepthTextureTypeTest, Generate_2dArray) {
  ast::type::DepthTextureType two_d_array(
      ast::type::TextureDimension::k2dArray);

  ast::Module mod;
  Builder b(&mod);

  auto id_two_d_array = b.GenerateTypeIfNeeded(&two_d_array);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id_two_d_array);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 1 1 0 1 Unknown
)");
}

TEST_F(DepthTextureTypeTest, Generate_Cube) {
  ast::type::DepthTextureType cube(ast::type::TextureDimension::kCube);

  ast::Module mod;
  Builder b(&mod);

  auto id_cube = b.GenerateTypeIfNeeded(&cube);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id_cube);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 Cube 1 0 0 1 Unknown
)");
}

TEST_F(DepthTextureTypeTest, Generate_CubeArray) {
  ast::type::DepthTextureType cube_array(
      ast::type::TextureDimension::kCubeArray);

  ast::Module mod;
  Builder b(&mod);

  auto id_cube_array = b.GenerateTypeIfNeeded(&cube_array);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id_cube_array);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 Cube 1 1 0 1 Unknown
)");
}

enum class TextureType { kF32, kI32, kU32 };
inline std::ostream& operator<<(std::ostream& out, TextureType data) {
  if (data == TextureType::kF32) {
    out << "f32";
  } else if (data == TextureType::kI32) {
    out << "i32";
  } else {
    out << "u32";
  }
  return out;
}

class SampledTextureTypeTest : public testing::TestWithParam<TextureType> {
 public:
  std::unique_ptr<ast::type::Type> get_type(TextureType param) {
    if (param == TextureType::kF32) {
      return std::make_unique<ast::type::F32Type>();
    }
    if (param == TextureType::kI32) {
      return std::make_unique<ast::type::I32Type>();
    }
    return std::make_unique<ast::type::U32Type>();
  }

  std::string get_type_line(TextureType param) {
    if (param == TextureType::kI32) {
      return "%2 = OpTypeInt 32 1\n";
    }
    if (param == TextureType::kU32) {
      return "%2 = OpTypeInt 32 0\n";
    }
    return "%2 = OpTypeFloat 32\n";
  }
};

TEST_P(SampledTextureTypeTest, Generate_1d) {
  auto param = GetParam();
  auto type = get_type(param);

  ast::type::SampledTextureType one_d(ast::type::TextureDimension::k1d,
                                      type.get());

  ast::Module mod;
  Builder b(&mod);

  auto id_one_d = b.GenerateTypeIfNeeded(&one_d);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id_one_d);

  EXPECT_EQ(DumpInstructions(b.types()),
            get_type_line(param) + "%1 = OpTypeImage %2 1D 0 0 0 1 Unknown\n");
}

TEST_P(SampledTextureTypeTest, Generate_1dArray) {
  auto param = GetParam();
  auto type = get_type(param);

  ast::type::SampledTextureType one_d_array(
      ast::type::TextureDimension::k1dArray, type.get());

  ast::Module mod;
  Builder b(&mod);

  auto id_one_d_array = b.GenerateTypeIfNeeded(&one_d_array);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id_one_d_array);

  EXPECT_EQ(DumpInstructions(b.types()),
            get_type_line(param) + "%1 = OpTypeImage %2 1D 0 1 0 1 Unknown\n");
}

TEST_P(SampledTextureTypeTest, Generate_2d) {
  auto param = GetParam();
  auto type = get_type(param);

  ast::type::SampledTextureType two_d(ast::type::TextureDimension::k2d,
                                      type.get());

  ast::Module mod;
  Builder b(&mod);

  auto id_two_d = b.GenerateTypeIfNeeded(&two_d);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id_two_d);

  EXPECT_EQ(DumpInstructions(b.types()),
            get_type_line(param) + "%1 = OpTypeImage %2 2D 0 0 0 1 Unknown\n");
}

TEST_P(SampledTextureTypeTest, Generate_2d_array) {
  auto param = GetParam();
  auto type = get_type(param);

  ast::type::SampledTextureType two_d_array(
      ast::type::TextureDimension::k2dArray, type.get());

  ast::Module mod;
  Builder b(&mod);

  auto id_two_d_array = b.GenerateTypeIfNeeded(&two_d_array);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id_two_d_array);

  EXPECT_EQ(DumpInstructions(b.types()),
            get_type_line(param) + "%1 = OpTypeImage %2 2D 0 1 0 1 Unknown\n");
}

TEST_P(SampledTextureTypeTest, Generate_3d) {
  auto param = GetParam();
  auto type = get_type(param);

  ast::type::SampledTextureType three_d(ast::type::TextureDimension::k3d,
                                        type.get());

  ast::Module mod;
  Builder b(&mod);

  auto id_three_d = b.GenerateTypeIfNeeded(&three_d);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id_three_d);

  EXPECT_EQ(DumpInstructions(b.types()),
            get_type_line(param) + "%1 = OpTypeImage %2 3D 0 0 0 1 Unknown\n");
}

TEST_P(SampledTextureTypeTest, Generate_Cube) {
  auto param = GetParam();
  auto type = get_type(param);

  ast::type::SampledTextureType cube(ast::type::TextureDimension::kCube,
                                     type.get());

  ast::Module mod;
  Builder b(&mod);

  auto id_cube = b.GenerateTypeIfNeeded(&cube);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id_cube);

  EXPECT_EQ(
      DumpInstructions(b.types()),
      get_type_line(param) + "%1 = OpTypeImage %2 Cube 0 0 0 1 Unknown\n");
}

TEST_P(SampledTextureTypeTest, Generate_CubeArray) {
  auto param = GetParam();
  auto type = get_type(param);

  ast::type::SampledTextureType cube_array(
      ast::type::TextureDimension::kCubeArray, type.get());

  ast::Module mod;
  Builder b(&mod);

  auto id_cube_array = b.GenerateTypeIfNeeded(&cube_array);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id_cube_array);

  EXPECT_EQ(
      DumpInstructions(b.types()),
      get_type_line(param) + "%1 = OpTypeImage %2 Cube 0 1 0 1 Unknown\n");
}

INSTANTIATE_TEST_SUITE_P(BuilderTest_Type,
                         SampledTextureTypeTest,
                         testing::Values(TextureType::kU32,
                                         TextureType::kI32,
                                         TextureType::kF32));

class StorageTextureTypeTest
    : public testing::TestWithParam<ast::type::ImageFormat> {
 public:
  std::string format_literal(ast::type::ImageFormat format) {
    if (format == ast::type::ImageFormat::kR16Float) {
      return "R16f\n";
    }
    if (format == ast::type::ImageFormat::kR8Snorm) {
      return "R8Snorm\n";
    }
    return "R8\n";
  }

  std::string type_line(ast::type::ImageFormat format) {
    if (format == ast::type::ImageFormat::kR8Snorm) {
      return "%2 = OpTypeInt 32 1\n";
    }
    if (format == ast::type::ImageFormat::kR8Unorm) {
      return "%2 = OpTypeInt 32 0\n";
    }
    return "%2 = OpTypeFloat 32\n";
  }
};

TEST_P(StorageTextureTypeTest, GenerateReadonly_1d) {
  auto format = GetParam();

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  Builder b(&mod);

  ast::type::Type* type =
      ctx.type_mgr().Get(std::make_unique<ast::type::StorageTextureType>(
          ast::type::TextureDimension::k1d, ast::type::StorageAccess::kRead,
          format));

  ASSERT_TRUE(td.Determine()) << td.error();

  auto id = b.GenerateTypeIfNeeded(type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.types()), type_line(format) +
                                             "%1 = OpTypeImage %2 1D 0 0 0 2 " +
                                             format_literal(format));
}

TEST_P(StorageTextureTypeTest, GenerateReadonly_1d_array) {
  auto format = GetParam();

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  Builder b(&mod);

  ast::type::Type* type =
      ctx.type_mgr().Get(std::make_unique<ast::type::StorageTextureType>(
          ast::type::TextureDimension::k1dArray,
          ast::type::StorageAccess::kRead, format));

  ASSERT_TRUE(td.Determine()) << td.error();

  auto id = b.GenerateTypeIfNeeded(type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.types()), type_line(format) +
                                             "%1 = OpTypeImage %2 1D 0 1 0 2 " +
                                             format_literal(format));
}

TEST_P(StorageTextureTypeTest, GenerateReadonly_2d) {
  auto format = GetParam();

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  Builder b(&mod);

  ast::type::Type* type =
      ctx.type_mgr().Get(std::make_unique<ast::type::StorageTextureType>(
          ast::type::TextureDimension::k2d, ast::type::StorageAccess::kRead,
          format));

  ASSERT_TRUE(td.Determine()) << td.error();

  auto id = b.GenerateTypeIfNeeded(type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.types()), type_line(format) +
                                             "%1 = OpTypeImage %2 2D 0 0 0 2 " +
                                             format_literal(format));
}

TEST_P(StorageTextureTypeTest, GenerateReadonly_2dArray) {
  auto format = GetParam();

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  Builder b(&mod);

  ast::type::Type* type =
      ctx.type_mgr().Get(std::make_unique<ast::type::StorageTextureType>(
          ast::type::TextureDimension::k2dArray,
          ast::type::StorageAccess::kRead, format));

  ASSERT_TRUE(td.Determine()) << td.error();

  auto id = b.GenerateTypeIfNeeded(type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.types()), type_line(format) +
                                             "%1 = OpTypeImage %2 2D 0 1 0 2 " +
                                             format_literal(format));
}

TEST_P(StorageTextureTypeTest, GenerateReadonly_3d) {
  auto format = GetParam();

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  Builder b(&mod);

  ast::type::Type* type =
      ctx.type_mgr().Get(std::make_unique<ast::type::StorageTextureType>(
          ast::type::TextureDimension::k3d, ast::type::StorageAccess::kRead,
          format));

  ASSERT_TRUE(td.Determine()) << td.error();

  auto id = b.GenerateTypeIfNeeded(type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.types()), type_line(format) +
                                             "%1 = OpTypeImage %2 3D 0 0 0 2 " +
                                             format_literal(format));
}

TEST_P(StorageTextureTypeTest, GenerateWriteonly_1d) {
  auto format = GetParam();

  ast::type::StorageTextureType type(ast::type::TextureDimension::k1d,
                                     ast::type::StorageAccess::kWrite, format);

  ast::Module mod;
  Builder b(&mod);

  auto id = b.GenerateTypeIfNeeded(&type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeImage %2 1D 0 0 0 2 )" + format_literal(format));
}

TEST_P(StorageTextureTypeTest, GenerateWriteonly_1dArray) {
  auto format = GetParam();

  ast::type::StorageTextureType type(ast::type::TextureDimension::k1dArray,
                                     ast::type::StorageAccess::kWrite, format);

  ast::Module mod;
  Builder b(&mod);

  auto id = b.GenerateTypeIfNeeded(&type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeImage %2 1D 0 1 0 2 )" + format_literal(format));
}

TEST_P(StorageTextureTypeTest, GenerateWriteonly_2d) {
  auto format = GetParam();

  ast::type::StorageTextureType type(ast::type::TextureDimension::k2d,
                                     ast::type::StorageAccess::kWrite, format);

  ast::Module mod;
  Builder b(&mod);

  auto id = b.GenerateTypeIfNeeded(&type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeImage %2 2D 0 0 0 2 )" + format_literal(format));
}

TEST_P(StorageTextureTypeTest, GenerateWriteonly_2dArray) {
  auto format = GetParam();

  ast::type::StorageTextureType type(ast::type::TextureDimension::k2dArray,
                                     ast::type::StorageAccess::kWrite, format);

  ast::Module mod;
  Builder b(&mod);

  auto id = b.GenerateTypeIfNeeded(&type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeImage %2 2D 0 1 0 2 )" + format_literal(format));
}

TEST_P(StorageTextureTypeTest, GenerateWriteonly_3d) {
  auto format = GetParam();

  ast::type::StorageTextureType type(ast::type::TextureDimension::k3d,
                                     ast::type::StorageAccess::kWrite, format);

  ast::Module mod;
  Builder b(&mod);

  auto id = b.GenerateTypeIfNeeded(&type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeImage %2 3D 0 0 0 2 )" + format_literal(format));
}

INSTANTIATE_TEST_SUITE_P(BuilderTest_Type,
                         StorageTextureTypeTest,
                         testing::Values(ast::type::ImageFormat::kR16Float,
                                         ast::type::ImageFormat::kR8Snorm,
                                         ast::type::ImageFormat::kR8Unorm));

TEST_F(BuilderTest_Type, Sampler) {
  ast::type::SamplerType sampler(ast::type::SamplerKind::kSampler);

  ast::Module mod;
  Builder b(&mod);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&sampler), 1u);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&sampler), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), "%1 = OpTypeSampler\n");
}

TEST_F(BuilderTest_Type, ComparisonSampler) {
  ast::type::SamplerType sampler(ast::type::SamplerKind::kComparisonSampler);

  ast::Module mod;
  Builder b(&mod);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&sampler), 1u);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&sampler), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), "%1 = OpTypeSampler\n");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
