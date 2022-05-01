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

#include "src/tint/ast/discard_statement.h"

#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using DiscardStatementTest = TestHelper;

TEST_F(DiscardStatementTest, Creation) {
    auto* stmt = create<DiscardStatement>();
    EXPECT_EQ(stmt->source.range.begin.line, 0u);
    EXPECT_EQ(stmt->source.range.begin.column, 0u);
    EXPECT_EQ(stmt->source.range.end.line, 0u);
    EXPECT_EQ(stmt->source.range.end.column, 0u);
}

TEST_F(DiscardStatementTest, Creation_WithSource) {
    auto* stmt = create<DiscardStatement>(
        Source{Source::Range{Source::Location{20, 2}, Source::Location{20, 5}}});
    EXPECT_EQ(stmt->source.range.begin.line, 20u);
    EXPECT_EQ(stmt->source.range.begin.column, 2u);
    EXPECT_EQ(stmt->source.range.end.line, 20u);
    EXPECT_EQ(stmt->source.range.end.column, 5u);
}

TEST_F(DiscardStatementTest, IsDiscard) {
    auto* stmt = create<DiscardStatement>();
    EXPECT_TRUE(stmt->Is<DiscardStatement>());
}

}  // namespace
}  // namespace tint::ast
