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
    auto* t = ty.type_name("ty");
    ASSERT_NE(t->name, nullptr);
    EXPECT_EQ(t->name->symbol, Symbols().Get("ty"));
}

TEST_F(TypeNameTest, Creation_WithSource) {
    auto* t = ty.type_name(Source{{20, 2}}, "ty");
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
            b.ty.type_name("");
        },
        "internal compiler error");
}

TEST_F(TypeNameTest, Assert_DifferentProgramID_Symbol) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.ty.type_name(b2.Sym("b2"));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
