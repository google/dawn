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
#include "src/ast/type/alias_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/uint_literal.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/type_determiner.h"
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
  ast::type::F32 f32;
  auto* var = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &f32,                      // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::SintLiteral>(Source{}, &f32, 3.14f)),  // constructor
      ast::VariableDecorationList{});                        // decorations

  auto* cond = create<ast::IdentifierExpression>(
      Source{Source::Location{12, 34}}, mod()->RegisterSymbol("a"), "a");
  ast::CaseSelectorList default_csl;
  auto* block_default =
      create<ast::BlockStatement>(Source{}, ast::StatementList{});
  ast::CaseStatementList body;
  body.push_back(
      create<ast::CaseStatement>(Source{}, default_csl, block_default));

  auto* block = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::VariableDeclStatement>(Source{}, var),
                    create<ast::SwitchStatement>(Source{}, cond, body),
                });

  EXPECT_TRUE(td()->DetermineStatements(block)) << td()->error();
  EXPECT_FALSE(v()->ValidateStatements(block));
  EXPECT_EQ(v()->error(),
            "12:34 v-0025: switch statement selector expression must be "
            "of a scalar integer type");
}

TEST_F(ValidateControlBlockTest, SwitchWithoutDefault_Fail) {
  // var a : i32 = 2;
  // switch (a) {
  //   case 1: {}
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

  auto* cond = create<ast::IdentifierExpression>(
      Source{}, mod()->RegisterSymbol("a"), "a");
  ast::CaseSelectorList csl;
  csl.push_back(create<ast::SintLiteral>(Source{}, &i32, 1));
  ast::CaseStatementList body;
  body.push_back(create<ast::CaseStatement>(
      Source{}, csl,
      create<ast::BlockStatement>(Source{}, ast::StatementList{})));

  auto* block = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::VariableDeclStatement>(Source{}, var),
                    create<ast::SwitchStatement>(
                        Source{Source::Location{12, 34}}, cond, body),
                });

  EXPECT_TRUE(td()->DetermineStatements(block)) << td()->error();
  EXPECT_FALSE(v()->ValidateStatements(block));
  EXPECT_EQ(v()->error(),
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

  ast::CaseStatementList switch_body;
  auto* cond = create<ast::IdentifierExpression>(
      Source{}, mod()->RegisterSymbol("a"), "a");

  ast::CaseSelectorList default_csl_1;
  auto* block_default_1 =
      create<ast::BlockStatement>(Source{}, ast::StatementList{});
  switch_body.push_back(
      create<ast::CaseStatement>(Source{}, default_csl_1, block_default_1));

  ast::CaseSelectorList csl_case_1;
  csl_case_1.push_back(create<ast::SintLiteral>(Source{}, &i32, 1));
  auto* block_case_1 =
      create<ast::BlockStatement>(Source{}, ast::StatementList{});
  switch_body.push_back(
      create<ast::CaseStatement>(Source{}, csl_case_1, block_case_1));

  ast::CaseSelectorList default_csl_2;
  auto* block_default_2 =
      create<ast::BlockStatement>(Source{}, ast::StatementList{});
  switch_body.push_back(
      create<ast::CaseStatement>(Source{}, default_csl_2, block_default_2));

  auto* block = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::VariableDeclStatement>(Source{}, var),
                    create<ast::SwitchStatement>(
                        Source{Source::Location{12, 34}}, cond, switch_body),
                });

  EXPECT_TRUE(td()->DetermineStatements(block)) << td()->error();
  EXPECT_FALSE(v()->ValidateStatements(block));
  EXPECT_EQ(v()->error(),
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
  ast::type::U32 u32;
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

  ast::CaseStatementList switch_body;
  auto* cond = create<ast::IdentifierExpression>(
      Source{}, mod()->RegisterSymbol("a"), "a");

  ast::CaseSelectorList csl;
  csl.push_back(create<ast::UintLiteral>(Source{}, &u32, 1));
  switch_body.push_back(create<ast::CaseStatement>(
      Source{Source::Location{12, 34}}, csl,
      create<ast::BlockStatement>(Source{}, ast::StatementList{})));

  ast::CaseSelectorList default_csl;
  auto* block_default =
      create<ast::BlockStatement>(Source{}, ast::StatementList{});
  switch_body.push_back(
      create<ast::CaseStatement>(Source{}, default_csl, block_default));

  auto* block = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::VariableDeclStatement>(Source{}, var),
                    create<ast::SwitchStatement>(Source{}, cond, switch_body),
                });
  EXPECT_TRUE(td()->DetermineStatements(block)) << td()->error();
  EXPECT_FALSE(v()->ValidateStatements(block));
  EXPECT_EQ(v()->error(),
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
  ast::type::U32 u32;
  ast::type::I32 i32;
  auto* var = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &u32,                      // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::UintLiteral>(Source{}, &u32, 2)),  // constructor
      ast::VariableDecorationList{});                    // decorations

  ast::CaseStatementList switch_body;
  auto* cond = create<ast::IdentifierExpression>(
      Source{}, mod()->RegisterSymbol("a"), "a");

  ast::CaseSelectorList csl;
  csl.push_back(create<ast::SintLiteral>(Source{}, &i32, -1));
  switch_body.push_back(create<ast::CaseStatement>(
      Source{Source::Location{12, 34}}, csl,
      create<ast::BlockStatement>(Source{}, ast::StatementList{})));

  ast::CaseSelectorList default_csl;
  auto* block_default =
      create<ast::BlockStatement>(Source{}, ast::StatementList{});
  switch_body.push_back(
      create<ast::CaseStatement>(Source{}, default_csl, block_default));

  auto* block = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::VariableDeclStatement>(Source{}, var),
                    create<ast::SwitchStatement>(Source{}, cond, switch_body),
                });
  EXPECT_TRUE(td()->DetermineStatements(block)) << td()->error();
  EXPECT_FALSE(v()->ValidateStatements(block));
  EXPECT_EQ(v()->error(),
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
  ast::type::U32 u32;
  auto* var = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &u32,                      // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::UintLiteral>(Source{}, &u32, 3)),  // constructor
      ast::VariableDecorationList{});                    // decorations

  ast::CaseStatementList switch_body;
  auto* cond = create<ast::IdentifierExpression>(
      Source{}, mod()->RegisterSymbol("a"), "a");

  ast::CaseSelectorList csl_1;
  csl_1.push_back(create<ast::UintLiteral>(Source{}, &u32, 0));
  switch_body.push_back(create<ast::CaseStatement>(
      Source{}, csl_1,
      create<ast::BlockStatement>(Source{}, ast::StatementList{})));

  ast::CaseSelectorList csl_2;
  csl_2.push_back(create<ast::UintLiteral>(Source{}, &u32, 2));
  csl_2.push_back(create<ast::UintLiteral>(Source{}, &u32, 2));
  switch_body.push_back(create<ast::CaseStatement>(
      Source{Source::Location{12, 34}}, csl_2,
      create<ast::BlockStatement>(Source{}, ast::StatementList{})));

  ast::CaseSelectorList default_csl;
  auto* block_default =
      create<ast::BlockStatement>(Source{}, ast::StatementList{});
  switch_body.push_back(
      create<ast::CaseStatement>(Source{}, default_csl, block_default));

  auto* block = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::VariableDeclStatement>(Source{}, var),
                    create<ast::SwitchStatement>(Source{}, cond, switch_body),
                });
  EXPECT_TRUE(td()->DetermineStatements(block)) << td()->error();
  EXPECT_FALSE(v()->ValidateStatements(block));
  EXPECT_EQ(v()->error(),
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

  ast::CaseStatementList switch_body;
  auto* cond = create<ast::IdentifierExpression>(
      Source{}, mod()->RegisterSymbol("a"), "a");

  ast::CaseSelectorList csl_1;
  csl_1.push_back(create<ast::SintLiteral>(Source{}, &i32, 10));
  switch_body.push_back(create<ast::CaseStatement>(
      Source{}, csl_1,
      create<ast::BlockStatement>(Source{}, ast::StatementList{})));

  ast::CaseSelectorList csl_2;
  csl_2.push_back(create<ast::SintLiteral>(Source{}, &i32, 0));
  csl_2.push_back(create<ast::SintLiteral>(Source{}, &i32, 1));
  csl_2.push_back(create<ast::SintLiteral>(Source{}, &i32, 2));
  csl_2.push_back(create<ast::SintLiteral>(Source{}, &i32, 10));
  switch_body.push_back(create<ast::CaseStatement>(
      Source{Source::Location{12, 34}}, csl_2,
      create<ast::BlockStatement>(Source{}, ast::StatementList{})));

  ast::CaseSelectorList default_csl;
  auto* block_default =
      create<ast::BlockStatement>(Source{}, ast::StatementList{});
  switch_body.push_back(
      create<ast::CaseStatement>(Source{}, default_csl, block_default));

  auto* block = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::VariableDeclStatement>(Source{}, var),
                    create<ast::SwitchStatement>(Source{}, cond, switch_body),
                });
  EXPECT_TRUE(td()->DetermineStatements(block)) << td()->error();
  EXPECT_FALSE(v()->ValidateStatements(block));
  EXPECT_EQ(v()->error(),
            "12:34 v-0027: a literal value must not appear more than once in "
            "the case selectors for a switch statement: '10'");
}

