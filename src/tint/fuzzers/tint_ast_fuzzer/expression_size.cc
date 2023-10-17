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

#include "src/tint/lang/wgsl/ast/traverse_expressions.h"

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
        ast::TraverseExpressions(expr_ast_node, [&](const ast::Expression* expression) {
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
