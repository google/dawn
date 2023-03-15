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

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/ast/binary_expression.h"
#include "src/tint/ast/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ast {
namespace {

using WhileStatementTest = TestHelper;

TEST_F(WhileStatementTest, Creation) {
    auto* cond = create<BinaryExpression>(BinaryOp::kLessThan, Expr("i"), Expr(5_u));
    auto* body = Block(Return());
    auto* l = While(cond, body);

    EXPECT_EQ(l->condition, cond);
    EXPECT_EQ(l->body, body);
}

TEST_F(WhileStatementTest, Creation_WithSource) {
    auto* cond = create<BinaryExpression>(BinaryOp::kLessThan, Expr("i"), Expr(5_u));
    auto* body = Block(Return());
    auto* l = While(Source{{20u, 2u}}, cond, body);
    auto src = l->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(WhileStatementTest, Creation_WithAttributes) {
    auto* attr1 = DiagnosticAttribute(builtin::DiagnosticSeverity::kOff, "foo");
    auto* attr2 = DiagnosticAttribute(builtin::DiagnosticSeverity::kOff, "bar");
    auto* cond = create<BinaryExpression>(BinaryOp::kLessThan, Expr("i"), Expr(5_u));
    auto* body = Block(Return());
    auto* l = While(cond, body, utils::Vector{attr1, attr2});

    EXPECT_THAT(l->attributes, testing::ElementsAre(attr1, attr2));
}

TEST_F(WhileStatementTest, Assert_Null_Cond) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            auto* body = b.Block();
            b.While(nullptr, body);
        },
        "internal compiler error");
}

TEST_F(WhileStatementTest, Assert_Null_Body) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            auto* cond = b.create<BinaryExpression>(BinaryOp::kLessThan, b.Expr("i"), b.Expr(5_u));
            b.While(cond, nullptr);
        },
        "internal compiler error");
}

TEST_F(WhileStatementTest, Assert_DifferentProgramID_Condition) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.While(b2.Expr(true), b1.Block());
        },
        "internal compiler error");
}

TEST_F(WhileStatementTest, Assert_DifferentProgramID_Body) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.While(b1.Expr(true), b2.Block());
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
