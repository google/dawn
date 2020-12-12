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
#include "spirv/unified1/GLSL.std.450.h"
#include "src/ast/array_accessor_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/bool_literal.h"
#include "src/ast/break_statement.h"
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/case_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/else_statement.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/int_literal.h"
#include "src/ast/loop_statement.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/stage_decoration.h"
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
#include "src/validator/validator_impl.h"
#include "src/validator/validator_test_helper.h"

namespace tint {
namespace {

class ValidatorTest : public ValidatorTestHelper, public testing::Test {};

TEST_F(ValidatorTest, DISABLED_AssignToScalar_Fail) {
  // 1 = my_var;
  ast::type::I32 i32;

  auto* lhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::SintLiteral>(Source{}, &i32, 1));
  auto* rhs = create<ast::IdentifierExpression>(
      Source{}, mod()->RegisterSymbol("my_var"), "my_var");
  ast::AssignmentStatement assign(Source{Source::Location{12, 34}}, lhs, rhs);

  // TODO(sarahM0): Invalidate assignment to scalar.
  ASSERT_TRUE(v()->has_error());
  // TODO(sarahM0): figure out what should be the error number.
  EXPECT_EQ(v()->error(), "12:34 v-000x: invalid assignment");
}

TEST_F(ValidatorTest, UsingUndefinedVariable_Fail) {
  // b = 2;
  ast::type::I32 i32;

  auto* lhs = create<ast::IdentifierExpression>(
      Source{Source::Location{12, 34}}, mod()->RegisterSymbol("b"), "b");
  auto* rhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::SintLiteral>(Source{}, &i32, 2));
  auto* assign = create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs);

  EXPECT_FALSE(td()->DetermineResultType(assign));
  EXPECT_EQ(td()->error(),
            "12:34: v-0006: identifier must be declared before use: b");
}

TEST_F(ValidatorTest, UsingUndefinedVariableInBlockStatement_Fail) {
  // {
  //  b = 2;
  // }
  ast::type::I32 i32;

  auto* lhs = create<ast::IdentifierExpression>(
      Source{Source::Location{12, 34}}, mod()->RegisterSymbol("b"), "b");
  auto* rhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::SintLiteral>(Source{}, &i32, 2));

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs));

  EXPECT_FALSE(td()->DetermineStatements(body));
  EXPECT_EQ(td()->error(),
            "12:34: v-0006: identifier must be declared before use: b");
}

TEST_F(ValidatorTest, AssignCompatibleTypes_Pass) {
  // var a :i32 = 2;
  // a = 2
  ast::type::I32 i32;
  auto* var = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &i32,                      // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::SintLiteral>(Source{}, &i32, 2)),  // constructor
      ast::VariableDecorationList{});                    // decorations

  auto* lhs = create<ast::IdentifierExpression>(
      Source{}, mod()->RegisterSymbol("a"), "a");
  auto* rhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::SintLiteral>(Source{}, &i32, 2));

  ast::AssignmentStatement assign(Source{Source::Location{12, 34}}, lhs, rhs);
  td()->RegisterVariableForTesting(var);
  EXPECT_TRUE(td()->DetermineResultType(&assign)) << td()->error();
  ASSERT_NE(lhs->result_type(), nullptr);
  ASSERT_NE(rhs->result_type(), nullptr);
  EXPECT_TRUE(v()->ValidateResultTypes(&assign));
}

TEST_F(ValidatorTest, AssignIncompatibleTypes_Fail) {
  // {
  //  var a :i32 = 2;
  //  a = 2.3;
  // }
  ast::type::F32 f32;
  ast::type::I32 i32;

  auto* var = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &i32,                      // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::SintLiteral>(Source{}, &i32, 2)),  // constructor
      ast::VariableDecorationList{});                    // decorations

  auto* lhs = create<ast::IdentifierExpression>(
      Source{}, mod()->RegisterSymbol("a"), "a");
  auto* rhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 2.3f));

  ast::AssignmentStatement assign(Source{Source::Location{12, 34}}, lhs, rhs);
  td()->RegisterVariableForTesting(var);
  EXPECT_TRUE(td()->DetermineResultType(&assign)) << td()->error();
  ASSERT_NE(lhs->result_type(), nullptr);
  ASSERT_NE(rhs->result_type(), nullptr);

  EXPECT_FALSE(v()->ValidateResultTypes(&assign));
  ASSERT_TRUE(v()->has_error());
  // TODO(sarahM0): figure out what should be the error number.
  EXPECT_EQ(v()->error(),
            "12:34 v-000x: invalid assignment of '__i32' to '__f32'");
}

