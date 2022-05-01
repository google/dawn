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

#include "src/tint/ast/break_statement.h"

#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using BreakStatementTest = TestHelper;

TEST_F(BreakStatementTest, Creation_WithSource) {
    auto* stmt = create<BreakStatement>(Source{Source::Location{20, 2}});
    auto src = stmt->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(BreakStatementTest, IsBreak) {
    auto* stmt = create<BreakStatement>();
    EXPECT_TRUE(stmt->Is<BreakStatement>());
}

}  // namespace
}  // namespace tint::ast
