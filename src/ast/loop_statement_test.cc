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

#include "src/ast/loop_statement.h"

#include <memory>
#include <sstream>

#include "src/ast/discard_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using LoopStatementTest = TestHelper;

TEST_F(LoopStatementTest, Creation) {
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});
  auto* b = body->last();

  auto* continuing = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});

  LoopStatement l(Source{}, body, continuing);
  ASSERT_EQ(l.body()->size(), 1u);
  EXPECT_EQ(l.body()->get(0), b);
  ASSERT_EQ(l.continuing()->size(), 1u);
  EXPECT_EQ(l.continuing()->get(0), continuing->last());
}

TEST_F(LoopStatementTest, Creation_WithSource) {
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});

  auto* continuing = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});

  LoopStatement l(Source{Source::Location{20, 2}}, body, continuing);
  auto src = l.source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(LoopStatementTest, IsLoop) {
  LoopStatement l(Source{}, create<BlockStatement>(Source{}, StatementList{}),
                  create<BlockStatement>(Source{}, StatementList{}));
  EXPECT_TRUE(l.Is<LoopStatement>());
}

TEST_F(LoopStatementTest, HasContinuing_WithoutContinuing) {
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});

  LoopStatement l(Source{}, body, {});
  EXPECT_FALSE(l.has_continuing());
}

TEST_F(LoopStatementTest, HasContinuing_WithContinuing) {
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});

  auto* continuing = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});

  LoopStatement l(Source{}, body, continuing);
  EXPECT_TRUE(l.has_continuing());
}

TEST_F(LoopStatementTest, IsValid) {
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});

  auto* continuing = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});

  LoopStatement l(Source{}, body, continuing);
  EXPECT_TRUE(l.IsValid());
}

TEST_F(LoopStatementTest, IsValid_WithoutContinuing) {
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});

  LoopStatement l(Source{}, body,
                  create<BlockStatement>(Source{}, StatementList{}));
  EXPECT_TRUE(l.IsValid());
}

TEST_F(LoopStatementTest, IsValid_WithoutBody) {
  LoopStatement l(Source{}, create<BlockStatement>(Source{}, StatementList{}),
                  create<BlockStatement>(Source{}, StatementList{}));
  EXPECT_TRUE(l.IsValid());
}

TEST_F(LoopStatementTest, IsValid_NullBodyStatement) {
  auto* body =
      create<BlockStatement>(Source{}, StatementList{
                                           create<DiscardStatement>(Source{}),
                                           nullptr,
                                       });

  auto* continuing = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});

  LoopStatement l(Source{}, body, continuing);
  EXPECT_FALSE(l.IsValid());
}

TEST_F(LoopStatementTest, IsValid_InvalidBodyStatement) {
  auto* body = create<BlockStatement>(
      Source{},
      StatementList{
          create<DiscardStatement>(Source{}),
          create<IfStatement>(Source{}, nullptr,
                              create<BlockStatement>(Source{}, StatementList{}),
                              ElseStatementList{}),
      });

  auto* continuing = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});

  LoopStatement l(Source{}, body, continuing);
  EXPECT_FALSE(l.IsValid());
}

TEST_F(LoopStatementTest, IsValid_NullContinuingStatement) {
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});

  auto* continuing =
      create<BlockStatement>(Source{}, StatementList{
                                           create<DiscardStatement>(Source{}),
                                           nullptr,
                                       });

  LoopStatement l(Source{}, body, continuing);
  EXPECT_FALSE(l.IsValid());
}

TEST_F(LoopStatementTest, IsValid_InvalidContinuingStatement) {
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});

  auto* continuing = create<BlockStatement>(
      Source{},
      StatementList{
          create<DiscardStatement>(Source{}),
          create<IfStatement>(Source{}, nullptr,
                              create<BlockStatement>(Source{}, StatementList{}),
                              ElseStatementList{}),
      });

  LoopStatement l(Source{}, body, continuing);
  EXPECT_FALSE(l.IsValid());
}

TEST_F(LoopStatementTest, ToStr) {
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});

  LoopStatement l(Source{}, body, {});
  std::ostringstream out;
  l.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Loop{
    Discard{}
  }
)");
}

TEST_F(LoopStatementTest, ToStr_WithContinuing) {
  auto* body = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});

  auto* continuing = create<BlockStatement>(
      Source{}, StatementList{create<DiscardStatement>(Source{})});

  LoopStatement l(Source{}, body, continuing);
  std::ostringstream out;
  l.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Loop{
    Discard{}
    continuing {
      Discard{}
    }
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
