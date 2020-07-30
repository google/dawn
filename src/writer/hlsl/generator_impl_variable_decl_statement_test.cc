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
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/writer/hlsl/generator_impl.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest = testing::Test;

TEST_F(HlslGeneratorImplTest, Emit_VariableDeclStatement) {
  ast::type::F32Type f32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32);

  ast::VariableDeclStatement stmt(std::move(var));

  ast::Module m;
  GeneratorImpl g(&m);
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&stmt)) << g.error();
  EXPECT_EQ(g.result(), "  float a;\n");
}

TEST_F(HlslGeneratorImplTest, Emit_VariableDeclStatement_Const) {
  ast::type::F32Type f32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32);
  var->set_is_const(true);

  ast::VariableDeclStatement stmt(std::move(var));

  ast::Module m;
  GeneratorImpl g(&m);
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&stmt)) << g.error();
  EXPECT_EQ(g.result(), "  const float a;\n");
}

TEST_F(HlslGeneratorImplTest, Emit_VariableDeclStatement_Array) {
  ast::type::F32Type f32;
  ast::type::ArrayType ary(&f32, 5);

  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &ary);

  ast::VariableDeclStatement stmt(std::move(var));

  ast::Module m;
  GeneratorImpl g(&m);
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&stmt)) << g.error();
  EXPECT_EQ(g.result(), "  float a[5];\n");
}

TEST_F(HlslGeneratorImplTest, Emit_VariableDeclStatement_Function) {
  ast::type::F32Type f32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kFunction, &f32);

  ast::VariableDeclStatement stmt(std::move(var));

  ast::Module m;
  GeneratorImpl g(&m);
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&stmt)) << g.error();
  EXPECT_EQ(g.result(), "  float a;\n");
}

TEST_F(HlslGeneratorImplTest, Emit_VariableDeclStatement_Private) {
  ast::type::F32Type f32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kPrivate, &f32);

  ast::VariableDeclStatement stmt(std::move(var));

  ast::Module m;
  GeneratorImpl g(&m);
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&stmt)) << g.error();
  EXPECT_EQ(g.result(), "  float a;\n");
}

TEST_F(HlslGeneratorImplTest, Emit_VariableDeclStatement_Initializer_Private) {
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

TEST_F(HlslGeneratorImplTest, Emit_VariableDeclStatement_Initializer_ZeroVec) {
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
  EXPECT_EQ(g.result(), R"(vector<float, 3> a = vector<float, 3>(0.0f);
)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
