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
using TypeNameTest = TestHelper;

TEST_F(TypeNameTest, Creation_NonTemplated) {
    auto* t = ty("ty");
    ASSERT_NE(t->name, nullptr);
    EXPECT_EQ(t->name->symbol, Symbols().Get("ty"));
}

TEST_F(TypeNameTest, Creation_Templated) {
    auto* t = ty("ty", 1_a, 2._a, false);
    auto* name = As<ast::TemplatedIdentifier>(t->name);
    ASSERT_NE(name, nullptr);
    EXPECT_EQ(name->symbol, Symbols().Get("ty"));
    ASSERT_EQ(name->arguments.Length(), 3u);
    EXPECT_TRUE(name->arguments[0]->Is<ast::IntLiteralExpression>());
    EXPECT_TRUE(name->arguments[1]->Is<ast::FloatLiteralExpression>());
    EXPECT_TRUE(name->arguments[2]->Is<ast::BoolLiteralExpression>());
}

TEST_F(TypeNameTest, Creation_WithSource) {
    auto* t = ty(Source{{20, 2}}, "ty");
    ASSERT_NE(t->name, nullptr);
    EXPECT_EQ(t->name->symbol, Symbols().Get("ty"));

    auto src = t->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(TypeNameTest, Assert_InvalidSymbol) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.ty("");
        },
        "internal compiler error");
}

TEST_F(TypeNameTest, Assert_DifferentProgramID_Symbol) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.ty(b2.Sym("b2"));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
