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

#include "gtest/gtest.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/bool_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/int_literal.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/i32_type.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, If_Empty) {
  ast::type::BoolType bool_type;

  auto cond = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::BoolLiteral>(&bool_type, true));

  ast::IfStatement expr(std::move(cond), ast::StatementList{});

  Context ctx;
  TypeDeterminer td(&ctx);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b;
  b.push_function(Function{});

  EXPECT_TRUE(b.GenerateIfStatement(&expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpSelectionMerge %3 None
OpBranchConditional %2 %4 %3
%4 = OpLabel
OpBranch %3
%3 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithStatements) {
  ast::type::BoolType bool_type;
  ast::type::I32Type i32;

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &i32);

  ast::StatementList body;
  body.push_back(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("v"),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::IntLiteral>(&i32, 2))));

  auto cond = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::BoolLiteral>(&bool_type, true));

  ast::IfStatement expr(std::move(cond), std::move(body));

  Context ctx;
  TypeDeterminer td(&ctx);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b;
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_TRUE(b.GenerateIfStatement(&expr)) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%4 = OpTypeBool
%5 = OpConstantTrue %4
%8 = OpConstant %3 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpSelectionMerge %6 None
OpBranchConditional %5 %7 %6
%7 = OpLabel
OpStore %1 %8
OpBranch %6
%6 = OpLabel
)");
}

TEST_F(BuilderTest, DISABLED_If_WithStatements_Returns) {
  // if (a) { return; }
}

TEST_F(BuilderTest, DISABLED_If_WithElse) {}

TEST_F(BuilderTest, DISABLED_If_WithElseIf) {}

TEST_F(BuilderTest, DISABLED_If_WithMultiple) {}

TEST_F(BuilderTest, DISABLED_If_WithBreak) {}

TEST_F(BuilderTest, DISABLED_If_WithContinue) {}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
