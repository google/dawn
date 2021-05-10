// Copyright 2021 The Tint Authors.
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

#include "src/ast/fallthrough_statement.h"
#include "src/ast/switch_statement.h"
#include "src/resolver/resolver_test_helper.h"

namespace tint {
namespace {

class ResolverControlBlockValidationTest : public resolver::TestHelper,
                                           public testing::Test {};

TEST_F(ResolverControlBlockValidationTest,
       SwitchSelectorExpressionNoneIntegerType_Fail) {
  // var a : f32 = 3.14;
  // switch (a) {
  //   default: {}
  // }
  auto* var = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(3.14f));

  ast::CaseStatementList body;
  auto* block_default = Block();
  body.push_back(
      create<ast::CaseStatement>(ast::CaseSelectorList{}, block_default));

  auto* block =
      Block(Decl(var), create<ast::SwitchStatement>(
                           Expr(Source{Source::Location{12, 34}}, "a"), body));

  WrapInFunction(block);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-0025: switch statement selector expression must be "
            "of a scalar integer type");
}

TEST_F(ResolverControlBlockValidationTest, SwitchWithoutDefault_Fail) {
  // var a : i32 = 2;
  // switch (a) {
  //   case 1: {}
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  ast::CaseSelectorList csl;
  csl.push_back(Literal(1));

  ast::CaseStatementList body;
  body.push_back(create<ast::CaseStatement>(csl, Block()));

  auto* block =
      Block(Decl(var), create<ast::SwitchStatement>(
                           Source{Source::Location{12, 34}}, Expr("a"), body));

  WrapInFunction(block);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: switch statement must have a default clause");
}

TEST_F(ResolverControlBlockValidationTest, SwitchWithTwoDefault_Fail) {
  // var a : i32 = 2;
  // switch (a) {
  //   default: {}
  //   case 1: {}
  //   default: {}
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  ast::CaseStatementList switch_body;
  ast::CaseSelectorList default_csl_1;
  auto* block_default_1 = Block();
  switch_body.push_back(
      create<ast::CaseStatement>(default_csl_1, block_default_1));

  ast::CaseSelectorList csl_case_1;
  csl_case_1.push_back(Literal(1));
  auto* block_case_1 = Block();
  switch_body.push_back(create<ast::CaseStatement>(csl_case_1, block_case_1));

  ast::CaseSelectorList default_csl_2;
  auto* block_default_2 = Block();
  switch_body.push_back(create<ast::CaseStatement>(
      Source{Source::Location{12, 34}}, default_csl_2, block_default_2));

  auto* block =
      Block(Decl(var), create<ast::SwitchStatement>(Expr("a"), switch_body));

  WrapInFunction(block);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error v-0008: switch statement must have exactly one default "
      "clause");
}

TEST_F(ResolverControlBlockValidationTest,
       SwitchConditionTypeMustMatchSelectorType2_Fail) {
  // var a : u32 = 2;
  // switch (a) {
  //   case 1: {}
  //   default: {}
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  ast::CaseStatementList switch_body;
  ast::CaseSelectorList csl;
  csl.push_back(create<ast::UintLiteral>(1u));
  switch_body.push_back(create<ast::CaseStatement>(
      Source{Source::Location{12, 34}}, csl, Block()));

  ast::CaseSelectorList default_csl;
  auto* block_default = Block();
  switch_body.push_back(create<ast::CaseStatement>(default_csl, block_default));

  auto* block =
      Block(Decl(var), create<ast::SwitchStatement>(Expr("a"), switch_body));
  WrapInFunction(block);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-0026: the case selector values must have the same "
            "type as the selector expression.");
}

TEST_F(ResolverControlBlockValidationTest,
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
      Source{Source::Location{12, 34}}, csl, Block()));

  ast::CaseSelectorList default_csl;
  auto* block_default = Block();
  switch_body.push_back(create<ast::CaseStatement>(default_csl, block_default));

  auto* block =
      Block(Decl(var), create<ast::SwitchStatement>(Expr("a"), switch_body));
  WrapInFunction(block);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-0026: the case selector values must have the same "
            "type as the selector expression.");
}

