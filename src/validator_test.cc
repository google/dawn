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
#include "src/ast/bool_literal.h"
#include "src/ast/break_statement.h"
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
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
#include "src/ast/type/void_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/type_determiner.h"
#include "src/validator_test_helper.h"

namespace tint {
namespace {

class ValidatorTest : public ValidatorTestHelper, public testing::Test {};

TEST_F(ValidatorTest, Import) {
  ast::Module m;
  m.AddImport(std::make_unique<ast::Import>("GLSL.std.450", "glsl"));

  EXPECT_TRUE(v()->CheckImports(&m));
}

TEST_F(ValidatorTest, Import_Fail_NotGLSL) {
  ast::Module m;
  m.AddImport(
      std::make_unique<ast::Import>(Source{12, 34}, "not.GLSL", "glsl"));

  EXPECT_FALSE(v()->CheckImports(&m));
  ASSERT_TRUE(v()->has_error());
  EXPECT_EQ(v()->error(), "12:34: v-0001: unknown import: not.GLSL");
}

TEST_F(ValidatorTest, Import_Fail_Typo) {
  ast::Module m;
  m.AddImport(
      std::make_unique<ast::Import>(Source{12, 34}, "GLSL.std.4501", "glsl"));

  EXPECT_FALSE(v()->CheckImports(&m));
  ASSERT_TRUE(v()->has_error());
  EXPECT_EQ(v()->error(), "12:34: v-0001: unknown import: GLSL.std.4501");
}

TEST_F(ValidatorTest, DISABLED_AssignToScalar_Fail) {
  // 1 = my_var;
  ast::type::I32Type i32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1));
  auto rhs = std::make_unique<ast::IdentifierExpression>("my_var");
  ast::AssignmentStatement assign(Source{12, 34}, std::move(lhs),
                                  std::move(rhs));

  // TODO(sarahM0): Invalidate assignment to scalar.
  ASSERT_TRUE(v()->has_error());
  // TODO(sarahM0): figure out what should be the error number.
  EXPECT_EQ(v()->error(), "12:34: v-000x: invalid assignment");
}

TEST_F(ValidatorTest, UsingUndefinedVariable_Fail) {
  // b = 2;
  ast::type::I32Type i32;

  auto lhs = std::make_unique<ast::IdentifierExpression>(Source{12, 34}, "b");
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto assign = std::make_unique<ast::AssignmentStatement>(
      Source{12, 34}, std::move(lhs), std::move(rhs));

  EXPECT_TRUE(td()->DetermineResultType(assign.get())) << td()->error();
  EXPECT_FALSE(v()->ValidateStatement(assign.get()));
  EXPECT_EQ(v()->error(), "12:34: v-0006: 'b' is not declared");
}

TEST_F(ValidatorTest, UsingUndefinedVariableInBlockStatement_Fail) {
  // {
  //  b = 2;
  // }
  ast::type::I32Type i32;

  auto lhs = std::make_unique<ast::IdentifierExpression>(Source{12, 34}, "b");
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      Source{12, 34}, std::move(lhs), std::move(rhs)));

  EXPECT_TRUE(td()->DetermineStatements(body.get())) << td()->error();
  EXPECT_FALSE(v()->ValidateStatements(body.get()));
  EXPECT_EQ(v()->error(), "12:34: v-0006: 'b' is not declared");
}

TEST_F(ValidatorTest, AssignCompatibleTypes_Pass) {
  // var a :i32 = 2;
  // a = 2
  ast::type::I32Type i32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &i32);
  var->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));

  auto lhs = std::make_unique<ast::IdentifierExpression>("a");
  auto* lhs_ptr = lhs.get();
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto* rhs_ptr = rhs.get();

  ast::AssignmentStatement assign(Source{12, 34}, std::move(lhs),
                                  std::move(rhs));
  td()->RegisterVariableForTesting(var.get());
  EXPECT_TRUE(td()->DetermineResultType(&assign)) << td()->error();
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);
  EXPECT_TRUE(v()->ValidateResultTypes(&assign));
}

