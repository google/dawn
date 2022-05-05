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

#include "src/tint/ast/assignment_statement.h"

#include "gtest/gtest-spi.h"
#include "src/tint/ast/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ast {
namespace {

using AssignmentStatementTest = TestHelper;

TEST_F(AssignmentStatementTest, Creation) {
    auto* lhs = Expr("lhs");
    auto* rhs = Expr("rhs");

    auto* stmt = create<AssignmentStatement>(lhs, rhs);
    EXPECT_EQ(stmt->lhs, lhs);
    EXPECT_EQ(stmt->rhs, rhs);
}

TEST_F(AssignmentStatementTest, CreationWithSource) {
    auto* lhs = Expr("lhs");
    auto* rhs = Expr("rhs");

    auto* stmt = create<AssignmentStatement>(Source{Source::Location{20, 2}}, lhs, rhs);
    auto src = stmt->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(AssignmentStatementTest, IsAssign) {
    auto* lhs = Expr("lhs");
    auto* rhs = Expr("rhs");

    auto* stmt = create<AssignmentStatement>(lhs, rhs);
    EXPECT_TRUE(stmt->Is<AssignmentStatement>());
}

TEST_F(AssignmentStatementTest, Assert_Null_LHS) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<AssignmentStatement>(nullptr, b.Expr(1_i));
        },
        "internal compiler error");
}

TEST_F(AssignmentStatementTest, Assert_Null_RHS) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<AssignmentStatement>(b.Expr(1_i), nullptr);
        },
        "internal compiler error");
}

TEST_F(AssignmentStatementTest, Assert_DifferentProgramID_LHS) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<AssignmentStatement>(b2.Expr("lhs"), b1.Expr("rhs"));
        },
        "internal compiler error");
}

TEST_F(AssignmentStatementTest, Assert_DifferentProgramID_RHS) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<AssignmentStatement>(b1.Expr("lhs"), b2.Expr("rhs"));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
