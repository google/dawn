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
#include "src/ast/type_constructor_expression.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/type/alias_type.h"
#include "src/type/array_type.h"
#include "src/type/bool_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/matrix_type.h"
#include "src/type/pointer_type.h"
#include "src/type/struct_type.h"
#include "src/type/vector_type.h"
#include "src/type/void_type.h"
#include "src/type_determiner.h"
#include "src/validator/validator_impl.h"
#include "src/validator/validator_test_helper.h"

namespace tint {
namespace {

class ValidatorTest : public ValidatorTestHelper, public testing::Test {};

TEST_F(ValidatorTest, AssignToScalar_Fail) {
  // var my_var : i32 = 2;
  // 1 = my_var;

  auto* var = Var("my_var", ast::StorageClass::kNone, ty.i32(), Expr(2),
                  ast::VariableDecorationList{});

  auto* lhs = Expr(1);
  auto* rhs = Expr("my_var");
  SetSource(Source{Source::Location{12, 34}});
  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);
  RegisterVariable(var);

  EXPECT_TRUE(td()->DetermineResultType(assign));

  ValidatorImpl& v = Build();

  // TODO(sarahM0): Invalidate assignment to scalar.
  EXPECT_FALSE(v.ValidateAssign(assign));
  ASSERT_TRUE(v.has_error());
  // TODO(sarahM0): figure out what should be the error number.
  EXPECT_EQ(v.error(),
            "12:34 v-000x: invalid assignment: left-hand-side does not "
            "reference storage: __i32");
}

TEST_F(ValidatorTest, UsingUndefinedVariable_Fail) {
  // b = 2;

  SetSource(Source{Source::Location{12, 34}});
  auto* lhs = Expr("b");
  auto* rhs = Expr(2);
  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);

  EXPECT_FALSE(td()->DetermineResultType(assign));
  EXPECT_EQ(td()->error(),
            "12:34: v-0006: identifier must be declared before use: b");
}

TEST_F(ValidatorTest, UsingUndefinedVariableInBlockStatement_Fail) {
  // {
  //  b = 2;
  // }

  SetSource(Source{Source::Location{12, 34}});
  auto* lhs = Expr("b");
  auto* rhs = Expr(2);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::AssignmentStatement>(lhs, rhs),
  });

  EXPECT_FALSE(td()->DetermineStatements(body));
  EXPECT_EQ(td()->error(),
            "12:34: v-0006: identifier must be declared before use: b");
}

TEST_F(ValidatorTest, AssignCompatibleTypes_Pass) {
  // var a :i32 = 2;
  // a = 2
  auto* var = Var("a", ast::StorageClass::kNone, ty.i32(), Expr(2),
                  ast::VariableDecorationList{});

  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* assign = create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs);
  RegisterVariable(var);
  EXPECT_TRUE(td()->DetermineResultType(assign)) << td()->error();
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateAssign(assign)) << v.error();
}

TEST_F(ValidatorTest, AssignCompatibleTypesThroughAlias_Pass) {
  // alias myint = i32;
  // var a :myint = 2;
  // a = 2
  auto* myint = ty.alias("myint", ty.i32());
  auto* var = Var("a", ast::StorageClass::kNone, myint, Expr(2),
                  ast::VariableDecorationList{});

  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* assign = create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs);
  RegisterVariable(var);
  EXPECT_TRUE(td()->DetermineResultType(assign)) << td()->error();
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateAssign(assign)) << v.error();
}

TEST_F(ValidatorTest, AssignCompatibleTypesInferRHSLoad_Pass) {
  // var a :i32 = 2;
  // var b :i32 = 3;
  // a = b;
  auto* var_a = Var("a", ast::StorageClass::kNone, ty.i32(), Expr(2),
                    ast::VariableDecorationList{});
  auto* var_b = Var("b", ast::StorageClass::kNone, ty.i32(), Expr(3),
                    ast::VariableDecorationList{});

  auto* lhs = Expr("a");
  auto* rhs = Expr("b");

  auto* assign = create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs);
  RegisterVariable(var_a);
  RegisterVariable(var_b);
  EXPECT_TRUE(td()->DetermineResultType(assign)) << td()->error();
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateAssign(assign)) << v.error();
}