TEST_F(ValidatorTest, AssignCompatibleTypesInBlockStatement_Pass) {
  // {
  //  var a :i32 = 2;
  //  a = 2
  // }
  ast::type::I32 i32;
  auto* var = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &i32,                      // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::SintLiteral>(Source{}, &i32, 2)),  // constructor
      ast::VariableDecorationList{});                    // decorations

  auto* lhs = create<ast::IdentifierExpression>(
      Source{}, mod()->RegisterSymbol("a"), "a");
  auto* rhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::SintLiteral>(Source{}, &i32, 2));

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::VariableDeclStatement>(var));
  body->append(create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs));

  EXPECT_TRUE(td()->DetermineStatements(body)) << td()->error();
  ASSERT_NE(lhs->result_type(), nullptr);
  ASSERT_NE(rhs->result_type(), nullptr);

  EXPECT_TRUE(v()->ValidateStatements(body)) << v()->error();
}

TEST_F(ValidatorTest, AssignIncompatibleTypesInBlockStatement_Fail) {
  // {
  //  var a :i32 = 2;
  //  a = 2.3;
  // }
  ast::type::F32 f32;
  ast::type::I32 i32;

  auto* var = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &i32,                      // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::SintLiteral>(Source{}, &i32, 2)),  // constructor
      ast::VariableDecorationList{});                    // decorations
  auto* lhs = create<ast::IdentifierExpression>(
      Source{}, mod()->RegisterSymbol("a"), "a");
  auto* rhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 2.3f));

  ast::BlockStatement block;
  block.append(create<ast::VariableDeclStatement>(var));
  block.append(create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs));

  EXPECT_TRUE(td()->DetermineStatements(&block)) << td()->error();
  ASSERT_NE(lhs->result_type(), nullptr);
  ASSERT_NE(rhs->result_type(), nullptr);

  EXPECT_FALSE(v()->ValidateStatements(&block));
  ASSERT_TRUE(v()->has_error());
  // TODO(sarahM0): figure out what should be the error number.
  EXPECT_EQ(v()->error(),
            "12:34 v-000x: invalid assignment of '__i32' to '__f32'");
}

TEST_F(ValidatorTest, GlobalVariableWithStorageClass_Pass) {
  // var<in> gloabl_var: f32;
  ast::type::F32 f32;
  mod()->AddGlobalVariable(
      create<ast::Variable>(Source{Source::Location{12, 34}},  // source
                            "global_var",                      // name
                            ast::StorageClass::kInput,         // storage_class
                            &f32,                              // type
                            false,                             // is_const
                            nullptr,                           // constructor
                            ast::VariableDecorationList{}));   // decorations
  EXPECT_TRUE(v()->ValidateGlobalVariables(mod()->global_variables()))
      << v()->error();
}

TEST_F(ValidatorTest, GlobalVariableNoStorageClass_Fail) {
  // var gloabl_var: f32;
  ast::type::F32 f32;
  mod()->AddGlobalVariable(
      create<ast::Variable>(Source{Source::Location{12, 34}},  // source
                            "global_var",                      // name
                            ast::StorageClass::kNone,          // storage_class
                            &f32,                              // type
                            false,                             // is_const
                            nullptr,                           // constructor
                            ast::VariableDecorationList{}));   // decorations
  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod()));
  EXPECT_EQ(v()->error(),
            "12:34 v-0022: global variables must have a storage class");
}

TEST_F(ValidatorTest, GlobalConstantWithStorageClass_Fail) {
  // const<in> gloabl_var: f32;
  ast::type::F32 f32;
  mod()->AddGlobalVariable(
      create<ast::Variable>(Source{Source::Location{12, 34}},  // source
                            "global_var",                      // name
                            ast::StorageClass::kInput,         // storage_class
                            &f32,                              // type
                            true,                              // is_const
                            nullptr,                           // constructor
                            ast::VariableDecorationList{}));   // decorations
  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod()));
  EXPECT_EQ(
      v()->error(),
      "12:34 v-global01: global constants shouldn't have a storage class");
}

