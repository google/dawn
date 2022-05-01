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

#include "src/tint/ast/increment_decrement_statement.h"

#include "gtest/gtest-spi.h"
#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using IncrementDecrementStatementTest = TestHelper;

TEST_F(IncrementDecrementStatementTest, Creation) {
    auto* expr = Expr("expr");

    auto* i = create<IncrementDecrementStatement>(expr, true);
    EXPECT_EQ(i->lhs, expr);
    EXPECT_TRUE(i->increment);
}

TEST_F(IncrementDecrementStatementTest, Creation_WithSource) {
    auto* expr = Expr("expr");
    auto* i = create<IncrementDecrementStatement>(Source{Source::Location{20, 2}}, expr, true);
    auto src = i->source;
    EXPECT_EQ(i->lhs, expr);
    EXPECT_TRUE(i->increment);
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(IncrementDecrementStatementTest, IsIncrementDecrement) {
    auto* expr = Expr("expr");
    auto* i = create<IncrementDecrementStatement>(expr, true);
    EXPECT_TRUE(i->Is<IncrementDecrementStatement>());
}

TEST_F(IncrementDecrementStatementTest, Decrement) {
    auto* expr = Expr("expr");
    auto* i = create<IncrementDecrementStatement>(expr, false);
    EXPECT_EQ(i->lhs, expr);
    EXPECT_FALSE(i->increment);
}

TEST_F(IncrementDecrementStatementTest, Assert_DifferentProgramID_Expr) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<IncrementDecrementStatement>(b2.Expr(true), true);
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
