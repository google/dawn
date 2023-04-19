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

#include "src/tint/transform/promote_side_effects_to_decl.h"

#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/ast/traverse_expressions.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/for_loop_statement.h"
#include "src/tint/sem/if_statement.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/variable.h"
#include "src/tint/sem/while_statement.h"
#include "src/tint/transform/manager.h"
#include "src/tint/transform/utils/get_insertion_point.h"
#include "src/tint/transform/utils/hoist_to_decl_before.h"
#include "src/tint/utils/scoped_assignment.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::PromoteSideEffectsToDecl);

namespace tint::transform {
namespace {

// Base state class for common members
class StateBase {
  protected:
    CloneContext& ctx;
    ProgramBuilder& b;
    const sem::Info& sem;

    explicit StateBase(CloneContext& ctx_in)
        : ctx(ctx_in), b(*ctx_in.dst), sem(ctx_in.src->Sem()) {}
};

// This first transform converts side-effecting for-loops to loops and else-ifs
// to else {if}s so that the next transform, DecomposeSideEffects, can insert
// hoisted expressions above their current location.
struct SimplifySideEffectStatements : tint::utils::Castable<PromoteSideEffectsToDecl, Transform> {
    ApplyResult Apply(const Program* src, const DataMap& inputs, DataMap& outputs) const override;
};

Transform::ApplyResult SimplifySideEffectStatements::Apply(const Program* src,
                                                           const DataMap&,
                                                           DataMap&) const {
    ProgramBuilder b;
    CloneContext ctx{&b, src, /* auto_clone_symbols */ true};

    bool made_changes = false;

    HoistToDeclBefore hoist_to_decl_before(ctx);
    for (auto* node : ctx.src->ASTNodes().Objects()) {
        if (auto* sem_expr = src->Sem().GetVal(node)) {
            if (!sem_expr->HasSideEffects()) {
                continue;
            }

            hoist_to_decl_before.Prepare(sem_expr);
            made_changes = true;
        }
    }

    if (!made_changes) {
        return SkipTransform;
    }

    ctx.Clone();
    return Program(std::move(b));
}

// Decomposes side-effecting expressions to ensure order of evaluation. This
// handles both breaking down logical binary expressions for short-circuit
// evaluation, as well as hoisting expressions to ensure order of evaluation.
struct DecomposeSideEffects : tint::utils::Castable<PromoteSideEffectsToDecl, Transform> {
    class CollectHoistsState;
    class DecomposeState;
    ApplyResult Apply(const Program* src, const DataMap& inputs, DataMap& outputs) const override;
};

// CollectHoistsState traverses the AST top-down, identifying which expressions
// need to be hoisted to ensure order of evaluation, both those that give
// side-effects, as well as those that receive, and returns a set of these
// expressions.
using ToHoistSet = std::unordered_set<const ast::Expression*>;
class DecomposeSideEffects::CollectHoistsState : public StateBase {
    // Expressions to hoist because they either cause or receive side-effects.
    ToHoistSet to_hoist;

    // Used to mark expressions as not or no longer having side-effects.
    std::unordered_set<const ast::Expression*> no_side_effects;

