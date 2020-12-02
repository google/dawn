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

#include "src/ast/call_statement.h"

#include "src/ast/call_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using CallStatementTest = TestHelper;

TEST_F(CallStatementTest, Creation) {
  auto* expr = create<CallExpression>(create<IdentifierExpression>("func"),
                                      ExpressionList{});

  CallStatement c(expr);
  EXPECT_EQ(c.expr(), expr);
}

TEST_F(CallStatementTest, IsCall) {
  CallStatement c(nullptr);
  EXPECT_TRUE(c.Is<CallStatement>());
}

TEST_F(CallStatementTest, IsValid) {
  CallStatement c(create<CallExpression>(create<IdentifierExpression>("func"),
                                         ExpressionList{}));
  EXPECT_TRUE(c.IsValid());
}

TEST_F(CallStatementTest, IsValid_MissingExpr) {
  CallStatement c(nullptr);
  EXPECT_FALSE(c.IsValid());
}

TEST_F(CallStatementTest, IsValid_InvalidExpr) {
  CallExpression stmt(nullptr, {});
  CallStatement c(&stmt);
  EXPECT_FALSE(c.IsValid());
}

TEST_F(CallStatementTest, ToStr) {
  CallStatement c(create<CallExpression>(create<IdentifierExpression>("func"),
                                         ExpressionList{}));

  std::ostringstream out;
  c.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Call[not set]{
    Identifier[not set]{func}
    (
    )
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
