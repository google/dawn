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

using IndexAccessorExpressionTest = TestHelper;

TEST_F(IndexAccessorExpressionTest, Create) {
    auto* obj = Expr("obj");
    auto* idx = Expr("idx");

    auto* exp = IndexAccessor(obj, idx);
    ASSERT_EQ(exp->object, obj);
    ASSERT_EQ(exp->index, idx);
}

TEST_F(IndexAccessorExpressionTest, CreateWithSource) {
    auto* obj = Expr("obj");
    auto* idx = Expr("idx");

    auto* exp = IndexAccessor(Source{{20, 2}}, obj, idx);
    auto src = exp->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(IndexAccessorExpressionTest, IsIndexAccessor) {
    auto* obj = Expr("obj");
    auto* idx = Expr("idx");

    auto* exp = IndexAccessor(obj, idx);
    EXPECT_TRUE(exp->Is<IndexAccessorExpression>());
}

TEST_F(IndexAccessorExpressionTest, Assert_Null_Array) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.IndexAccessor(nullptr, b.Expr("idx"));
        },
        "internal compiler error");
}

TEST_F(IndexAccessorExpressionTest, Assert_Null_Index) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.IndexAccessor(b.Expr("arr"), nullptr);
        },
        "internal compiler error");
}

TEST_F(IndexAccessorExpressionTest, Assert_DifferentProgramID_Array) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.IndexAccessor(b2.Expr("arr"), b1.Expr("idx"));
        },
        "internal compiler error");
}

TEST_F(IndexAccessorExpressionTest, Assert_DifferentProgramID_Index) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.IndexAccessor(b1.Expr("arr"), b2.Expr("idx"));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
