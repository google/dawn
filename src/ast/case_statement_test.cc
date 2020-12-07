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

#include "src/ast/case_statement.h"

#include "src/ast/discard_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/sint_literal.h"
#include "src/ast/test_helper.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/uint_literal.h"

namespace tint {
namespace ast {
namespace {

using CaseStatementTest = TestHelper;

TEST_F(CaseStatementTest, Creation_i32) {
  type::I32 i32;

  CaseSelectorList b;
  auto* selector = create<SintLiteral>(&i32, 2);
  b.push_back(selector);

  auto* body = create<BlockStatement>();
  auto* discard = create<DiscardStatement>();
  body->append(discard);

  CaseStatement c(b, body);
  ASSERT_EQ(c.selectors().size(), 1u);
  EXPECT_EQ(c.selectors()[0], selector);
  ASSERT_EQ(c.body()->size(), 1u);
  EXPECT_EQ(c.body()->get(0), discard);
}

TEST_F(CaseStatementTest, Creation_u32) {
  type::U32 u32;

  CaseSelectorList b;
  auto* selector = create<SintLiteral>(&u32, 2);
  b.push_back(selector);

  auto* body = create<BlockStatement>();
  auto* discard = create<DiscardStatement>();
  body->append(discard);

  CaseStatement c(b, body);
  ASSERT_EQ(c.selectors().size(), 1u);
  EXPECT_EQ(c.selectors()[0], selector);
  ASSERT_EQ(c.body()->size(), 1u);
  EXPECT_EQ(c.body()->get(0), discard);
}

TEST_F(CaseStatementTest, Creation_WithSource) {
  type::I32 i32;
  CaseSelectorList b;
  b.push_back(create<SintLiteral>(&i32, 2));

  auto* body = create<BlockStatement>();
  body->append(create<DiscardStatement>());

  CaseStatement c(Source{Source::Location{20, 2}}, b, body);
  auto src = c.source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(CaseStatementTest, IsDefault_WithoutSelectors) {
  auto* body = create<BlockStatement>();
  body->append(create<DiscardStatement>());

  CaseStatement c(body);
  EXPECT_TRUE(c.IsDefault());
}

TEST_F(CaseStatementTest, IsDefault_WithSelectors) {
  type::I32 i32;
  CaseSelectorList b;
  b.push_back(create<SintLiteral>(&i32, 2));

  CaseStatement c(b, create<BlockStatement>());
  EXPECT_FALSE(c.IsDefault());
}

TEST_F(CaseStatementTest, IsCase) {
  CaseStatement c(create<BlockStatement>());
  EXPECT_TRUE(c.Is<CaseStatement>());
}

TEST_F(CaseStatementTest, IsValid) {
  CaseStatement c(create<BlockStatement>());
  EXPECT_TRUE(c.IsValid());
}

TEST_F(CaseStatementTest, IsValid_NullBodyStatement) {
  type::I32 i32;
  CaseSelectorList b;
  b.push_back(create<SintLiteral>(&i32, 2));

  auto* body = create<BlockStatement>();
  body->append(create<DiscardStatement>());
  body->append(nullptr);

  CaseStatement c(b, body);
  EXPECT_FALSE(c.IsValid());
}

TEST_F(CaseStatementTest, IsValid_InvalidBodyStatement) {
  type::I32 i32;
  CaseSelectorList b;
  b.push_back(create<SintLiteral>(&i32, 2));

  auto* body = create<BlockStatement>();
  body->append(create<IfStatement>(Source{}, nullptr, create<BlockStatement>(),
                                   ElseStatementList{}));

  CaseStatement c({b}, body);
  EXPECT_FALSE(c.IsValid());
}

TEST_F(CaseStatementTest, ToStr_WithSelectors_i32) {
  type::I32 i32;
  CaseSelectorList b;
  b.push_back(create<SintLiteral>(&i32, -2));

  auto* body = create<BlockStatement>();
  body->append(create<DiscardStatement>());
  CaseStatement c({b}, body);

  std::ostringstream out;
  c.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Case -2{
    Discard{}
  }
)");
}

TEST_F(CaseStatementTest, ToStr_WithSelectors_u32) {
  type::U32 u32;
  CaseSelectorList b;
  b.push_back(create<UintLiteral>(&u32, 2));

  auto* body = create<BlockStatement>();
  body->append(create<DiscardStatement>());
  CaseStatement c({b}, body);

  std::ostringstream out;
  c.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Case 2{
    Discard{}
  }
)");
}

TEST_F(CaseStatementTest, ToStr_WithMultipleSelectors) {
  type::I32 i32;

  CaseSelectorList b;
  b.push_back(create<SintLiteral>(&i32, 1));
  b.push_back(create<SintLiteral>(&i32, 2));

  auto* body = create<BlockStatement>();
  body->append(create<DiscardStatement>());
  CaseStatement c(b, body);

  std::ostringstream out;
  c.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Case 1, 2{
    Discard{}
  }
)");
}

TEST_F(CaseStatementTest, ToStr_WithoutSelectors) {
  auto* body = create<BlockStatement>();
  body->append(create<DiscardStatement>());
  CaseStatement c(CaseSelectorList{}, body);

  std::ostringstream out;
  c.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Default{
    Discard{}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
