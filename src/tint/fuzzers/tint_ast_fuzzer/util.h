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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_UTIL_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_UTIL_H_

#include <vector>

#include "src/tint/ast/module.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/program.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/variable.h"
#include "src/tint/utils/castable.h"

namespace tint::fuzzers::ast_fuzzer::util {
/// @file

/// @brief Returns all in-scope variables (including formal function parameters)
/// related to statement `curr_stmt`.
///
/// These variables are additionally filtered by applying a predicate `pred`.
///
/// @tparam Pred - a predicate that accepts a `const sem::Variable*` and returns
///     `bool`.
/// @param program - the program to look for variables in.
/// @param curr_stmt - the current statement. Everything below it is not in
///     scope.
/// @param pred - a predicate (e.g. a function pointer, functor, lambda etc) of
///     type `Pred`.
/// @return a vector of all variables that can be accessed from `curr_stmt`.
template <typename Pred>
std::vector<const sem::Variable*> GetAllVarsInScope(const tint::Program& program,
                                                    const sem::Statement* curr_stmt,
                                                    Pred&& pred) {
    std::vector<const sem::Variable*> result;

    // Walk up the hierarchy of blocks in which `curr_stmt` is contained.
    for (const auto* block = curr_stmt->Block(); block;
         block = tint::As<sem::BlockStatement>(block->Parent())) {
        for (const auto* stmt : block->Declaration()->statements) {
            if (stmt == curr_stmt->Declaration()) {
                // `curr_stmt` was found. This is only possible if `block is the
                // enclosing block of `curr_stmt` since the AST nodes are not shared.
                // Because of all this, skip the iteration of the inner loop since
                // the rest of the instructions in the `block` are not visible from the
                // `curr_stmt`.
                break;
            }

            if (const auto* var_node = tint::As<ast::VariableDeclStatement>(stmt)) {
                const auto* sem_var = program.Sem().Get(var_node->variable);
                if (pred(sem_var)) {
                    result.push_back(sem_var);
                }
            }
        }
    }

    // Process function parameters.
    for (const auto* param : curr_stmt->Function()->Parameters()) {
        if (pred(param)) {
            result.push_back(param);
        }
    }

    // Global variables do not belong to any ast::BlockStatement.
    for (const auto* global_decl : program.AST().GlobalDeclarations()) {
        if (global_decl == curr_stmt->Function()->Declaration()) {
            // The same situation as in the previous loop. The current function has
            // been reached. If there are any variables declared below, they won't be
            // visible in this function. Thus, exit the loop.
            break;
        }

        if (const auto* global_var = tint::As<ast::Variable>(global_decl)) {
            const auto* sem_node = program.Sem().Get(global_var);
            if (pred(sem_node)) {
                result.push_back(sem_node);
            }
        }
    }

    return result;
}

}  // namespace tint::fuzzers::ast_fuzzer::util

#endif  // SRC_TINT_FUZZERS_TINT_AST_FUZZER_UTIL_H_
