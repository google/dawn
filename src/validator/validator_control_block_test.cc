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
#include "src/ast/case_statement.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/switch_statement.h"
#include "src/ast/uint_literal.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/type/alias_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/u32_type.h"
#include "src/validator/validator_impl.h"
#include "src/validator/validator_test_helper.h"

namespace tint {
namespace {

class ValidateControlBlockTest : public ValidatorTestHelper,
                                 public testing::Test {};

TEST_F(ValidateControlBlockTest, SwitchSelectorExpressionNoneIntegerType_Fail) {
  // var a : f32 = 3.14;
  // switch (a) {
  //   default: {}
  // }
  auto* var = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(3.14f));

  ast::CaseStatementList body;
  auto* block_default = create<ast::BlockStatement>(ast::StatementList{});
  body.push_back(
      create<ast::CaseStatement>(ast::CaseSelectorList{}, block_default));

  auto* block = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::SwitchStatement>(Expr(Source{Source::Location{12, 34}}, "a"),
                                   body),
  });

  WrapInFunction(block);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.ValidateStatements(block));
  EXPECT_EQ(v.error(),
            "12:34 v-0025: switch statement selector expression must be "
            "of a scalar integer type");
}

TEST_F(ValidateControlBlockTest, SwitchWithoutDefault_Fail) {
  // var a : i32 = 2;
  // switch (a) {
  //   case 1: {}
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  ast::CaseSelectorList csl;
  csl.push_back(Literal(1));

  ast::CaseStatementList body;
  body.push_back(create<ast::CaseStatement>(
      csl, create<ast::BlockStatement>(ast::StatementList{})));

  auto* block = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::SwitchStatement>(Source{Source::Location{12, 34}}, Expr("a"),
                                   body),
  });

  WrapInFunction(block);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.ValidateStatements(block));
  EXPECT_EQ(v.error(),
            "12:34 v-0008: switch statement must have exactly one default "
            "clause");
}

TEST_F(ValidateControlBlockTest, SwitchWithTwoDefault_Fail) {
  // var a : i32 = 2;
  // switch (a) {
  //   default: {}
  //   case 1: {}
  //   default: {}
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  ast::CaseStatementList switch_body;
  ast::CaseSelectorList default_csl_1;
  auto* block_default_1 = create<ast::BlockStatement>(ast::StatementList{});
  switch_body.push_back(
      create<ast::CaseStatement>(default_csl_1, block_default_1));

  ast::CaseSelectorList csl_case_1;
  csl_case_1.push_back(Literal(1));
  auto* block_case_1 = create<ast::BlockStatement>(ast::StatementList{});
  switch_body.push_back(create<ast::CaseStatement>(csl_case_1, block_case_1));

  ast::CaseSelectorList default_csl_2;
  auto* block_default_2 = create<ast::BlockStatement>(ast::StatementList{});
  switch_body.push_back(
      create<ast::CaseStatement>(default_csl_2, block_default_2));

  auto* block = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::SwitchStatement>(Source{Source::Location{12, 34}}, Expr("a"),
                                   switch_body),
  });

  WrapInFunction(block);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.ValidateStatements(block));
  EXPECT_EQ(v.error(),
            "12:34 v-0008: switch statement must have exactly one default "
            "clause");
}

TEST_F(ValidateControlBlockTest,
       SwitchConditionTypeMustMatchSelectorType2_Fail) {
  // var a : i32 = 2;
  // switch (a) {
  //   case 1: {}
  //   default: {}
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  ast::CaseStatementList switch_body;
  ast::CaseSelectorList csl;
  csl.push_back(create<ast::UintLiteral>(ty.u32(), 1));
  switch_body.push_back(create<ast::CaseStatement>(
      Source{Source::Location{12, 34}}, csl,
      create<ast::BlockStatement>(ast::StatementList{})));

  ast::CaseSelectorList default_csl;
  auto* block_default = create<ast::BlockStatement>(ast::StatementList{});
  switch_body.push_back(create<ast::CaseStatement>(default_csl, block_default));

  auto* block = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::SwitchStatement>(Expr("a"), switch_body),
  });
  WrapInFunction(block);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.ValidateStatements(block));
  EXPECT_EQ(v.error(),
            "12:34 v-0026: the case selector values must have the same "
            "type as the selector expression.");
}

TEST_F(ValidateControlBlockTest,
       SwitchConditionTypeMustMatchSelectorType_Fail) {
  // var a : u32 = 2;
  // switch (a) {
  //   case -1: {}
  //   default: {}
  // }
  auto* var = Var("a", ty.u32(), ast::StorageClass::kNone, Expr(2u));

  ast::CaseStatementList switch_body;
  ast::CaseSelectorList csl;
  csl.push_back(Literal(-1));
  switch_body.push_back(create<ast::CaseStatement>(
      Source{Source::Location{12, 34}}, csl,
      create<ast::BlockStatement>(ast::StatementList{})));

  ast::CaseSelectorList default_csl;
  auto* block_default = create<ast::BlockStatement>(ast::StatementList{});
  switch_body.push_back(create<ast::CaseStatement>(default_csl, block_default));

  auto* block = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::SwitchStatement>(Expr("a"), switch_body),
  });
  WrapInFunction(block);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.ValidateStatements(block));
  EXPECT_EQ(v.error(),
            "12:34 v-0026: the case selector values must have the same "
            "type as the selector expression.");
}