TEST_F(ValidatorTest, GlobalConstNoStorageClass_Pass) {
  // const gloabl_var: f32;
  ast::type::F32 f32;
  mod()->AddGlobalVariable(
      create<ast::Variable>(Source{Source::Location{12, 34}},  // source
                            "global_var",                      // name
                            ast::StorageClass::kNone,          // storage_class
                            &f32,                              // type
                            true,                              // is_const
                            nullptr,                           // constructor
                            ast::VariableDecorationList{}));   // decorations
  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod())) << v()->error();
}

TEST_F(ValidatorTest, UsingUndefinedVariableGlobalVariable_Fail) {
  // var global_var: f32 = 2.1;
  // fn my_func() -> f32 {
  //   not_global_var = 3.14f;
  // }
  ast::type::F32 f32;
  mod()->AddGlobalVariable(create<ast::Variable>(
      Source{},                     // source
      "global_var",                 // name
      ast::StorageClass::kPrivate,  // storage_class
      &f32,                         // type
      false,                        // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::FloatLiteral>(Source{}, &f32, 2.1)),  // constructor
      ast::VariableDecorationList{}));                      // decorations

  auto* lhs = create<ast::IdentifierExpression>(
      Source{Source::Location{12, 34}}, mod()->RegisterSymbol("not_global_var"),
      "not_global_var");
  auto* rhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 3.14f));

  ast::VariableList params;
  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs));

  auto* func = create<ast::Function>(Source{}, mod()->RegisterSymbol("my_func"),
                                     "my_func", params, &f32, body,
                                     ast::FunctionDecorationList{});
  mod()->AddFunction(func);

  EXPECT_FALSE(v()->Validate(mod()));
  EXPECT_EQ(v()->error(), "12:34 v-0006: 'not_global_var' is not declared");
}

TEST_F(ValidatorTest, UsingUndefinedVariableGlobalVariable_Pass) {
  // var global_var: f32 = 2.1;
  // fn my_func() -> void {
  //   global_var = 3.14;
  //   return;
  // }
  ast::type::F32 f32;
  ast::type::Void void_type;

  mod()->AddGlobalVariable(create<ast::Variable>(
      Source{},                     // source
      "global_var",                 // name
      ast::StorageClass::kPrivate,  // storage_class
      &f32,                         // type
      false,                        // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::FloatLiteral>(Source{}, &f32, 2.1)),  // constructor
      ast::VariableDecorationList{}));                      // decorations

  auto* lhs = create<ast::IdentifierExpression>(
      Source{}, mod()->RegisterSymbol("global_var"), "global_var");
  auto* rhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 3.14f));

  ast::VariableList params;

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs));
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func = create<ast::Function>(
      Source{}, mod()->RegisterSymbol("my_func"), "my_func", params, &void_type,
      body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(func);

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_TRUE(v()->Validate(mod())) << v()->error();
}

TEST_F(ValidatorTest, UsingUndefinedVariableInnerScope_Fail) {
  // {
  //   if (true) { var a : f32 = 2.0; }
  //   a = 3.14;
  // }
  ast::type::F32 f32;
  auto* var = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &f32,                      // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::FloatLiteral>(Source{}, &f32, 2.0)),  // constructor
      ast::VariableDecorationList{});                       // decorations

  ast::type::Bool bool_type;
  auto* cond = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::BoolLiteral>(Source{}, &bool_type, true));
  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::VariableDeclStatement>(var));

  auto* lhs = create<ast::IdentifierExpression>(
      Source{Source::Location{12, 34}}, mod()->RegisterSymbol("a"), "a");
  auto* rhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 3.14f));

  auto* outer_body = create<ast::BlockStatement>();
  outer_body->append(
      create<ast::IfStatement>(Source{}, cond, body, ast::ElseStatementList{}));
  outer_body->append(create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs));

  EXPECT_TRUE(td()->DetermineStatements(outer_body)) << td()->error();
  ASSERT_NE(lhs->result_type(), nullptr);
  ASSERT_NE(rhs->result_type(), nullptr);
  EXPECT_FALSE(v()->ValidateStatements(outer_body));
  EXPECT_EQ(v()->error(), "12:34 v-0006: 'a' is not declared");
}

