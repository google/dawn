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

#include "src/tint/transform/fold_trivial_single_use_lets.h"

#include "src/tint/program_builder.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/variable.h"
#include "src/tint/utils/scoped_assignment.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::FoldTrivialSingleUseLets);

namespace tint::transform {
namespace {

const ast::VariableDeclStatement* AsTrivialLetDecl(const ast::Statement* stmt) {
    auto* var_decl = stmt->As<ast::VariableDeclStatement>();
    if (!var_decl) {
        return nullptr;
    }
    auto* var = var_decl->variable;
    if (!var->is_const) {
        return nullptr;
    }
    auto* ctor = var->constructor;
    if (!IsAnyOf<ast::IdentifierExpression, ast::LiteralExpression>(ctor)) {
        return nullptr;
    }
    return var_decl;
}

}  // namespace

FoldTrivialSingleUseLets::FoldTrivialSingleUseLets() = default;

FoldTrivialSingleUseLets::~FoldTrivialSingleUseLets() = default;

void FoldTrivialSingleUseLets::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    for (auto* node : ctx.src->ASTNodes().Objects()) {
        if (auto* block = node->As<ast::BlockStatement>()) {
            auto& stmts = block->statements;
            for (size_t stmt_idx = 0; stmt_idx < stmts.size(); stmt_idx++) {
                auto* stmt = stmts[stmt_idx];
                if (auto* let_decl = AsTrivialLetDecl(stmt)) {
                    auto* let = let_decl->variable;
                    auto* sem_let = ctx.src->Sem().Get(let);
                    auto& users = sem_let->Users();
                    if (users.size() != 1) {
                        continue;  // Does not have a single user.
                    }

                    auto* user = users[0];
                    auto* user_stmt = user->Stmt()->Declaration();

                    for (size_t i = stmt_idx; i < stmts.size(); i++) {
                        if (user_stmt == stmts[i]) {
                            auto* user_expr = user->Declaration();
                            ctx.Remove(stmts, let_decl);
                            ctx.Replace(user_expr, ctx.Clone(let->constructor));
                        }
                        if (!AsTrivialLetDecl(stmts[i])) {
                            // Stop if we hit a statement that isn't the single use of the
                            // let, and isn't a let itself.
                            break;
                        }
                    }
                }
            }
        }
    }

    ctx.Clone();
}

}  // namespace tint::transform
