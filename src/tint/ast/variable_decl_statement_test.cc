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

#include "src/tint/ast/variable_decl_statement.h"

#include "gtest/gtest-spi.h"
#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using VariableDeclStatementTest = TestHelper;

TEST_F(VariableDeclStatementTest, Creation) {
    auto* var = Var("a", ty.f32());

    auto* stmt = create<VariableDeclStatement>(var);
    EXPECT_EQ(stmt->variable, var);
}

TEST_F(VariableDeclStatementTest, Creation_WithSource) {
    auto* var = Var("a", ty.f32());

    auto* stmt = create<VariableDeclStatement>(Source{Source::Location{20, 2}}, var);
    auto src = stmt->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(VariableDeclStatementTest, IsVariableDecl) {
    auto* var = Var("a", ty.f32());

    auto* stmt = create<VariableDeclStatement>(var);
    EXPECT_TRUE(stmt->Is<VariableDeclStatement>());
}

TEST_F(VariableDeclStatementTest, Assert_Null_Variable) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.create<VariableDeclStatement>(nullptr);
        },
        "internal compiler error");
}

TEST_F(VariableDeclStatementTest, Assert_DifferentProgramID_Variable) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<VariableDeclStatement>(b2.Var("a", b2.ty.f32()));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
