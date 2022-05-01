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

#include "src/tint/ast/unary_op_expression.h"

#include "gtest/gtest-spi.h"
#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using UnaryOpExpressionTest = TestHelper;

TEST_F(UnaryOpExpressionTest, Creation) {
    auto* ident = Expr("ident");

    auto* u = create<UnaryOpExpression>(UnaryOp::kNot, ident);
    EXPECT_EQ(u->op, UnaryOp::kNot);
    EXPECT_EQ(u->expr, ident);
}

TEST_F(UnaryOpExpressionTest, Creation_WithSource) {
    auto* ident = Expr("ident");
    auto* u = create<UnaryOpExpression>(Source{Source::Location{20, 2}}, UnaryOp::kNot, ident);
    auto src = u->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(UnaryOpExpressionTest, IsUnaryOp) {
    auto* ident = Expr("ident");
    auto* u = create<UnaryOpExpression>(UnaryOp::kNot, ident);
    EXPECT_TRUE(u->Is<UnaryOpExpression>());
}

TEST_F(UnaryOpExpressionTest, Assert_Null_Expression) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<UnaryOpExpression>(UnaryOp::kNot, nullptr);
        },
        "internal compiler error");
}

TEST_F(UnaryOpExpressionTest, Assert_DifferentProgramID_Expression) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<UnaryOpExpression>(UnaryOp::kNot, b2.Expr(true));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
