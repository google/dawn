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
    utils::Vector params{
        Expr("param1"),
        Expr("param2"),
    };

    auto* stmt = Call(func, params);
    EXPECT_EQ(stmt->target, func);

    const auto& vec = stmt->args;
    ASSERT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec[0], params[0]);
    EXPECT_EQ(vec[1], params[1]);
}

TEST_F(CallExpressionTest, CreationIdentifier_WithSource) {
    auto* func = Expr("func");
    auto* stmt = Call(Source{{20, 2}}, func);
    EXPECT_EQ(stmt->target, func);

    auto src = stmt->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(CallExpressionTest, CreationType) {
    auto* type = Expr(ty.f32());
    utils::Vector params{
        Expr("param1"),
        Expr("param2"),
    };

    auto* stmt = Call(type, params);
    EXPECT_EQ(stmt->target, type);

    const auto& vec = stmt->args;
    ASSERT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec[0], params[0]);
    EXPECT_EQ(vec[1], params[1]);
}

TEST_F(CallExpressionTest, CreationType_WithSource) {
    auto* type = Expr(ty.f32());
    auto* stmt = Call(Source{{20, 2}}, type);
    EXPECT_EQ(stmt->target, type);

    auto src = stmt->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(CallExpressionTest, IsCall) {
    auto* func = Expr("func");
    auto* stmt = Call(func);
    EXPECT_TRUE(stmt->Is<CallExpression>());
}

TEST_F(CallExpressionTest, Assert_Null_Identifier) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.Call(static_cast<Identifier*>(nullptr));
        },
        "internal compiler error");
}

TEST_F(CallExpressionTest, Assert_Null_Param) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.Call(b.Ident("func"), utils::Vector{
                                        b.Expr("param1"),
                                        nullptr,
                                        b.Expr("param2"),
                                    });
        },
        "internal compiler error");
}

TEST_F(CallExpressionTest, Assert_DifferentProgramID_Identifier) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Call(b2.Ident("func"));
        },
        "internal compiler error");
}

TEST_F(CallExpressionTest, Assert_DifferentProgramID_Type) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Call(b2.ty.f32());
        },
        "internal compiler error");
}

TEST_F(CallExpressionTest, Assert_DifferentProgramID_Param) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Call(b1.Ident("func"), b2.Expr("param1"));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
