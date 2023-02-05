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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/wrap_unary_operator.h"

#include <utility>
#include <vector>

#include "src/tint/program_builder.h"
#include "src/tint/type/abstract_float.h"
#include "src/tint/type/abstract_int.h"

namespace tint::fuzzers::ast_fuzzer {

MutationWrapUnaryOperator::MutationWrapUnaryOperator(protobufs::MutationWrapUnaryOperator message)
    : message_(std::move(message)) {}

MutationWrapUnaryOperator::MutationWrapUnaryOperator(uint32_t expression_id,
                                                     uint32_t fresh_id,
                                                     ast::UnaryOp unary_op_wrapper) {
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

    ast::UnaryOp unary_op_wrapper = static_cast<ast::UnaryOp>(message_.unary_op_wrapper());

    std::vector<ast::UnaryOp> valid_ops = GetValidUnaryWrapper(*expression_sem_node);

    // There is no available unary operator or |unary_op_wrapper| is a
    // type that is not allowed for the given expression.
    if (std::find(valid_ops.begin(), valid_ops.end(), unary_op_wrapper) == valid_ops.end()) {
        return false;
    }

    return true;
}

void MutationWrapUnaryOperator::Apply(const NodeIdMap& node_id_map,
                                      tint::CloneContext* clone_context,
                                      NodeIdMap* new_node_id_map) const {
    auto* expression_node =
        tint::As<ast::Expression>(node_id_map.GetNode(message_.expression_id()));

    auto* replacement_expression_node = clone_context->dst->create<ast::UnaryOpExpression>(
        static_cast<ast::UnaryOp>(message_.unary_op_wrapper()),
        clone_context->Clone(expression_node));

    clone_context->Replace(expression_node, replacement_expression_node);

    new_node_id_map->Add(replacement_expression_node, message_.fresh_id());
}

protobufs::Mutation MutationWrapUnaryOperator::ToMessage() const {
    protobufs::Mutation mutation;
    *mutation.mutable_wrap_unary_operator() = message_;
    return mutation;
}

std::vector<ast::UnaryOp> MutationWrapUnaryOperator::GetValidUnaryWrapper(
    const sem::ValueExpression& expr) {
    const auto* expr_type = expr.Type();
    if (expr_type->is_bool_scalar_or_vector()) {
        return {ast::UnaryOp::kNot};
    }

    if (expr_type->is_signed_integer_scalar_or_vector() ||
        expr_type->is_abstract_integer_scalar_or_vector()) {
        return {ast::UnaryOp::kNegation, ast::UnaryOp::kComplement};
    }

    if (expr_type->is_unsigned_integer_scalar_or_vector()) {
        return {ast::UnaryOp::kComplement};
    }

    if (expr_type->is_float_scalar_or_vector() || expr_type->is_abstract_float_scalar_or_vector()) {
        return {ast::UnaryOp::kNegation};
    }

    return {};
}

}  // namespace tint::fuzzers::ast_fuzzer
