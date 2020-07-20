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

#include "gtest/gtest.h"
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
#include "src/writer/msl/generator_impl.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = testing::Test;

TEST_F(MslGeneratorImplTest, EmitType_Alias) {
  ast::type::F32Type f32;
  ast::type::AliasType alias("alias", &f32);

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&alias, "")) << g.error();
  EXPECT_EQ(g.result(), "alias");
}

TEST_F(MslGeneratorImplTest, EmitType_Alias_NameCollision) {
  ast::type::F32Type f32;
  ast::type::AliasType alias("bool", &f32);

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&alias, "")) << g.error();
  EXPECT_EQ(g.result(), "bool_tint_0");
}

TEST_F(MslGeneratorImplTest, EmitType_Array) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b, 4);

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&a, "ary")) << g.error();
  EXPECT_EQ(g.result(), "bool ary[4]");
}

TEST_F(MslGeneratorImplTest, EmitType_ArrayOfArray) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b, 4);
  ast::type::ArrayType c(&a, 5);

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&c, "ary")) << g.error();
  EXPECT_EQ(g.result(), "bool ary[5][4]");
}

// TODO(dsinclair): Is this possible? What order should it output in?
TEST_F(MslGeneratorImplTest, DISABLED_EmitType_ArrayOfArrayOfRuntimeArray) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b, 4);
  ast::type::ArrayType c(&a, 5);
  ast::type::ArrayType d(&c);

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&c, "ary")) << g.error();
  EXPECT_EQ(g.result(), "bool ary[5][4][1]");
}

TEST_F(MslGeneratorImplTest, EmitType_ArrayOfArrayOfArray) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b, 4);
  ast::type::ArrayType c(&a, 5);
  ast::type::ArrayType d(&c, 6);

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&d, "ary")) << g.error();
  EXPECT_EQ(g.result(), "bool ary[6][5][4]");
}

TEST_F(MslGeneratorImplTest, EmitType_Array_NameCollision) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b, 4);

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&a, "bool")) << g.error();
  EXPECT_EQ(g.result(), "bool bool_tint_0[4]");
}

TEST_F(MslGeneratorImplTest, EmitType_Array_WithoutName) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b, 4);

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&a, "")) << g.error();
  EXPECT_EQ(g.result(), "bool[4]");
}

TEST_F(MslGeneratorImplTest, EmitType_RuntimeArray) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b);

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&a, "ary")) << g.error();
  EXPECT_EQ(g.result(), "bool ary[1]");
}

TEST_F(MslGeneratorImplTest, EmitType_RuntimeArray_NameCollision) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b);

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&a, "discard_fragment")) << g.error();
  EXPECT_EQ(g.result(), "bool discard_fragment_tint_0[1]");
}

TEST_F(MslGeneratorImplTest, EmitType_Bool) {
  ast::type::BoolType b;

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&b, "")) << g.error();
  EXPECT_EQ(g.result(), "bool");
}

TEST_F(MslGeneratorImplTest, EmitType_F32) {
  ast::type::F32Type f32;

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&f32, "")) << g.error();
  EXPECT_EQ(g.result(), "float");
}

TEST_F(MslGeneratorImplTest, EmitType_I32) {
  ast::type::I32Type i32;

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&i32, "")) << g.error();
  EXPECT_EQ(g.result(), "int");
}

TEST_F(MslGeneratorImplTest, EmitType_Matrix) {
  ast::type::F32Type f32;
  ast::type::MatrixType m(&f32, 3, 2);

  ast::Module mod;
  GeneratorImpl g(&mod);
  ASSERT_TRUE(g.EmitType(&m, "")) << g.error();
  EXPECT_EQ(g.result(), "float2x3");
}

// TODO(dsinclair): How to annotate as workgroup?
TEST_F(MslGeneratorImplTest, DISABLED_EmitType_Pointer) {
  ast::type::F32Type f32;
  ast::type::PointerType p(&f32, ast::StorageClass::kWorkgroup);

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&p, "")) << g.error();
  EXPECT_EQ(g.result(), "float*");
}

TEST_F(MslGeneratorImplTest, EmitType_Struct) {
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

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&s, "")) << g.error();
  EXPECT_EQ(g.result(), R"(struct {
  int a;
  float b;
})");
}

TEST_F(MslGeneratorImplTest, EmitType_Struct_InjectPadding) {
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

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&s, "")) << g.error();
  EXPECT_EQ(g.result(), R"(struct {
  int8_t pad_0[4];
  int a;
  int8_t pad_1[24];
  float b;
  int8_t pad_2[92];
  float c;
})");
}

TEST_F(MslGeneratorImplTest, EmitType_Struct_NameCollision) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::StructMemberList members;
  members.push_back(std::make_unique<ast::StructMember>(
      "main", &i32, ast::StructMemberDecorationList{}));

  ast::StructMemberDecorationList b_deco;
  members.push_back(
      std::make_unique<ast::StructMember>("float", &f32, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&s, "")) << g.error();
  EXPECT_EQ(g.result(), R"(struct {
  int main_tint_0;
  float float_tint_0;
})");
}

// TODO(dsinclair): How to translate [[block]]
TEST_F(MslGeneratorImplTest, DISABLED_EmitType_Struct_WithDecoration) {
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

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&s, "")) << g.error();
  EXPECT_EQ(g.result(), R"(struct {
  int a;
  float b;
})");
}

TEST_F(MslGeneratorImplTest, EmitType_U32) {
  ast::type::U32Type u32;

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&u32, "")) << g.error();
  EXPECT_EQ(g.result(), "uint");
}

TEST_F(MslGeneratorImplTest, EmitType_Vector) {
  ast::type::F32Type f32;
  ast::type::VectorType v(&f32, 3);

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&v, "")) << g.error();
  EXPECT_EQ(g.result(), "float3");
}

TEST_F(MslGeneratorImplTest, EmitType_Void) {
  ast::type::VoidType v;

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitType(&v, "")) << g.error();
  EXPECT_EQ(g.result(), "void");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
