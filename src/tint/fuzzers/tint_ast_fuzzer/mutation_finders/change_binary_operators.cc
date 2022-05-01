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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation_finders/change_binary_operators.h"

#include <memory>
#include <vector>

#include "src/tint/ast/binary_expression.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/change_binary_operator.h"

namespace tint::fuzzers::ast_fuzzer {

MutationList MutationFinderChangeBinaryOperators::FindMutations(
    const tint::Program& program,
    NodeIdMap* node_id_map,
    ProbabilityContext* probability_context) const {
    MutationList result;

    // Go through each binary expression in the AST and add a mutation that
    // replaces its operator with some other type-compatible operator.

    const std::vector<ast::BinaryOp> all_binary_operators = {ast::BinaryOp::kAnd,
                                                             ast::BinaryOp::kOr,
                                                             ast::BinaryOp::kXor,
                                                             ast::BinaryOp::kLogicalAnd,
                                                             ast::BinaryOp::kLogicalOr,
                                                             ast::BinaryOp::kEqual,
                                                             ast::BinaryOp::kNotEqual,
                                                             ast::BinaryOp::kLessThan,
                                                             ast::BinaryOp::kGreaterThan,
                                                             ast::BinaryOp::kLessThanEqual,
                                                             ast::BinaryOp::kGreaterThanEqual,
                                                             ast::BinaryOp::kShiftLeft,
                                                             ast::BinaryOp::kShiftRight,
                                                             ast::BinaryOp::kAdd,
                                                             ast::BinaryOp::kSubtract,
                                                             ast::BinaryOp::kMultiply,
                                                             ast::BinaryOp::kDivide,
                                                             ast::BinaryOp::kModulo};

    for (const auto* node : program.ASTNodes().Objects()) {
        const auto* binary_expr = As<ast::BinaryExpression>(node);
        if (!binary_expr) {
            continue;
        }

        // Get vector of all operators this could be replaced with.
        std::vector<ast::BinaryOp> allowed_replacements;
        for (auto candidate_op : all_binary_operators) {
            if (MutationChangeBinaryOperator::CanReplaceBinaryOperator(program, *binary_expr,
                                                                       candidate_op)) {
                allowed_replacements.push_back(candidate_op);
            }
        }

        if (!allowed_replacements.empty()) {
            // Choose an available replacement operator at random.
            const ast::BinaryOp replacement =
                allowed_replacements[probability_context->GetRandomIndex(allowed_replacements)];
            // Add a mutation according to the chosen replacement.
            result.push_back(std::make_unique<MutationChangeBinaryOperator>(
                node_id_map->GetId(binary_expr), replacement));
        }
    }

    return result;
}

uint32_t MutationFinderChangeBinaryOperators::GetChanceOfApplyingMutation(
    ProbabilityContext* probability_context) const {
    return probability_context->GetChanceOfChangingBinaryOperators();
}

}  // namespace tint::fuzzers::ast_fuzzer
