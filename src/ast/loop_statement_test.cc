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

#include "gtest/gtest-spi.h"
#include "src/ast/discard_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using LoopStatementTest = TestHelper;

TEST_F(LoopStatementTest, Creation) {
  auto* body = Block(create<DiscardStatement>());
  auto* b = body->last();

  auto* continuing = Block(create<DiscardStatement>());

  auto* l = create<LoopStatement>(body, continuing);
  ASSERT_EQ(l->body()->size(), 1u);
  EXPECT_EQ(l->body()->get(0), b);
  ASSERT_EQ(l->continuing()->size(), 1u);
  EXPECT_EQ(l->continuing()->get(0), continuing->last());
}

TEST_F(LoopStatementTest, Creation_WithSource) {
  auto* body = Block(create<DiscardStatement>());

  auto* continuing = Block(create<DiscardStatement>());

  auto* l =
      create<LoopStatement>(Source{Source::Location{20, 2}}, body, continuing);
  auto src = l->source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(LoopStatementTest, IsLoop) {
  auto* l = create<LoopStatement>(Block(), Block());
  EXPECT_TRUE(l->Is<LoopStatement>());
}

TEST_F(LoopStatementTest, HasContinuing_WithoutContinuing) {
  auto* body = Block(create<DiscardStatement>());

  auto* l = create<LoopStatement>(body, nullptr);
  EXPECT_FALSE(l->has_continuing());
}

TEST_F(LoopStatementTest, HasContinuing_WithContinuing) {
  auto* body = Block(create<DiscardStatement>());

  auto* continuing = Block(create<DiscardStatement>());

  auto* l = create<LoopStatement>(body, continuing);
  EXPECT_TRUE(l->has_continuing());
}

TEST_F(LoopStatementTest, Assert_Null_Body) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.create<LoopStatement>(nullptr, nullptr);
      },
      "internal compiler error");
}

TEST_F(LoopStatementTest, Assert_DifferentProgramID_Body) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.create<LoopStatement>(b2.Block(), b1.Block());
      },
      "internal compiler error");
}

TEST_F(LoopStatementTest, Assert_DifferentProgramID_Continuing) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.create<LoopStatement>(b1.Block(), b2.Block());
      },
      "internal compiler error");
}

TEST_F(LoopStatementTest, ToStr) {
  auto* body = Block(create<DiscardStatement>());

  auto* l = create<LoopStatement>(body, nullptr);
  EXPECT_EQ(str(l), R"(Loop{
  Discard{}
}
)");
}

TEST_F(LoopStatementTest, ToStr_WithContinuing) {
  auto* body = Block(create<DiscardStatement>());

  auto* continuing = Block(create<DiscardStatement>());

  auto* l = create<LoopStatement>(body, continuing);
  EXPECT_EQ(str(l), R"(Loop{
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
