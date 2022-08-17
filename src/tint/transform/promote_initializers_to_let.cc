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

#include "src/tint/transform/promote_initializers_to_let.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/type_constructor.h"
#include "src/tint/transform/utils/hoist_to_decl_before.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::PromoteInitializersToLet);

namespace tint::transform {

PromoteInitializersToLet::PromoteInitializersToLet() = default;

PromoteInitializersToLet::~PromoteInitializersToLet() = default;

void PromoteInitializersToLet::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    HoistToDeclBefore hoist_to_decl_before(ctx);

    // Hoists array and structure initializers to a constant variable, declared
    // just before the statement of usage.
    auto promote = [&](const sem::Expression* expr) {
        auto* sem_stmt = expr->Stmt();
        if (!sem_stmt) {
            // Expression is outside of a statement. This usually means the
            // expression is part of a global (module-scope) constant declaration.
            // These must be constexpr, and so cannot contain the type of
            // expressions that must be sanitized.
            return true;
        }

        auto* stmt = sem_stmt->Declaration();

        if (auto* src_var_decl = stmt->As<ast::VariableDeclStatement>()) {
            if (src_var_decl->variable->constructor == expr->Declaration()) {
                // This statement is just a variable declaration with the
                // initializer as the constructor value. This is what we're
                // attempting to transform to, and so ignore.
                return true;
            }
        }

        auto* src_ty = expr->Type();
        if (!src_ty->IsAnyOf<sem::Array, sem::Struct>()) {
            // We only care about array and struct initializers
            return true;
        }

        return hoist_to_decl_before.Add(expr, expr->Declaration(), true);
    };

    for (auto* node : ctx.src->ASTNodes().Objects()) {
        bool ok = Switch(
            node,  //
            [&](const ast::CallExpression* expr) {
                if (auto* sem = ctx.src->Sem().Get(expr)) {
                    auto* ctor = sem->UnwrapMaterialize()->As<sem::Call>();
                    if (ctor->Target()->Is<sem::TypeConstructor>()) {
                        return promote(sem);
                    }
                }
                return true;
            },
            [&](const ast::IdentifierExpression* expr) {
                if (auto* sem = ctx.src->Sem().Get(expr)) {
                    if (auto* user = sem->UnwrapMaterialize()->As<sem::VariableUser>()) {
                        // Identifier resolves to a variable
                        if (auto* stmt = user->Stmt()) {
                            if (auto* decl = stmt->Declaration()->As<ast::VariableDeclStatement>();
                                decl && decl->variable->Is<ast::Const>()) {
                                // The identifier is used on the RHS of a 'const' declaration.
                                // Ignore.
                                return true;
                            }
                        }
                        if (user->Variable()->Declaration()->Is<ast::Const>()) {
                            // The identifier resolves to a 'const' variable, but isn't used to
                            // initialize another 'const'. This needs promoting.
                            return promote(user);
                        }
                    }
                }
                return true;
            },
            [&](Default) { return true; });

        if (!ok) {
            return;
        }
    }

    hoist_to_decl_before.Apply();
    ctx.Clone();
}

}  // namespace tint::transform
