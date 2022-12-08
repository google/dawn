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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/change_unary_operator.h"

#include <utility>

#include "src/tint/fuzzers/tint_ast_fuzzer/util.h"
#include "src/tint/program_builder.h"
#include "src/tint/type/reference.h"

namespace tint::fuzzers::ast_fuzzer {

MutationChangeUnaryOperator::MutationChangeUnaryOperator(
    protobufs::MutationChangeUnaryOperator message)
    : message_(std::move(message)) {}

MutationChangeUnaryOperator::MutationChangeUnaryOperator(uint32_t unary_expr_id,
                                                         ast::UnaryOp new_operator) {
    message_.set_unary_expr_id(unary_expr_id);
    message_.set_new_operator(static_cast<uint32_t>(new_operator));
}

bool MutationChangeUnaryOperator::IsApplicable(const tint::Program& program,
                                               const NodeIdMap& node_id_map) const {
    const auto* unary_expr_node =
        tint::As<ast::UnaryOpExpression>(node_id_map.GetNode(message_.unary_expr_id()));

    if (!unary_expr_node) {
        // Either the id does not exist, or does not correspond to a unary
        // expression.
        return false;
    }

    auto new_unary_operator = static_cast<ast::UnaryOp>(message_.new_operator());

    // Get the type of the unary expression.
    const auto* type = program.Sem().Get(unary_expr_node)->Type();
    const auto* basic_type =
        type->Is<type::Reference>() ? type->As<type::Reference>()->StoreType() : type;

    // Only signed integer or vector of signed integer has more than 1
    // unary operators to change between.
    if (!basic_type->is_signed_integer_scalar_or_vector()) {
        return false;
    }

    // The new unary operator must not be the same as the original one.
    if (new_unary_operator != ToggleOperator(unary_expr_node->op)) {
        return false;
    }

    return true;
}

void MutationChangeUnaryOperator::Apply(const NodeIdMap& node_id_map,
                                        tint::CloneContext* clone_context,
                                        NodeIdMap* new_node_id_map) const {
    const auto* unary_expr_node =
        tint::As<ast::UnaryOpExpression>(node_id_map.GetNode(message_.unary_expr_id()));

    const ast::UnaryOpExpression* cloned_replacement;
    switch (static_cast<ast::UnaryOp>(message_.new_operator())) {
        case ast::UnaryOp::kComplement:
            cloned_replacement =
                clone_context->dst->Complement(clone_context->Clone(unary_expr_node->expr));
            break;
        case ast::UnaryOp::kNegation:
            cloned_replacement =
                clone_context->dst->Negation(clone_context->Clone(unary_expr_node->expr));
            break;
        default:
            cloned_replacement = nullptr;
            assert(false && "Unreachable");
    }
    // Set things up so that the original unary expression will be replaced with
    // its clone, and update the id mapping.
    clone_context->Replace(unary_expr_node, cloned_replacement);
    new_node_id_map->Add(cloned_replacement, message_.unary_expr_id());
}

protobufs::Mutation MutationChangeUnaryOperator::ToMessage() const {
    protobufs::Mutation mutation;
    *mutation.mutable_change_unary_operator() = message_;
    return mutation;
}

ast::UnaryOp MutationChangeUnaryOperator::ToggleOperator(const ast::UnaryOp& original_op) {
    if (original_op == ast::UnaryOp::kComplement) {
        return ast::UnaryOp::kNegation;
    }
    assert(original_op == ast::UnaryOp::kNegation && "Unexpected operator.");
    return ast::UnaryOp::kComplement;
}

}  // namespace tint::fuzzers::ast_fuzzer
