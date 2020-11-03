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
#include <vector>

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/struct.h"
#include "src/ast/struct_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/writer/msl/generator_impl.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = testing::Test;

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement) {
  ast::type::F32Type f32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32);

  ast::VariableDeclStatement stmt(std::move(var));

  ast::Module m;
  GeneratorImpl g(&m);
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&stmt)) << g.error();
  EXPECT_EQ(g.result(), "  float a = 0.0f;\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Const) {
  ast::type::F32Type f32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32);
  var->set_is_const(true);

  ast::VariableDeclStatement stmt(std::move(var));

  ast::Module m;
  GeneratorImpl g(&m);
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&stmt)) << g.error();
  EXPECT_EQ(g.result(), "  const float a = 0.0f;\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Array) {
  ast::type::F32Type f32;
  ast::type::ArrayType ary(&f32, 5);

  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &ary);

  ast::VariableDeclStatement stmt(std::move(var));

  ast::Module m;
  GeneratorImpl g(&m);
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&stmt)) << g.error();
  EXPECT_EQ(g.result(), "  float a[5] = {0.0f};\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Struct) {
  ast::type::F32Type f32;

  ast::StructMemberList members;
  members.push_back(std::make_unique<ast::StructMember>(
      "a", &f32, ast::StructMemberDecorationList{}));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(
      std::make_unique<ast::StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s("S", std::move(str));

  auto var = std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &s);

  ast::VariableDeclStatement stmt(std::move(var));

  ast::Module m;
  GeneratorImpl g(&m);
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&stmt)) << g.error();
  EXPECT_EQ(g.result(), R"(  S a = {};
)");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Vector) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 2);

  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kFunction, &vec);

  ast::VariableDeclStatement stmt(std::move(var));

  ast::Module m;
  GeneratorImpl g(&m);
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&stmt)) << g.error();
  EXPECT_EQ(g.result(), "  float2 a = 0.0f;\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Matrix) {
  ast::type::F32Type f32;
  ast::type::MatrixType mat(&f32, 2, 3);
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kFunction, &mat);

  ast::VariableDeclStatement stmt(std::move(var));

  ast::Module m;
  GeneratorImpl g(&m);
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&stmt)) << g.error();
  EXPECT_EQ(g.result(), "  float3x2 a = 0.0f;\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Private) {
  ast::type::F32Type f32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kPrivate, &f32);

  ast::VariableDeclStatement stmt(std::move(var));

  ast::Module m;
  GeneratorImpl g(&m);
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&stmt)) << g.error();
  EXPECT_EQ(g.result(), "  float a = 0.0f;\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Initializer_Private) {
  auto ident = std::make_unique<ast::IdentifierExpression>("initializer");

  ast::type::F32Type f32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32);
  var->set_constructor(std::move(ident));

  ast::VariableDeclStatement stmt(std::move(var));

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitStatement(&stmt)) << g.error();
  EXPECT_EQ(g.result(), R"(float a = initializer;
)");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Initializer_ZeroVec) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList values;
  auto zero_vec =
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(values));

  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &vec);
  var->set_constructor(std::move(zero_vec));

  ast::VariableDeclStatement stmt(std::move(var));

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitStatement(&stmt)) << g.error();
  EXPECT_EQ(g.result(), R"(float3 a = float3(0.0f);
)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