    // Returns true if `expr` has side-effects. Unlike invoking
    // sem::ValueExpression::HasSideEffects(), this function takes into account whether
    // `expr` has been hoisted, returning false in that case. Furthermore, it
    // returns the correct result on parent expression nodes by traversing the
    // expression tree, memoizing the results to ensure O(1) amortized lookup.
    bool HasSideEffects(const ast::Expression* expr) {
        if (no_side_effects.count(expr)) {
            return false;
        }

        return Switch(
            expr,
            [&](const ast::CallExpression* e) -> bool { return sem.Get(e)->HasSideEffects(); },
            [&](const ast::BinaryExpression* e) {
                if (HasSideEffects(e->lhs) || HasSideEffects(e->rhs)) {
                    return true;
                }
                no_side_effects.insert(e);
                return false;
            },
            [&](const ast::IndexAccessorExpression* e) {
                if (HasSideEffects(e->object) || HasSideEffects(e->index)) {
                    return true;
                }
                no_side_effects.insert(e);
                return false;
            },
            [&](const ast::MemberAccessorExpression* e) {
                if (HasSideEffects(e->object)) {
                    return true;
                }
                no_side_effects.insert(e);
                return false;
            },
            [&](const ast::BitcastExpression* e) {  //
                if (HasSideEffects(e->expr)) {
                    return true;
                }
                no_side_effects.insert(e);
                return false;
            },

            [&](const ast::UnaryOpExpression* e) {  //
                if (HasSideEffects(e->expr)) {
                    return true;
                }
                no_side_effects.insert(e);
                return false;
            },
            [&](const ast::IdentifierExpression* e) {
                no_side_effects.insert(e);
                return false;
            },
            [&](const ast::LiteralExpression* e) {
                no_side_effects.insert(e);
                return false;
            },
            [&](const ast::PhonyExpression* e) {
                no_side_effects.insert(e);
                return false;
            },
            [&](Default) {
                TINT_ICE(Transform, b.Diagnostics()) << "Unhandled expression type";
                return false;
            });
    }

    // Adds `e` to `to_hoist` for hoisting to a let later on.
    void Hoist(const ast::Expression* e) {
        no_side_effects.insert(e);
        to_hoist.emplace(e);
    }

    // Hoists any expressions in `maybe_hoist` and clears it
    template <size_t N>
    void Flush(tint::utils::Vector<const ast::Expression*, N>& maybe_hoist) {
        for (auto* m : maybe_hoist) {
            Hoist(m);
        }
        maybe_hoist.Clear();
    }