TEST_F(ValidateControlBlockTest, LastClauseLastStatementIsFallthrough_Fail) {
  // var a : i32 = 2;
  // switch (a) {
  //   default: { fallthrough; }
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

  auto* cond = create<ast::IdentifierExpression>(
      Source{}, mod()->RegisterSymbol("a"), "a");
  ast::CaseSelectorList default_csl;
  auto* block_default = create<ast::BlockStatement>(
      Source{},
      ast::StatementList{
          create<ast::FallthroughStatement>(Source{Source::Location{12, 34}}),
      });
  ast::CaseStatementList body;
  body.push_back(
      create<ast::CaseStatement>(Source{}, default_csl, block_default));

  auto* block = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::VariableDeclStatement>(Source{}, var),
                    create<ast::SwitchStatement>(Source{}, cond, body),
                });
  EXPECT_TRUE(td()->DetermineStatements(block)) << td()->error();
  EXPECT_FALSE(v()->ValidateStatements(block));
  EXPECT_EQ(v()->error(),
            "12:34 v-0028: a fallthrough statement must not appear as the "
            "last statement in last clause of a switch");
}

TEST_F(ValidateControlBlockTest, SwitchCase_Pass) {
  // var a : i32 = 2;
  // switch (a) {
  //   default: {}
  //   case 5: {}
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

  auto* cond = create<ast::IdentifierExpression>(
      Source{}, mod()->RegisterSymbol("a"), "a");
  ast::CaseSelectorList default_csl;
  auto* block_default =
      create<ast::BlockStatement>(Source{}, ast::StatementList{});
  ast::CaseStatementList body;
  body.push_back(create<ast::CaseStatement>(Source{Source::Location{12, 34}},
                                            default_csl, block_default));
  ast::CaseSelectorList case_csl;
  case_csl.push_back(create<ast::SintLiteral>(Source{}, &i32, 5));
  auto* block_case =
      create<ast::BlockStatement>(Source{}, ast::StatementList{});
  body.push_back(create<ast::CaseStatement>(Source{}, case_csl, block_case));

  auto* block = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::VariableDeclStatement>(Source{}, var),
                    create<ast::SwitchStatement>(Source{}, cond, body),
                });
  EXPECT_TRUE(td()->DetermineStatements(block)) << td()->error();
  EXPECT_TRUE(v()->ValidateStatements(block)) << v()->error();
}

