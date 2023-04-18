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

#include "src/tint/ast/bitcast_expression.h"

#include "gtest/gtest-spi.h"
#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using BitcastExpressionTest = TestHelper;

TEST_F(BitcastExpressionTest, Create) {
    auto* expr = Expr("expr");
    auto* exp = Bitcast(ty.f32(), expr);
    CheckIdentifier(exp->type, "f32");
    ASSERT_EQ(exp->expr, expr);
}

TEST_F(BitcastExpressionTest, CreateWithSource) {
    auto* expr = Expr("expr");

    auto* exp = Bitcast(Source{Source::Location{20, 2}}, ty.f32(), expr);
    auto src = exp->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(BitcastExpressionTest, IsBitcast) {
    auto* expr = Expr("expr");

    auto* exp = Bitcast(ty.f32(), expr);
    EXPECT_TRUE(exp->Is<BitcastExpression>());
}

TEST_F(BitcastExpressionTest, Assert_Null_Type) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.Bitcast(ast::Type(), "idx");
        },
        "internal compiler error");
}

TEST_F(BitcastExpressionTest, Assert_Null_Expr) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.Bitcast(b.ty.f32(), nullptr);
        },
        "internal compiler error");
}

TEST_F(BitcastExpressionTest, Assert_DifferentProgramID_Expr) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Bitcast(b1.ty.f32(), b2.Expr("idx"));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
