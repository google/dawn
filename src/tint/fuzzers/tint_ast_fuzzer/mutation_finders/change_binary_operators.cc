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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation_finders/change_binary_operators.h"

#include <memory>
#include <vector>

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/change_binary_operator.h"
#include "src/tint/lang/wgsl/ast/binary_expression.h"

namespace tint::fuzzers::ast_fuzzer {

MutationList MutationFinderChangeBinaryOperators::FindMutations(
    const tint::Program& program,
    NodeIdMap* node_id_map,
    ProbabilityContext* probability_context) const {
    MutationList result;

    // Go through each binary expression in the AST and add a mutation that
    // replaces its operator with some other type-compatible operator.

    const std::vector<core::BinaryOp> all_binary_operators = {core::BinaryOp::kAnd,
                                                              core::BinaryOp::kOr,
                                                              core::BinaryOp::kXor,
                                                              core::BinaryOp::kLogicalAnd,
                                                              core::BinaryOp::kLogicalOr,
                                                              core::BinaryOp::kEqual,
                                                              core::BinaryOp::kNotEqual,
                                                              core::BinaryOp::kLessThan,
                                                              core::BinaryOp::kGreaterThan,
                                                              core::BinaryOp::kLessThanEqual,
                                                              core::BinaryOp::kGreaterThanEqual,
                                                              core::BinaryOp::kShiftLeft,
                                                              core::BinaryOp::kShiftRight,
                                                              core::BinaryOp::kAdd,
                                                              core::BinaryOp::kSubtract,
                                                              core::BinaryOp::kMultiply,
                                                              core::BinaryOp::kDivide,
                                                              core::BinaryOp::kModulo};

    for (const auto* node : program.ASTNodes().Objects()) {
        const auto* binary_expr = As<ast::BinaryExpression>(node);
        if (!binary_expr) {
            continue;
        }

        // Get vector of all operators this could be replaced with.
        std::vector<core::BinaryOp> allowed_replacements;
        for (auto candidate_op : all_binary_operators) {
            if (MutationChangeBinaryOperator::CanReplaceBinaryOperator(program, *binary_expr,
                                                                       candidate_op)) {
                allowed_replacements.push_back(candidate_op);
            }
        }

        if (!allowed_replacements.empty()) {
            // Choose an available replacement operator at random.
            const core::BinaryOp replacement =
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
