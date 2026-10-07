// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/interpolated_string_expression.h"

#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/ast/string_literal_expression.h"

namespace tint::ast {
namespace {

using InterpolatedStringExpressionTest = TestHelper;
using InterpolatedStringExpressionDeathTest = InterpolatedStringExpressionTest;

TEST_F(InterpolatedStringExpressionTest, Creation) {
    auto* s1 = create<StringLiteralExpression>("hello ");
    auto* id = Expr("name");
    auto* s2 = create<StringLiteralExpression>("!");

    auto* tmpl = create<InterpolatedStringExpression>(Vector{s1, id, s2});
    ASSERT_TRUE(tmpl->Is<InterpolatedStringExpression>());
    ASSERT_EQ(tmpl->elements.Length(), 3u);
    EXPECT_EQ(tmpl->elements[0], s1);
    EXPECT_EQ(tmpl->elements[1], id);
    EXPECT_EQ(tmpl->elements[2], s2);
}

TEST_F(InterpolatedStringExpressionTest, WithSource) {
    auto* s = create<StringLiteralExpression>("test");
    auto* tmpl = create<InterpolatedStringExpression>(Source{{5, 10}}, Vector{s});
    ASSERT_TRUE(tmpl->Is<InterpolatedStringExpression>());
    EXPECT_EQ(tmpl->source.range.begin.line, 5u);
    EXPECT_EQ(tmpl->source.range.begin.column, 10u);
    ASSERT_EQ(tmpl->elements.Length(), 1u);
    EXPECT_EQ(tmpl->elements[0], s);
}

TEST_F(InterpolatedStringExpressionDeathTest, Assert_Null_Element) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.create<InterpolatedStringExpression>(tint::Vector<const Expression*, 2>{
                b.create<StringLiteralExpression>("hello"),
                nullptr,
            });
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
