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

#include "src/tint/ast/switch_statement.h"

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/ast/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ast {
namespace {

using SwitchStatementTest = TestHelper;

TEST_F(SwitchStatementTest, Creation) {
    auto* case_stmt = create<CaseStatement>(utils::Vector{CaseSelector(1_u)}, Block());
    auto* ident = Expr("ident");
    utils::Vector body{case_stmt};

    auto* stmt = create<SwitchStatement>(ident, body, utils::Empty, utils::Empty);
    EXPECT_EQ(stmt->condition, ident);
    ASSERT_EQ(stmt->body.Length(), 1u);
    EXPECT_EQ(stmt->body[0], case_stmt);
}

TEST_F(SwitchStatementTest, Creation_WithSource) {
    auto* ident = Expr("ident");
    auto* stmt = create<SwitchStatement>(Source{Source::Location{20, 2}}, ident, utils::Empty,
                                         utils::Empty, utils::Empty);
    auto src = stmt->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(SwitchStatementTest, Creation_WithAttributes) {
    auto* attr1 = DiagnosticAttribute(builtin::DiagnosticSeverity::kOff, "foo");
    auto* attr2 = DiagnosticAttribute(builtin::DiagnosticSeverity::kOff, "bar");
    auto* ident = Expr("ident");
    auto* stmt =
        create<SwitchStatement>(ident, utils::Empty, utils::Vector{attr1, attr2}, utils::Empty);

    EXPECT_THAT(stmt->attributes, testing::ElementsAre(attr1, attr2));
}

TEST_F(SwitchStatementTest, Creation_WithBodyAttributes) {
    auto* attr1 = DiagnosticAttribute(builtin::DiagnosticSeverity::kOff, "foo");
    auto* attr2 = DiagnosticAttribute(builtin::DiagnosticSeverity::kOff, "bar");
    auto* ident = Expr("ident");
    auto* stmt =
        create<SwitchStatement>(ident, utils::Empty, utils::Empty, utils::Vector{attr1, attr2});

    EXPECT_THAT(stmt->body_attributes, testing::ElementsAre(attr1, attr2));
}

TEST_F(SwitchStatementTest, IsSwitch) {
    utils::Vector lit{CaseSelector(2_i)};
    auto* ident = Expr("ident");
    utils::Vector body{create<CaseStatement>(lit, Block())};

    auto* stmt = create<SwitchStatement>(ident, body, utils::Empty, utils::Empty);
    EXPECT_TRUE(stmt->Is<SwitchStatement>());
}

TEST_F(SwitchStatementTest, Assert_Null_Condition) {
    using CaseStatementList = utils::Vector<const CaseStatement*, 2>;
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            CaseStatementList cases;
            cases.Push(
                b.create<CaseStatement>(utils::Vector{b.CaseSelector(b.Expr(1_i))}, b.Block()));
            b.create<SwitchStatement>(nullptr, cases, utils::Empty, utils::Empty);
        },
        "internal compiler error");
}

TEST_F(SwitchStatementTest, Assert_Null_CaseStatement) {
    using CaseStatementList = utils::Vector<const CaseStatement*, 2>;
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<SwitchStatement>(b.Expr(true), CaseStatementList{nullptr}, utils::Empty,
                                      utils::Empty);
        },
        "internal compiler error");
}

TEST_F(SwitchStatementTest, Assert_DifferentProgramID_Condition) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<SwitchStatement>(b2.Expr(true),
                                       utils::Vector{
                                           b1.create<CaseStatement>(
                                               utils::Vector{
                                                   b1.CaseSelector(b1.Expr(1_i)),
                                               },
                                               b1.Block()),
                                       },
                                       utils::Empty, utils::Empty);
        },
        "internal compiler error");
}

TEST_F(SwitchStatementTest, Assert_DifferentProgramID_CaseStatement) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<SwitchStatement>(b1.Expr(true),
                                       utils::Vector{
                                           b2.create<CaseStatement>(
                                               utils::Vector{
                                                   b2.CaseSelector(b2.Expr(1_i)),
                                               },
                                               b2.Block()),
                                       },
                                       utils::Empty, utils::Empty);
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
