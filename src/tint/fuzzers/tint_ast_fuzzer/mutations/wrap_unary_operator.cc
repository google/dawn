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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/wrap_unary_operator.h"

#include <utility>
#include <vector>

#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/statement.h"

namespace tint::fuzzers::ast_fuzzer {

MutationWrapUnaryOperator::MutationWrapUnaryOperator(protobufs::MutationWrapUnaryOperator message)
    : message_(std::move(message)) {}

MutationWrapUnaryOperator::MutationWrapUnaryOperator(uint32_t expression_id,
                                                     uint32_t fresh_id,
                                                     core::UnaryOp unary_op_wrapper) {
    message_.set_expression_id(expression_id);
    message_.set_fresh_id(fresh_id);
    message_.set_unary_op_wrapper(static_cast<uint32_t>(unary_op_wrapper));
}

bool MutationWrapUnaryOperator::IsApplicable(const tint::Program& program,
                                             const NodeIdMap& node_id_map) const {
    // Check if id that will be assigned is fresh.
    if (!node_id_map.IdIsFreshAndValid(message_.fresh_id())) {
        return false;
    }

    const auto* expression_ast_node =
        tint::As<ast::Expression>(node_id_map.GetNode(message_.expression_id()));

    if (!expression_ast_node) {
        // Either the node is not present with the given id or
        // the node is not a valid expression type.
        return false;
    }

    const auto* expression_sem_node = program.Sem().GetVal(expression_ast_node);

    if (!expression_sem_node) {
        // Semantic information for the expression ast node is not present
        // or the semantic node is not a valid expression type node.
        return false;
    }

    core::UnaryOp unary_op_wrapper = static_cast<core::UnaryOp>(message_.unary_op_wrapper());

    std::vector<core::UnaryOp> valid_ops = GetValidUnaryWrapper(*expression_sem_node);

    // There is no available unary operator or |unary_op_wrapper| is a
    // type that is not allowed for the given expression.
    if (std::find(valid_ops.begin(), valid_ops.end(), unary_op_wrapper) == valid_ops.end()) {
        return false;
    }

    return true;
}

void MutationWrapUnaryOperator::Apply(const NodeIdMap& node_id_map,
                                      tint::program::CloneContext& clone_context,
                                      NodeIdMap* new_node_id_map) const {
    auto* expression_node =
        tint::As<ast::Expression>(node_id_map.GetNode(message_.expression_id()));

    auto* replacement_expression_node = clone_context.dst->create<ast::UnaryOpExpression>(
        static_cast<core::UnaryOp>(message_.unary_op_wrapper()),
        clone_context.Clone(expression_node));

    clone_context.Replace(expression_node, replacement_expression_node);

    new_node_id_map->Add(replacement_expression_node, message_.fresh_id());
}

protobufs::Mutation MutationWrapUnaryOperator::ToMessage() const {
    protobufs::Mutation mutation;
    *mutation.mutable_wrap_unary_operator() = message_;
    return mutation;
}

std::vector<core::UnaryOp> MutationWrapUnaryOperator::GetValidUnaryWrapper(
    const sem::ValueExpression& expr) {
    if (auto* call_expr = expr.As<sem::Call>()) {
        if (auto* stmt = call_expr->Stmt()) {
            if (auto* call_stmt = stmt->Declaration()->As<ast::CallStatement>()) {
                if (call_stmt->expr == expr.Declaration()) {
                    return {};  // A call statement must only wrap a call expression.
                }
            }
        }
    }

    const auto* expr_type = expr.Type();
    if (expr_type->is_bool_scalar_or_vector()) {
        return {core::UnaryOp::kNot};
    }

    if (expr_type->is_signed_integer_scalar_or_vector() ||
        expr_type->is_abstract_integer_scalar_or_vector()) {
        return {core::UnaryOp::kNegation, core::UnaryOp::kComplement};
    }

    if (expr_type->is_unsigned_integer_scalar_or_vector()) {
        return {core::UnaryOp::kComplement};
    }

    if (expr_type->is_float_scalar_or_vector() || expr_type->is_abstract_float_scalar_or_vector()) {
        return {core::UnaryOp::kNegation};
    }

    return {};
}

}  // namespace tint::fuzzers::ast_fuzzer