TEST_F(ValidatorTest, AssignIncompatibleTypes_Fail) {
  // {
  //  var a :i32 = 2;
  //  a = 2.3;
  // }
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &i32);
  var->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));
  auto lhs = std::make_unique<ast::IdentifierExpression>("a");
  auto* lhs_ptr = lhs.get();
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto* rhs_ptr = rhs.get();

  ast::AssignmentStatement assign(Source{12, 34}, std::move(lhs),
                                  std::move(rhs));
  td()->RegisterVariableForTesting(var.get());
  EXPECT_TRUE(td()->DetermineResultType(&assign)) << td()->error();
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);

  EXPECT_FALSE(v()->ValidateResultTypes(&assign));
  ASSERT_TRUE(v()->has_error());
  // TODO(sarahM0): figure out what should be the error number.
  EXPECT_EQ(v()->error(),
            "12:34: v-000x: invalid assignment of '__i32' to '__f32'");
}

TEST_F(ValidatorTest, AssignCompatibleTypesInBlockStatement_Pass) {
  // {
  //  var a :i32 = 2;
  //  a = 2
  // }
  ast::type::I32Type i32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &i32);
  var->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));

  auto lhs = std::make_unique<ast::IdentifierExpression>("a");
  auto* lhs_ptr = lhs.get();
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto* rhs_ptr = rhs.get();

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));
  body->append(std::make_unique<ast::AssignmentStatement>(
      Source{12, 34}, std::move(lhs), std::move(rhs)));

  EXPECT_TRUE(td()->DetermineStatements(body.get())) << td()->error();
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);

  EXPECT_TRUE(v()->ValidateStatements(body.get())) << v()->error();
}

TEST_F(ValidatorTest, AssignIncompatibleTypesInBlockStatement_Fail) {
  // {
  //  var a :i32 = 2;
  //  a = 2.3;
  // }
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &i32);
  var->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));
  auto lhs = std::make_unique<ast::IdentifierExpression>("a");
  auto* lhs_ptr = lhs.get();
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto* rhs_ptr = rhs.get();

  ast::BlockStatement block;
  block.append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));
  block.append(std::make_unique<ast::AssignmentStatement>(
      Source{12, 34}, std::move(lhs), std::move(rhs)));

  EXPECT_TRUE(td()->DetermineStatements(&block)) << td()->error();
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);

  EXPECT_FALSE(v()->ValidateStatements(&block));
  ASSERT_TRUE(v()->has_error());
  // TODO(sarahM0): figure out what should be the error number.
  EXPECT_EQ(v()->error(),
            "12:34: v-000x: invalid assignment of '__i32' to '__f32'");
}

TEST_F(ValidatorTest, GlobalVariableWithStorageClass_Pass) {
  // var<in> gloabl_var: f32;
  ast::type::F32Type f32;
  auto global_var = std::make_unique<ast::Variable>(
      Source{12, 34}, "global_var", ast::StorageClass::kInput, &f32);
  mod()->AddGlobalVariable(std::move(global_var));
  AddFakeEntryPoint();

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_TRUE(v()->Validate(mod())) << v()->error();
}

TEST_F(ValidatorTest, GlobalVariableNoStorageClass_Fail) {
  // var gloabl_var: f32;
  ast::type::F32Type f32;
  auto global_var = std::make_unique<ast::Variable>(
      Source{12, 34}, "global_var", ast::StorageClass::kNone, &f32);
  mod()->AddGlobalVariable(std::move(global_var));
  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod()));
  EXPECT_EQ(v()->error(),
            "12:34: v-0022: global variables must have a storage class");
}

TEST_F(ValidatorTest, UsingUndefinedVariableGlobalVariable_Fail) {
  // var global_var: f32 = 2.1;
  // fn my_func() -> f32 {
  //   not_global_var = 3.14f;
  // }
  ast::type::F32Type f32;
  auto global_var = std::make_unique<ast::Variable>(
      "global_var", ast::StorageClass::kPrivate, &f32);
  global_var->set_constructor(
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::FloatLiteral>(&f32, 2.1)));
  mod()->AddGlobalVariable(std::move(global_var));

  auto lhs = std::make_unique<ast::IdentifierExpression>(Source{12, 34},
                                                         "not_global_var");
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.14f));

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("my_func", std::move(params), &f32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      Source{12, 34}, std::move(lhs), std::move(rhs)));
  func->set_body(std::move(body));
  mod()->AddFunction(std::move(func));

  EXPECT_FALSE(v()->Validate(mod()));
  EXPECT_EQ(v()->error(), "12:34: v-0006: 'not_global_var' is not declared");
}

