// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/fuzzers/tint_ast_fuzzer/jump_tracker.h"

#include <cassert>
#include <unordered_set>

#include "src/tint/lang/wgsl/ast/break_statement.h"
#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/ast/for_loop_statement.h"
#include "src/tint/lang/wgsl/ast/loop_statement.h"
#include "src/tint/lang/wgsl/ast/return_statement.h"
#include "src/tint/lang/wgsl/ast/switch_statement.h"
#include "src/tint/lang/wgsl/ast/while_statement.h"
#include "src/tint/lang/wgsl/sem/statement.h"

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
