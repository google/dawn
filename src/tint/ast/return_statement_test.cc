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

#include "src/tint/ast/return_statement.h"

#include "gtest/gtest-spi.h"
#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using ReturnStatementTest = TestHelper;

TEST_F(ReturnStatementTest, Creation) {
    auto* expr = Expr("expr");

    auto* r = create<ReturnStatement>(expr);
    EXPECT_EQ(r->value, expr);
}

TEST_F(ReturnStatementTest, Creation_WithSource) {
    auto* r = create<ReturnStatement>(Source{Source::Location{20, 2}});
    auto src = r->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(ReturnStatementTest, IsReturn) {
    auto* r = create<ReturnStatement>();
    EXPECT_TRUE(r->Is<ReturnStatement>());
}

TEST_F(ReturnStatementTest, WithoutValue) {
    auto* r = create<ReturnStatement>();
    EXPECT_EQ(r->value, nullptr);
}

TEST_F(ReturnStatementTest, WithValue) {
    auto* expr = Expr("expr");
    auto* r = create<ReturnStatement>(expr);
    EXPECT_NE(r->value, nullptr);
}

TEST_F(ReturnStatementTest, Assert_DifferentProgramID_Expr) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<ReturnStatement>(b2.Expr(true));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
