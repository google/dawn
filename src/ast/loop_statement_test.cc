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

#include <sstream>

#include "gtest/gtest.h"
#include "src/ast/if_statement.h"
#include "src/ast/kill_statement.h"
#include "src/ast/nop_statement.h"

namespace tint {
namespace ast {

using LoopStatementTest = testing::Test;

TEST_F(LoopStatementTest, Creation) {
  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<KillStatement>());
  auto b_ptr = body[0].get();

  std::vector<std::unique_ptr<Statement>> continuing;
  continuing.push_back(std::make_unique<NopStatement>());
  auto c_ptr = continuing[0].get();

  LoopStatement l(std::move(body), std::move(continuing));
  ASSERT_EQ(l.body().size(), 1);
  EXPECT_EQ(l.body()[0].get(), b_ptr);
  ASSERT_EQ(l.continuing().size(), 1);
  EXPECT_EQ(l.continuing()[0].get(), c_ptr);
}

TEST_F(LoopStatementTest, Creation_WithSource) {
  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<KillStatement>());

  std::vector<std::unique_ptr<Statement>> continuing;
  continuing.push_back(std::make_unique<NopStatement>());

  LoopStatement l(Source{20, 2}, std::move(body), std::move(continuing));
  auto src = l.source();
  EXPECT_EQ(src.line, 20);
  EXPECT_EQ(src.column, 2);
}

TEST_F(LoopStatementTest, IsLoop) {
  LoopStatement l;
  EXPECT_TRUE(l.IsLoop());
}

TEST_F(LoopStatementTest, IsValid) {
  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<KillStatement>());

  std::vector<std::unique_ptr<Statement>> continuing;
  continuing.push_back(std::make_unique<NopStatement>());

  LoopStatement l(std::move(body), std::move(continuing));
  EXPECT_TRUE(l.IsValid());
}

TEST_F(LoopStatementTest, IsValid_WithoutContinuing) {
  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<KillStatement>());

  LoopStatement l(std::move(body), {});
  EXPECT_TRUE(l.IsValid());
}

TEST_F(LoopStatementTest, IsValid_WithoutBody) {
  LoopStatement l({}, {});
  EXPECT_TRUE(l.IsValid());
}

TEST_F(LoopStatementTest, IsValid_NullBodyStatement) {
  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<KillStatement>());
  body.push_back(nullptr);

  std::vector<std::unique_ptr<Statement>> continuing;
  continuing.push_back(std::make_unique<NopStatement>());

  LoopStatement l(std::move(body), std::move(continuing));
  EXPECT_FALSE(l.IsValid());
}

TEST_F(LoopStatementTest, IsValid_InvalidBodyStatement) {
  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<KillStatement>());
  body.push_back(std::make_unique<IfStatement>());

  std::vector<std::unique_ptr<Statement>> continuing;
  continuing.push_back(std::make_unique<NopStatement>());

  LoopStatement l(std::move(body), std::move(continuing));
  EXPECT_FALSE(l.IsValid());
}

TEST_F(LoopStatementTest, IsValid_NullContinuingStatement) {
  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<KillStatement>());

  std::vector<std::unique_ptr<Statement>> continuing;
  continuing.push_back(std::make_unique<NopStatement>());
  continuing.push_back(nullptr);

  LoopStatement l(std::move(body), std::move(continuing));
  EXPECT_FALSE(l.IsValid());
}

TEST_F(LoopStatementTest, IsValid_InvalidContinuingStatement) {
  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<KillStatement>());

  std::vector<std::unique_ptr<Statement>> continuing;
  continuing.push_back(std::make_unique<NopStatement>());
  continuing.push_back(std::make_unique<IfStatement>());

  LoopStatement l(std::move(body), std::move(continuing));
  EXPECT_FALSE(l.IsValid());
}

TEST_F(LoopStatementTest, ToStr) {
  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<KillStatement>());

  LoopStatement l(std::move(body), {});
  std::ostringstream out;
  l.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Loop{
    Kill{}
  }
)");
}

TEST_F(LoopStatementTest, ToStr_WithContinuing) {
  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<KillStatement>());

  std::vector<std::unique_ptr<Statement>> continuing;
  continuing.push_back(std::make_unique<NopStatement>());

  LoopStatement l(std::move(body), std::move(continuing));
  std::ostringstream out;
  l.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Loop{
    Kill{}
    continuing {
      Nop{}
    }
  }
)");
}

}  // namespace ast
}  // namespace tint