TEST_F(ValidatorTest, UsingUndefinedVariableGlobalVariable_Pass) {
  // var global_var: f32 = 2.1;
  // fn my_func() -> f32 {
  //   global_var = 3.14;
  //   return 3.14;
  // }
  ast::type::F32Type f32;
  auto global_var = std::make_unique<ast::Variable>(
      "global_var", ast::StorageClass::kPrivate, &f32);
  global_var->set_constructor(
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::FloatLiteral>(&f32, 2.1)));
  mod()->AddGlobalVariable(std::move(global_var));

  auto lhs = std::make_unique<ast::IdentifierExpression>("global_var");
  auto* lhs_ptr = lhs.get();
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.14f));
  auto* rhs_ptr = rhs.get();
  auto return_expr = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.14f));

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("my_func", std::move(params), &f32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      Source{12, 34}, std::move(lhs), std::move(rhs)));
  body->append(std::make_unique<ast::ReturnStatement>(std::move(return_expr)));
  func->set_body(std::move(body));
  auto* func_ptr = func.get();
  mod()->AddFunction(std::move(func));
  AddFakeEntryPoint();

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_TRUE(td()->DetermineFunction(func_ptr)) << td()->error();
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);
  EXPECT_TRUE(v()->Validate(mod())) << v()->error();
}

TEST_F(ValidatorTest, UsingUndefinedVariableInnerScope_Fail) {
  // {
  //   if (true) { var a : f32 = 2.0; }
  //   a = 3.14;
  // }
  ast::type::F32Type f32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32);
  var->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::type::BoolType bool_type;
  auto cond = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::BoolLiteral>(&bool_type, true));
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));

  auto lhs = std::make_unique<ast::IdentifierExpression>(Source{12, 34}, "a");
  auto* lhs_ptr = lhs.get();
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.14f));
  auto* rhs_ptr = rhs.get();

  auto outer_body = std::make_unique<ast::BlockStatement>();
  outer_body->append(
      std::make_unique<ast::IfStatement>(std::move(cond), std::move(body)));
  outer_body->append(std::make_unique<ast::AssignmentStatement>(
      Source{12, 34}, std::move(lhs), std::move(rhs)));

  EXPECT_TRUE(td()->DetermineStatements(outer_body.get())) << td()->error();
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);
  EXPECT_FALSE(v()->ValidateStatements(outer_body.get()));
  EXPECT_EQ(v()->error(), "12:34: v-0006: 'a' is not declared");
}

TEST_F(ValidatorTest, UsingUndefinedVariableOuterScope_Pass) {
  // {
  //   var a : f32 = 2.0;
  //   if (true) { a = 3.14; }
  // }
  ast::type::F32Type f32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32);
  var->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  auto lhs = std::make_unique<ast::IdentifierExpression>(Source{12, 34}, "a");
  auto* lhs_ptr = lhs.get();
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.14f));
  auto* rhs_ptr = rhs.get();
  ast::type::BoolType bool_type;
  auto cond = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::BoolLiteral>(&bool_type, true));
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      Source{12, 34}, std::move(lhs), std::move(rhs)));

  auto outer_body = std::make_unique<ast::BlockStatement>();
  outer_body->append(
      std::make_unique<ast::VariableDeclStatement>(std::move(var)));
  outer_body->append(
      std::make_unique<ast::IfStatement>(std::move(cond), std::move(body)));
  EXPECT_TRUE(td()->DetermineStatements(outer_body.get())) << td()->error();
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);
  EXPECT_TRUE(v()->ValidateStatements(outer_body.get())) << v()->error();
}

TEST_F(ValidatorTest, GlobalVariableUnique_Pass) {
  // var global_var0 : f32 = 0.1;
  // var global_var1 : i32 = 0;
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  auto var0 = std::make_unique<ast::Variable>(
      "global_var0", ast::StorageClass::kPrivate, &f32);
  var0->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 0.1)));
  mod()->AddGlobalVariable(std::move(var0));

  auto var1 = std::make_unique<ast::Variable>(
      Source{12, 34}, "global_var1", ast::StorageClass::kPrivate, &f32);
  var1->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 0)));
  mod()->AddGlobalVariable(std::move(var1));
  AddFakeEntryPoint();

  EXPECT_TRUE(v()->Validate(mod())) << v()->error();
}