TEST_F(ValidatorTest, AssignThroughPointer_Pass) {
  // var a :i32;
  // const b : ptr<function,i32> = a;
  // b = 2;
  const auto func = ast::StorageClass::kFunction;
  auto* var_a = Var("a", func, ty.i32(), Expr(2), {});
  auto* var_b = Const("b", ast::StorageClass::kNone, ty.pointer<int>(func),
                      Expr("a"), {});

  auto* lhs = Expr("b");
  auto* rhs = Expr(2);

  auto* assign = create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs);
  RegisterVariable(var_a);
  RegisterVariable(var_b);
  EXPECT_TRUE(td()->DetermineResultType(assign)) << td()->error();
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateAssign(assign)) << v.error();
}

TEST_F(ValidatorTest, AssignIncompatibleTypes_Fail) {
  // {
  //  var a :i32 = 2;
  //  a = 2.3;
  // }

  auto* var = Var("a", ast::StorageClass::kNone, ty.i32(), Expr(2),
                  ast::VariableDecorationList{});

  auto* lhs = Expr("a");
  auto* rhs = Expr(2.3f);

  auto* assign = create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs);
  RegisterVariable(var);
  EXPECT_TRUE(td()->DetermineResultType(assign)) << td()->error();
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.ValidateAssign(assign));
  ASSERT_TRUE(v.has_error());
  // TODO(sarahM0): figure out what should be the error number.
  EXPECT_EQ(v.error(),
            "12:34 v-000x: invalid assignment: can't assign value of type "
            "'__f32' to '__i32'");
}

TEST_F(ValidatorTest, AssignThroughPointerWrongeStoreType_Fail) {
  // var a :f32;
  // const b : ptr<function,f32> = a;
  // b = 2;
  const auto priv = ast::StorageClass::kFunction;
  auto* var_a = Var("a", priv, ty.f32(), Expr(2), {});
  auto* var_b = Const("b", ast::StorageClass::kNone, ty.pointer<float>(priv),
                      Expr("a"), {});

  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* assign = create<ast::AssignmentStatement>(
      Source{Source::Location{12, 34}}, lhs, rhs);
  RegisterVariable(var_a);
  RegisterVariable(var_b);
  EXPECT_TRUE(td()->DetermineResultType(assign)) << td()->error();
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.ValidateAssign(assign));
  EXPECT_EQ(v.error(),
            "12:34 v-000x: invalid assignment: can't assign value of type "
            "'__i32' to '__f32'");
}

TEST_F(ValidatorTest, AssignCompatibleTypesInBlockStatement_Pass) {
  // {
  //  var a :i32 = 2;
  //  a = 2
  // }
  auto* var = Var("a", ast::StorageClass::kNone, ty.i32(), Expr(2),
                  ast::VariableDecorationList{});

  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::AssignmentStatement>(Source{Source::Location{12, 34}}, lhs,
                                       rhs),
  });

  EXPECT_TRUE(td()->DetermineStatements(body)) << td()->error();
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateStatements(body)) << v.error();
}

TEST_F(ValidatorTest, AssignIncompatibleTypesInBlockStatement_Fail) {
  // {
  //  var a :i32 = 2;
  //  a = 2.3;
  // }

  auto* var = Var("a", ast::StorageClass::kNone, ty.i32(), Expr(2),
                  ast::VariableDecorationList{});

  auto* lhs = Expr("a");
  auto* rhs = Expr(2.3f);

  auto* block = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::AssignmentStatement>(Source{Source::Location{12, 34}}, lhs,
                                       rhs),
  });

  EXPECT_TRUE(td()->DetermineStatements(block)) << td()->error();
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.ValidateStatements(block));
  ASSERT_TRUE(v.has_error());
  // TODO(sarahM0): figure out what should be the error number.
  EXPECT_EQ(v.error(),
            "12:34 v-000x: invalid assignment: can't assign value of type "
            "'__f32' to '__i32'");
}

