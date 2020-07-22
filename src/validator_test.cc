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

#include "src/validator_impl.h"

#include "gtest/gtest.h"
#include "spirv/unified1/GLSL.std.450.h"
#include "src/ast/array_accessor_expression.h"
#include "src/ast/as_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/break_statement.h"
#include "src/ast/call_expression.h"
#include "src/ast/case_statement.h"
#include "src/ast/cast_expression.h"
#include "src/ast/continue_statement.h"
#include "src/ast/else_statement.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/int_literal.h"
#include "src/ast/loop_statement.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"

namespace tint {
namespace {

using ValidatorTest = testing::Test;

TEST_F(ValidatorTest, Import) {
  ast::Module m;
  m.AddImport(std::make_unique<ast::Import>("GLSL.std.450", "glsl"));

  tint::ValidatorImpl v;
  EXPECT_TRUE(v.CheckImports(m));
}

TEST_F(ValidatorTest, Import_Fail_NotGLSL) {
  ast::Module m;
  m.AddImport(std::make_unique<ast::Import>(Source{1, 1}, "not.GLSL", "glsl"));

  tint::ValidatorImpl v;
  EXPECT_FALSE(v.CheckImports(m));
  ASSERT_TRUE(v.has_error());
  EXPECT_EQ(v.error(), "1:1: v-0001: unknown import: not.GLSL");
}

TEST_F(ValidatorTest, Import_Fail_Typo) {
  ast::Module m;
  m.AddImport(
      std::make_unique<ast::Import>(Source{1, 1}, "GLSL.std.4501", "glsl"));

  tint::ValidatorImpl v;
  EXPECT_FALSE(v.CheckImports(m));
  ASSERT_TRUE(v.has_error());
  EXPECT_EQ(v.error(), "1:1: v-0001: unknown import: GLSL.std.4501");
}

TEST_F(ValidatorTest, DISABLED_AssignToScalar_Fail) {
  // 1 = my_var
  ast::type::I32Type i32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1));
  auto rhs = std::make_unique<ast::IdentifierExpression>("my_var");
  ast::AssignmentStatement assign(std::move(lhs), std::move(rhs));

  tint::ValidatorImpl v;
  // TODO(sarahM0): Invalidate assignment to scalar.
  ASSERT_TRUE(v.has_error());
  EXPECT_EQ(v.error(), "1:1: v-000x: invalid assignment");
}

TEST_F(ValidatorTest, DISABLED_AssignUncompatibleTypes_Fail) {
  // var a :i32;
  // a = 2.3
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  ast::Variable var("a", ast::StorageClass::kPrivate, &i32);
  auto lhs = std::make_unique<ast::IdentifierExpression>("a");

  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));

  ast::AssignmentStatement assign(std::move(lhs), std::move(rhs));

  tint::ValidatorImpl v;
  // TODO(SarahM0): Invalidate assignments of different types.
  ASSERT_TRUE(v.has_error());
  EXPECT_EQ(v.error(), "1:1: v-000x: invalid assignment");
}

}  // namespace
}  // namespace tint
