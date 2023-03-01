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

#include <utility>

#include "src/tint/ast/traverse_expressions.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/value_constructor.h"
#include "src/tint/transform/utils/hoist_to_decl_before.h"
#include "src/tint/type/struct.h"
#include "src/tint/utils/hashset.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::PromoteInitializersToLet);

namespace tint::transform {

PromoteInitializersToLet::PromoteInitializersToLet() = default;

PromoteInitializersToLet::~PromoteInitializersToLet() = default;

Transform::ApplyResult PromoteInitializersToLet::Apply(const Program* src,
                                                       const DataMap&,
                                                       DataMap&) const {
    ProgramBuilder b;
    CloneContext ctx{&b, src, /* auto_clone_symbols */ true};

    // Returns true if the expression should be hoisted to a new let statement before the
    // expression's statement.
    auto should_hoist = [&](const sem::ValueExpression* expr) {
        if (!expr->Type()->IsAnyOf<type::Array, type::Struct>()) {
            // We only care about array and struct initializers
            return false;
        }

        // Check whether the expression is an array or structure constructor
        {
            // Follow const-chains
            auto* root_expr = expr;
            if (expr->Stage() == sem::EvaluationStage::kConstant) {
                if (expr->Type()->HoldsAbstract()) {
                    // Do not hoist expressions that are not materialized, as doing so would cause
                    // premature materialization.
                    return false;
                }
                while (auto* user = root_expr->UnwrapMaterialize()->As<sem::VariableUser>()) {
                    root_expr = user->Variable()->Initializer();
                }
            }

            auto* ctor = root_expr->UnwrapMaterialize()->As<sem::Call>();
            if (!ctor || !ctor->Target()->Is<sem::ValueConstructor>()) {
                // Root expression is not a value constructor. Not interested in this.
                return false;
            }
        }

        if (auto* src_var_decl = expr->Stmt()->Declaration()->As<ast::VariableDeclStatement>()) {
            if (src_var_decl->variable->initializer == expr->Declaration()) {
                // This statement is just a variable declaration with the initializer as the
                // initializer value. This is what we're attempting to transform to, and so
                // ignore.
                return false;
            }
        }

        return true;
    };

    // A list of expressions that should be hoisted.
    utils::Vector<const sem::ValueExpression*, 32> to_hoist;
    // A set of expressions that are constant, which _may_ need to be hoisted.
    utils::Hashset<const ast::Expression*, 32> const_chains;

    // Walk the AST nodes. This order guarantees that leaf-expressions are visited first.
    for (auto* node : src->ASTNodes().Objects()) {
        if (auto* sem = src->Sem().GetVal(node)) {
            auto* stmt = sem->Stmt();
            if (!stmt) {
                // Expression is outside of a statement. This usually means the expression is part
                // of a global (module-scope) constant declaration. These must be constexpr, and so
                // cannot contain the type of expressions that must be sanitized.
                continue;
            }

            if (sem->Stage() == sem::EvaluationStage::kConstant) {
                // Expression is constant. We only need to hoist expressions if they're the
                // outermost constant expression in a chain. Remove the immediate child nodes of the
                // expression from const_chains, and add this expression to the const_chains. As we
                // visit leaf-expressions first, this means the content of const_chains only
                // contains the outer-most constant expressions.
                auto* expr = sem->Declaration();
                bool ok = ast::TraverseExpressions(
                    expr, b.Diagnostics(), [&](const ast::Expression* child) {
                        const_chains.Remove(child);
                        return child == expr ? ast::TraverseAction::Descend
                                             : ast::TraverseAction::Skip;
                    });
                if (!ok) {
                    return Program(std::move(b));
                }
                const_chains.Add(expr);
            } else if (should_hoist(sem)) {
                to_hoist.Push(sem);
            }
        }
    }

    // After walking the full AST, const_chains only contains the outer-most constant expressions.
    // Check if any of these need hoisting, and append those to to_hoist.
    for (auto* expr : const_chains) {
        if (auto* sem = src->Sem().GetVal(expr); should_hoist(sem)) {
            to_hoist.Push(sem);
        }
    }

    if (to_hoist.IsEmpty()) {
        // Nothing to do. Skip.
        return SkipTransform;
    }

    // The order of to_hoist is currently undefined. Sort by AST node id, which will make this
    // deterministic.
    to_hoist.Sort([&](auto* expr_a, auto* expr_b) {
        return expr_a->Declaration()->node_id < expr_b->Declaration()->node_id;
    });

    // Hoist all the expressions in to_hoist to a constant variable, declared just before the
    // statement of usage.
    HoistToDeclBefore hoist_to_decl_before(ctx);
    for (auto* expr : to_hoist) {
        if (!hoist_to_decl_before.Add(expr, expr->Declaration(),
                                      HoistToDeclBefore::VariableKind::kLet)) {
            return Program(std::move(b));
        }
    }

    ctx.Clone();
    return Program(std::move(b));
}

}  // namespace tint::transform
