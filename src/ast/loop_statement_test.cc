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
  auto body = create<BlockStatement>();
  body->append(create<DiscardStatement>());
  auto* b_ptr = body->last();

  auto continuing = create<BlockStatement>();
  continuing->append(create<DiscardStatement>());
  auto* c_ptr = continuing->last();

  LoopStatement l(std::move(body), std::move(continuing));
  ASSERT_EQ(l.body()->size(), 1u);
  EXPECT_EQ(l.body()->get(0), b_ptr);
  ASSERT_EQ(l.continuing()->size(), 1u);
  EXPECT_EQ(l.continuing()->get(0), c_ptr);
}

TEST_F(LoopStatementTest, Creation_WithSource) {
  auto body = create<BlockStatement>();
  body->append(create<DiscardStatement>());

  auto continuing = create<BlockStatement>();
  continuing->append(create<DiscardStatement>());

  LoopStatement l(Source{Source::Location{20, 2}}, std::move(body),
                  std::move(continuing));
  auto src = l.source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(LoopStatementTest, IsLoop) {
  LoopStatement l(create<BlockStatement>(), create<BlockStatement>());
  EXPECT_TRUE(l.IsLoop());
}

TEST_F(LoopStatementTest, HasContinuing_WithoutContinuing) {
  auto body = create<BlockStatement>();
  body->append(create<DiscardStatement>());

  LoopStatement l(std::move(body), {});
  EXPECT_FALSE(l.has_continuing());
}

TEST_F(LoopStatementTest, HasContinuing_WithContinuing) {
  auto body = create<BlockStatement>();
  body->append(create<DiscardStatement>());

  auto continuing = create<BlockStatement>();
  continuing->append(create<DiscardStatement>());

  LoopStatement l(std::move(body), std::move(continuing));
  EXPECT_TRUE(l.has_continuing());
}

TEST_F(LoopStatementTest, IsValid) {
  auto body = create<BlockStatement>();
  body->append(create<DiscardStatement>());

  auto continuing = create<BlockStatement>();
  continuing->append(create<DiscardStatement>());

  LoopStatement l(std::move(body), std::move(continuing));
  EXPECT_TRUE(l.IsValid());
}

TEST_F(LoopStatementTest, IsValid_WithoutContinuing) {
  auto body = create<BlockStatement>();
  body->append(create<DiscardStatement>());

  LoopStatement l(std::move(body), create<BlockStatement>());
  EXPECT_TRUE(l.IsValid());
}

TEST_F(LoopStatementTest, IsValid_WithoutBody) {
  LoopStatement l(create<BlockStatement>(), create<BlockStatement>());
  EXPECT_TRUE(l.IsValid());
}

TEST_F(LoopStatementTest, IsValid_NullBodyStatement) {
  auto body = create<BlockStatement>();
  body->append(create<DiscardStatement>());
  body->append(nullptr);

  auto continuing = create<BlockStatement>();
  continuing->append(create<DiscardStatement>());

  LoopStatement l(std::move(body), std::move(continuing));
  EXPECT_FALSE(l.IsValid());
}

TEST_F(LoopStatementTest, IsValid_InvalidBodyStatement) {
  auto body = create<BlockStatement>();
  body->append(create<DiscardStatement>());
  body->append(create<IfStatement>(nullptr, create<BlockStatement>()));

  auto continuing = create<BlockStatement>();
  continuing->append(create<DiscardStatement>());

  LoopStatement l(std::move(body), std::move(continuing));
  EXPECT_FALSE(l.IsValid());
}

TEST_F(LoopStatementTest, IsValid_NullContinuingStatement) {
  auto body = create<BlockStatement>();
  body->append(create<DiscardStatement>());

  auto continuing = create<BlockStatement>();
  continuing->append(create<DiscardStatement>());
  continuing->append(nullptr);

  LoopStatement l(std::move(body), std::move(continuing));
  EXPECT_FALSE(l.IsValid());
}

TEST_F(LoopStatementTest, IsValid_InvalidContinuingStatement) {
  auto body = create<BlockStatement>();
  body->append(create<DiscardStatement>());

  auto continuing = create<BlockStatement>();
  continuing->append(create<DiscardStatement>());
  continuing->append(create<IfStatement>(nullptr, create<BlockStatement>()));

  LoopStatement l(std::move(body), std::move(continuing));
  EXPECT_FALSE(l.IsValid());
}

TEST_F(LoopStatementTest, ToStr) {
  auto body = create<BlockStatement>();
  body->append(create<DiscardStatement>());

  LoopStatement l(std::move(body), {});
  std::ostringstream out;
  l.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Loop{
    Discard{}
  }
)");
}

TEST_F(LoopStatementTest, ToStr_WithContinuing) {
  auto body = create<BlockStatement>();
  body->append(create<DiscardStatement>());

  auto continuing = create<BlockStatement>();
  continuing->append(create<DiscardStatement>());

  LoopStatement l(std::move(body), std::move(continuing));
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
