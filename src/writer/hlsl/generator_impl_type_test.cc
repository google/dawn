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

#include "src/ast/module.h"
#include "src/ast/struct.h"
#include "src/ast/struct_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
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
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Type = TestHelper;

TEST_F(HlslGeneratorImplTest_Type, EmitType_Alias) {
  ast::type::F32Type f32;
  ast::type::AliasType alias("alias", &f32);

  ASSERT_TRUE(gen().EmitType(out(), &alias, "")) << gen().error();
  EXPECT_EQ(result(), "alias");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Alias_NameCollision) {
  ast::type::F32Type f32;
  ast::type::AliasType alias("bool", &f32);

  ASSERT_TRUE(gen().EmitType(out(), &alias, "")) << gen().error();
  EXPECT_EQ(result(), "bool_tint_0");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Array) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b, 4);

  ASSERT_TRUE(gen().EmitType(out(), &a, "ary")) << gen().error();
  EXPECT_EQ(result(), "bool ary[4]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_ArrayOfArray) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b, 4);
  ast::type::ArrayType c(&a, 5);

  ASSERT_TRUE(gen().EmitType(out(), &c, "ary")) << gen().error();
  EXPECT_EQ(result(), "bool ary[5][4]");
}

// TODO(dsinclair): Is this possible? What order should it output in?
TEST_F(HlslGeneratorImplTest_Type,
       DISABLED_EmitType_ArrayOfArrayOfRuntimeArray) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b, 4);
  ast::type::ArrayType c(&a, 5);
  ast::type::ArrayType d(&c);

  ASSERT_TRUE(gen().EmitType(out(), &c, "ary")) << gen().error();
  EXPECT_EQ(result(), "bool ary[5][4][1]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_ArrayOfArrayOfArray) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b, 4);
  ast::type::ArrayType c(&a, 5);
  ast::type::ArrayType d(&c, 6);

  ASSERT_TRUE(gen().EmitType(out(), &d, "ary")) << gen().error();
  EXPECT_EQ(result(), "bool ary[6][5][4]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Array_NameCollision) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b, 4);

  ASSERT_TRUE(gen().EmitType(out(), &a, "bool")) << gen().error();
  EXPECT_EQ(result(), "bool bool_tint_0[4]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Array_WithoutName) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b, 4);

  ASSERT_TRUE(gen().EmitType(out(), &a, "")) << gen().error();
  EXPECT_EQ(result(), "bool[4]");
}

TEST_F(HlslGeneratorImplTest_Type, DISABLED_EmitType_RuntimeArray) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b);

  ASSERT_TRUE(gen().EmitType(out(), &a, "ary")) << gen().error();
  EXPECT_EQ(result(), "bool ary[]");
}

TEST_F(HlslGeneratorImplTest_Type,
       DISABLED_EmitType_RuntimeArray_NameCollision) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b);

  ASSERT_TRUE(gen().EmitType(out(), &a, "double")) << gen().error();
  EXPECT_EQ(result(), "bool double_tint_0[]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Bool) {
  ast::type::BoolType b;

  ASSERT_TRUE(gen().EmitType(out(), &b, "")) << gen().error();
  EXPECT_EQ(result(), "bool");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_F32) {
  ast::type::F32Type f32;

  ASSERT_TRUE(gen().EmitType(out(), &f32, "")) << gen().error();
  EXPECT_EQ(result(), "float");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_I32) {
  ast::type::I32Type i32;

  ASSERT_TRUE(gen().EmitType(out(), &i32, "")) << gen().error();
  EXPECT_EQ(result(), "int");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Matrix) {
  ast::type::F32Type f32;
  ast::type::MatrixType m(&f32, 3, 2);

  ASSERT_TRUE(gen().EmitType(out(), &m, "")) << gen().error();
  EXPECT_EQ(result(), "matrix<float, 3, 2>");
}

// TODO(dsinclair): How to annotate as workgroup?
TEST_F(HlslGeneratorImplTest_Type, DISABLED_EmitType_Pointer) {
  ast::type::F32Type f32;
  ast::type::PointerType p(&f32, ast::StorageClass::kWorkgroup);

  ASSERT_TRUE(gen().EmitType(out(), &p, "")) << gen().error();
  EXPECT_EQ(result(), "float*");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Struct) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::StructMemberList members;
  members.push_back(std::make_unique<ast::StructMember>(
      "a", &i32, ast::StructMemberDecorationList{}));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));

  ASSERT_TRUE(gen().EmitType(out(), &s, "")) << gen().error();
  EXPECT_EQ(result(), R"(struct {
  int a;
  float b;
})");
}

TEST_F(HlslGeneratorImplTest_Type, DISABLED_EmitType_Struct_InjectPadding) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::StructMemberDecorationList decos;
  decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));

  ast::StructMemberList members;
  members.push_back(
      std::make_unique<ast::StructMember>("a", &i32, std::move(decos)));

  decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(32));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(decos)));

  decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(128));
  members.push_back(
      std::make_unique<ast::StructMember>("c", &f32, std::move(decos)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));

  ASSERT_TRUE(gen().EmitType(out(), &s, "")) << gen().error();
  EXPECT_EQ(result(), R"(struct {
  int8_t pad_0[4];
  int a;
  int8_t pad_1[24];
  float b;
  int8_t pad_2[92];
  float c;
})");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Struct_NameCollision) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::StructMemberList members;
  members.push_back(std::make_unique<ast::StructMember>(
      "double", &i32, ast::StructMemberDecorationList{}));

  ast::StructMemberDecorationList b_deco;
  members.push_back(
      std::make_unique<ast::StructMember>("float", &f32, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));

  ASSERT_TRUE(gen().EmitType(out(), &s, "")) << gen().error();
  EXPECT_EQ(result(), R"(struct {
  int double_tint_0;
  float float_tint_0;
})");
}

// TODO(dsinclair): How to translate [[block]]
TEST_F(HlslGeneratorImplTest_Type, DISABLED_EmitType_Struct_WithDecoration) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::StructMemberList members;
  members.push_back(std::make_unique<ast::StructMember>(
      "a", &i32, ast::StructMemberDecorationList{}));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));
  str->set_decoration(ast::StructDecoration::kBlock);

  ast::type::StructType s(std::move(str));

  ASSERT_TRUE(gen().EmitType(out(), &s, "")) << gen().error();
  EXPECT_EQ(result(), R"(struct {
  int a;
  float b;
})");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_U32) {
  ast::type::U32Type u32;

  ASSERT_TRUE(gen().EmitType(out(), &u32, "")) << gen().error();
  EXPECT_EQ(result(), "uint");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Vector) {
  ast::type::F32Type f32;
  ast::type::VectorType v(&f32, 3);

  ASSERT_TRUE(gen().EmitType(out(), &v, "")) << gen().error();
  EXPECT_EQ(result(), "vector<float, 3>");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Void) {
  ast::type::VoidType v;

  ASSERT_TRUE(gen().EmitType(out(), &v, "")) << gen().error();
  EXPECT_EQ(result(), "void");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
