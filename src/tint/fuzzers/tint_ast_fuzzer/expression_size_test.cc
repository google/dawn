// Copyright 2022 The Tint Authors.
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

#include "src/tint/fuzzers/tint_ast_fuzzer/expression_size.h"

#include <string>

#include "gtest/gtest.h"

#include "src/tint/ast/binary_expression.h"
#include "src/tint/ast/expression.h"
#include "src/tint/ast/int_literal_expression.h"
#include "src/tint/program.h"
#include "src/tint/reader/wgsl/parser.h"

namespace tint::fuzzers::ast_fuzzer {
namespace {

TEST(ExpressionSizeTest, Basic) {
    std::string content = R"(
    fn main() {
      let a = (0 + 0) * (0 + 0);
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

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
                case ast::BinaryOp::kAdd:
                    ASSERT_EQ(3, expression_size(expr));
                    break;
                case ast::BinaryOp::kMultiply:
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
