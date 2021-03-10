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

#include "src/ast/fallthrough_statement.h"

#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using FallthroughStatementTest = TestHelper;

TEST_F(FallthroughStatementTest, Creation) {
  auto* stmt = create<FallthroughStatement>();
  EXPECT_EQ(stmt->source().range.begin.line, 0u);
  EXPECT_EQ(stmt->source().range.begin.column, 0u);
  EXPECT_EQ(stmt->source().range.end.line, 0u);
  EXPECT_EQ(stmt->source().range.end.column, 0u);
}

TEST_F(FallthroughStatementTest, Creation_WithSource) {
  auto* stmt = create<FallthroughStatement>(Source{Source::Location{20, 2}});
  auto src = stmt->source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(FallthroughStatementTest, IsFallthrough) {
  auto* stmt = create<FallthroughStatement>();
  EXPECT_TRUE(stmt->Is<FallthroughStatement>());
}

TEST_F(FallthroughStatementTest, ToStr) {
  auto* stmt = create<FallthroughStatement>();
  EXPECT_EQ(str(stmt), R"(Fallthrough{}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
