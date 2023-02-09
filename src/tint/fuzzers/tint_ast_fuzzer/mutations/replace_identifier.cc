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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/replace_identifier.h"

#include <utility>

#include "src/tint/fuzzers/tint_ast_fuzzer/util.h"
#include "src/tint/program_builder.h"

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
                                      tint::CloneContext* clone_context,
                                      NodeIdMap* new_node_id_map) const {
    const auto* use_node = node_id_map.GetNode(message_.use_id());
    const auto* replacement_var =
        tint::As<ast::Variable>(node_id_map.GetNode(message_.replacement_id()));

    auto* cloned_replacement =
        clone_context->dst->Expr(clone_context->Clone(use_node->source),
                                 clone_context->Clone(replacement_var->name->symbol));
    clone_context->Replace(use_node, cloned_replacement);
    new_node_id_map->Add(cloned_replacement, message_.use_id());
}

protobufs::Mutation MutationReplaceIdentifier::ToMessage() const {
    protobufs::Mutation mutation;
    *mutation.mutable_replace_identifier() = message_;
    return mutation;
}

}  // namespace tint::fuzzers::ast_fuzzer
