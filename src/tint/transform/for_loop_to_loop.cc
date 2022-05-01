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

#include "src/tint/transform/for_loop_to_loop.h"

#include "src/tint/ast/break_statement.h"
#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::ForLoopToLoop);

namespace tint::transform {
ForLoopToLoop::ForLoopToLoop() = default;

ForLoopToLoop::~ForLoopToLoop() = default;

bool ForLoopToLoop::ShouldRun(const Program* program, const DataMap&) const {
    for (auto* node : program->ASTNodes().Objects()) {
        if (node->Is<ast::ForLoopStatement>()) {
            return true;
        }
    }
    return false;
}

void ForLoopToLoop::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    ctx.ReplaceAll([&](const ast::ForLoopStatement* for_loop) -> const ast::Statement* {
        ast::StatementList stmts;
        if (auto* cond = for_loop->condition) {
            // !condition
            auto* not_cond =
                ctx.dst->create<ast::UnaryOpExpression>(ast::UnaryOp::kNot, ctx.Clone(cond));

            // { break; }
            auto* break_body = ctx.dst->Block(ctx.dst->create<ast::BreakStatement>());

            // if (!condition) { break; }
            stmts.emplace_back(ctx.dst->If(not_cond, break_body));
        }
        for (auto* stmt : for_loop->body->statements) {
            stmts.emplace_back(ctx.Clone(stmt));
        }

        const ast::BlockStatement* continuing = nullptr;
        if (auto* cont = for_loop->continuing) {
            continuing = ctx.dst->Block(ctx.Clone(cont));
        }

        auto* body = ctx.dst->Block(stmts);
        auto* loop = ctx.dst->create<ast::LoopStatement>(body, continuing);

        if (auto* init = for_loop->initializer) {
            return ctx.dst->Block(ctx.Clone(init), loop);
        }

        return loop;
    });

    ctx.Clone();
}

}  // namespace tint::transform
