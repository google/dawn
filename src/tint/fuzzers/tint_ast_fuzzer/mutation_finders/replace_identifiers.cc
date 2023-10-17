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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation_finders/replace_identifiers.h"

#include <memory>

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/replace_identifier.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/util.h"

#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/lang/wgsl/sem/variable.h"

namespace tint::fuzzers::ast_fuzzer {

MutationList MutationFinderReplaceIdentifiers::FindMutations(
    const tint::Program& program,
    NodeIdMap* node_id_map,
    ProbabilityContext* probability_context) const {
    MutationList result;

    // Go through each variable in the AST and for each user of that variable, try
    // to replace it with some other variable usage.

    for (const auto* node : program.SemNodes().Objects()) {
        const auto* sem_variable = tint::As<sem::Variable>(node);
        if (!sem_variable) {
            continue;
        }

        // Iterate over all users of `sem_variable`.
        for (const auto* user : sem_variable->Users()) {
            // Get all variables that can be used to replace the `user` of
            // `sem_variable`.
            auto candidate_variables =
                util::GetAllVarsInScope(program, user->Stmt(), [user](const sem::Variable* var) {
                    return var != user->Variable() && var->Type() == user->Type();
                });

            if (candidate_variables.empty()) {
                // No suitable replacements have been found.
                continue;
            }

            const auto* replacement =
                candidate_variables[probability_context->GetRandomIndex(candidate_variables)];

            result.push_back(std::make_unique<MutationReplaceIdentifier>(
                node_id_map->GetId(user->Declaration()),
                node_id_map->GetId(replacement->Declaration())));
        }
    }

    return result;
}

uint32_t MutationFinderReplaceIdentifiers::GetChanceOfApplyingMutation(
    ProbabilityContext* probability_context) const {
    return probability_context->GetChanceOfReplacingIdentifiers();
}

}  // namespace tint::fuzzers::ast_fuzzer