TEST_F(ValidateControlBlockTest, SwitchCaseAlias_Pass) {
  // type MyInt = u32;
  // var v: MyInt;
  // switch(v){
  //   default: {}
  // }

  ast::type::U32 u32;
  ast::type::Alias my_int{mod()->RegisterSymbol("MyInt"), "MyInt", &u32};

  auto* var = create<ast::Variable>(
      Source{},                  // source
      "a",                       // name
      ast::StorageClass::kNone,  // storage_class
      &my_int,                   // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          Source{},
          create<ast::SintLiteral>(Source{}, &u32, 2)),  // constructor
      ast::VariableDecorationList{});                    // decorations

  auto* cond = create<ast::IdentifierExpression>(
      Source{}, mod()->RegisterSymbol("a"), "a");
  ast::CaseSelectorList default_csl;
  auto* block_default =
      create<ast::BlockStatement>(Source{}, ast::StatementList{});
  ast::CaseStatementList body;
  body.push_back(create<ast::CaseStatement>(Source{Source::Location{12, 34}},
                                            default_csl, block_default));

  auto* block = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::VariableDeclStatement>(Source{}, var),
                    create<ast::SwitchStatement>(Source{}, cond, body),
                });
  mod()->AddConstructedType(&my_int);

  EXPECT_TRUE(td()->DetermineStatements(block)) << td()->error();
  EXPECT_TRUE(v()->ValidateStatements(block)) << v()->error();
}

}  // namespace
}  // namespace tint
