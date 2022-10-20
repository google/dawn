// Copyright 2022 The Tint Authors.
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

#include "src/tint/ast/break_if_statement.h"

#include "gtest/gtest-spi.h"
#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using BreakIfStatementTest = TestHelper;

TEST_F(BreakIfStatementTest, Creation) {
    auto* cond = Expr("cond");
    auto* stmt = BreakIf(Source{Source::Location{20, 2}}, cond);
    auto src = stmt->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(BreakIfStatementTest, IsBreakIf) {
    auto* stmt = BreakIf(Expr(true));
    EXPECT_TRUE(stmt->Is<BreakIfStatement>());
}

TEST_F(BreakIfStatementTest, Assert_Null_Condition) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.BreakIf(nullptr);
        },
        "internal compiler error");
}

TEST_F(BreakIfStatementTest, Assert_DifferentProgramID_Cond) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.BreakIf(b2.Expr(true));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
