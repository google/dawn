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

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/ast/if_statement.h"

namespace tint::ast {
namespace {

using BlockStatementTest = TestHelper;

TEST_F(BlockStatementTest, Creation) {
    auto* d = create<DiscardStatement>();
    auto* ptr = d;

    auto* b = create<BlockStatement>(tint::Vector{d}, tint::Empty);

    ASSERT_EQ(b->statements.Length(), 1u);
    EXPECT_EQ(b->statements[0], ptr);
    EXPECT_EQ(b->attributes.Length(), 0u);
}

TEST_F(BlockStatementTest, Creation_WithSource) {
    auto* b = create<BlockStatement>(Source{Source::Location{20, 2}}, tint::Empty, tint::Empty);
    auto src = b->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(BlockStatementTest, Creation_WithAttributes) {
    auto* d = create<DiscardStatement>();
    auto* ptr = d;

    auto* attr1 = DiagnosticAttribute(core::DiagnosticSeverity::kOff, "foo");
    auto* attr2 = DiagnosticAttribute(core::DiagnosticSeverity::kOff, "bar");
    auto* b = create<BlockStatement>(tint::Vector{d}, tint::Vector{attr1, attr2});

    ASSERT_EQ(b->statements.Length(), 1u);
    EXPECT_EQ(b->statements[0], ptr);
    EXPECT_THAT(b->attributes, testing::ElementsAre(attr1, attr2));
}

TEST_F(BlockStatementTest, IsBlock) {
    auto* b = create<BlockStatement>(tint::Empty, tint::Empty);
    EXPECT_TRUE(b->Is<BlockStatement>());
}

TEST_F(BlockStatementTest, Assert_Null_Statement) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<BlockStatement>(tint::Vector<const Statement*, 1>{nullptr}, tint::Empty);
        },
        "internal compiler error");
}

TEST_F(BlockStatementTest, Assert_DifferentGenerationID_Statement) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<BlockStatement>(tint::Vector{b2.create<DiscardStatement>()}, tint::Empty);
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
