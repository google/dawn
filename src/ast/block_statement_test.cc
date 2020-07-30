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

#include "src/ast/block_statement.h"

#include <memory>
#include <sstream>

#include "gtest/gtest.h"
#include "src/ast/discard_statement.h"
#include "src/ast/if_statement.h"

namespace tint {
namespace ast {
namespace {

using BlockStatementTest = testing::Test;

TEST_F(BlockStatementTest, Creation) {
  auto d = std::make_unique<DiscardStatement>();
  auto* ptr = d.get();

  BlockStatement b;
  b.append(std::move(d));

  ASSERT_EQ(b.size(), 1u);
  EXPECT_EQ(b[0], ptr);
}

TEST_F(BlockStatementTest, Creation_WithInsert) {
  auto s1 = std::make_unique<DiscardStatement>();
  auto s2 = std::make_unique<DiscardStatement>();
  auto s3 = std::make_unique<DiscardStatement>();
  auto* p1 = s1.get();
  auto* p2 = s2.get();
  auto* p3 = s3.get();

  BlockStatement b;
  b.insert(0, std::move(s1));
  b.insert(0, std::move(s2));
  b.insert(1, std::move(s3));

  // |b| should contain s2, s3, s1

  ASSERT_EQ(b.size(), 3u);
  EXPECT_EQ(b[0], p2);
  EXPECT_EQ(b[1], p3);
  EXPECT_EQ(b[2], p1);
}

TEST_F(BlockStatementTest, Creation_WithSource) {
  BlockStatement b(Source{20, 2});
  auto src = b.source();
  EXPECT_EQ(src.line, 20u);
  EXPECT_EQ(src.column, 2u);
}

TEST_F(BlockStatementTest, IsBlock) {
  BlockStatement b;
  EXPECT_TRUE(b.IsBlock());
}

TEST_F(BlockStatementTest, IsValid) {
  BlockStatement b;
  b.append(std::make_unique<DiscardStatement>());
  EXPECT_TRUE(b.IsValid());
}

TEST_F(BlockStatementTest, IsValid_Empty) {
  BlockStatement b;
  EXPECT_TRUE(b.IsValid());
}

TEST_F(BlockStatementTest, IsValid_NullBodyStatement) {
  BlockStatement b;
  b.append(std::make_unique<DiscardStatement>());
  b.append(nullptr);
  EXPECT_FALSE(b.IsValid());
}

TEST_F(BlockStatementTest, IsValid_InvalidBodyStatement) {
  BlockStatement b;
  b.append(std::make_unique<IfStatement>());
  EXPECT_FALSE(b.IsValid());
}

TEST_F(BlockStatementTest, ToStr) {
  BlockStatement b;
  b.append(std::make_unique<DiscardStatement>());

  std::ostringstream out;
  b.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Block{
    Discard{}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
