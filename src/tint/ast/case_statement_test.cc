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

#include "src/tint/ast/case_statement.h"

#include "gtest/gtest-spi.h"
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using CaseStatementTest = TestHelper;

TEST_F(CaseStatementTest, Creation_i32) {
    CaseSelectorList b;
    auto* selector = Expr(2);
    b.push_back(selector);

    auto* discard = create<DiscardStatement>();
    auto* body = create<BlockStatement>(StatementList{discard});

    auto* c = create<CaseStatement>(b, body);
    ASSERT_EQ(c->selectors.size(), 1u);
    EXPECT_EQ(c->selectors[0], selector);
    ASSERT_EQ(c->body->statements.size(), 1u);
    EXPECT_EQ(c->body->statements[0], discard);
}

TEST_F(CaseStatementTest, Creation_u32) {
    CaseSelectorList b;
    auto* selector = Expr(2u);
    b.push_back(selector);

    auto* discard = create<DiscardStatement>();
    auto* body = create<BlockStatement>(StatementList{discard});

    auto* c = create<CaseStatement>(b, body);
    ASSERT_EQ(c->selectors.size(), 1u);
    EXPECT_EQ(c->selectors[0], selector);
    ASSERT_EQ(c->body->statements.size(), 1u);
    EXPECT_EQ(c->body->statements[0], discard);
}

TEST_F(CaseStatementTest, Creation_WithSource) {
    CaseSelectorList b;
    b.push_back(Expr(2));

    auto* body = create<BlockStatement>(StatementList{
        create<DiscardStatement>(),
    });
    auto* c = create<CaseStatement>(Source{Source::Location{20, 2}}, b, body);
    auto src = c->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(CaseStatementTest, IsDefault_WithoutSelectors) {
    auto* body = create<BlockStatement>(StatementList{
        create<DiscardStatement>(),
    });
    auto* c = create<CaseStatement>(CaseSelectorList{}, body);
    EXPECT_TRUE(c->IsDefault());
}

TEST_F(CaseStatementTest, IsDefault_WithSelectors) {
    CaseSelectorList b;
    b.push_back(Expr(2));

    auto* c = create<CaseStatement>(b, create<BlockStatement>(StatementList{}));
    EXPECT_FALSE(c->IsDefault());
}

TEST_F(CaseStatementTest, IsCase) {
    auto* c = create<CaseStatement>(CaseSelectorList{}, create<BlockStatement>(StatementList{}));
    EXPECT_TRUE(c->Is<CaseStatement>());
}

TEST_F(CaseStatementTest, Assert_Null_Body) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<CaseStatement>(CaseSelectorList{}, nullptr);
        },
        "internal compiler error");
}

TEST_F(CaseStatementTest, Assert_Null_Selector) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<CaseStatement>(CaseSelectorList{nullptr},
                                    b.create<BlockStatement>(StatementList{}));
        },
        "internal compiler error");
}

TEST_F(CaseStatementTest, Assert_DifferentProgramID_Call) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<CaseStatement>(CaseSelectorList{},
                                     b2.create<BlockStatement>(StatementList{}));
        },
        "internal compiler error");
}

TEST_F(CaseStatementTest, Assert_DifferentProgramID_Selector) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<CaseStatement>(CaseSelectorList{b2.Expr(2)},
                                     b1.create<BlockStatement>(StatementList{}));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