TEST_F(ValidatorTest, UsingUndefinedVariableOuterScope_Pass) {
  // {
  //   var a : f32 = 2.0;
  //   if (true) { a = 3.14; }
  // }
  ast::type::F32 f32;
  auto* var = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &f32,                      // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::FloatLiteral>(Source{}, &f32, 2.0)),  // constructor
      ast::VariableDecorationList{});                       // decorations

  auto* lhs = create<ast::IdentifierExpression>(
      Source{Source::Location{12, 34}}, mod()->RegisterSymbol("a"), "a");
  auto* rhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 3.14f));

  ast::type::Bool bool_type;
  auto* cond = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::BoolLiteral>(Source{}, &bool_type, true));
  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs));

  auto* outer_body = create<ast::BlockStatement>();
  outer_body->append(create<ast::VariableDeclStatement>(var));
  outer_body->append(
      create<ast::IfStatement>(Source{}, cond, body, ast::ElseStatementList{}));
  EXPECT_TRUE(td()->DetermineStatements(outer_body)) << td()->error();
  ASSERT_NE(lhs->result_type(), nullptr);
  ASSERT_NE(rhs->result_type(), nullptr);
  EXPECT_TRUE(v()->ValidateStatements(outer_body)) << v()->error();
}

TEST_F(ValidatorTest, GlobalVariableUnique_Pass) {
  // var global_var0 : f32 = 0.1;
  // var global_var1 : i32 = 0;
  ast::type::F32 f32;
  ast::type::I32 i32;
  auto* var0 = create<ast::Variable>(
      Source{},                     // source
      "global_var0",                // name
      ast::StorageClass::kPrivate,  // storage_class
      &f32,                         // type
      false,                        // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::FloatLiteral>(Source{}, &f32, 0.1)),  // constructor
      ast::VariableDecorationList{});                       // decorations
  mod()->AddGlobalVariable(var0);

  auto* var1 = create<ast::Variable>(
      Source{Source::Location{12, 34}},  // source
      "global_var1",                     // name
      ast::StorageClass::kPrivate,       // storage_class
      &f32,                              // type
      false,                             // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::SintLiteral>(Source{}, &i32, 0)),  // constructor
      ast::VariableDecorationList{});                    // decorations
  mod()->AddGlobalVariable(var1);

  EXPECT_TRUE(v()->ValidateGlobalVariables(mod()->global_variables()))
      << v()->error();
}

TEST_F(ValidatorTest, GlobalVariableNotUnique_Fail) {
  // var global_var : f32 = 0.1;
  // var global_var : i32 = 0;
  ast::type::F32 f32;
  ast::type::I32 i32;
  auto* var0 = create<ast::Variable>(
      Source{},                     // source
      "global_var",                 // name
      ast::StorageClass::kPrivate,  // storage_class
      &f32,                         // type
      false,                        // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::FloatLiteral>(Source{}, &f32, 0.1)),  // constructor
      ast::VariableDecorationList{});                       // decorations
  mod()->AddGlobalVariable(var0);

  auto* var1 = create<ast::Variable>(
      Source{Source::Location{12, 34}},  // source
      "global_var",                      // name
      ast::StorageClass::kPrivate,       // storage_class
      &f32,                              // type
      false,                             // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::SintLiteral>(Source{}, &i32, 0)),  // constructor
      ast::VariableDecorationList{});                    // decorations
  mod()->AddGlobalVariable(var1);

  EXPECT_FALSE(v()->ValidateGlobalVariables(mod()->global_variables()));
  EXPECT_EQ(v()->error(),
            "12:34 v-0011: redeclared global identifier 'global_var'");
}