TEST_F(ValidateControlBlockTest, NonUniqueCaseSelectorValueUint_Fail) {
  // var a : u32 = 3;
  // switch (a) {
  //   case 0: {}
  //   case 2, 2: {}
  //   default: {}
  // }
  auto* var = Var("a", ty.u32(), ast::StorageClass::kNone, Expr(3u));

  ast::CaseStatementList switch_body;
  ast::CaseSelectorList csl_1;
  csl_1.push_back(create<ast::UintLiteral>(ty.u32(), 0));
  switch_body.push_back(create<ast::CaseStatement>(
      csl_1, create<ast::BlockStatement>(ast::StatementList{})));

  ast::CaseSelectorList csl_2;
  csl_2.push_back(create<ast::UintLiteral>(ty.u32(), 2));
  csl_2.push_back(create<ast::UintLiteral>(ty.u32(), 2));
  switch_body.push_back(create<ast::CaseStatement>(
      Source{Source::Location{12, 34}}, csl_2,
      create<ast::BlockStatement>(ast::StatementList{})));

  ast::CaseSelectorList default_csl;
  auto* block_default = create<ast::BlockStatement>(ast::StatementList{});
  switch_body.push_back(create<ast::CaseStatement>(default_csl, block_default));

  auto* block = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::SwitchStatement>(Expr("a"), switch_body),
  });
  WrapInFunction(block);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.ValidateStatements(block));
  EXPECT_EQ(v.error(),
            "12:34 v-0027: a literal value must not appear more than once "
            "in the case selectors for a switch statement: '2'");
}

TEST_F(ValidateControlBlockTest, NonUniqueCaseSelectorValueSint_Fail) {
  // var a : i32 = 2;
  // switch (a) {
  //   case 10: {}
  //   case 0,1,2,10: {}
  //   default: {}
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  ast::CaseStatementList switch_body;
  ast::CaseSelectorList csl_1;
  csl_1.push_back(Literal(10));
  switch_body.push_back(create<ast::CaseStatement>(
      csl_1, create<ast::BlockStatement>(ast::StatementList{})));

  ast::CaseSelectorList csl_2;
  csl_2.push_back(Literal(0));
  csl_2.push_back(Literal(1));
  csl_2.push_back(Literal(2));
  csl_2.push_back(Literal(10));
  switch_body.push_back(create<ast::CaseStatement>(
      Source{Source::Location{12, 34}}, csl_2,
      create<ast::BlockStatement>(ast::StatementList{})));

  ast::CaseSelectorList default_csl;
  auto* block_default = create<ast::BlockStatement>(ast::StatementList{});
  switch_body.push_back(create<ast::CaseStatement>(default_csl, block_default));

  auto* block = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::SwitchStatement>(Expr("a"), switch_body),
  });
  WrapInFunction(block);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.ValidateStatements(block));
  EXPECT_EQ(v.error(),
            "12:34 v-0027: a literal value must not appear more than once in "
            "the case selectors for a switch statement: '10'");
}

TEST_F(ValidateControlBlockTest, LastClauseLastStatementIsFallthrough_Fail) {
  // var a : i32 = 2;
  // switch (a) {
  //   default: { fallthrough; }
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  ast::CaseSelectorList default_csl;
  auto* block_default = create<ast::BlockStatement>(

      ast::StatementList{
          create<ast::FallthroughStatement>(Source{Source::Location{12, 34}}),
      });
  ast::CaseStatementList body;
  body.push_back(create<ast::CaseStatement>(default_csl, block_default));

  auto* block = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::SwitchStatement>(Expr("a"), body),
  });
  WrapInFunction(block);

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.ValidateStatements(block));
  EXPECT_EQ(v.error(),
            "12:34 v-0028: a fallthrough statement must not appear as the "
            "last statement in last clause of a switch");
}

TEST_F(ValidateControlBlockTest, SwitchCase_Pass) {
  // var a : i32 = 2;
  // switch (a) {
  //   default: {}
  //   case 5: {}
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  ast::CaseSelectorList default_csl;
  auto* block_default = create<ast::BlockStatement>(ast::StatementList{});
  ast::CaseStatementList body;
  body.push_back(create<ast::CaseStatement>(Source{Source::Location{12, 34}},
                                            default_csl, block_default));
  ast::CaseSelectorList case_csl;
  case_csl.push_back(Literal(5));
  auto* block_case = create<ast::BlockStatement>(ast::StatementList{});
  body.push_back(create<ast::CaseStatement>(case_csl, block_case));

  auto* block = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::SwitchStatement>(Expr("a"), body),
  });
  WrapInFunction(block);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateStatements(block)) << v.error();
}

TEST_F(ValidateControlBlockTest, SwitchCaseAlias_Pass) {
  // type MyInt = u32;
  // var v: MyInt;
  // switch(v){
  //   default: {}
  // }

  auto* my_int = ty.alias("MyInt", ty.u32());
  auto* var = Var("a", my_int, ast::StorageClass::kNone, Expr(2u));

  ast::CaseSelectorList default_csl;
  auto* block_default = create<ast::BlockStatement>(ast::StatementList{});
  ast::CaseStatementList body;
  body.push_back(create<ast::CaseStatement>(Source{Source::Location{12, 34}},
                                            default_csl, block_default));

  auto* block = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::SwitchStatement>(Expr("a"), body),
  });
  AST().AddConstructedType(my_int);

  WrapInFunction(block);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateStatements(block)) << v.error();
}

}  // namespace
}  // namespace tint