TEST_F(ValidatorTest, AssignIncompatibleTypesInNestedBlockStatement_Fail) {
  // {
  //  {
  //   var a :i32 = 2;
  //   a = 2.3;
  //  }
  // }

  auto* var = Var("a", ast::StorageClass::kNone, ty.i32(), Expr(2),
                  ast::VariableDecorationList{});

  auto* lhs = Expr("a");
  auto* rhs = Expr(2.3f);

  auto* inner_block = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::AssignmentStatement>(Source{Source::Location{12, 34}}, lhs,
                                       rhs),
  });

  auto* outer_block = create<ast::BlockStatement>(ast::StatementList{
      inner_block,
  });

  EXPECT_TRUE(td()->DetermineStatements(outer_block)) << td()->error();
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.ValidateStatements(outer_block));
  ASSERT_TRUE(v.has_error());
  // TODO(sarahM0): figure out what should be the error number.
  EXPECT_EQ(v.error(),
            "12:34 v-000x: invalid assignment: can't assign value of type "
            "'__f32' to '__i32'");
}

TEST_F(ValidatorTest, GlobalVariableWithStorageClass_Pass) {
  // var<in> gloabl_var: f32;
  AST().AddGlobalVariable(Var(Source{Source::Location{12, 34}}, "global_var",
                              ast::StorageClass::kInput, ty.f32(), nullptr,
                              ast::VariableDecorationList{}));

  ValidatorImpl& v = Build();
  const Program* program = v.program();

  EXPECT_TRUE(v.ValidateGlobalVariables(program->AST().GlobalVariables()))
      << v.error();
}

TEST_F(ValidatorTest, GlobalVariableNoStorageClass_Fail) {
  // var gloabl_var: f32;
  AST().AddGlobalVariable(Var(Source{Source::Location{12, 34}}, "global_var",
                              ast::StorageClass::kNone, ty.f32(), nullptr,
                              ast::VariableDecorationList{}));
  EXPECT_TRUE(td()->Determine()) << td()->error();

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(v.error(),
            "12:34 v-0022: global variables must have a storage class");
}

TEST_F(ValidatorTest, GlobalConstantWithStorageClass_Fail) {
  // const<in> gloabl_var: f32;
  AST().AddGlobalVariable(Const(Source{Source::Location{12, 34}}, "global_var",
                                ast::StorageClass::kInput, ty.f32(), nullptr,
                                ast::VariableDecorationList{}));
  EXPECT_TRUE(td()->Determine()) << td()->error();

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(
      v.error(),
      "12:34 v-global01: global constants shouldn't have a storage class");
}

TEST_F(ValidatorTest, GlobalConstNoStorageClass_Pass) {
  // const gloabl_var: f32;
  AST().AddGlobalVariable(Const(Source{Source::Location{12, 34}}, "global_var",
                                ast::StorageClass::kNone, ty.f32(), nullptr,
                                ast::VariableDecorationList{}));
  EXPECT_TRUE(td()->Determine()) << td()->error();

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate()) << v.error();
}

TEST_F(ValidatorTest, UsingUndefinedVariableGlobalVariable_Fail) {
  // var global_var: f32 = 2.1;
  // fn my_func() -> f32 {
  //   not_global_var = 3.14f;
  // }
  AST().AddGlobalVariable(Var("global_var", ast::StorageClass::kPrivate,
                              ty.f32(), Expr(2.1f),
                              ast::VariableDecorationList{}));

  SetSource(Source{Source::Location{12, 34}});
  auto* lhs = Expr("not_global_var");
  auto* rhs = Expr(3.14f);

  Func("my_func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           create<ast::AssignmentStatement>(Source{Source::Location{12, 34}},
                                            lhs, rhs),
       },
       ast::FunctionDecorationList{});

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(v.error(), "12:34 v-0006: 'not_global_var' is not declared");
}

TEST_F(ValidatorTest, UsingUndefinedVariableGlobalVariable_Pass) {
  // var global_var: f32 = 2.1;
  // fn my_func() -> void {
  //   global_var = 3.14;
  //   return;
  // }

  AST().AddGlobalVariable(Var("global_var", ast::StorageClass::kPrivate,
                              ty.f32(), Expr(2.1f),
                              ast::VariableDecorationList{}));

  Func("my_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::AssignmentStatement>(Source{Source::Location{12, 34}},
                                            Expr("global_var"), Expr(3.14f)),
           create<ast::ReturnStatement>(),
       },
       ast::FunctionDecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.Validate()) << v.error();
}