TEST_F(ValidatorTest, AssignToConstant_Fail) {
  // {
  //  const a :i32 = 2;
  //  a = 2
  // }
  ast::type::I32Type i32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &i32);
  var->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));
  var->set_is_const(true);

  auto lhs = std::make_unique<ast::IdentifierExpression>("a");
  auto* lhs_ptr = lhs.get();
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto* rhs_ptr = rhs.get();

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));
  body->append(std::make_unique<ast::AssignmentStatement>(
      Source{12, 34}, std::move(lhs), std::move(rhs)));

  EXPECT_TRUE(td()->DetermineStatements(body.get())) << td()->error();
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);

  EXPECT_FALSE(v()->ValidateStatements(body.get()));
  EXPECT_EQ(v()->error(), "12:34: v-0021: cannot re-assign a constant: 'a'");
}

TEST_F(ValidatorTest, GlobalVariableNotUnique_Fail) {
  // var global_var : f32 = 0.1;
  // var global_var : i32 = 0;
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  auto var0 = std::make_unique<ast::Variable>(
      "global_var", ast::StorageClass::kPrivate, &f32);
  var0->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 0.1)));
  mod()->AddGlobalVariable(std::move(var0));

  auto var1 = std::make_unique<ast::Variable>(
      Source{12, 34}, "global_var", ast::StorageClass::kPrivate, &f32);
  var1->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 0)));
  mod()->AddGlobalVariable(std::move(var1));

  EXPECT_FALSE(v()->Validate(mod()));
  EXPECT_EQ(v()->error(),
            "12:34: v-0011: redeclared global identifier 'global_var'");
}

TEST_F(ValidatorTest, GlobalVariableFunctionVariableNotUnique_Fail) {
  // var a: f32 = 2.1;
  // fn my_func -> void {
  //   var a: f32 = 2.0;
  //   return 0;
  // }

  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  auto global_var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kPrivate, &f32);
  global_var->set_constructor(
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::FloatLiteral>(&f32, 2.1)));
  mod()->AddGlobalVariable(std::move(global_var));

  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32);
  var->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("my_func", std::move(params), &void_type);
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::VariableDeclStatement>(Source{12, 34},
                                                            std::move(var)));
  func->set_body(std::move(body));
  auto* func_ptr = func.get();
  mod()->AddFunction(std::move(func));

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_TRUE(td()->DetermineFunction(func_ptr)) << td()->error();
  EXPECT_FALSE(v()->Validate(mod())) << v()->error();
  EXPECT_EQ(v()->error(), "12:34: v-0013: redeclared identifier 'a'");
}

TEST_F(ValidatorTest, RedeclaredIndentifier_Fail) {
  // fn my_func() -> void {
  //  var a :i32 = 2;
  //  var a :f21 = 2.0;
  // }
  ast::type::VoidType void_type;
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &i32);
  var->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));

  auto var_a_float =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32);
  var_a_float->set_constructor(
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::FloatLiteral>(&f32, 0.1)));

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("my_func", std::move(params), &void_type);
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));
  body->append(std::make_unique<ast::VariableDeclStatement>(
      Source{12, 34}, std::move(var_a_float)));
  func->set_body(std::move(body));
  auto* func_ptr = func.get();
  mod()->AddFunction(std::move(func));

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_TRUE(td()->DetermineFunction(func_ptr)) << td()->error();
  EXPECT_FALSE(v()->Validate(mod()));
  EXPECT_EQ(v()->error(), "12:34: v-0014: redeclared identifier 'a'");
}