    // Recursive function that processes expressions for side-effects. It
    // traverses the expression tree child before parent, left-to-right. Each call
    // returns whether the input expression should maybe be hoisted, allowing the
    // parent node to decide whether to hoist or not. Generally:
    // * When 'true' is returned, the expression is added to the maybe_hoist list.
    // * When a side-effecting expression is met, we flush the expressions in the
    // maybe_hoist list, as they are potentially receivers of the side-effects.
    // * For index and member accessor expressions, special care is taken to not
    // over-hoist the lhs expressions, as these may be be chained to refer to a
    // single memory location.
    template <size_t N>
    bool ProcessExpression(const ast::Expression* expr,
                           tint::utils::Vector<const ast::Expression*, N>& maybe_hoist) {
        auto process = [&](const ast::Expression* e) -> bool {
            return ProcessExpression(e, maybe_hoist);
        };

        auto default_process = [&](const ast::Expression* e) {
            auto maybe = process(e);
            if (maybe) {
                maybe_hoist.Push(e);
            }
            if (HasSideEffects(e)) {
                Flush(maybe_hoist);
            }
            return false;
        };

        auto binary_process = [&](const ast::Expression* lhs, const ast::Expression* rhs) {
            // If neither side causes side-effects, but at least one receives them,
            // let parent node hoist. This avoids over-hoisting side-effect receivers
            // of compound binary expressions (e.g. for "((a && b) && c) && f()", we
            // don't want to hoist each of "a", "b", and "c" separately, but want to
            // hoist "((a && b) && c)".
            if (!HasSideEffects(lhs) && !HasSideEffects(rhs)) {
                auto lhs_maybe = process(lhs);
                auto rhs_maybe = process(rhs);
                if (lhs_maybe || rhs_maybe) {
                    return true;
                }
                return false;
            }

            default_process(lhs);
            default_process(rhs);
            return false;
        };

        auto accessor_process = [&](const ast::Expression* lhs,
                                    const ast::Expression* rhs = nullptr) {
            auto maybe = process(lhs);
            // If lhs is a variable, let parent node hoist otherwise flush it right
            // away. This is to avoid over-hoisting the lhs of accessor chains (e.g.
            // for "v[a][b][c] + g()" we want to hoist all of "v[a][b][c]", not "t1 =
            // v[a]", then "t2 = t1[b]" then "t3 = t2[c]").
            if (maybe && HasSideEffects(lhs)) {
                maybe_hoist.Push(lhs);
                Flush(maybe_hoist);
                maybe = false;
            }
            if (rhs) {
                default_process(rhs);
            }
            return maybe;
        };

        return Switch(
            expr,
            [&](const ast::CallExpression* e) -> bool {
                // We eagerly flush any variables in maybe_hoist for the current
                // call expression. Then we scope maybe_hoist to the processing of
                // the call args. This ensures that given: g(c, a(0), d) we hoist
                // 'c' because of 'a(0)', but not 'd' because there's no need, since
                // the call to g() will be hoisted if necessary.
                if (HasSideEffects(e)) {
                    Flush(maybe_hoist);
                }

                TINT_SCOPED_ASSIGNMENT(maybe_hoist, {});
                for (auto* a : e->args) {
                    default_process(a);
                }

                // Always hoist this call, even if it has no side-effects to ensure
                // left-to-right order of evaluation.
                // E.g. for "no_side_effects() + side_effects()", we want to hoist
                // no_side_effects() first.
                return true;
            },
            [&](const ast::IdentifierExpression* e) {
                if (auto* sem_e = sem.GetVal(e)) {
                    if (auto* var_user = sem_e->UnwrapLoad()->As<sem::VariableUser>()) {
                        // Don't hoist constants.
                        if (var_user->ConstantValue()) {
                            return false;
                        }
                        // Don't hoist read-only variables as they cannot receive side-effects.
                        if (var_user->Variable()->Access() == builtin::Access::kRead) {
                            return false;
                        }
                        // Don't hoist textures / samplers as they can't be placed into a let, nor
                        // can they have side effects.
                        if (var_user->Variable()->Type()->IsAnyOf<type::Texture, type::Sampler>()) {
                            return false;
                        }
                        return true;
                    }
                }
                return false;
            },
            [&](const ast::BinaryExpression* e) {
                if (e->IsLogical() && HasSideEffects(e)) {
                    // Don't hoist children of logical binary expressions with
                    // side-effects. These will be handled by DecomposeState.
                    process(e->lhs);
                    process(e->rhs);
                    return false;
                }
                return binary_process(e->lhs, e->rhs);
            },
            [&](const ast::BitcastExpression* e) {  //
                return process(e->expr);
            },
            [&](const ast::UnaryOpExpression* e) {  //
                auto r = process(e->expr);
                // Don't hoist address-of expressions.
                // E.g. for "g(&b, a(0))", we hoist "a(0)" only.
                if (e->op == ast::UnaryOp::kAddressOf) {
                    return false;
                }
                return r;
            },
            [&](const ast::IndexAccessorExpression* e) {
                return accessor_process(e->object, e->index);
            },
            [&](const ast::MemberAccessorExpression* e) { return accessor_process(e->object); },
            [&](const ast::LiteralExpression*) {
                // Leaf
                return false;
            },
            [&](const ast::PhonyExpression*) {
                // Leaf
                return false;
            },
            [&](Default) {
                TINT_ICE(Transform, b.Diagnostics()) << "Unhandled expression type";
                return false;
            });
    }

    // Starts the recursive processing of a statement's expression(s) to hoist side-effects to lets.
    void ProcessExpression(const ast::Expression* expr) {
        if (!expr) {
            return;
        }

        tint::utils::Vector<const ast::Expression*, 8> maybe_hoist;
        ProcessExpression(expr, maybe_hoist);
    }

  public:
    explicit CollectHoistsState(CloneContext& ctx_in) : StateBase(ctx_in) {}

