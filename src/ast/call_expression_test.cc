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

#include "src/ast/call_expression.h"

#include "src/ast/identifier_expression.h"
#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using CallExpressionTest = TestHelper;

TEST_F(CallExpressionTest, Creation) {
  auto func = create<IdentifierExpression>("func");
  ExpressionList params;
  params.push_back(create<IdentifierExpression>("param1"));
  params.push_back(create<IdentifierExpression>("param2"));

  auto* func_ptr = func.get();
  auto* param1_ptr = params[0].get();
  auto* param2_ptr = params[1].get();

  CallExpression stmt(std::move(func), std::move(params));
  EXPECT_EQ(stmt.func(), func_ptr);

  const auto& vec = stmt.params();
  ASSERT_EQ(vec.size(), 2u);
  EXPECT_EQ(vec[0].get(), param1_ptr);
  EXPECT_EQ(vec[1].get(), param2_ptr);
}

TEST_F(CallExpressionTest, Creation_WithSource) {
  auto func = create<IdentifierExpression>("func");
  CallExpression stmt(Source{Source::Location{20, 2}}, std::move(func), {});
  auto src = stmt.source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(CallExpressionTest, IsCall) {
  auto func = create<IdentifierExpression>("func");
  CallExpression stmt(std::move(func), {});
  EXPECT_TRUE(stmt.IsCall());
}

TEST_F(CallExpressionTest, IsValid) {
  auto func = create<IdentifierExpression>("func");
  CallExpression stmt(std::move(func), {});
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(CallExpressionTest, IsValid_MissingFunction) {
  CallExpression stmt;
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(CallExpressionTest, IsValid_NullParam) {
  auto func = create<IdentifierExpression>("func");
  ExpressionList params;
  params.push_back(create<IdentifierExpression>("param1"));
  params.push_back(nullptr);
  params.push_back(create<IdentifierExpression>("param2"));

  CallExpression stmt(std::move(func), std::move(params));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(CallExpressionTest, IsValid_InvalidFunction) {
  auto func = create<IdentifierExpression>("");
  ExpressionList params;
  params.push_back(create<IdentifierExpression>("param1"));

  CallExpression stmt(std::move(func), std::move(params));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(CallExpressionTest, IsValid_InvalidParam) {
  auto func = create<IdentifierExpression>("func");
  ExpressionList params;
  params.push_back(create<IdentifierExpression>(""));

  CallExpression stmt(std::move(func), std::move(params));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(CallExpressionTest, ToStr_NoParams) {
  auto func = create<IdentifierExpression>("func");
  CallExpression stmt(std::move(func), {});
  std::ostringstream out;
  stmt.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Call{
    Identifier{func}
    (
    )
  }
)");
}

TEST_F(CallExpressionTest, ToStr_WithParams) {
  auto func = create<IdentifierExpression>("func");
  ExpressionList params;
  params.push_back(create<IdentifierExpression>("param1"));
  params.push_back(create<IdentifierExpression>("param2"));

  CallExpression stmt(std::move(func), std::move(params));
  std::ostringstream out;
  stmt.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Call{
    Identifier{func}
    (
      Identifier{param1}
      Identifier{param2}
    )
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