TEST_F(ValidatorTest, UsingUndefinedVariableInnerScope_Fail) {
  // {
  //   if (true) { var a : f32 = 2.0; }
  //   a = 3.14;
  // }
  auto* var = Var("a", ast::StorageClass::kNone, ty.f32(), Expr(2.0f),
                  ast::VariableDecorationList{});

  auto* cond = Expr(true);
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
  });

  SetSource(Source{Source::Location{12, 34}});
  auto* lhs = Expr("a");
  auto* rhs = Expr(3.14f);

  auto* outer_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::IfStatement>(cond, body, ast::ElseStatementList{}),
      create<ast::AssignmentStatement>(Source{Source::Location{12, 34}}, lhs,
                                       rhs),
  });

  EXPECT_TRUE(td()->DetermineStatements(outer_body)) << td()->error();
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.ValidateStatements(outer_body));
  EXPECT_EQ(v.error(), "12:34 v-0006: 'a' is not declared");
}

TEST_F(ValidatorTest, UsingUndefinedVariableOuterScope_Pass) {
  // {
  //   var a : f32 = 2.0;
  //   if (true) { a = 3.14; }
  // }
  auto* var = Var("a", ast::StorageClass::kNone, ty.f32(), Expr(2.0f),
                  ast::VariableDecorationList{});

  SetSource(Source{Source::Location{12, 34}});
  auto* lhs = Expr("a");
  auto* rhs = Expr(3.14f);

  auto* cond = Expr(true);
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::AssignmentStatement>(Source{Source::Location{12, 34}}, lhs,
                                       rhs),
  });

  auto* outer_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::IfStatement>(cond, body, ast::ElseStatementList{}),
  });

  EXPECT_TRUE(td()->DetermineStatements(outer_body)) << td()->error();
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateStatements(outer_body)) << v.error();
}

TEST_F(ValidatorTest, UsingUndefinedVariableDifferentScope_Fail) {
  // {
  //  { var a : f32 = 2.0; }
  //  { a = 3.14; }
  // }
  auto* var = Var("a", ast::StorageClass::kNone, ty.f32(), Expr(2.0f),
                  ast::VariableDecorationList{});
  auto* first_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
  });

  SetSource(Source{Source::Location{12, 34}});
  auto* lhs = Expr("a");
  auto* rhs = Expr(3.14f);
  auto* second_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::AssignmentStatement>(Source{Source::Location{12, 34}}, lhs,
                                       rhs),
  });

  auto* outer_body = create<ast::BlockStatement>(ast::StatementList{
      first_body,
      second_body,
  });

  EXPECT_TRUE(td()->DetermineStatements(outer_body)) << td()->error();
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.ValidateStatements(outer_body));
  EXPECT_EQ(v.error(), "12:34 v-0006: 'a' is not declared");
}

TEST_F(ValidatorTest, GlobalVariableUnique_Pass) {
  // var global_var0 : f32 = 0.1;
  // var global_var1 : i32 = 0;
  auto* var0 = Var("global_var0", ast::StorageClass::kPrivate, ty.f32(),
                   Expr(0.1f), ast::VariableDecorationList{});
  AST().AddGlobalVariable(var0);

  auto* var1 = Var(Source{Source::Location{12, 34}}, "global_var1",
                   ast::StorageClass::kPrivate, ty.f32(), Expr(0),
                   ast::VariableDecorationList{});
  AST().AddGlobalVariable(var1);

  ValidatorImpl& v = Build();
  const Program* program = v.program();

  EXPECT_TRUE(v.ValidateGlobalVariables(program->AST().GlobalVariables()))
      << v.error();
}

TEST_F(ValidatorTest, GlobalVariableNotUnique_Fail) {
  // var global_var : f32 = 0.1;
  // var global_var : i32 = 0;
  auto* var0 = Var("global_var", ast::StorageClass::kPrivate, ty.f32(),
                   Expr(0.1f), ast::VariableDecorationList{});
  AST().AddGlobalVariable(var0);

  auto* var1 = Var(Source{Source::Location{12, 34}}, "global_var",
                   ast::StorageClass::kPrivate, ty.i32(), Expr(0),
                   ast::VariableDecorationList{});
  AST().AddGlobalVariable(var1);

  ValidatorImpl& v = Build();
  const Program* program = v.program();

  EXPECT_FALSE(v.ValidateGlobalVariables(program->AST().GlobalVariables()));
  EXPECT_EQ(v.error(),
            "12:34 v-0011: redeclared global identifier 'global_var'");
}

