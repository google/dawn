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
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {

using BuilderTest_Type = testing::Test;

TEST_F(BuilderTest_Type, GenerateAlias) {
  ast::type::F32Type f32;
  ast::type::AliasType alias_type("my_type", &f32);

  Builder b;
  auto id = b.GenerateTypeIfNeeded(&alias_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1);

  EXPECT_EQ(b.types().size(), 1);
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedAlias) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  ast::type::AliasType alias_type("my_type", &f32);

  Builder b;
  EXPECT_EQ(b.GenerateTypeIfNeeded(&alias_type), 1);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 2);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&alias_type), 1);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&f32), 1);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateBool) {
  ast::type::BoolType bool_type;

  Builder b;
  auto id = b.GenerateTypeIfNeeded(&bool_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1);

  ASSERT_EQ(b.types().size(), 1);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeBool
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedBool) {
  ast::type::I32Type i32;
  ast::type::BoolType bool_type;

  Builder b;
  EXPECT_EQ(b.GenerateTypeIfNeeded(&bool_type), 1);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 2);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&bool_type), 1);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateF32) {
  ast::type::F32Type f32;

  Builder b;
  auto id = b.GenerateTypeIfNeeded(&f32);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1);

  ASSERT_EQ(b.types().size(), 1);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeFloat 32
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedF32) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  Builder b;
  EXPECT_EQ(b.GenerateTypeIfNeeded(&f32), 1);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 2);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&f32), 1);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateI32) {
  ast::type::I32Type i32;

  Builder b;
  auto id = b.GenerateTypeIfNeeded(&i32);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1);

  ASSERT_EQ(b.types().size(), 1);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeInt 32 1
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedI32) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  Builder b;
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 1);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&f32), 2);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 1);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateMatrix) {
  ast::type::F32Type f32;
  ast::type::MatrixType mat_type(&f32, 3, 2);

  Builder b;
  auto id = b.GenerateTypeIfNeeded(&mat_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1);

  EXPECT_EQ(b.types().size(), 3);
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 2
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedMatrix) {
  ast::type::I32Type i32;
  ast::type::MatrixType mat_type(&i32, 3, 4);

  Builder b;
  EXPECT_EQ(b.GenerateTypeIfNeeded(&mat_type), 1);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 3);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&mat_type), 1);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateStruct_Empty) {
  auto s = std::make_unique<ast::Struct>();
  ast::type::StructType s_type(std::move(s));

  Builder b;
  auto id = b.GenerateTypeIfNeeded(&s_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1);

  EXPECT_EQ(b.types().size(), 1);
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeStruct
)");
}

TEST_F(BuilderTest_Type, GenerateStruct) {
  ast::type::F32Type f32;

  std::vector<std::unique_ptr<ast::StructMemberDecoration>> decos;
  std::vector<std::unique_ptr<ast::StructMember>> members;
  members.push_back(
      std::make_unique<ast::StructMember>("a", &f32, std::move(decos)));

  auto s = std::make_unique<ast::Struct>(ast::StructDecoration::kNone,
                                         std::move(members));
  ast::type::StructType s_type(std::move(s));

  Builder b;
  auto id = b.GenerateTypeIfNeeded(&s_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeStruct %2
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpMemberName %1 0 "a"
)");
}

TEST_F(BuilderTest_Type, GenerateStruct_Decorated) {
  ast::type::F32Type f32;

  std::vector<std::unique_ptr<ast::StructMemberDecoration>> decos;
  std::vector<std::unique_ptr<ast::StructMember>> members;
  members.push_back(
      std::make_unique<ast::StructMember>("a", &f32, std::move(decos)));

  auto s = std::make_unique<ast::Struct>(ast::StructDecoration::kBlock,
                                         std::move(members));
  ast::type::StructType s_type(std::move(s));

  Builder b;
  auto id = b.GenerateTypeIfNeeded(&s_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeStruct %2
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpMemberName %1 0 "a"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 Block
)");
}

TEST_F(BuilderTest_Type, GenerateStruct_DecoratedMembers) {
  ast::type::F32Type f32;

  std::vector<std::unique_ptr<ast::StructMemberDecoration>> a_decos;
  a_decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  std::vector<std::unique_ptr<ast::StructMemberDecoration>> b_decos;
  b_decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(8));

  std::vector<std::unique_ptr<ast::StructMember>> members;
  members.push_back(
      std::make_unique<ast::StructMember>("a", &f32, std::move(a_decos)));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(b_decos)));

  auto s = std::make_unique<ast::Struct>(ast::StructDecoration::kNone,
                                         std::move(members));
  ast::type::StructType s_type(std::move(s));

  Builder b;
  auto id = b.GenerateTypeIfNeeded(&s_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1);

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

TEST_F(BuilderTest_Type, GenerateU32) {
  ast::type::U32Type u32;

  Builder b;
  auto id = b.GenerateTypeIfNeeded(&u32);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1);

  ASSERT_EQ(b.types().size(), 1);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeInt 32 0
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedU32) {
  ast::type::U32Type u32;
  ast::type::F32Type f32;

  Builder b;
  EXPECT_EQ(b.GenerateTypeIfNeeded(&u32), 1);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&f32), 2);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&u32), 1);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateVector) {
  ast::type::F32Type f32;
  ast::type::VectorType vec_type(&f32, 3);

  Builder b;
  auto id = b.GenerateTypeIfNeeded(&vec_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1);

  EXPECT_EQ(b.types().size(), 2);
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedVector) {
  ast::type::I32Type i32;
  ast::type::VectorType vec_type(&i32, 3);

  Builder b;
  EXPECT_EQ(b.GenerateTypeIfNeeded(&vec_type), 1);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 2);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&vec_type), 1);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateVoid) {
  ast::type::VoidType void_type;

  Builder b;
  auto id = b.GenerateTypeIfNeeded(&void_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1);

  ASSERT_EQ(b.types().size(), 1);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeVoid
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedVoid) {
  ast::type::I32Type i32;
  ast::type::VoidType void_type;

  Builder b;
  EXPECT_EQ(b.GenerateTypeIfNeeded(&void_type), 1);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 2);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&void_type), 1);
  ASSERT_FALSE(b.has_error()) << b.error();
}

}  // namespace spirv
}  // namespace writer
}  // namespace tint