    ToHoistSet Run() {
        // Traverse all statements, recursively processing their expression tree(s)
        // to hoist side-effects to lets.
        for (auto* node : ctx.src->ASTNodes().Objects()) {
            auto* stmt = node->As<ast::Statement>();
            if (!stmt) {
                continue;
            }

            Switch(
                stmt,  //
                [&](const ast::AssignmentStatement* s) {
                    tint::utils::Vector<const ast::Expression*, 8> maybe_hoist;
                    ProcessExpression(s->lhs, maybe_hoist);
                    ProcessExpression(s->rhs, maybe_hoist);
                },
                [&](const ast::CallStatement* s) {  //
                    ProcessExpression(s->expr);
                },
                [&](const ast::ForLoopStatement* s) { ProcessExpression(s->condition); },
                [&](const ast::WhileStatement* s) { ProcessExpression(s->condition); },
                [&](const ast::IfStatement* s) {  //
                    ProcessExpression(s->condition);
                },
                [&](const ast::ReturnStatement* s) {  //
                    ProcessExpression(s->value);
                },
                [&](const ast::SwitchStatement* s) { ProcessExpression(s->condition); },
                [&](const ast::VariableDeclStatement* s) {
                    ProcessExpression(s->variable->initializer);
                });
        }

        return std::move(to_hoist);
    }
};

// DecomposeState performs the actual transforming of the AST to ensure order of
// evaluation, using the set of expressions to hoist collected by
// CollectHoistsState.
class DecomposeSideEffects::DecomposeState : public StateBase {
    ToHoistSet to_hoist;

    // Returns true if `binary_expr` should be decomposed for short-circuit eval.
    bool IsLogicalWithSideEffects(const ast::BinaryExpression* binary_expr) {
        return binary_expr->IsLogical() && (sem.GetVal(binary_expr->lhs)->HasSideEffects() ||
                                            sem.GetVal(binary_expr->rhs)->HasSideEffects());
    }

    // Recursive function used to decompose an expression for short-circuit eval.
    template <size_t N>
    const ast::Expression* Decompose(const ast::Expression* expr,
                                     tint::utils::Vector<const ast::Statement*, N>* curr_stmts) {
        // Helper to avoid passing in same args.
        auto decompose = [&](auto& e) { return Decompose(e, curr_stmts); };

        // Clones `expr`, possibly hoisting it to a let.
        auto clone_maybe_hoisted = [&](const ast::Expression* e) -> const ast::Expression* {
            if (to_hoist.count(e)) {
                auto name = b.Symbols().New();
                auto* v = b.Let(name, ctx.Clone(e));
                auto* decl = b.Decl(v);
                curr_stmts->Push(decl);
                return b.Expr(name);
            }
            return ctx.Clone(e);
        };

        return Switch(
            expr,
            [&](const ast::BinaryExpression* bin_expr) -> const ast::Expression* {
                if (!IsLogicalWithSideEffects(bin_expr)) {
                    // No short-circuit, emit usual binary expr
                    ctx.Replace(bin_expr->lhs, decompose(bin_expr->lhs));
                    ctx.Replace(bin_expr->rhs, decompose(bin_expr->rhs));
                    return clone_maybe_hoisted(bin_expr);
                }

                // Decompose into ifs to implement short-circuiting
                // For example, 'let r = a && b' becomes:
                //
                // var temp = a;
                // if (temp) {
                //   temp = b;
                // }
                // let r = temp;
                //
                // and similarly, 'let r = a || b' becomes:
                //
                // var temp = a;
                // if (!temp) {
                //     temp = b;
                // }
                // let r = temp;
                //
                // Further, compound logical binary expressions are also handled
                // recursively, for example, 'let r = (a && (b && c))' becomes:
                //
                // var temp = a;
                // if (temp) {
                //     var temp2 = b;
                //     if (temp2) {
                //         temp2 = c;
                //     }
                //     temp = temp2;
                // }
                // let r = temp;

                auto name = b.Sym();
                curr_stmts->Push(b.Decl(b.Var(name, decompose(bin_expr->lhs))));

                const ast::Expression* if_cond = nullptr;
                if (bin_expr->IsLogicalOr()) {
                    if_cond = b.Not(name);
                } else {
                    if_cond = b.Expr(name);
                }

                const ast::BlockStatement* if_body = nullptr;
                {
                    tint::utils::Vector<const ast::Statement*, N> stmts;
                    TINT_SCOPED_ASSIGNMENT(curr_stmts, &stmts);
                    auto* new_rhs = decompose(bin_expr->rhs);
                    curr_stmts->Push(b.Assign(name, new_rhs));
                    if_body = b.Block(std::move(*curr_stmts));
                }

                curr_stmts->Push(b.If(if_cond, if_body));

                return b.Expr(name);
            },
            [&](const ast::IndexAccessorExpression* idx) {
                ctx.Replace(idx->object, decompose(idx->object));
                ctx.Replace(idx->index, decompose(idx->index));
                return clone_maybe_hoisted(idx);
            },
            [&](const ast::BitcastExpression* bitcast) {
                ctx.Replace(bitcast->expr, decompose(bitcast->expr));
                return clone_maybe_hoisted(bitcast);
            },
            [&](const ast::CallExpression* call) {
                for (auto* a : call->args) {
                    ctx.Replace(a, decompose(a));
                }
                return clone_maybe_hoisted(call);
            },
            [&](const ast::MemberAccessorExpression* member) {
                ctx.Replace(member->object, decompose(member->object));
                return clone_maybe_hoisted(member);
            },
            [&](const ast::UnaryOpExpression* unary) {
                ctx.Replace(unary->expr, decompose(unary->expr));
                return clone_maybe_hoisted(unary);
            },
            [&](const ast::LiteralExpression* lit) {
                return clone_maybe_hoisted(lit);  // Leaf expression, just clone as is
            },
            [&](const ast::IdentifierExpression* id) {
                return clone_maybe_hoisted(id);  // Leaf expression, just clone as is
            },
            [&](const ast::PhonyExpression* phony) {
                return clone_maybe_hoisted(phony);  // Leaf expression, just clone as is
            },
            [&](Default) {
                TINT_ICE(AST, b.Diagnostics())
                    << "unhandled expression type: " << expr->TypeInfo().name;
                return nullptr;
            });
    }

