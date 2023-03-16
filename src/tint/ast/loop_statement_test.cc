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

#include "src/tint/ast/loop_statement.h"

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using LoopStatementTest = TestHelper;

TEST_F(LoopStatementTest, Creation) {
    auto* body = Block(create<DiscardStatement>());
    auto* b = body->Last();

    auto* continuing = Block(create<DiscardStatement>());

    auto* l = create<LoopStatement>(body, continuing, utils::Empty);
    ASSERT_EQ(l->body->statements.Length(), 1u);
    EXPECT_EQ(l->body->statements[0], b);
    ASSERT_EQ(l->continuing->statements.Length(), 1u);
    EXPECT_EQ(l->continuing->statements[0], continuing->Last());
}

TEST_F(LoopStatementTest, Creation_WithSource) {
    auto* body = Block(create<DiscardStatement>());

    auto* continuing = Block(create<DiscardStatement>());

    auto* l =
        create<LoopStatement>(Source{Source::Location{20, 2}}, body, continuing, utils::Empty);
    auto src = l->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(LoopStatementTest, Creation_WithAttributes) {
    auto* attr1 = DiagnosticAttribute(builtin::DiagnosticSeverity::kOff, "foo");
    auto* attr2 = DiagnosticAttribute(builtin::DiagnosticSeverity::kOff, "bar");

    auto* body = Block(Return());
    auto* l = create<LoopStatement>(body, nullptr, utils::Vector{attr1, attr2});

    EXPECT_THAT(l->attributes, testing::ElementsAre(attr1, attr2));
}

TEST_F(LoopStatementTest, IsLoop) {
    auto* l = create<LoopStatement>(Block(), Block(), utils::Empty);
    EXPECT_TRUE(l->Is<LoopStatement>());
}

TEST_F(LoopStatementTest, HasContinuing_WithoutContinuing) {
    auto* body = Block(create<DiscardStatement>());

    auto* l = create<LoopStatement>(body, nullptr, utils::Empty);
    EXPECT_FALSE(l->continuing);
}

TEST_F(LoopStatementTest, HasContinuing_WithContinuing) {
    auto* body = Block(create<DiscardStatement>());

    auto* continuing = Block(create<DiscardStatement>());

    auto* l = create<LoopStatement>(body, continuing, utils::Empty);
    EXPECT_TRUE(l->continuing);
}

TEST_F(LoopStatementTest, Assert_Null_Body) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<LoopStatement>(nullptr, nullptr, utils::Empty);
        },
        "internal compiler error");
}

TEST_F(LoopStatementTest, Assert_DifferentProgramID_Body) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<LoopStatement>(b2.Block(), b1.Block(), utils::Empty);
        },
        "internal compiler error");
}

TEST_F(LoopStatementTest, Assert_DifferentProgramID_Continuing) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<LoopStatement>(b1.Block(), b2.Block(), utils::Empty);
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
