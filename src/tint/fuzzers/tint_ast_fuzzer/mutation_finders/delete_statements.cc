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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation_finders/delete_statements.h"

#include <memory>

#include "src/tint/fuzzers/tint_ast_fuzzer/jump_tracker.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/delete_statement.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/util.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/lang/wgsl/sem/variable.h"

namespace tint::fuzzers::ast_fuzzer {

MutationList MutationFinderDeleteStatements::FindMutations(const tint::Program& program,
                                                           NodeIdMap* node_id_map,
                                                           ProbabilityContext* /*unused*/) const {
    MutationList result;

    JumpTracker jump_tracker(program);

    // Consider every statement node in the AST.
    for (auto* node : program.ASTNodes().Objects()) {
        auto* statement_node = tint::As<ast::Statement>(node);

        if (!statement_node) {
            continue;
        }

        const auto* statement_sem_node =
            tint::As<sem::Statement>(program.Sem().Get(statement_node));

        // Semantic information for the node is required in order to delete it.
        if (!statement_sem_node) {
            continue;
        }

        // Check that this kind of statement can be deleted.
        if (!MutationDeleteStatement::CanBeDeleted(*statement_node, program, jump_tracker)) {
            continue;
        }

        result.push_back(
            std::make_unique<MutationDeleteStatement>(node_id_map->GetId(statement_node)));
    }

    return result;
}

uint32_t MutationFinderDeleteStatements::GetChanceOfApplyingMutation(
    ProbabilityContext* probability_context) const {
    return probability_context->GetChanceOfDeletingStatements();
}

}  // namespace tint::fuzzers::ast_fuzzer
