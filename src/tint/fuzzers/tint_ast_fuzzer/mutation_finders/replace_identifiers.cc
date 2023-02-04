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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation_finders/replace_identifiers.h"

#include <memory>

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/replace_identifier.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/util.h"

#include "src/tint/sem/statement.h"
#include "src/tint/sem/value_expression.h"
#include "src/tint/sem/variable.h"

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
