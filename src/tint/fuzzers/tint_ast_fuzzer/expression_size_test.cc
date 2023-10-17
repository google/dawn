// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/fuzzers/tint_ast_fuzzer/expression_size.h"

#include <string>

#include "gtest/gtest.h"

#include "src/tint/lang/wgsl/ast/binary_expression.h"
#include "src/tint/lang/wgsl/ast/expression.h"
#include "src/tint/lang/wgsl/ast/int_literal_expression.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/reader/reader.h"

namespace tint::fuzzers::ast_fuzzer {
namespace {

TEST(ExpressionSizeTest, Basic) {
    std::string content = R"(
    fn main() {
      let a = (0 + 0) * (0 + 0);
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    ExpressionSize expression_size(program);
    for (const auto* node : program.ASTNodes().Objects()) {
        const auto* expr = node->As<ast::Expression>();
        if (expr == nullptr) {
            continue;
        }
        if (expr->Is<ast::IntLiteralExpression>()) {
            ASSERT_EQ(1, expression_size(expr));
        } else {
            const auto* binary_expr = expr->As<ast::BinaryExpression>();
            ASSERT_TRUE(binary_expr != nullptr);
            switch (binary_expr->op) {
                case core::BinaryOp::kAdd:
                    ASSERT_EQ(3, expression_size(expr));
                    break;
                case core::BinaryOp::kMultiply:
                    ASSERT_EQ(7, expression_size(expr));
                    break;
                default:
                    FAIL();
            }
        }
    }
}

}  // namespace
}  // namespace tint::fuzzers::ast_fuzzer
