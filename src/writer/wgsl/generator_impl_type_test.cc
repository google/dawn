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
#include "src/writer/wgsl/generator_impl.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = testing::Test;

TEST_F(WgslGeneratorImplTest, EmitType_Alias) {
  ast::type::F32Type f32;
  ast::type::AliasType alias("alias", &f32);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&alias)) << g.error();
  EXPECT_EQ(g.result(), "alias");
}

TEST_F(WgslGeneratorImplTest, EmitType_Array) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b, 4);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&a)) << g.error();
  EXPECT_EQ(g.result(), "array<bool, 4>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Array_WithStride) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b, 4);
  a.set_array_stride(16);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&a)) << g.error();
  EXPECT_EQ(g.result(), "[[stride 16]] array<bool, 4>");
}

TEST_F(WgslGeneratorImplTest, EmitType_RuntimeArray) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&a)) << g.error();
  EXPECT_EQ(g.result(), "array<bool>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Bool) {
  ast::type::BoolType b;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&b)) << g.error();
  EXPECT_EQ(g.result(), "bool");
}

TEST_F(WgslGeneratorImplTest, EmitType_F32) {
  ast::type::F32Type f32;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&f32)) << g.error();
  EXPECT_EQ(g.result(), "f32");
}

TEST_F(WgslGeneratorImplTest, EmitType_I32) {
  ast::type::I32Type i32;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&i32)) << g.error();
  EXPECT_EQ(g.result(), "i32");
}

TEST_F(WgslGeneratorImplTest, EmitType_Matrix) {
  ast::type::F32Type f32;
  ast::type::MatrixType m(&f32, 3, 2);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&m)) << g.error();
  EXPECT_EQ(g.result(), "mat2x3<f32>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Pointer) {
  ast::type::F32Type f32;
  ast::type::PointerType p(&f32, ast::StorageClass::kWorkgroup);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&p)) << g.error();
  EXPECT_EQ(g.result(), "ptr<workgroup, f32>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Struct) {
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

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&s)) << g.error();
  EXPECT_EQ(g.result(), R"(struct {
  a : i32;
  [[offset 4]] b : f32;
})");
}

TEST_F(WgslGeneratorImplTest, EmitType_Struct_WithDecoration) {
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

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&s)) << g.error();
  EXPECT_EQ(g.result(), R"([[block]] struct {
  a : i32;
  [[offset 4]] b : f32;
})");
}

TEST_F(WgslGeneratorImplTest, EmitType_U32) {
  ast::type::U32Type u32;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&u32)) << g.error();
  EXPECT_EQ(g.result(), "u32");
}

TEST_F(WgslGeneratorImplTest, EmitType_Vector) {
  ast::type::F32Type f32;
  ast::type::VectorType v(&f32, 3);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&v)) << g.error();
  EXPECT_EQ(g.result(), "vec3<f32>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Void) {
  ast::type::VoidType v;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&v)) << g.error();
  EXPECT_EQ(g.result(), "void");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
