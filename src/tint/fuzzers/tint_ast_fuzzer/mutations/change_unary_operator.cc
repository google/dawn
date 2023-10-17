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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/change_unary_operator.h"

#include <utility>

#include "src/tint/fuzzers/tint_ast_fuzzer/util.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/wgsl/program/program_builder.h"

namespace tint::fuzzers::ast_fuzzer {

MutationChangeUnaryOperator::MutationChangeUnaryOperator(
    protobufs::MutationChangeUnaryOperator message)
    : message_(std::move(message)) {}

MutationChangeUnaryOperator::MutationChangeUnaryOperator(uint32_t unary_expr_id,
                                                         core::UnaryOp new_operator) {
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

    auto new_unary_operator = static_cast<core::UnaryOp>(message_.new_operator());

    // Get the type of the unary expression.
    const auto* type = program.Sem().Get(unary_expr_node)->Type();
    const auto* basic_type =
        type->Is<core::type::Reference>() ? type->As<core::type::Reference>()->StoreType() : type;

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
                                        tint::program::CloneContext& clone_context,
                                        NodeIdMap* new_node_id_map) const {
    const auto* unary_expr_node =
        tint::As<ast::UnaryOpExpression>(node_id_map.GetNode(message_.unary_expr_id()));

    const ast::UnaryOpExpression* cloned_replacement;
    switch (static_cast<core::UnaryOp>(message_.new_operator())) {
        case core::UnaryOp::kComplement:
            cloned_replacement =
                clone_context.dst->Complement(clone_context.Clone(unary_expr_node->expr));
            break;
        case core::UnaryOp::kNegation:
            cloned_replacement =
                clone_context.dst->Negation(clone_context.Clone(unary_expr_node->expr));
            break;
        default:
            cloned_replacement = nullptr;
            assert(false && "Unreachable");
    }
    // Set things up so that the original unary expression will be replaced with
    // its clone, and update the id mapping.
    clone_context.Replace(unary_expr_node, cloned_replacement);
    new_node_id_map->Add(cloned_replacement, message_.unary_expr_id());
}

protobufs::Mutation MutationChangeUnaryOperator::ToMessage() const {
    protobufs::Mutation mutation;
    *mutation.mutable_change_unary_operator() = message_;
    return mutation;
}

core::UnaryOp MutationChangeUnaryOperator::ToggleOperator(const core::UnaryOp& original_op) {
    if (original_op == core::UnaryOp::kComplement) {
        return core::UnaryOp::kNegation;
    }
    assert(original_op == core::UnaryOp::kNegation && "Unexpected operator.");
    return core::UnaryOp::kComplement;
}

}  // namespace tint::fuzzers::ast_fuzzer
