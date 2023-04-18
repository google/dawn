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

#include "src/tint/transform/simplify_pointers.h"

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/program_builder.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/variable.h"
#include "src/tint/switch.h"
#include "src/tint/transform/unshadow.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::SimplifyPointers);

namespace tint::transform {

namespace {

/// PointerOp describes either possible indirection or address-of action on an
/// expression.
struct PointerOp {
    /// Positive: Number of times the `expr` was dereferenced (*expr)
    /// Negative: Number of times the `expr` was 'addressed-of' (&expr)
    /// Zero: no pointer op on `expr`
    int indirections = 0;
    /// The expression being operated on
    const ast::Expression* expr = nullptr;
};

}  // namespace

/// PIMPL state for the transform
struct SimplifyPointers::State {
    /// The source program
    const Program* const src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    CloneContext ctx = {&b, src, /* auto_clone_symbols */ true};

    /// Constructor
    /// @param program the source program
    explicit State(const Program* program) : src(program) {}

    /// Traverses the expression `expr` looking for non-literal array indexing
    /// expressions that would affect the computed address of a pointer
    /// expression. The function-like argument `cb` is called for each found.
    /// @param expr the expression to traverse
    /// @param cb a function-like object with the signature
    /// `void(const ast::Expression*)`, which is called for each array index
    /// expression
    template <typename F>
    static void CollectSavedArrayIndices(const ast::Expression* expr, F&& cb) {
        if (auto* a = expr->As<ast::IndexAccessorExpression>()) {
            CollectSavedArrayIndices(a->object, cb);
            if (!a->index->Is<ast::LiteralExpression>()) {
                cb(a->index);
            }
            return;
        }

        if (auto* m = expr->As<ast::MemberAccessorExpression>()) {
            CollectSavedArrayIndices(m->object, cb);
            return;
        }

        if (auto* u = expr->As<ast::UnaryOpExpression>()) {
            CollectSavedArrayIndices(u->expr, cb);
            return;
        }

        // Note: Other ast::Expression types can be safely ignored as they cannot be
        // used to generate a reference or pointer.
        // See https://gpuweb.github.io/gpuweb/wgsl/#forming-references-and-pointers
    }

