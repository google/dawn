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
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using BlockStatementTest = TestHelper;

TEST_F(BlockStatementTest, Creation) {
    auto* d = create<DiscardStatement>();
    auto* ptr = d;

    auto* b = create<BlockStatement>(StatementList{d});

    ASSERT_EQ(b->statements.size(), 1u);
    EXPECT_EQ(b->statements[0], ptr);
}

TEST_F(BlockStatementTest, Creation_WithSource) {
    auto* b = create<BlockStatement>(Source{Source::Location{20, 2}}, ast::StatementList{});
    auto src = b->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(BlockStatementTest, IsBlock) {
    auto* b = create<BlockStatement>(ast::StatementList{});
    EXPECT_TRUE(b->Is<BlockStatement>());
}

TEST_F(BlockStatementTest, Assert_Null_Statement) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<BlockStatement>(ast::StatementList{nullptr});
        },
        "internal compiler error");
}

TEST_F(BlockStatementTest, Assert_DifferentProgramID_Statement) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<BlockStatement>(ast::StatementList{b2.create<DiscardStatement>()});
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