TEST_F(ValidatorTest, AssignToConstant_Fail) {
  // {
  //  const a :i32 = 2;
  //  a = 2
  // }
  auto* var = Const("a", ast::StorageClass::kNone, ty.i32(), Expr(2),
                    ast::VariableDecorationList{});

  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::AssignmentStatement>(Source{Source::Location{12, 34}}, lhs,
                                       rhs),
  });

  EXPECT_TRUE(td()->DetermineStatements(body)) << td()->error();
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.ValidateStatements(body));
  EXPECT_EQ(v.error(), "12:34 v-0021: cannot re-assign a constant: 'a'");
}

TEST_F(ValidatorTest, GlobalVariableFunctionVariableNotUnique_Fail) {
  // var a: f32 = 2.1;
  // fn my_func -> void {
  //   var a: f32 = 2.0;
  //   return 0;
  // }

  auto* global_var = Var("a", ast::StorageClass::kPrivate, ty.f32(), Expr(2.1f),
                         ast::VariableDecorationList{});
  AST().AddGlobalVariable(global_var);

  auto* var = Var("a", ast::StorageClass::kNone, ty.f32(), Expr(2.0f),
                  ast::VariableDecorationList{});

  Func("my_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(Source{Source::Location{12, 34}},
                                              var),
       },
       ast::FunctionDecorationList{});

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate()) << v.error();
  EXPECT_EQ(v.error(), "12:34 v-0013: redeclared identifier 'a'");
}

TEST_F(ValidatorTest, RedeclaredIndentifier_Fail) {
  // fn my_func() -> void {
  //  var a :i32 = 2;
  //  var a :f21 = 2.0;
  // }
  auto* var = Var("a", ast::StorageClass::kNone, ty.i32(), Expr(2),
                  ast::VariableDecorationList{});

  auto* var_a_float = Var("a", ast::StorageClass::kNone, ty.f32(), Expr(0.1f),
                          ast::VariableDecorationList{});

  Func("my_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::VariableDeclStatement>(Source{Source::Location{12, 34}},
                                              var_a_float),
       },
       ast::FunctionDecorationList{});

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(v.error(), "12:34 v-0014: redeclared identifier 'a'");
}

TEST_F(ValidatorTest, RedeclaredIdentifierInnerScope_Pass) {
  // {
  // if (true) { var a : f32 = 2.0; }
  // var a : f32 = 3.14;
  // }
  auto* var = Var("a", ast::StorageClass::kNone, ty.f32(), Expr(2.0f),
                  ast::VariableDecorationList{});

  auto* cond = Expr(true);
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
  });

  auto* var_a_float = Var("a", ast::StorageClass::kNone, ty.f32(), Expr(3.1f),
                          ast::VariableDecorationList{});

  auto* outer_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::IfStatement>(cond, body, ast::ElseStatementList{}),
      create<ast::VariableDeclStatement>(Source{Source::Location{12, 34}},
                                         var_a_float),
  });

  EXPECT_TRUE(td()->DetermineStatements(outer_body)) << td()->error();

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateStatements(outer_body)) << v.error();
}

TEST_F(ValidatorTest, DISABLED_RedeclaredIdentifierInnerScope_False) {
  // TODO(sarahM0): remove DISABLED after implementing ValidateIfStatement
  // and it should just work
  // {
  // var a : f32 = 3.14;
  // if (true) { var a : f32 = 2.0; }
  // }
  auto* var_a_float = Var("a", ast::StorageClass::kNone, ty.f32(), Expr(3.1f),
                          ast::VariableDecorationList{});

  auto* var = Var("a", ast::StorageClass::kNone, ty.f32(), Expr(2.0f),
                  ast::VariableDecorationList{});

  auto* cond = Expr(true);
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(Source{Source::Location{12, 34}}, var),
  });

  auto* outer_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var_a_float),
      create<ast::IfStatement>(cond, body, ast::ElseStatementList{}),
  });

  EXPECT_TRUE(td()->DetermineStatements(outer_body)) << td()->error();

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.ValidateStatements(outer_body));
  EXPECT_EQ(v.error(), "12:34 v-0014: redeclared identifier 'a'");
}

