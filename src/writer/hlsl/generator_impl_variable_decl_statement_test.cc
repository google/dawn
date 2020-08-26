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

#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_VariableDecl = TestHelper;

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement) {
  ast::type::F32Type f32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32);

  ast::VariableDeclStatement stmt(std::move(var));
  gen().increment_indent();

  ASSERT_TRUE(gen().EmitStatement(out(), &stmt)) << gen().error();
  EXPECT_EQ(result(), "  float a;\n");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const) {
  ast::type::F32Type f32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32);
  var->set_is_const(true);

  ast::VariableDeclStatement stmt(std::move(var));
  gen().increment_indent();

  ASSERT_TRUE(gen().EmitStatement(out(), &stmt)) << gen().error();
  EXPECT_EQ(result(), "  const float a;\n");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Array) {
  ast::type::F32Type f32;
  ast::type::ArrayType ary(&f32, 5);

  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &ary);

  ast::VariableDeclStatement stmt(std::move(var));
  gen().increment_indent();

  ASSERT_TRUE(gen().EmitStatement(out(), &stmt)) << gen().error();
  EXPECT_EQ(result(), "  float a[5];\n");
}

TEST_F(HlslGeneratorImplTest_VariableDecl,
       Emit_VariableDeclStatement_Function) {
  ast::type::F32Type f32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kFunction, &f32);

  ast::VariableDeclStatement stmt(std::move(var));
  gen().increment_indent();

  ASSERT_TRUE(gen().EmitStatement(out(), &stmt)) << gen().error();
  EXPECT_EQ(result(), "  float a;\n");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Private) {
  ast::type::F32Type f32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kPrivate, &f32);

  ast::VariableDeclStatement stmt(std::move(var));
  gen().increment_indent();

  ASSERT_TRUE(gen().EmitStatement(out(), &stmt)) << gen().error();
  EXPECT_EQ(result(), "  float a;\n");
}

TEST_F(HlslGeneratorImplTest_VariableDecl,
       Emit_VariableDeclStatement_Initializer_Private) {
  auto ident = std::make_unique<ast::IdentifierExpression>("initializer");

  ast::type::F32Type f32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32);
  var->set_constructor(std::move(ident));

  ast::VariableDeclStatement stmt(std::move(var));
  ASSERT_TRUE(gen().EmitStatement(out(), &stmt)) << gen().error();
  EXPECT_EQ(result(), R"(float a = initializer;
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl,
       Emit_VariableDeclStatement_Initializer_ZeroVec) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList values;
  auto zero_vec =
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(values));

  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &vec);
  var->set_constructor(std::move(zero_vec));

  ast::VariableDeclStatement stmt(std::move(var));
  ASSERT_TRUE(gen().EmitStatement(out(), &stmt)) << gen().error();
  EXPECT_EQ(result(), R"(vector<float, 3> a = vector<float, 3>(0.0f);
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl,
       Emit_VariableDeclStatement_Initializer_ZeroMat) {
  ast::type::F32Type f32;
  ast::type::MatrixType mat(&f32, 3, 2);

  ast::ExpressionList values;
  auto zero_mat =
      std::make_unique<ast::TypeConstructorExpression>(&mat, std::move(values));

  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &mat);
  var->set_constructor(std::move(zero_mat));

  ast::VariableDeclStatement stmt(std::move(var));
  ASSERT_TRUE(gen().EmitStatement(out(), &stmt)) << gen().error();
  EXPECT_EQ(
      result(),
      R"(matrix<float, 3, 2> a = matrix<float, 3, 2>(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
