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

#include "src/ast/if_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/validator/validator_test_helper.h"

namespace tint {
namespace {

class ValidatorTest : public ValidatorTestHelper, public testing::Test {};


TEST_F(ValidatorTest, GlobalVariableWithStorageClass_Pass) {
  // var<in> global_var: f32;
  auto* var = Global(Source{Source::Location{12, 34}}, "global_var", ty.f32(),
                     ast::StorageClass::kInput);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateGlobalVariable(var)) << v.error();
}

TEST_F(ValidatorTest, GlobalVariableNoStorageClass_Fail) {
  // var global_var: f32;
  Global(Source{Source::Location{12, 34}}, "global_var", ty.f32(),
         ast::StorageClass::kNone);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(v.error(),
            "12:34 v-0022: global variables must have a storage class");
}

TEST_F(ValidatorTest, GlobalConstantWithStorageClass_Fail) {
  // const<in> global_var: f32;
  AST().AddGlobalVariable(create<ast::Variable>(
      Source{Source::Location{12, 34}}, Symbols().Register("global_var"),
      ast::StorageClass::kInput, ty.f32(), true, nullptr,
      ast::DecorationList{}));

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(
      v.error(),
      "12:34 v-global01: global constants shouldn't have a storage class");
}

TEST_F(ValidatorTest, GlobalConstNoStorageClass_Pass) {
  // const global_var: f32;
  GlobalConst(Source{Source::Location{12, 34}}, "global_var", ty.f32());

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate()) << v.error();
}

TEST_F(ValidatorTest, GlobalVariableUnique_Pass) {
  // var global_var0 : f32 = 0.1;
  // var global_var1 : i32 = 0;
  auto* var0 =
      Global("global_var0", ty.f32(), ast::StorageClass::kPrivate, Expr(0.1f));

  auto* var1 = Global(Source{Source::Location{12, 34}}, "global_var1", ty.f32(),
                      ast::StorageClass::kPrivate, Expr(0));

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateGlobalVariable(var0)) << v.error();
  EXPECT_TRUE(v.ValidateGlobalVariable(var1)) << v.error();
}

TEST_F(ValidatorTest, GlobalVariableNotUnique_Fail) {
  // var global_var : f32 = 0.1;
  // var global_var : i32 = 0;
  Global("global_var", ty.f32(), ast::StorageClass::kPrivate, Expr(0.1f));

  Global(Source{Source::Location{12, 34}}, "global_var", ty.i32(),
         ast::StorageClass::kPrivate, Expr(0));

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(v.error(),
            "12:34 v-0011: redeclared global identifier 'global_var'");
}

TEST_F(ValidatorTest, GlobalVariableFunctionVariableNotUnique_Pass) {
  // fn my_func -> void {
  //   var a: f32 = 2.0;
  // }
  // var a: f32 = 2.1;

  auto* var = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));

  Func("my_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex)});

  Global("a", ty.f32(), ast::StorageClass::kPrivate, Expr(2.1f));

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.Validate()) << v.error();
}

TEST_F(ValidatorTest, GlobalVariableFunctionVariableNotUnique_Fail) {
  // var a: f32 = 2.1;
  // fn my_func -> void {
  //   var a: f32 = 2.0;
  //   return 0;
  // }

  Global("a", ty.f32(), ast::StorageClass::kPrivate, Expr(2.1f));

  auto* var = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));

  Func("my_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(Source{Source::Location{12, 34}},
                                              var),
       },
       ast::DecorationList{});

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate()) << v.error();
  EXPECT_EQ(v.error(), "12:34 v-0013: redeclared identifier 'a'");
}

TEST_F(ValidatorTest, RedeclaredIdentifier_Fail) {
  // fn my_func() -> void {
  //  var a :i32 = 2;
  //  var a :f21 = 2.0;
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  auto* var_a_float = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(0.1f));

  Func("my_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::VariableDeclStatement>(Source{Source::Location{12, 34}},
                                              var_a_float),
       },
       ast::DecorationList{});

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(v.error(), "12:34 v-0014: redeclared identifier 'a'");
}

TEST_F(ValidatorTest, RedeclaredIdentifierInnerScope_Pass) {
  // {
  // if (true) { var a : f32 = 2.0; }
  // var a : f32 = 3.14;
  // }
  auto* var = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));

  auto* cond = Expr(true);
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
  });

  auto* var_a_float = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(3.1f));

  auto* outer_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::IfStatement>(cond, body, ast::ElseStatementList{}),
      create<ast::VariableDeclStatement>(Source{Source::Location{12, 34}},
                                         var_a_float),
  });

  WrapInFunction(outer_body);

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
  auto* var_a_float = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(3.1f));

  auto* var = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));

  auto* cond = Expr(true);
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(Source{Source::Location{12, 34}}, var),
  });

  auto* outer_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var_a_float),
      create<ast::IfStatement>(cond, body, ast::ElseStatementList{}),
  });

  WrapInFunction(outer_body);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.ValidateStatements(outer_body));
  EXPECT_EQ(v.error(), "12:34 v-0014: redeclared identifier 'a'");
}

TEST_F(ValidatorTest, RedeclaredIdentifierInnerScopeBlock_Pass) {
  // {
  //  { var a : f32; }
  //  var a : f32;
  // }
  auto* var_inner = Var("a", ty.f32(), ast::StorageClass::kNone);
  auto* inner = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(Source{Source::Location{12, 34}},
                                         var_inner),
  });

  auto* var_outer = Var("a", ty.f32(), ast::StorageClass::kNone);
  auto* outer_body = create<ast::BlockStatement>(ast::StatementList{
      inner,
      create<ast::VariableDeclStatement>(var_outer),
  });

  WrapInFunction(outer_body);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateStatements(outer_body));
}

TEST_F(ValidatorTest, RedeclaredIdentifierInnerScopeBlock_Fail) {
  // {
  //  var a : f32;
  //  { var a : f32; }
  // }
  auto* var_inner = Var("a", ty.f32(), ast::StorageClass::kNone);
  auto* inner = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(Source{Source::Location{12, 34}},
                                         var_inner),
  });

  auto* var_outer = Var("a", ty.f32(), ast::StorageClass::kNone);
  auto* outer_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var_outer),
      inner,
  });

  WrapInFunction(outer_body);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.ValidateStatements(outer_body));
  EXPECT_EQ(v.error(), "12:34 v-0014: redeclared identifier 'a'");
}

TEST_F(ValidatorTest, RedeclaredIdentifierDifferentFunctions_Pass) {
  // func0 { var a : f32 = 2.0; return; }
  // func1 { var a : f32 = 3.0; return; }
  auto* var0 = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));

  auto* var1 = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(1.0f));

  Func("func0", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(Source{Source::Location{12, 34}},
                                              var0),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{});

  Func("func1", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(Source{Source::Location{13, 34}},
                                              var1),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.Validate()) << v.error();
}

TEST_F(ValidatorTest, VariableDeclNoConstructor_Pass) {
  // {
  // var a :i32;
  // a = 2;
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, nullptr);
  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::AssignmentStatement>(Source{Source::Location{12, 34}}, lhs,
                                       rhs),
  });

  WrapInFunction(body);

  ValidatorImpl& v = Build();

  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  EXPECT_TRUE(v.ValidateStatements(body)) << v.error();
}

}  // namespace
}  // namespace tint
