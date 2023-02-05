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

using MemberAccessorExpressionTest = TestHelper;

TEST_F(MemberAccessorExpressionTest, Creation) {
    auto* str = Expr("structure");
    auto* mem = Ident("member");

    auto* stmt = create<MemberAccessorExpression>(str, mem);
    EXPECT_EQ(stmt->object, str);
    EXPECT_EQ(stmt->member, mem);
}

TEST_F(MemberAccessorExpressionTest, Creation_WithSource) {
    auto* stmt = create<MemberAccessorExpression>(Source{Source::Location{20, 2}},
                                                  Expr("structure"), Ident("member"));
    auto src = stmt->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(MemberAccessorExpressionTest, IsMemberAccessor) {
    auto* stmt = create<MemberAccessorExpression>(Expr("structure"), Ident("member"));
    EXPECT_TRUE(stmt->Is<MemberAccessorExpression>());
}

TEST_F(MemberAccessorExpressionTest, Assert_Null_Struct) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<MemberAccessorExpression>(nullptr, b.Ident("member"));
        },
        "internal compiler error");
}

TEST_F(MemberAccessorExpressionTest, Assert_Null_Member) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<MemberAccessorExpression>(b.Expr("struct"), nullptr);
        },
        "internal compiler error");
}

TEST_F(MemberAccessorExpressionTest, Assert_DifferentProgramID_Struct) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<MemberAccessorExpression>(b2.Expr("structure"), b1.Ident("member"));
        },
        "internal compiler error");
}

TEST_F(MemberAccessorExpressionTest, Assert_DifferentProgramID_Member) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<MemberAccessorExpression>(b1.Expr("structure"), b2.Ident("member"));
        },
        "internal compiler error");
}

TEST_F(MemberAccessorExpressionTest, Assert_MemberNotTemplated) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<MemberAccessorExpression>(b.Expr("structure"),
                                               b.Ident("member", "a", "b", "c"));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
