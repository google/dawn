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

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ast {
namespace {

using CaseStatementTest = TestHelper;

TEST_F(CaseStatementTest, Creation_i32) {
    auto* selector = CaseSelector(2_i);
    utils::Vector b{selector};

    auto* discard = create<DiscardStatement>();
    auto* body = create<BlockStatement>(utils::Vector{discard}, utils::Empty);

    auto* c = create<CaseStatement>(b, body);
    ASSERT_EQ(c->selectors.Length(), 1u);
    EXPECT_EQ(c->selectors[0], selector);
    ASSERT_EQ(c->body->statements.Length(), 1u);
    EXPECT_EQ(c->body->statements[0], discard);
}

TEST_F(CaseStatementTest, Creation_u32) {
    auto* selector = CaseSelector(2_u);
    utils::Vector b{selector};

    auto* discard = create<DiscardStatement>();
    auto* body = create<BlockStatement>(utils::Vector{discard}, utils::Empty);

    auto* c = create<CaseStatement>(b, body);
    ASSERT_EQ(c->selectors.Length(), 1u);
    EXPECT_EQ(c->selectors[0], selector);
    ASSERT_EQ(c->body->statements.Length(), 1u);
    EXPECT_EQ(c->body->statements[0], discard);
}

TEST_F(CaseStatementTest, ContainsDefault_WithDefault) {
    utils::Vector b{CaseSelector(2_u), DefaultCaseSelector()};
    auto* c = create<CaseStatement>(b, create<BlockStatement>(utils::Empty, utils::Empty));
    EXPECT_TRUE(c->ContainsDefault());
}

TEST_F(CaseStatementTest, ContainsDefault_WithOutDefault) {
    utils::Vector b{CaseSelector(2_u), CaseSelector(3_u)};
    auto* c = create<CaseStatement>(b, create<BlockStatement>(utils::Empty, utils::Empty));
    EXPECT_FALSE(c->ContainsDefault());
}

TEST_F(CaseStatementTest, Creation_WithSource) {
    utils::Vector b{CaseSelector(2_i)};

    auto* body = create<BlockStatement>(
        utils::Vector{
            create<DiscardStatement>(),
        },
        utils::Empty);
    auto* c = create<CaseStatement>(Source{Source::Location{20, 2}}, b, body);
    auto src = c->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(CaseStatementTest, IsCase) {
    auto* c = create<CaseStatement>(utils::Vector{DefaultCaseSelector()},
                                    create<BlockStatement>(utils::Empty, utils::Empty));
    EXPECT_TRUE(c->Is<CaseStatement>());
}

TEST_F(CaseStatementTest, Assert_Null_Body) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<CaseStatement>(utils::Vector{b.DefaultCaseSelector()}, nullptr);
        },
        "internal compiler error");
}

TEST_F(CaseStatementTest, Assert_Null_Selector) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<CaseStatement>(utils::Vector<const ast::CaseSelector*, 1>{nullptr},
                                    b.create<BlockStatement>(utils::Empty, utils::Empty));
        },
        "internal compiler error");
}

TEST_F(CaseStatementTest, Assert_DifferentProgramID_Call) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<CaseStatement>(utils::Vector{b1.DefaultCaseSelector()},
                                     b2.create<BlockStatement>(utils::Empty, utils::Empty));
        },
        "internal compiler error");
}

TEST_F(CaseStatementTest, Assert_DifferentProgramID_Selector) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<CaseStatement>(utils::Vector{b2.CaseSelector(b2.Expr(2_i))},
                                     b1.create<BlockStatement>(utils::Empty, utils::Empty));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