TEST_F(ResolverControlBlockValidationTest,
       NonUniqueCaseSelectorValueUint_Fail) {
  // var a : u32 = 3;
  // switch (a) {
  //   case 0: {}
  //   case 2, 2: {}
  //   default: {}
  // }
  auto* var = Var("a", ty.u32(), ast::StorageClass::kNone, Expr(3u));

  ast::CaseStatementList switch_body;
  ast::CaseSelectorList csl_1;
  csl_1.push_back(create<ast::UintLiteral>(0u));
  switch_body.push_back(create<ast::CaseStatement>(csl_1, Block()));

  ast::CaseSelectorList csl_2;
  csl_2.push_back(create<ast::UintLiteral>(2u));
  csl_2.push_back(create<ast::UintLiteral>(2u));
  switch_body.push_back(create<ast::CaseStatement>(
      Source{Source::Location{12, 34}}, csl_2, Block()));

  ast::CaseSelectorList default_csl;
  auto* block_default = Block();
  switch_body.push_back(create<ast::CaseStatement>(default_csl, block_default));

  auto* block =
      Block(Decl(var), create<ast::SwitchStatement>(Expr("a"), switch_body));
  WrapInFunction(block);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-0027: a literal value must not appear more than "
            "once in the case selectors for a switch statement: '2u'");
}

TEST_F(ResolverControlBlockValidationTest,
       NonUniqueCaseSelectorValueSint_Fail) {
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
  switch_body.push_back(create<ast::CaseStatement>(csl_1, Block()));

  ast::CaseSelectorList csl_2;
  csl_2.push_back(Literal(0));
  csl_2.push_back(Literal(1));
  csl_2.push_back(Literal(2));
  csl_2.push_back(Literal(10));
  switch_body.push_back(create<ast::CaseStatement>(
      Source{Source::Location{12, 34}}, csl_2, Block()));

  ast::CaseSelectorList default_csl;
  auto* block_default = Block();
  switch_body.push_back(create<ast::CaseStatement>(default_csl, block_default));

  auto* block =
      Block(Decl(var), create<ast::SwitchStatement>(Expr("a"), switch_body));
  WrapInFunction(block);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error v-0027: a literal value must not appear more than once in "
      "the case selectors for a switch statement: '10'");
}

TEST_F(ResolverControlBlockValidationTest,
       LastClauseLastStatementIsFallthrough_Fail) {
  // var a : i32 = 2;
  // switch (a) {
  //   default: { fallthrough; }
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  ast::CaseSelectorList default_csl;
  auto* block_default = Block(
      create<ast::FallthroughStatement>(Source{Source::Location{12, 34}}));
  ast::CaseStatementList body;
  body.push_back(create<ast::CaseStatement>(default_csl, block_default));

  auto* block = Block(Decl(var), create<ast::SwitchStatement>(Expr("a"), body));
  WrapInFunction(block);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error v-0028: a fallthrough statement must not appear as the "
      "last statement in last clause of a switch");
}

TEST_F(ResolverControlBlockValidationTest, SwitchCase_Pass) {
  // var a : i32 = 2;
  // switch (a) {
  //   default: {}
  //   case 5: {}
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  ast::CaseSelectorList default_csl;
  auto* block_default = Block();
  ast::CaseStatementList body;
  body.push_back(create<ast::CaseStatement>(Source{Source::Location{12, 34}},
                                            default_csl, block_default));
  ast::CaseSelectorList case_csl;
  case_csl.push_back(Literal(5));
  auto* block_case = Block();
  body.push_back(create<ast::CaseStatement>(case_csl, block_case));

  auto* block = Block(Decl(var), create<ast::SwitchStatement>(Expr("a"), body));
  WrapInFunction(block);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverControlBlockValidationTest, SwitchCaseAlias_Pass) {
  // type MyInt = u32;
  // var v: MyInt;
  // switch(v){
  //   default: {}
  // }

  auto* my_int = ty.alias("MyInt", ty.u32());
  auto* var = Var("a", my_int, ast::StorageClass::kNone, Expr(2u));

  ast::CaseSelectorList default_csl;
  auto* block_default = Block();
  ast::CaseStatementList body;
  body.push_back(create<ast::CaseStatement>(Source{Source::Location{12, 34}},
                                            default_csl, block_default));

  auto* block = Block(Decl(var), create<ast::SwitchStatement>(Expr("a"), body));
  AST().AddConstructedType(my_int);

  WrapInFunction(block);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace
}  // namespace tint