    /// Reduce walks the expression chain, collapsing all address-of and
    /// indirection ops into a PointerOp.
    /// @param in the expression to walk
    /// @returns the reduced PointerOp
    PointerOp Reduce(const ast::Expression* in) const {
        PointerOp op{0, in};
        while (true) {
            if (auto* unary = op.expr->As<ast::UnaryOpExpression>()) {
                switch (unary->op) {
                    case ast::UnaryOp::kIndirection:
                        op.indirections++;
                        op.expr = unary->expr;
                        continue;
                    case ast::UnaryOp::kAddressOf:
                        op.indirections--;
                        op.expr = unary->expr;
                        continue;
                    default:
                        break;
                }
            }
            if (auto* user = ctx.src->Sem().Get<sem::VariableUser>(op.expr)) {
                auto* var = user->Variable();
                if (var->Is<sem::LocalVariable>() &&       //
                    var->Declaration()->Is<ast::Let>() &&  //
                    var->Type()->Is<type::Pointer>()) {
                    op.expr = var->Declaration()->initializer;
                    continue;
                }
            }
            return op;
        }
    }

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    ApplyResult Run() {
        // A map of saved expressions to their saved variable name
        utils::Hashmap<const ast::Expression*, Symbol, 8> saved_vars;

        bool needs_transform = false;
        for (auto* ty : ctx.src->Types()) {
            if (ty->Is<type::Pointer>()) {
                // Program contains pointers which need removing.
                needs_transform = true;
                break;
            }
        }

        // Find all the pointer-typed `let` declarations.
        // Note that these must be function-scoped, as module-scoped `let`s are not
        // permitted.
        for (auto* node : ctx.src->ASTNodes().Objects()) {
            Switch(
                node,  //
                [&](const ast::VariableDeclStatement* let) {
                    if (!let->variable->Is<ast::Let>()) {
                        return;  // Not a `let` declaration. Ignore.
                    }

                    auto* var = ctx.src->Sem().Get(let->variable);
                    if (!var->Type()->Is<type::Pointer>()) {
                        return;  // Not a pointer type. Ignore.
                    }

                    // We're dealing with a pointer-typed `let` declaration.

                    // Scan the initializer expression for array index expressions that need
                    // to be hoist to temporary "saved" variables.
                    utils::Vector<const ast::VariableDeclStatement*, 8> saved;
                    CollectSavedArrayIndices(
                        var->Declaration()->initializer, [&](const ast::Expression* idx_expr) {
                            // We have a sub-expression that needs to be saved.
                            // Create a new variable
                            auto saved_name = ctx.dst->Symbols().New(
                                var->Declaration()->name->symbol.Name() + "_save");
                            auto* decl =
                                ctx.dst->Decl(ctx.dst->Let(saved_name, ctx.Clone(idx_expr)));
                            saved.Push(decl);
                            // Record the substitution of `idx_expr` to the saved variable
                            // with the symbol `saved_name`. This will be used by the
                            // ReplaceAll() handler above.
                            saved_vars.Add(idx_expr, saved_name);
                        });

                    // Find the place to insert the saved declarations.
                    // Special care needs to be made for lets declared as the initializer
                    // part of for-loops. In this case the block will hold the for-loop
                    // statement, not the let.
                    if (!saved.IsEmpty()) {
                        auto* stmt = ctx.src->Sem().Get(let);
                        auto* block = stmt->Block();
                        // Find the statement owned by the block (either the let decl or a
                        // for-loop)
                        while (block != stmt->Parent()) {
                            stmt = stmt->Parent();
                        }
                        // Declare the stored variables just before stmt. Order here is
                        // important as order-of-operations needs to be preserved.
                        // CollectSavedArrayIndices() visits the LHS of an index accessor
                        // before the index expression.
                        for (auto* decl : saved) {
                            // Note that repeated calls to InsertBefore() with the same `before`
                            // argument will result in nodes to inserted in the order the
                            // calls are made (last call is inserted last).
                            ctx.InsertBefore(block->Declaration()->statements, stmt->Declaration(),
                                             decl);
                        }
                    }

                    // As the original `let` declaration will be fully inlined, there's no
                    // need for the original declaration to exist. Remove it.
                    RemoveStatement(ctx, let);
                },
                [&](const ast::UnaryOpExpression* op) {
                    if (op->op == ast::UnaryOp::kAddressOf) {
                        // Transform can be skipped if no address-of operator is used, as there
                        // will be no pointers that can be inlined.
                        needs_transform = true;
                    }
                });
        }

        if (!needs_transform) {
            return SkipTransform;
        }

        // Register the ast::Expression transform handler.
        // This performs two different transformations:
        // * Identifiers that resolve to the pointer-typed `let` declarations are
        // replaced with the recursively inlined initializer expression for the
        // `let` declaration.
        // * Sub-expressions inside the pointer-typed `let` initializer expression
        // that have been hoisted to a saved variable are replaced with the saved
        // variable identifier.
        ctx.ReplaceAll([&](const ast::Expression* expr) -> const ast::Expression* {
            // Look to see if we need to swap this Expression with a saved variable.
            if (auto saved_var = saved_vars.Find(expr)) {
                return ctx.dst->Expr(*saved_var);
            }

            // Reduce the expression, folding away chains of address-of / indirections
            auto op = Reduce(expr);

            // Clone the reduced root expression
            expr = ctx.CloneWithoutTransform(op.expr);

            // And reapply the minimum number of address-of / indirections
            for (int i = 0; i < op.indirections; i++) {
                expr = ctx.dst->Deref(expr);
            }
            for (int i = 0; i > op.indirections; i--) {
                expr = ctx.dst->AddressOf(expr);
            }
            return expr;
        });

        ctx.Clone();
        return Program(std::move(b));
    }
};

SimplifyPointers::SimplifyPointers() = default;

SimplifyPointers::~SimplifyPointers() = default;

Transform::ApplyResult SimplifyPointers::Apply(const Program* src, const DataMap&, DataMap&) const {
    return State(src).Run();
}

}  // namespace tint::transform
