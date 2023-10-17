// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation_finders/wrap_unary_operators.h"

#include <memory>
#include <vector>

#include "src/tint/fuzzers/tint_ast_fuzzer/expression_size.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/wrap_unary_operator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/util.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"

namespace tint::fuzzers::ast_fuzzer {

namespace {
const size_t kMaxExpressionSize = 50;
}  // namespace

MutationList MutationFinderWrapUnaryOperators::FindMutations(
    const tint::Program& program,
    NodeIdMap* node_id_map,
    ProbabilityContext* probability_context) const {
    MutationList result;

    ExpressionSize expression_size(program);

    // Iterate through all ast nodes and for each expression node, try to wrap
    // the inside a valid unary operator based on the type of the expression.
    for (const auto* node : program.ASTNodes().Objects()) {
        const auto* expr_ast_node = tint::As<ast::Expression>(node);

        // Transformation applies only when the node represents a valid expression.
        if (!expr_ast_node) {
            continue;
        }

        if (expression_size(expr_ast_node) > kMaxExpressionSize) {
            continue;
        }

        const auto* expr_sem_node = program.Sem().GetVal(expr_ast_node);

        // Transformation applies only when the semantic node for the given
        // expression is present.
        if (!expr_sem_node) {
            continue;
        }

        std::vector<core::UnaryOp> valid_operators =
            MutationWrapUnaryOperator::GetValidUnaryWrapper(*expr_sem_node);

        // Transformation only applies when there are available unary operators
        // for the given expression.
        if (valid_operators.empty()) {
            continue;
        }

        core::UnaryOp unary_op_wrapper =
            valid_operators[probability_context->GetRandomIndex(valid_operators)];

        result.push_back(std::make_unique<MutationWrapUnaryOperator>(
            node_id_map->GetId(expr_ast_node), node_id_map->TakeFreshId(), unary_op_wrapper));
    }

    return result;
}

uint32_t MutationFinderWrapUnaryOperators::GetChanceOfApplyingMutation(
    ProbabilityContext* probability_context) const {
    return probability_context->GetChanceOfWrappingUnaryOperators();
}

}  // namespace tint::fuzzers::ast_fuzzer