TEST_F(ValidatorTest, RedeclaredIdentifierInnerScopeBlock_Pass) {
  // {
  //  { var a : f32; }
  //  var a : f32;
  // }
  auto* var_inner = Var("a", ast::StorageClass::kNone, ty.f32());
  auto* inner = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(Source{Source::Location{12, 34}},
                                         var_inner),
  });

  auto* var_outer = Var("a", ast::StorageClass::kNone, ty.f32());
  auto* outer_body = create<ast::BlockStatement>(ast::StatementList{
      inner,
      create<ast::VariableDeclStatement>(var_outer),
  });

  EXPECT_TRUE(td()->DetermineStatements(outer_body)) << td()->error();

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateStatements(outer_body));
}

TEST_F(ValidatorTest, RedeclaredIdentifierInnerScopeBlock_Fail) {
  // {
  //  var a : f32;
  //  { var a : f32; }
  // }
  auto* var_inner = Var("a", ast::StorageClass::kNone, ty.f32());
  auto* inner = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(Source{Source::Location{12, 34}},
                                         var_inner),
  });

  auto* var_outer = Var("a", ast::StorageClass::kNone, ty.f32());
  auto* outer_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var_outer),
      inner,
  });

  EXPECT_TRUE(td()->DetermineStatements(outer_body)) << td()->error();

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.ValidateStatements(outer_body));
  EXPECT_EQ(v.error(), "12:34 v-0014: redeclared identifier 'a'");
}

TEST_F(ValidatorTest, RedeclaredIdentifierDifferentFunctions_Pass) {
  // func0 { var a : f32 = 2.0; return; }
  // func1 { var a : f32 = 3.0; return; }
  auto* var0 = Var("a", ast::StorageClass::kNone, ty.f32(), Expr(2.0f),
                   ast::VariableDecorationList{});

  auto* var1 = Var("a", ast::StorageClass::kNone, ty.void_(), Expr(1.0f),
                   ast::VariableDecorationList{});

  Func("func0", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(Source{Source::Location{12, 34}},
                                              var0),
           create<ast::ReturnStatement>(),
       },
       ast::FunctionDecorationList{});

  Func("func1", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(Source{Source::Location{13, 34}},
                                              var1),
           create<ast::ReturnStatement>(),
       },
       ast::FunctionDecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.Validate()) << v.error();
}

TEST_F(ValidatorTest, VariableDeclNoConstructor_Pass) {
  // {
  // var a :i32;
  // a = 2;
  // }
  auto* var = Var("a", ast::StorageClass::kNone, ty.i32(), nullptr,
                  ast::VariableDecorationList{});

  td()->RegisterVariableForTesting(var);
  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::AssignmentStatement>(Source{Source::Location{12, 34}}, lhs,
                                       rhs),
  });

  EXPECT_TRUE(td()->DetermineStatements(body)) << td()->error();
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateStatements(body)) << v.error();
}

TEST_F(ValidatorTest, IsStorable_Void) {
  auto* void_ty = ty.void_();

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.IsStorable(void_ty));
}

TEST_F(ValidatorTest, IsStorable_Scalar) {
  auto* bool_ = ty.bool_();
  auto* i32 = ty.i32();
  auto* u32 = ty.u32();
  auto* f32 = ty.f32();

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.IsStorable(bool_));
  EXPECT_TRUE(v.IsStorable(i32));
  EXPECT_TRUE(v.IsStorable(u32));
  EXPECT_TRUE(v.IsStorable(f32));
}