    // Inserts statements in `stmts` before `stmt`
    template <size_t N>
    void InsertBefore(tint::utils::Vector<const ast::Statement*, N>& stmts,
                      const ast::Statement* stmt) {
        if (!stmts.IsEmpty()) {
            auto ip = utils::GetInsertionPoint(ctx, stmt);
            for (auto* s : stmts) {
                ctx.InsertBefore(ip.first->Declaration()->statements, ip.second, s);
            }
        }
    }

    // Decomposes expressions of `stmt`, returning a replacement statement or
    // nullptr if not replacing it.
    const ast::Statement* DecomposeStatement(const ast::Statement* stmt) {
        return Switch(
            stmt,
            [&](const ast::AssignmentStatement* s) -> const ast::Statement* {
                if (!sem.GetVal(s->lhs)->HasSideEffects() &&
                    !sem.GetVal(s->rhs)->HasSideEffects()) {
                    return nullptr;
                }
                // lhs before rhs
                tint::utils::Vector<const ast::Statement*, 8> stmts;
                ctx.Replace(s->lhs, Decompose(s->lhs, &stmts));
                ctx.Replace(s->rhs, Decompose(s->rhs, &stmts));
                InsertBefore(stmts, s);
                return ctx.CloneWithoutTransform(s);
            },
            [&](const ast::CallStatement* s) -> const ast::Statement* {
                if (!sem.Get(s->expr)->HasSideEffects()) {
                    return nullptr;
                }
                tint::utils::Vector<const ast::Statement*, 8> stmts;
                ctx.Replace(s->expr, Decompose(s->expr, &stmts));
                InsertBefore(stmts, s);
                return ctx.CloneWithoutTransform(s);
            },
            [&](const ast::ForLoopStatement* s) -> const ast::Statement* {
                if (!s->condition || !sem.GetVal(s->condition)->HasSideEffects()) {
                    return nullptr;
                }
                tint::utils::Vector<const ast::Statement*, 8> stmts;
                ctx.Replace(s->condition, Decompose(s->condition, &stmts));
                InsertBefore(stmts, s);
                return ctx.CloneWithoutTransform(s);
            },
            [&](const ast::WhileStatement* s) -> const ast::Statement* {
                if (!sem.GetVal(s->condition)->HasSideEffects()) {
                    return nullptr;
                }
                tint::utils::Vector<const ast::Statement*, 8> stmts;
                ctx.Replace(s->condition, Decompose(s->condition, &stmts));
                InsertBefore(stmts, s);
                return ctx.CloneWithoutTransform(s);
            },
            [&](const ast::IfStatement* s) -> const ast::Statement* {
                if (!sem.GetVal(s->condition)->HasSideEffects()) {
                    return nullptr;
                }
                tint::utils::Vector<const ast::Statement*, 8> stmts;
                ctx.Replace(s->condition, Decompose(s->condition, &stmts));
                InsertBefore(stmts, s);
                return ctx.CloneWithoutTransform(s);
            },
            [&](const ast::ReturnStatement* s) -> const ast::Statement* {
                if (!s->value || !sem.GetVal(s->value)->HasSideEffects()) {
                    return nullptr;
                }
                tint::utils::Vector<const ast::Statement*, 8> stmts;
                ctx.Replace(s->value, Decompose(s->value, &stmts));
                InsertBefore(stmts, s);
                return ctx.CloneWithoutTransform(s);
            },
            [&](const ast::SwitchStatement* s) -> const ast::Statement* {
                if (!sem.Get(s->condition)) {
                    return nullptr;
                }
                tint::utils::Vector<const ast::Statement*, 8> stmts;
                ctx.Replace(s->condition, Decompose(s->condition, &stmts));
                InsertBefore(stmts, s);
                return ctx.CloneWithoutTransform(s);
            },
            [&](const ast::VariableDeclStatement* s) -> const ast::Statement* {
                auto* var = s->variable;
                if (!var->initializer || !sem.GetVal(var->initializer)->HasSideEffects()) {
                    return nullptr;
                }
                tint::utils::Vector<const ast::Statement*, 8> stmts;
                ctx.Replace(var->initializer, Decompose(var->initializer, &stmts));
                InsertBefore(stmts, s);
                return b.Decl(ctx.CloneWithoutTransform(var));
            },
            [](Default) -> const ast::Statement* {
                // Other statement types don't have expressions
                return nullptr;
            });
    }

