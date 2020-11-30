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

#include "src/ast/discard_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using BlockStatementTest = TestHelper;

TEST_F(BlockStatementTest, Creation) {
  auto* d = create<DiscardStatement>();
  auto* ptr = d;

  BlockStatement b;
  b.append(d);

  ASSERT_EQ(b.size(), 1u);
  EXPECT_EQ(b[0], ptr);
}

TEST_F(BlockStatementTest, Creation_WithInsert) {
  auto* s1 = create<DiscardStatement>();
  auto* s2 = create<DiscardStatement>();
  auto* s3 = create<DiscardStatement>();

  BlockStatement b;
  b.insert(0, s1);
  b.insert(0, s2);
  b.insert(1, s3);

  // |b| should contain s2, s3, s1

  ASSERT_EQ(b.size(), 3u);
  EXPECT_EQ(b[0], s2);
  EXPECT_EQ(b[1], s3);
  EXPECT_EQ(b[2], s1);
}

TEST_F(BlockStatementTest, Creation_WithSource) {
  BlockStatement b(Source{Source::Location{20, 2}});
  auto src = b.source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(BlockStatementTest, IsBlock) {
  BlockStatement b;
  EXPECT_TRUE(b.Is<BlockStatement>());
}

TEST_F(BlockStatementTest, IsValid) {
  BlockStatement b;
  b.append(create<DiscardStatement>());
  EXPECT_TRUE(b.IsValid());
}

TEST_F(BlockStatementTest, IsValid_Empty) {
  BlockStatement b;
  EXPECT_TRUE(b.IsValid());
}

TEST_F(BlockStatementTest, IsValid_NullBodyStatement) {
  BlockStatement b;
  b.append(create<DiscardStatement>());
  b.append(nullptr);
  EXPECT_FALSE(b.IsValid());
}

TEST_F(BlockStatementTest, IsValid_InvalidBodyStatement) {
  BlockStatement b;
  b.append(create<IfStatement>(nullptr, create<BlockStatement>()));
  EXPECT_FALSE(b.IsValid());
}

TEST_F(BlockStatementTest, ToStr) {
  BlockStatement b;
  b.append(create<DiscardStatement>());

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