TEST_F(ValidatorTest, RedeclaredIdentifierInnerScope_Pass) {
  // {
  // if (true) { var a : f32 = 2.0; }
  // var a : f32 = 3.14;
  // }
  ast::type::F32Type f32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32);
  var->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::type::BoolType bool_type;
  auto cond = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::BoolLiteral>(&bool_type, true));
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));

  auto var_a_float =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32);
  var_a_float->set_constructor(
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::FloatLiteral>(&f32, 3.14)));

  auto outer_body = std::make_unique<ast::BlockStatement>();
  outer_body->append(
      std::make_unique<ast::IfStatement>(std::move(cond), std::move(body)));
  outer_body->append(std::make_unique<ast::VariableDeclStatement>(
      Source{12, 34}, std::move(var_a_float)));

  EXPECT_TRUE(td()->DetermineStatements(outer_body.get())) << td()->error();
  EXPECT_TRUE(v()->ValidateStatements(outer_body.get())) << v()->error();
}

TEST_F(ValidatorTest, DISABLED_RedeclaredIdentifierInnerScope_False) {
  // TODO(sarahM0): remove DISABLED after implementing ValidateIfStatement
  // and it should just work
  // {
  // var a : f32 = 3.14;
  // if (true) { var a : f32 = 2.0; }
  // }
  ast::type::F32Type f32;
  auto var_a_float =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32);
  var_a_float->set_constructor(
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::FloatLiteral>(&f32, 3.14)));

  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32);
  var->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::type::BoolType bool_type;
  auto cond = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::BoolLiteral>(&bool_type, true));
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::VariableDeclStatement>(Source{12, 34},
                                                            std::move(var)));

  auto outer_body = std::make_unique<ast::BlockStatement>();
  outer_body->append(
      std::make_unique<ast::VariableDeclStatement>(std::move(var_a_float)));
  outer_body->append(
      std::make_unique<ast::IfStatement>(std::move(cond), std::move(body)));

  EXPECT_TRUE(td()->DetermineStatements(outer_body.get())) << td()->error();
  EXPECT_FALSE(v()->ValidateStatements(outer_body.get()));
  EXPECT_EQ(v()->error(), "12:34: v-0014: redeclared identifier 'a'");
}

TEST_F(ValidatorTest, RedeclaredIdentifierDifferentFunctions_Pass) {
  // func0 { var a : f32 = 2.0; return; }
  // func1 { var a : f32 = 3.0; return; }
  ast::type::F32Type f32;
  ast::type::VoidType void_type;
  auto var0 =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32);
  var0->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  auto var1 = std::make_unique<ast::Variable>("a", ast::StorageClass::kNone,
                                              &void_type);
  var1->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0)));

  ast::VariableList params0;
  auto func0 =
      std::make_unique<ast::Function>("func0", std::move(params0), &void_type);
  auto body0 = std::make_unique<ast::BlockStatement>();
  body0->append(std::make_unique<ast::VariableDeclStatement>(Source{12, 34},
                                                             std::move(var0)));
  body0->append(std::make_unique<ast::ReturnStatement>());
  func0->set_body(std::move(body0));

  ast::VariableList params1;
  auto func1 =
      std::make_unique<ast::Function>("func1", std::move(params1), &void_type);
  auto body1 = std::make_unique<ast::BlockStatement>();
  body1->append(std::make_unique<ast::VariableDeclStatement>(Source{13, 34},
                                                             std::move(var1)));
  body1->append(std::make_unique<ast::ReturnStatement>());
  func1->set_body(std::move(body1));

  mod()->AddFunction(std::move(func0));
  mod()->AddFunction(std::move(func1));
  AddFakeEntryPoint();

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_TRUE(v()->Validate(mod())) << v()->error();
}

TEST_F(ValidatorTest, VariableDeclNoConstructor_Pass) {
  // {
  // var a :i32;
  // a = 2;
  // }
  ast::type::I32Type i32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &i32);

  td()->RegisterVariableForTesting(var.get());
  auto lhs = std::make_unique<ast::IdentifierExpression>("a");
  auto* lhs_ptr = lhs.get();
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto* rhs_ptr = rhs.get();

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));
  body->append(std::make_unique<ast::AssignmentStatement>(
      Source{12, 34}, std::move(lhs), std::move(rhs)));

  EXPECT_TRUE(td()->DetermineStatements(body.get())) << td()->error();
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);
  EXPECT_TRUE(v()->ValidateStatements(body.get())) << v()->error();
}

}  // namespace
}  // namespace tint
