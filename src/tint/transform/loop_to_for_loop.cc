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

#include "src/tint/transform/loop_to_for_loop.h"

#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/for_loop_statement.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/variable.h"
#include "src/tint/utils/scoped_assignment.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::LoopToForLoop);

namespace tint::transform {
namespace {

bool IsBlockWithSingleBreak(const ast::BlockStatement* block) {
    if (block->statements.size() != 1) {
        return false;
    }
    return block->statements[0]->Is<ast::BreakStatement>();
}

bool IsVarUsedByStmt(const sem::Info& sem, const ast::Variable* var, const ast::Statement* stmt) {
    auto* var_sem = sem.Get(var);
    for (auto* user : var_sem->Users()) {
        if (auto* s = user->Stmt()) {
            if (s->Declaration() == stmt) {
                return true;
            }
        }
    }
    return false;
}

}  // namespace

LoopToForLoop::LoopToForLoop() = default;

LoopToForLoop::~LoopToForLoop() = default;

bool LoopToForLoop::ShouldRun(const Program* program, const DataMap&) const {
    for (auto* node : program->ASTNodes().Objects()) {
        if (node->Is<ast::LoopStatement>()) {
            return true;
        }
    }
    return false;
}

void LoopToForLoop::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    ctx.ReplaceAll([&](const ast::LoopStatement* loop) -> const ast::Statement* {
        // For loop condition is taken from the first statement in the loop.
        // This requires an if-statement with either:
        //  * A true block with no else statements, and the true block contains a
        //    single 'break' statement.
        //  * An empty true block with a single, no-condition else statement
        //    containing a single 'break' statement.
        // Examples:
        //   loop {  if (condition) { break; } ... }
        //   loop {  if (condition) {} else { break; } ... }
        auto& stmts = loop->body->statements;
        if (stmts.empty()) {
            return nullptr;
        }
        auto* if_stmt = stmts[0]->As<ast::IfStatement>();
        if (!if_stmt) {
            return nullptr;
        }
        auto* else_stmt = tint::As<ast::BlockStatement>(if_stmt->else_statement);

        bool negate_condition = false;
        if (IsBlockWithSingleBreak(if_stmt->body) && if_stmt->else_statement == nullptr) {
            negate_condition = true;
        } else if (if_stmt->body->Empty() && else_stmt && IsBlockWithSingleBreak(else_stmt)) {
            negate_condition = false;
        } else {
            return nullptr;
        }

        // The continuing block must be empty or contain a single, assignment or
        // function call statement.
        const ast::Statement* continuing = nullptr;
        if (auto* loop_cont = loop->continuing) {
            if (loop_cont->statements.size() != 1) {
                return nullptr;
            }

            continuing = loop_cont->statements[0];
            if (!continuing->IsAnyOf<ast::AssignmentStatement, ast::CallStatement>()) {
                return nullptr;
            }

            // And the continuing statement must not use any of the variables declared
            // in the loop body.
            for (auto* stmt : loop->body->statements) {
                if (auto* var_decl = stmt->As<ast::VariableDeclStatement>()) {
                    if (IsVarUsedByStmt(ctx.src->Sem(), var_decl->variable, continuing)) {
                        return nullptr;
                    }
                }
            }

            continuing = ctx.Clone(continuing);
        }

        auto* condition = ctx.Clone(if_stmt->condition);
        if (negate_condition) {
            condition = ctx.dst->create<ast::UnaryOpExpression>(ast::UnaryOp::kNot, condition);
        }

        ast::Statement* initializer = nullptr;

        ctx.Remove(loop->body->statements, if_stmt);
        auto* body = ctx.Clone(loop->body);
        return ctx.dst->create<ast::ForLoopStatement>(initializer, condition, continuing, body);
    });

    ctx.Clone();
}

}  // namespace tint::transform
