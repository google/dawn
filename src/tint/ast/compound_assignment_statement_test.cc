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

#include "src/tint/ast/compound_assignment_statement.h"

#include "gtest/gtest-spi.h"
#include "src/tint/ast/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ast {
namespace {

using CompoundAssignmentStatementTest = TestHelper;

TEST_F(CompoundAssignmentStatementTest, Creation) {
    auto* lhs = Expr("lhs");
    auto* rhs = Expr("rhs");
    auto op = BinaryOp::kAdd;

    auto* stmt = create<CompoundAssignmentStatement>(lhs, rhs, op);
    EXPECT_EQ(stmt->lhs, lhs);
    EXPECT_EQ(stmt->rhs, rhs);
    EXPECT_EQ(stmt->op, op);
}

TEST_F(CompoundAssignmentStatementTest, CreationWithSource) {
    auto* lhs = Expr("lhs");
    auto* rhs = Expr("rhs");
    auto op = BinaryOp::kMultiply;

    auto* stmt = create<CompoundAssignmentStatement>(Source{Source::Location{20, 2}}, lhs, rhs, op);
    auto src = stmt->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(CompoundAssignmentStatementTest, IsCompoundAssign) {
    auto* lhs = Expr("lhs");
    auto* rhs = Expr("rhs");
    auto op = BinaryOp::kSubtract;

    auto* stmt = create<CompoundAssignmentStatement>(lhs, rhs, op);
    EXPECT_TRUE(stmt->Is<CompoundAssignmentStatement>());
}

TEST_F(CompoundAssignmentStatementTest, Assert_Null_LHS) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<CompoundAssignmentStatement>(nullptr, b.Expr(1_i), BinaryOp::kAdd);
        },
        "internal compiler error");
}

TEST_F(CompoundAssignmentStatementTest, Assert_Null_RHS) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<CompoundAssignmentStatement>(b.Expr(1_i), nullptr, BinaryOp::kAdd);
        },
        "internal compiler error");
}

TEST_F(CompoundAssignmentStatementTest, Assert_DifferentProgramID_LHS) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<CompoundAssignmentStatement>(b2.Expr("lhs"), b1.Expr("rhs"), BinaryOp::kAdd);
        },
        "internal compiler error");
}

TEST_F(CompoundAssignmentStatementTest, Assert_DifferentProgramID_RHS) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<CompoundAssignmentStatement>(b1.Expr("lhs"), b2.Expr("rhs"), BinaryOp::kAdd);
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
