// Copyright 2022 The Tint Authors.
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

#include "src/tint/fuzzers/tint_ast_fuzzer/jump_tracker.h"

#include <cassert>
#include <unordered_set>

#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/for_loop_statement.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/ast/while_statement.h"
#include "src/tint/sem/statement.h"

namespace tint::fuzzers::ast_fuzzer {

JumpTracker::JumpTracker(const Program& program) {
    // Consider every AST node, looking for break and return statements.
    for (auto* node : program.ASTNodes().Objects()) {
        auto* stmt = node->As<ast::Statement>();
        if (stmt == nullptr) {
            continue;
        }
        if (stmt->As<ast::BreakStatement>()) {
            // This break statement either exits a loop or a switch statement.
            // Walk up the AST until either a loop or switch statement is found. In the former case,
            // it is the innermost loop containing the break statement, and thus all the nodes
            // encountered along the way are nodes that contain a break from the innermost loop.

            // This records the statements encountered when walking up the AST from the break
            // statement to the innermost enclosing loop or switch statement.
            std::unordered_set<const ast::Statement*> candidate_statements;
            for (const ast::Statement* current = stmt;;
                 current =
                     As<sem::Statement>(program.Sem().Get(current))->Parent()->Declaration()) {
                if (current->Is<ast::ForLoopStatement>() || current->Is<ast::LoopStatement>() ||
                    current->Is<ast::WhileStatement>()) {
                    // A loop has been encountered, thus all that statements recorded until this
                    // point contain a break from their innermost loop.
                    for (auto* candidate : candidate_statements) {
                        contains_break_for_innermost_loop_.insert(candidate);
                    }
                    break;
                }
                if (current->Is<ast::SwitchStatement>()) {
                    // A switch statement has been encountered, so the break does not correspond to
                    // a loop break.
                    break;
                }
                candidate_statements.insert(current);
            }
        } else if (stmt->As<ast::ReturnStatement>()) {
            // Walk up the AST from the return statement, recording that every node encountered
            // along the way contains a return.
            const ast::Statement* current = stmt;
            while (true) {
                contains_return_.insert(current);
                auto* parent = program.Sem().Get(current)->Parent();
                if (parent == nullptr) {
                    break;
                }
                current = parent->Declaration();
            }
        }
    }
}
}  // namespace tint::fuzzers::ast_fuzzer