TEST_F(ValidatorTest, IsStorable_Vector) {
  auto* vec2_i32 = ty.vec2<int>();
  auto* vec3_i32 = ty.vec3<int>();
  auto* vec4_i32 = ty.vec4<int>();
  auto* vec2_u32 = ty.vec2<unsigned>();
  auto* vec3_u32 = ty.vec3<unsigned>();
  auto* vec4_u32 = ty.vec4<unsigned>();
  auto* vec2_f32 = ty.vec2<float>();
  auto* vec3_f32 = ty.vec3<float>();
  auto* vec4_f32 = ty.vec4<float>();

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.IsStorable(vec2_i32));
  EXPECT_TRUE(v.IsStorable(vec3_i32));
  EXPECT_TRUE(v.IsStorable(vec4_i32));
  EXPECT_TRUE(v.IsStorable(vec2_u32));
  EXPECT_TRUE(v.IsStorable(vec3_u32));
  EXPECT_TRUE(v.IsStorable(vec4_u32));
  EXPECT_TRUE(v.IsStorable(vec2_f32));
  EXPECT_TRUE(v.IsStorable(vec3_f32));
  EXPECT_TRUE(v.IsStorable(vec4_f32));
}

TEST_F(ValidatorTest, IsStorable_Matrix) {
  auto* mat2x2 = ty.mat2x2<float>();
  auto* mat2x3 = ty.mat2x3<float>();
  auto* mat2x4 = ty.mat2x4<float>();
  auto* mat3x2 = ty.mat3x2<float>();
  auto* mat3x3 = ty.mat3x3<float>();
  auto* mat3x4 = ty.mat3x4<float>();
  auto* mat4x2 = ty.mat4x2<float>();
  auto* mat4x3 = ty.mat4x3<float>();
  auto* mat4x4 = ty.mat4x4<float>();

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.IsStorable(mat2x2));
  EXPECT_TRUE(v.IsStorable(mat2x3));
  EXPECT_TRUE(v.IsStorable(mat2x4));
  EXPECT_TRUE(v.IsStorable(mat3x2));
  EXPECT_TRUE(v.IsStorable(mat3x3));
  EXPECT_TRUE(v.IsStorable(mat3x4));
  EXPECT_TRUE(v.IsStorable(mat4x2));
  EXPECT_TRUE(v.IsStorable(mat4x3));
  EXPECT_TRUE(v.IsStorable(mat4x4));
}

TEST_F(ValidatorTest, IsStorable_Pointer) {
  auto* ptr_ty = ty.pointer<int>(ast::StorageClass::kPrivate);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.IsStorable(ptr_ty));
}

TEST_F(ValidatorTest, IsStorable_AliasVoid) {
  auto* alias = ty.alias("myalias", ty.void_());

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.IsStorable(alias));
}

TEST_F(ValidatorTest, IsStorable_AliasI32) {
  auto* alias = ty.alias("myalias", ty.i32());

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.IsStorable(alias));
}

TEST_F(ValidatorTest, IsStorable_ArraySizedOfStorable) {
  auto* arr = ty.array(ty.i32(), 5);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.IsStorable(arr));
}

TEST_F(ValidatorTest, IsStorable_ArraySizedOfNonStorable) {
  auto* arr = ty.array(ty.void_(), 5);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.IsStorable(arr));
}

TEST_F(ValidatorTest, IsStorable_ArrayUnsizedOfStorable) {
  auto* arr = ty.array<int>();

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.IsStorable(arr));
}

TEST_F(ValidatorTest, IsStorable_ArrayUnsizedOfNonStorable) {
  auto* arr = ty.array<void>();

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.IsStorable(arr));
}

TEST_F(ValidatorTest, IsStorable_Struct_AllMembersStorable) {
  ast::StructMemberList members{Member("a", ty.i32()), Member("b", ty.f32())};
  auto* s = create<ast::Struct>(Source{}, members, ast::StructDecorationList{});
  auto* s_ty = ty.struct_("mystruct", s);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.IsStorable(s_ty));
}

TEST_F(ValidatorTest, IsStorable_Struct_SomeMembersNonStorable) {
  auto* ptr_ty = ty.pointer<int>(ast::StorageClass::kPrivate);
  ast::StructMemberList members{Member("a", ty.i32()), Member("b", ptr_ty)};
  auto* s = create<ast::Struct>(Source{}, members, ast::StructDecorationList{});
  auto* s_ty = ty.struct_("mystruct", s);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.IsStorable(s_ty));
}

}  // namespace
}  // namespace tint
