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
#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IdentifierExpressionTest = TestHelper;

TEST_F(IdentifierExpressionTest, Creation) {
    auto* i = Expr("ident");
    EXPECT_EQ(i->identifier->symbol, Symbol(1, ID(), "ident"));
}

TEST_F(IdentifierExpressionTest, CreationTemplated) {
    auto* i = Expr(Ident("ident", true));
    EXPECT_EQ(i->identifier->symbol, Symbol(1, ID(), "ident"));
    auto* tmpl_ident = i->identifier->As<TemplatedIdentifier>();
    ASSERT_NE(tmpl_ident, nullptr);
    EXPECT_EQ(tmpl_ident->arguments.Length(), 1_u);
    EXPECT_TRUE(tmpl_ident->arguments[0]->Is<BoolLiteralExpression>());
}

TEST_F(IdentifierExpressionTest, Creation_WithSource) {
    auto* i = Expr(Source{{20, 2}}, "ident");
    EXPECT_EQ(i->identifier->symbol, Symbol(1, ID(), "ident"));

    EXPECT_EQ(i->source.range, (Source::Range{{20, 2}}));
    EXPECT_EQ(i->identifier->source.range, (Source::Range{{20, 2}}));
}

TEST_F(IdentifierExpressionTest, Assert_InvalidSymbol) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.Expr("");
        },
        "internal compiler error");
}

TEST_F(IdentifierExpressionTest, Assert_DifferentProgramID_Symbol) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Expr(b2.Sym("b2"));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
