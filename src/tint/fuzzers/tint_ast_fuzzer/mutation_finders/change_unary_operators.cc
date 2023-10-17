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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation_finders/change_unary_operators.h"

#include <memory>

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/change_unary_operator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/util.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/wgsl/ast/unary_op_expression.h"

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
        const auto* basic_type = type->Is<core::type::Reference>()
                                     ? type->As<core::type::Reference>()->StoreType()
                                     : type;

        // Only signed integer or vector of signed integer can be mutated.
        if (!basic_type->is_signed_integer_scalar_or_vector()) {
            continue;
        }

        // Only complement and negation operators can be swapped.
        if (!(unary_expr->op == core::UnaryOp::kComplement ||
              unary_expr->op == core::UnaryOp::kNegation)) {
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
