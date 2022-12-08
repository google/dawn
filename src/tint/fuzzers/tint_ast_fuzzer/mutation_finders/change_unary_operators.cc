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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation_finders/change_unary_operators.h"

#include <memory>

#include "src/tint/ast/unary_op_expression.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/change_unary_operator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/util.h"
#include "src/tint/type/reference.h"

namespace tint::fuzzers::ast_fuzzer {

MutationList MutationFinderChangeUnaryOperators::FindMutations(
    const tint::Program& program,
    NodeIdMap* node_id_map,
    ProbabilityContext* /*unused*/) const {
    MutationList result;

    // Iterate through all AST nodes and for each valid unary op expression node,
    // try to replace the node's unary operator.
    for (const auto* node : program.ASTNodes().Objects()) {
        const auto* unary_expr = tint::As<ast::UnaryOpExpression>(node);

        // Transformation applies only when the node represents a valid unary
        // expression.
        if (!unary_expr) {
            continue;
        }

        // Get the type of the unary expression.
        const auto* type = program.Sem().Get(unary_expr)->Type();
        const auto* basic_type =
            type->Is<type::Reference>() ? type->As<type::Reference>()->StoreType() : type;

        // Only signed integer or vector of signed integer can be mutated.
        if (!basic_type->is_signed_integer_scalar_or_vector()) {
            continue;
        }

        // Only complement and negation operators can be swapped.
        if (!(unary_expr->op == ast::UnaryOp::kComplement ||
              unary_expr->op == ast::UnaryOp::kNegation)) {
            continue;
        }

        result.push_back(std::make_unique<MutationChangeUnaryOperator>(
            node_id_map->GetId(unary_expr),
            MutationChangeUnaryOperator::ToggleOperator(unary_expr->op)));
    }

    return result;
}

uint32_t MutationFinderChangeUnaryOperators::GetChanceOfApplyingMutation(
    ProbabilityContext* probability_context) const {
    return probability_context->GetChanceOfChangingUnaryOperators();
}

}  // namespace tint::fuzzers::ast_fuzzer