TEST_F(ValidatorTest, AssignToConstant_Fail) {
  // {
  //  const a :i32 = 2;
  //  a = 2
  // }
  ast::type::I32 i32;
  auto* var = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &i32,                      // type
      true,                      // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::SintLiteral>(Source{}, &i32, 2)),  // constructor
      ast::VariableDecorationList{});                    // decorations

  auto* lhs = create<ast::IdentifierExpression>(
      Source{}, mod()->RegisterSymbol("a"), "a");
  auto* rhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::SintLiteral>(Source{}, &i32, 2));

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::VariableDeclStatement>(var));
  body->append(create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs));

  EXPECT_TRUE(td()->DetermineStatements(body)) << td()->error();
  ASSERT_NE(lhs->result_type(), nullptr);
  ASSERT_NE(rhs->result_type(), nullptr);

  EXPECT_FALSE(v()->ValidateStatements(body));
  EXPECT_EQ(v()->error(), "12:34 v-0021: cannot re-assign a constant: 'a'");
}

TEST_F(ValidatorTest, GlobalVariableFunctionVariableNotUnique_Fail) {
  // var a: f32 = 2.1;
  // fn my_func -> void {
  //   var a: f32 = 2.0;
  //   return 0;
  // }

  ast::type::Void void_type;
  ast::type::F32 f32;
  auto* global_var = create<ast::Variable>(
      Source{},                     // source
      "a",                          // name
      ast::StorageClass::kPrivate,  // storage_class
      &f32,                         // type
      false,                        // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::FloatLiteral>(Source{}, &f32, 2.1)),  // constructor
      ast::VariableDecorationList{});                       // decorations
  mod()->AddGlobalVariable(global_var);

  auto* var = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &f32,                      // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::FloatLiteral>(Source{}, &f32, 2.0)),  // constructor
      ast::VariableDecorationList{});                       // decorations
  ast::VariableList params;
  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::VariableDeclStatement>(
      Source{Source::Location{12, 34}}, var));
  auto* func = create<ast::Function>(Source{}, mod()->RegisterSymbol("my_func"),
                                     "my_func", params, &void_type, body,
                                     ast::FunctionDecorationList{});

  mod()->AddFunction(func);

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_TRUE(td()->DetermineFunction(func)) << td()->error();
  EXPECT_FALSE(v()->Validate(mod())) << v()->error();
  EXPECT_EQ(v()->error(), "12:34 v-0013: redeclared identifier 'a'");
}

TEST_F(ValidatorTest, RedeclaredIndentifier_Fail) {
  // fn my_func() -> void {
  //  var a :i32 = 2;
  //  var a :f21 = 2.0;
  // }
  ast::type::Void void_type;
  ast::type::I32 i32;
  ast::type::F32 f32;
  auto* var = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &i32,                      // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::SintLiteral>(Source{}, &i32, 2)),  // constructor
      ast::VariableDecorationList{});                    // decorations

  auto* var_a_float = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &f32,                      // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::FloatLiteral>(Source{}, &f32, 0.1)),  // constructor
      ast::VariableDecorationList{});                       // decorations

  ast::VariableList params;
  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::VariableDeclStatement>(var));
  body->append(create<ast::VariableDeclStatement>(
      Source{Source::Location{12, 34}}, var_a_float));
  auto* func = create<ast::Function>(Source{}, mod()->RegisterSymbol("my_func"),
                                     "my_func", params, &void_type, body,
                                     ast::FunctionDecorationList{});

  mod()->AddFunction(func);

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_TRUE(td()->DetermineFunction(func)) << td()->error();
  EXPECT_FALSE(v()->Validate(mod()));
  EXPECT_EQ(v()->error(), "12:34 v-0014: redeclared identifier 'a'");
}

TEST_F(ValidatorTest, RedeclaredIdentifierInnerScope_Pass) {
  // {
  // if (true) { var a : f32 = 2.0; }
  // var a : f32 = 3.14;
  // }
  ast::type::F32 f32;
  auto* var = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &f32,                      // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::FloatLiteral>(Source{}, &f32, 2.0)),  // constructor
      ast::VariableDecorationList{});                       // decorations

  ast::type::Bool bool_type;
  auto* cond = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::BoolLiteral>(Source{}, &bool_type, true));
  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::VariableDeclStatement>(var));

  auto* var_a_float = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &f32,                      // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::FloatLiteral>(Source{}, &f32, 3.14)),  // constructor
      ast::VariableDecorationList{});                        // decorations

  auto* outer_body = create<ast::BlockStatement>();
  outer_body->append(
      create<ast::IfStatement>(Source{}, cond, body, ast::ElseStatementList{}));
  outer_body->append(create<ast::VariableDeclStatement>(
      Source{Source::Location{12, 34}}, var_a_float));

  EXPECT_TRUE(td()->DetermineStatements(outer_body)) << td()->error();
  EXPECT_TRUE(v()->ValidateStatements(outer_body)) << v()->error();
}

