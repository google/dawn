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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/replace_identifier.h"

#include <utility>

#include "src/tint/fuzzers/tint_ast_fuzzer/util.h"
#include "src/tint/lang/wgsl/program/program_builder.h"

namespace tint::fuzzers::ast_fuzzer {

MutationReplaceIdentifier::MutationReplaceIdentifier(protobufs::MutationReplaceIdentifier message)
    : message_(std::move(message)) {}

MutationReplaceIdentifier::MutationReplaceIdentifier(uint32_t use_id, uint32_t replacement_id) {
    message_.set_use_id(use_id);
    message_.set_replacement_id(replacement_id);
}

bool MutationReplaceIdentifier::IsApplicable(const tint::Program& program,
                                             const NodeIdMap& node_id_map) const {
    const auto* use_ast_node =
        tint::As<ast::IdentifierExpression>(node_id_map.GetNode(message_.use_id()));
    if (!use_ast_node) {
        // Either the `use_id` is invalid or the node is not an
        // `IdentifierExpression`.
        return false;
    }

    const auto* use_sem_node = tint::As<sem::VariableUser>(program.Sem().Get(use_ast_node));
    if (!use_sem_node) {
        // Either the semantic information is not present for a `use_node` or that
        // node is not a variable user.
        return false;
    }

    const auto* replacement_ast_node =
        tint::As<ast::Variable>(node_id_map.GetNode(message_.replacement_id()));
    if (!replacement_ast_node) {
        // Either the `replacement_id` is invalid or is not an id of a variable.
        return false;
    }

    const auto* replacement_sem_node = program.Sem().Get(replacement_ast_node);
    if (!replacement_sem_node) {
        return false;
    }

    if (replacement_sem_node == use_sem_node->Variable()) {
        return false;
    }

    auto in_scope = util::GetAllVarsInScope(
        program, use_sem_node->Stmt(),
        [replacement_sem_node](const sem::Variable* var) { return var == replacement_sem_node; });
    if (in_scope.empty()) {
        // The replacement variable is not in scope.
        return false;
    }

    return use_sem_node->Type() == replacement_sem_node->Type();
}

void MutationReplaceIdentifier::Apply(const NodeIdMap& node_id_map,
                                      tint::program::CloneContext& clone_context,
                                      NodeIdMap* new_node_id_map) const {
    const auto* use_node = node_id_map.GetNode(message_.use_id());
    const auto* replacement_var =
        tint::As<ast::Variable>(node_id_map.GetNode(message_.replacement_id()));

    auto* cloned_replacement = clone_context.dst->Expr(
        clone_context.Clone(use_node->source), clone_context.Clone(replacement_var->name->symbol));
    clone_context.Replace(use_node, cloned_replacement);
    new_node_id_map->Add(cloned_replacement, message_.use_id());
}

protobufs::Mutation MutationReplaceIdentifier::ToMessage() const {
    protobufs::Mutation mutation;
    *mutation.mutable_replace_identifier() = message_;
    return mutation;
}

}  // namespace tint::fuzzers::ast_fuzzer
