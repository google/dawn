// Copyright 2023 The Tint Authors.
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

using namespace tint::number_suffixes;  // NOLINT

using TemplatedIdentifierTest = TestHelper;

TEST_F(TemplatedIdentifierTest, Creation) {
    auto* i = Ident("ident", 1_a, Add("x", "y"), false, "x");
    EXPECT_EQ(i->symbol, Symbols().Get("ident"));
    auto* t = i->As<TemplatedIdentifier>();
    ASSERT_NE(t, nullptr);
    ASSERT_EQ(t->arguments.Length(), 4u);
    EXPECT_TRUE(t->arguments[0]->Is<IntLiteralExpression>());
    EXPECT_TRUE(t->arguments[1]->Is<BinaryExpression>());
    EXPECT_TRUE(t->arguments[2]->Is<BoolLiteralExpression>());
    EXPECT_TRUE(t->arguments[3]->Is<IdentifierExpression>());
}

TEST_F(TemplatedIdentifierTest, Creation_WithSource) {
    auto* i = Ident(Source{{20, 2}}, "ident", 1_a, Add("x", "y"), false, "x");
    EXPECT_EQ(i->symbol, Symbols().Get("ident"));
    auto* t = i->As<TemplatedIdentifier>();
    ASSERT_NE(t, nullptr);
    ASSERT_EQ(t->arguments.Length(), 4u);
    EXPECT_TRUE(t->arguments[0]->Is<IntLiteralExpression>());
    EXPECT_TRUE(t->arguments[1]->Is<BinaryExpression>());
    EXPECT_TRUE(t->arguments[2]->Is<BoolLiteralExpression>());
    EXPECT_TRUE(t->arguments[3]->Is<IdentifierExpression>());

    auto src = i->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(TemplatedIdentifierTest, Assert_InvalidSymbol) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.Expr("");
        },
        "internal compiler error");
}

TEST_F(TemplatedIdentifierTest, Assert_DifferentProgramID_Symbol) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Ident(b2.Sym("b2"), b1.Expr(1_i));
        },
        "internal compiler error");
}

TEST_F(TemplatedIdentifierTest, Assert_DifferentProgramID_TemplateArg) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Ident("b1", b2.Expr(1_i));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
