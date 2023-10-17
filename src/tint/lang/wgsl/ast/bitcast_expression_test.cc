// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/wgsl/ast/bitcast_expression.h"

#include "gtest/gtest-spi.h"
#include "src/tint/lang/wgsl/ast/helper_test.h"

namespace tint::ast {
namespace {

using BitcastExpressionTest = TestHelper;

TEST_F(BitcastExpressionTest, Create) {
    auto* expr = Expr("expr");
    auto* exp = Bitcast(ty.f32(), expr);
    CheckIdentifier(exp->type, "f32");
    ASSERT_EQ(exp->expr, expr);
}

TEST_F(BitcastExpressionTest, CreateWithSource) {
    auto* expr = Expr("expr");

    auto* exp = Bitcast(Source{Source::Location{20, 2}}, ty.f32(), expr);
    auto src = exp->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(BitcastExpressionTest, IsBitcast) {
    auto* expr = Expr("expr");

    auto* exp = Bitcast(ty.f32(), expr);
    EXPECT_TRUE(exp->Is<BitcastExpression>());
}

TEST_F(BitcastExpressionTest, Assert_Null_Type) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.Bitcast(ast::Type(), "idx");
        },
        "internal compiler error");
}

TEST_F(BitcastExpressionTest, Assert_Null_Expr) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.Bitcast(b.ty.f32(), nullptr);
        },
        "internal compiler error");
}

TEST_F(BitcastExpressionTest, Assert_DifferentGenerationID_Expr) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Bitcast(b1.ty.f32(), b2.Expr("idx"));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
