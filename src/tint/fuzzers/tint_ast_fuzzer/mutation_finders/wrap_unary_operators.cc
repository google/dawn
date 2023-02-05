// Copyright 2021 The Tint Authors.
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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation_finders/wrap_unary_operators.h"

#include <memory>
#include <vector>

#include "src/tint/fuzzers/tint_ast_fuzzer/expression_size.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/wrap_unary_operator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/util.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/value_expression.h"

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

        std::vector<ast::UnaryOp> valid_operators =
            MutationWrapUnaryOperator::GetValidUnaryWrapper(*expr_sem_node);

        // Transformation only applies when there are available unary operators
        // for the given expression.
        if (valid_operators.empty()) {
            continue;
        }

        ast::UnaryOp unary_op_wrapper =
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
