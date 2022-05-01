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

#include "src/tint/transform/promote_initializers_to_const_var.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/type_constructor.h"
#include "src/tint/transform/utils/hoist_to_decl_before.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::PromoteInitializersToConstVar);

namespace tint::transform {

PromoteInitializersToConstVar::PromoteInitializersToConstVar() = default;

PromoteInitializersToConstVar::~PromoteInitializersToConstVar() = default;

void PromoteInitializersToConstVar::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    HoistToDeclBefore hoist_to_decl_before(ctx);

    // Hoists array and structure initializers to a constant variable, declared
    // just before the statement of usage.
    auto type_ctor_to_let = [&](const ast::CallExpression* expr) {
        auto* ctor = ctx.src->Sem().Get(expr);
        if (!ctor->Target()->Is<sem::TypeConstructor>()) {
            return true;
        }
        auto* sem_stmt = ctor->Stmt();
        if (!sem_stmt) {
            // Expression is outside of a statement. This usually means the
            // expression is part of a global (module-scope) constant declaration.
            // These must be constexpr, and so cannot contain the type of
            // expressions that must be sanitized.
            return true;
        }

        auto* stmt = sem_stmt->Declaration();

        if (auto* src_var_decl = stmt->As<ast::VariableDeclStatement>()) {
            if (src_var_decl->variable->constructor == expr) {
                // This statement is just a variable declaration with the
                // initializer as the constructor value. This is what we're
                // attempting to transform to, and so ignore.
                return true;
            }
        }

        auto* src_ty = ctor->Type();
        if (!src_ty->IsAnyOf<sem::Array, sem::Struct>()) {
            // We only care about array and struct initializers
            return true;
        }

        return hoist_to_decl_before.Add(ctor, expr, true);
    };

    for (auto* node : ctx.src->ASTNodes().Objects()) {
        if (auto* call_expr = node->As<ast::CallExpression>()) {
            if (!type_ctor_to_let(call_expr)) {
                return;
            }
        }
    }

    hoist_to_decl_before.Apply();
    ctx.Clone();
}

}  // namespace tint::transform
