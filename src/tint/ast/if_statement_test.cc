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

#include "src/tint/ast/if_statement.h"

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using IfStatementTest = TestHelper;

TEST_F(IfStatementTest, Creation) {
    auto* cond = Expr("cond");
    auto* stmt = If(Source{Source::Location{20, 2}}, cond, Block(create<DiscardStatement>()));
    auto src = stmt->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(IfStatementTest, Creation_WithAttributes) {
    auto* attr1 = DiagnosticAttribute(builtin::DiagnosticSeverity::kOff, "foo");
    auto* attr2 = DiagnosticAttribute(builtin::DiagnosticSeverity::kOff, "bar");
    auto* cond = Expr("cond");
    auto* stmt = If(cond, Block(), ElseStmt(), utils::Vector{attr1, attr2});

    EXPECT_THAT(stmt->attributes, testing::ElementsAre(attr1, attr2));
}

TEST_F(IfStatementTest, IsIf) {
    auto* stmt = If(Expr(true), Block());
    EXPECT_TRUE(stmt->Is<IfStatement>());
}

TEST_F(IfStatementTest, Assert_Null_Condition) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.If(nullptr, b.Block());
        },
        "internal compiler error");
}

TEST_F(IfStatementTest, Assert_Null_Body) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.If(b.Expr(true), nullptr);
        },
        "internal compiler error");
}

TEST_F(IfStatementTest, Assert_InvalidElse) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.If(b.Expr(true), b.Block(), b.Else(b.CallStmt(b.Call("foo"))));
        },
        "internal compiler error");
}

TEST_F(IfStatementTest, Assert_DifferentProgramID_Cond) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.If(b2.Expr(true), b1.Block());
        },
        "internal compiler error");
}

TEST_F(IfStatementTest, Assert_DifferentProgramID_Body) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.If(b1.Expr(true), b2.Block());
        },
        "internal compiler error");
}

TEST_F(IfStatementTest, Assert_DifferentProgramID_ElseStatement) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.If(b1.Expr(true), b1.Block(), b2.Else(b2.If(b2.Expr("ident"), b2.Block())));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
