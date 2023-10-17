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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/delete_statement.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "src/tint/fuzzers/tint_ast_fuzzer/jump_tracker.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/util.h"
#include "src/tint/lang/wgsl/ast/block_statement.h"
#include "src/tint/lang/wgsl/ast/for_loop_statement.h"
#include "src/tint/lang/wgsl/ast/if_statement.h"
#include "src/tint/lang/wgsl/ast/loop_statement.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/sem/for_loop_statement.h"
#include "src/tint/lang/wgsl/sem/if_statement.h"
#include "src/tint/lang/wgsl/sem/loop_statement.h"
#include "src/tint/lang/wgsl/sem/statement.h"

namespace tint::fuzzers::ast_fuzzer {

MutationDeleteStatement::MutationDeleteStatement(protobufs::MutationDeleteStatement message)
    : message_(std::move(message)) {}

MutationDeleteStatement::MutationDeleteStatement(uint32_t statement_id) {
    message_.set_statement_id(statement_id);
}

bool MutationDeleteStatement::IsApplicable(const tint::Program& program,
                                           const NodeIdMap& node_id_map) const {
    auto* statement_node = tint::As<ast::Statement>(node_id_map.GetNode(message_.statement_id()));

    if (!statement_node) {
        // The statement id is invalid or does not refer to a statement.
        return false;
    }

    const auto* statement_sem_node = tint::As<sem::Statement>(program.Sem().Get(statement_node));

    if (!statement_sem_node) {
        // Semantic information for the statement is not available. This
        // information is required in order to perform the deletion.
        return false;
    }

    // Check whether it is OK to delete this statement.
    if (!CanBeDeleted(*statement_node, program, JumpTracker(program))) {
        return false;
    }

    return true;
}

void MutationDeleteStatement::Apply(const NodeIdMap& node_id_map,
                                    tint::program::CloneContext& clone_context,
                                    NodeIdMap* /* unused */) const {
    const auto* statement_node =
        tint::As<ast::Statement>(node_id_map.GetNode(message_.statement_id()));
    const auto* statement_sem_node =
        tint::As<sem::Statement>(clone_context.src->Sem().Get(statement_node));
    const auto* sem_parent = statement_sem_node->Parent();

    if (tint::Is<sem::IfStatement>(sem_parent) &&
        tint::As<ast::IfStatement>(sem_parent->Declaration())->else_statement == statement_node) {
        // Remove the "else" part of an if statement.
        clone_context.Replace(statement_node, static_cast<const ast::Statement*>(nullptr));
    } else if (tint::Is<sem::ForLoopStatement>(sem_parent) &&
               tint::As<ast::ForLoopStatement>(sem_parent->Declaration())->initializer ==
                   statement_node) {
        // Remove the initializer of a for loop.
        clone_context.Replace(statement_node, static_cast<const ast::Statement*>(nullptr));
    } else if (tint::Is<sem::ForLoopStatement>(sem_parent) &&
               tint::As<ast::ForLoopStatement>(sem_parent->Declaration())->continuing ==
                   statement_node) {
        // Remove the "continuing" statement of a for loop.
        clone_context.Replace(statement_node, static_cast<const ast::Statement*>(nullptr));
    } else if (tint::Is<sem::LoopContinuingBlockStatement>(statement_sem_node)) {
        // Remove the "continuing" block of a loop.
        clone_context.Replace(statement_node, static_cast<const ast::Statement*>(nullptr));
    } else if (tint::Is<ast::CaseStatement>(statement_node)) {
        // Remove a case statement from its enclosing switch statement.
        const auto& case_statement_list =
            &sem_parent->Declaration()->As<ast::SwitchStatement>()->body;
        assert(std::find(case_statement_list->begin(), case_statement_list->end(),
                         statement_node) != case_statement_list->end() &&
               "Statement not found.");
        clone_context.Remove(*case_statement_list, statement_node);
    } else if (tint::Is<ast::BlockStatement>(statement_node)) {
        // Remove a block statement from the block that encloses it. A special case is required for
        // this, since a sem::Block has itself as its associated sem::Block, so it is necessary to
        // look at the parent to get the enclosing block.
        const auto& statement_list =
            sem_parent->Declaration()->As<ast::BlockStatement>()->statements;
        assert(std::find(statement_list.begin(), statement_list.end(), statement_node) !=
                   statement_list.end() &&
               "Statement not found.");
        clone_context.Remove(statement_list, statement_node);
    } else {
        // Remove a non-block statement from the block that encloses it.
        const auto& statement_list =
            statement_sem_node->Block()->Declaration()->As<ast::BlockStatement>()->statements;
        assert(std::find(statement_list.begin(), statement_list.end(), statement_node) !=
                   statement_list.end() &&
               "Statement not found.");
        clone_context.Remove(statement_list, statement_node);
    }
}

protobufs::Mutation MutationDeleteStatement::ToMessage() const {
    protobufs::Mutation mutation;
    *mutation.mutable_delete_statement() = message_;
    return mutation;
}

bool MutationDeleteStatement::CanBeDeleted(const ast::Statement& statement_node,
                                           const Program& program,
                                           const JumpTracker& jump_tracker) {
    if (statement_node.Is<ast::VariableDeclStatement>()) {
        // This is conservative. It would be possible to delete variable declarations if they are
        // not used. Further analysis could allow that.
        return false;
    }

    if (jump_tracker.ContainsReturn(statement_node)) {
        // This is conservative. It would be possible to delete a return statement as long as there
        // is still a return on every control flow path.
        return false;
    }

    if (jump_tracker.ContainsBreakForInnermostLoop(statement_node)) {
        // This is conservative. Disallowing the removal of breaks ensures that loops cannot become
        // statically infinite. However, a loop might in practice have multiple breaks, some of
        // which can be removed.
        return false;
    }

    if (auto* case_statement = statement_node.As<ast::CaseStatement>()) {
        // It is not OK to delete the case statement which contains the default selector.
        if (case_statement->ContainsDefault()) {
            return false;
        }
    }

    auto* parent_sem = program.Sem().Get(&statement_node)->Parent();
    if (parent_sem == nullptr) {
        // Semantic information for the parent node is required.
        return false;
    }

    auto* parent_stmt = parent_sem->Declaration();

    // It does not make sense to delete the entire body of a loop or if statement.
    if (auto* for_loop = parent_stmt->As<ast::ForLoopStatement>()) {
        if (for_loop->body == &statement_node) {
            return false;
        }
    }
    if (auto* loop = parent_stmt->As<ast::LoopStatement>()) {
        if (loop->body == &statement_node) {
            return false;
        }
    }
    if (auto* while_loop = parent_stmt->As<ast::WhileStatement>()) {
        if (while_loop->body == &statement_node) {
            return false;
        }
    }
    if (auto* if_statement = parent_stmt->As<ast::IfStatement>()) {
        if (if_statement->body == &statement_node) {
            return false;
        }
    }

    return true;
}

}  // namespace tint::fuzzers::ast_fuzzer
