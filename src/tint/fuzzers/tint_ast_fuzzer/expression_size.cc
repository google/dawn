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

#include "src/tint/ast/traverse_expressions.h"

namespace tint::fuzzers::ast_fuzzer {

ExpressionSize::ExpressionSize(const Program& program) {
    // By construction, all the children of an AST node are encountered before the
    // node itself when iterating through a program's AST nodes. Computing
    // expression sizes exploits this property: the size of a compound expression
    // is computed based on the already-computed sizes of its sub-expressions.
    for (const auto* node : program.ASTNodes().Objects()) {
        const auto* expr_ast_node = node->As<ast::Expression>();
        if (expr_ast_node == nullptr) {
            continue;
        }
        size_t expr_size = 0;
        diag::List empty;
        ast::TraverseExpressions(expr_ast_node, empty, [&](const ast::Expression* expression) {
            if (expression == expr_ast_node) {
                expr_size++;
                return ast::TraverseAction::Descend;
            }
            expr_size += expr_to_size_.at(expression);
            return ast::TraverseAction::Skip;
        });
        expr_to_size_[expr_ast_node] = expr_size;
    }
}

}  // namespace tint::fuzzers::ast_fuzzer
