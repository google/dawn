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

#include "gtest/gtest-spi.h"
#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using CallExpressionTest = TestHelper;

TEST_F(CallExpressionTest, CreationIdentifier) {
    auto* func = Expr("func");
    ExpressionList params;
    params.push_back(Expr("param1"));
    params.push_back(Expr("param2"));

    auto* stmt = create<CallExpression>(func, params);
    EXPECT_EQ(stmt->target.name, func);
    EXPECT_EQ(stmt->target.type, nullptr);

    const auto& vec = stmt->args;
    ASSERT_EQ(vec.size(), 2u);
    EXPECT_EQ(vec[0], params[0]);
    EXPECT_EQ(vec[1], params[1]);
}

TEST_F(CallExpressionTest, CreationIdentifier_WithSource) {
    auto* func = Expr("func");
    auto* stmt = create<CallExpression>(Source{{20, 2}}, func, ExpressionList{});
    EXPECT_EQ(stmt->target.name, func);
    EXPECT_EQ(stmt->target.type, nullptr);

    auto src = stmt->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(CallExpressionTest, CreationType) {
    auto* type = ty.f32();
    ExpressionList params;
    params.push_back(Expr("param1"));
    params.push_back(Expr("param2"));

    auto* stmt = create<CallExpression>(type, params);
    EXPECT_EQ(stmt->target.name, nullptr);
    EXPECT_EQ(stmt->target.type, type);

    const auto& vec = stmt->args;
    ASSERT_EQ(vec.size(), 2u);
    EXPECT_EQ(vec[0], params[0]);
    EXPECT_EQ(vec[1], params[1]);
}

TEST_F(CallExpressionTest, CreationType_WithSource) {
    auto* type = ty.f32();
    auto* stmt = create<CallExpression>(Source{{20, 2}}, type, ExpressionList{});
    EXPECT_EQ(stmt->target.name, nullptr);
    EXPECT_EQ(stmt->target.type, type);

    auto src = stmt->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(CallExpressionTest, IsCall) {
    auto* func = Expr("func");
    auto* stmt = create<CallExpression>(func, ExpressionList{});
    EXPECT_TRUE(stmt->Is<CallExpression>());
}

TEST_F(CallExpressionTest, Assert_Null_Identifier) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<CallExpression>(static_cast<IdentifierExpression*>(nullptr), ExpressionList{});
        },
        "internal compiler error");
}

TEST_F(CallExpressionTest, Assert_Null_Type) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<CallExpression>(static_cast<Type*>(nullptr), ExpressionList{});
        },
        "internal compiler error");
}

TEST_F(CallExpressionTest, Assert_Null_Param) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            ExpressionList params;
            params.push_back(b.Expr("param1"));
            params.push_back(nullptr);
            params.push_back(b.Expr("param2"));
            b.create<CallExpression>(b.Expr("func"), params);
        },
        "internal compiler error");
}

TEST_F(CallExpressionTest, Assert_DifferentProgramID_Identifier) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<CallExpression>(b2.Expr("func"), ExpressionList{});
        },
        "internal compiler error");
}

TEST_F(CallExpressionTest, Assert_DifferentProgramID_Type) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<CallExpression>(b2.ty.f32(), ExpressionList{});
        },
        "internal compiler error");
}

TEST_F(CallExpressionTest, Assert_DifferentProgramID_Param) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<CallExpression>(b1.Expr("func"), ExpressionList{b2.Expr("param1")});
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