TEST_F(ValidatorTest, DISABLED_RedeclaredIdentifierInnerScope_False) {
  // TODO(sarahM0): remove DISABLED after implementing ValidateIfStatement
  // and it should just work
  // {
  // var a : f32 = 3.14;
  // if (true) { var a : f32 = 2.0; }
  // }
  ast::type::F32 f32;
  auto* var_a_float = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &f32,                      // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::FloatLiteral>(Source{}, &f32, 3.14)),  // constructor
      ast::VariableDecorationList{});                        // decorations

  auto* var = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &f32,                      // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::FloatLiteral>(Source{}, &f32, 2.0)),  // constructor
      ast::VariableDecorationList{});                       // decorations

  ast::type::Bool bool_type;
  auto* cond = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::BoolLiteral>(Source{}, &bool_type, true));
  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::VariableDeclStatement>(
      Source{Source::Location{12, 34}}, var));

  auto* outer_body = create<ast::BlockStatement>();
  outer_body->append(create<ast::VariableDeclStatement>(var_a_float));
  outer_body->append(
      create<ast::IfStatement>(Source{}, cond, body, ast::ElseStatementList{}));

  EXPECT_TRUE(td()->DetermineStatements(outer_body)) << td()->error();
  EXPECT_FALSE(v()->ValidateStatements(outer_body));
  EXPECT_EQ(v()->error(), "12:34 v-0014: redeclared identifier 'a'");
}

TEST_F(ValidatorTest, RedeclaredIdentifierDifferentFunctions_Pass) {
  // func0 { var a : f32 = 2.0; return; }
  // func1 { var a : f32 = 3.0; return; }
  ast::type::F32 f32;
  ast::type::Void void_type;
  auto* var0 = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &f32,                      // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::FloatLiteral>(Source{}, &f32, 2.0)),  // constructor
      ast::VariableDecorationList{});                       // decorations

  auto* var1 = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &void_type,                // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::FloatLiteral>(Source{}, &f32, 1.0)),  // constructor
      ast::VariableDecorationList{});                       // decorations

  ast::VariableList params0;
  auto* body0 = create<ast::BlockStatement>();
  body0->append(create<ast::VariableDeclStatement>(
      Source{Source::Location{12, 34}}, var0));
  body0->append(create<ast::ReturnStatement>(Source{}));
  auto* func0 = create<ast::Function>(Source{}, mod()->RegisterSymbol("func0"),
                                      "func0", params0, &void_type, body0,
                                      ast::FunctionDecorationList{});

  ast::VariableList params1;
  auto* body1 = create<ast::BlockStatement>();
  body1->append(create<ast::VariableDeclStatement>(
      Source{Source::Location{13, 34}}, var1));
  body1->append(create<ast::ReturnStatement>(Source{}));
  auto* func1 = create<ast::Function>(
      Source{}, mod()->RegisterSymbol("func1"), "func1", params1, &void_type,
      body1,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });

  mod()->AddFunction(func0);
  mod()->AddFunction(func1);

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_TRUE(v()->Validate(mod())) << v()->error();
}

TEST_F(ValidatorTest, VariableDeclNoConstructor_Pass) {
  // {
  // var a :i32;
  // a = 2;
  // }
  ast::type::I32 i32;
  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "a",                             // name
                            ast::StorageClass::kNone,        // storage_class
                            &i32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  td()->RegisterVariableForTesting(var);
  auto* lhs = create<ast::IdentifierExpression>(
      Source{}, mod()->RegisterSymbol("a"), "a");
  auto* rhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::SintLiteral>(Source{}, &i32, 2));

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::VariableDeclStatement>(var));
  body->append(create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs));

  EXPECT_TRUE(td()->DetermineStatements(body)) << td()->error();
  ASSERT_NE(lhs->result_type(), nullptr);
  ASSERT_NE(rhs->result_type(), nullptr);
  EXPECT_TRUE(v()->ValidateStatements(body)) << v()->error();
}

}  // namespace
}  // namespace tint