  public:
    explicit DecomposeState(CloneContext& ctx_in, ToHoistSet to_hoist_in)
        : StateBase(ctx_in), to_hoist(std::move(to_hoist_in)) {}

    void Run() {
        // We replace all BlockStatements as this allows us to iterate over the
        // block statements and ctx.InsertBefore hoisted declarations on them.
        ctx.ReplaceAll([&](const ast::BlockStatement* block) -> const ast::Statement* {
            for (auto* stmt : block->statements) {
                if (auto* new_stmt = DecomposeStatement(stmt)) {
                    ctx.Replace(stmt, new_stmt);
                }

                // Handle for loops, as they are the only other AST node that
                // contains statements outside of BlockStatements.
                if (auto* fl = stmt->As<ast::ForLoopStatement>()) {
                    if (auto* new_stmt = DecomposeStatement(fl->initializer)) {
                        ctx.Replace(fl->initializer, new_stmt);
                    }
                    if (auto* new_stmt = DecomposeStatement(fl->continuing)) {
                        ctx.Replace(fl->continuing, new_stmt);
                    }
                }
            }
            return nullptr;
        });
    }
};

Transform::ApplyResult DecomposeSideEffects::Apply(const Program* src,
                                                   const DataMap&,
                                                   DataMap&) const {
    ProgramBuilder b;
    CloneContext ctx{&b, src, /* auto_clone_symbols */ true};

    // First collect side-effecting expressions to hoist
    CollectHoistsState collect_hoists_state{ctx};
    auto to_hoist = collect_hoists_state.Run();

    // Now decompose these expressions
    DecomposeState decompose_state{ctx, std::move(to_hoist)};
    decompose_state.Run();

    ctx.Clone();
    return Program(std::move(b));
}

}  // namespace

PromoteSideEffectsToDecl::PromoteSideEffectsToDecl() = default;
PromoteSideEffectsToDecl::~PromoteSideEffectsToDecl() = default;

Transform::ApplyResult PromoteSideEffectsToDecl::Apply(const Program* src,
                                                       const DataMap& inputs,
                                                       DataMap& outputs) const {
    transform::Manager manager;
    manager.Add<SimplifySideEffectStatements>();
    manager.Add<DecomposeSideEffects>();
    return manager.Apply(src, inputs, outputs);
}

}  // namespace tint::transform
