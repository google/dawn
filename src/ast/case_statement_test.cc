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
  CaseSelectorList b;
  auto* selector = create<SintLiteral>(ty.i32, 2);
  b.push_back(selector);

  auto* discard = create<DiscardStatement>();
  auto* body = create<BlockStatement>(StatementList{discard});

  auto* c = create<CaseStatement>(b, body);
  ASSERT_EQ(c->selectors().size(), 1u);
  EXPECT_EQ(c->selectors()[0], selector);
  ASSERT_EQ(c->body()->size(), 1u);
  EXPECT_EQ(c->body()->get(0), discard);
}

TEST_F(CaseStatementTest, Creation_u32) {
  CaseSelectorList b;
  auto* selector = create<SintLiteral>(ty.u32, 2);
  b.push_back(selector);

  auto* discard = create<DiscardStatement>();
  auto* body = create<BlockStatement>(StatementList{discard});

  auto* c = create<CaseStatement>(b, body);
  ASSERT_EQ(c->selectors().size(), 1u);
  EXPECT_EQ(c->selectors()[0], selector);
  ASSERT_EQ(c->body()->size(), 1u);
  EXPECT_EQ(c->body()->get(0), discard);
}

TEST_F(CaseStatementTest, Creation_WithSource) {
  CaseSelectorList b;
  b.push_back(create<SintLiteral>(ty.i32, 2));

  auto* body = create<BlockStatement>(StatementList{
      create<DiscardStatement>(),
  });
  auto* c = create<CaseStatement>(Source{Source::Location{20, 2}}, b, body);
  auto src = c->source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(CaseStatementTest, IsDefault_WithoutSelectors) {
  auto* body = create<BlockStatement>(StatementList{
      create<DiscardStatement>(),
  });
  auto* c = create<CaseStatement>(CaseSelectorList{}, body);
  EXPECT_TRUE(c->IsDefault());
}

TEST_F(CaseStatementTest, IsDefault_WithSelectors) {
  CaseSelectorList b;
  b.push_back(create<SintLiteral>(ty.i32, 2));

  auto* c = create<CaseStatement>(b, create<BlockStatement>(StatementList{}));
  EXPECT_FALSE(c->IsDefault());
}

TEST_F(CaseStatementTest, IsCase) {
  auto* c = create<CaseStatement>(CaseSelectorList{},
                                  create<BlockStatement>(StatementList{}));
  EXPECT_TRUE(c->Is<CaseStatement>());
}

TEST_F(CaseStatementTest, IsValid) {
  auto* c = create<CaseStatement>(CaseSelectorList{},
                                  create<BlockStatement>(StatementList{}));
  EXPECT_TRUE(c->IsValid());
}

TEST_F(CaseStatementTest, IsValid_NullBodyStatement) {
  CaseSelectorList b;
  b.push_back(create<SintLiteral>(ty.i32, 2));

  auto* body = create<BlockStatement>(StatementList{
      create<DiscardStatement>(),
      nullptr,
  });
  auto* c = create<CaseStatement>(b, body);
  EXPECT_FALSE(c->IsValid());
}

TEST_F(CaseStatementTest, IsValid_InvalidBodyStatement) {
  CaseSelectorList b;
  b.push_back(create<SintLiteral>(ty.i32, 2));

  auto* body = create<BlockStatement>(

      StatementList{
          create<IfStatement>(nullptr, create<BlockStatement>(StatementList{}),
                              ElseStatementList{}),
      });
  auto* c = create<CaseStatement>(CaseSelectorList{b}, body);
  EXPECT_FALSE(c->IsValid());
}

TEST_F(CaseStatementTest, ToStr_WithSelectors_i32) {
  CaseSelectorList b;
  b.push_back(create<SintLiteral>(ty.i32, -2));

  auto* body = create<BlockStatement>(StatementList{
      create<DiscardStatement>(),
  });
  auto* c = create<CaseStatement>(CaseSelectorList{b}, body);

  std::ostringstream out;
  c->to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Case -2{
    Discard{}
  }
)");
}

TEST_F(CaseStatementTest, ToStr_WithSelectors_u32) {
  CaseSelectorList b;
  b.push_back(create<UintLiteral>(ty.u32, 2));

  auto* body = create<BlockStatement>(StatementList{
      create<DiscardStatement>(),
  });
  auto* c = create<CaseStatement>(CaseSelectorList{b}, body);

  std::ostringstream out;
  c->to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Case 2{
    Discard{}
  }
)");
}

TEST_F(CaseStatementTest, ToStr_WithMultipleSelectors) {
  CaseSelectorList b;
  b.push_back(create<SintLiteral>(ty.i32, 1));
  b.push_back(create<SintLiteral>(ty.i32, 2));

  auto* body = create<BlockStatement>(StatementList{
      create<DiscardStatement>(),
  });
  auto* c = create<CaseStatement>(b, body);

  std::ostringstream out;
  c->to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Case 1, 2{
    Discard{}
  }
)");
}

TEST_F(CaseStatementTest, ToStr_WithoutSelectors) {
  auto* body = create<BlockStatement>(StatementList{
      create<DiscardStatement>(),
  });
  auto* c = create<CaseStatement>(CaseSelectorList{}, body);

  std::ostringstream out;
  c->to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Default{
    Discard{}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
