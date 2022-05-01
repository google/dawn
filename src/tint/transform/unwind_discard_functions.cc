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

#include "src/tint/transform/unwind_discard_functions.h"

#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/traverse_expressions.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/for_loop_statement.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/if_statement.h"
#include "src/tint/transform/utils/get_insertion_point.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::UnwindDiscardFunctions);

namespace tint::transform {
namespace {

class State {
  private:
    CloneContext& ctx;
    ProgramBuilder& b;
    const sem::Info& sem;
    Symbol module_discard_var_name;   // Use ModuleDiscardVarName() to read
    Symbol module_discard_func_name;  // Use ModuleDiscardFuncName() to read

    // Returns true if `sem_expr` contains a call expression that may
    // (transitively) execute a discard statement.
    bool MayDiscard(const sem::Expression* sem_expr) {
        return sem_expr && sem_expr->Behaviors().Contains(sem::Behavior::kDiscard);
    }

    // Lazily creates and returns the name of the module bool variable for whether
    // to discard: "tint_discard".
    Symbol ModuleDiscardVarName() {
        if (!module_discard_var_name.IsValid()) {
            module_discard_var_name = b.Symbols().New("tint_discard");
            ctx.dst->Global(module_discard_var_name, b.ty.bool_(), b.Expr(false),
                            ast::StorageClass::kPrivate);
        }
        return module_discard_var_name;
    }

    // Lazily creates and returns the name of the function that contains a single
    // discard statement: "tint_discard_func".
    // We do this to avoid having multiple discard statements in a single program,
    // which causes problems in certain backends (see crbug.com/1118).
    Symbol ModuleDiscardFuncName() {
        if (!module_discard_func_name.IsValid()) {
            module_discard_func_name = b.Symbols().New("tint_discard_func");
            b.Func(module_discard_func_name, {}, b.ty.void_(), {b.Discard()});
        }
        return module_discard_func_name;
    }

    // Creates "return <default return value>;" based on the return type of
    // `stmt`'s owning function.
    const ast::ReturnStatement* Return(const ast::Statement* stmt) {
        const ast::Expression* ret_val = nullptr;
        auto* ret_type = sem.Get(stmt)->Function()->Declaration()->return_type;
        if (!ret_type->Is<ast::Void>()) {
            ret_val = b.Construct(ctx.Clone(ret_type));
        }
        return b.Return(ret_val);
    }

    // Returns true if the function `stmt` is in is an entry point
    bool IsInEntryPointFunc(const ast::Statement* stmt) {
        return sem.Get(stmt)->Function()->Declaration()->IsEntryPoint();
    }

    // Creates "tint_discard_func();"
    const ast::CallStatement* CallDiscardFunc() {
        auto func_name = ModuleDiscardFuncName();
        return b.CallStmt(b.Call(func_name));
    }

    // Creates and returns a new if-statement of the form:
    //
    //    if (tint_discard) {
    //      return <default value>;
    //    }
    //
    // or if `stmt` is in a entry point function:
    //
    //    if (tint_discard) {
    //      tint_discard_func();
    //      return <default value>;
    //    }
    //
    const ast::IfStatement* IfDiscardReturn(const ast::Statement* stmt) {
        ast::StatementList stmts;

        // For entry point functions, also emit the discard statement
        if (IsInEntryPointFunc(stmt)) {
            stmts.emplace_back(CallDiscardFunc());
        }

        stmts.emplace_back(Return(stmt));

        auto var_name = ModuleDiscardVarName();
        return b.If(var_name, b.Block(stmts));
    }

    // Hoists `sem_expr` to a let followed by an `IfDiscardReturn` before `stmt`.
    // For example, if `stmt` is:
    //
    //    return f();
    //
    // This function will transform this to:
    //
    //    let t1 = f();
    //    if (tint_discard) {
    //      return;
    //    }
    //    return t1;
    //
    const ast::Statement* HoistAndInsertBefore(const ast::Statement* stmt,
                                               const sem::Expression* sem_expr) {
        auto* expr = sem_expr->Declaration();

        auto ip = utils::GetInsertionPoint(ctx, stmt);
        auto var_name = b.Sym();
        auto* decl = b.Decl(b.Var(var_name, nullptr, ctx.Clone(expr)));
        ctx.InsertBefore(ip.first->Declaration()->statements, ip.second, decl);

        ctx.InsertBefore(ip.first->Declaration()->statements, ip.second, IfDiscardReturn(stmt));

        auto* var_expr = b.Expr(var_name);

        // Special handling for CallStatement as we can only replace its expression
        // with a CallExpression.
        if (stmt->Is<ast::CallStatement>()) {
            // We could replace the call statement with no statement, but we can't do
            // that with transforms (yet), so just return a phony assignment.
            return b.Assign(b.Phony(), var_expr);
        }

        ctx.Replace(expr, var_expr);
        return ctx.CloneWithoutTransform(stmt);
    }

    // Returns true if `stmt` is a for-loop initializer statement.
    bool IsForLoopInitStatement(const ast::Statement* stmt) {
        if (auto* sem_stmt = sem.Get(stmt)) {
            if (auto* sem_fl = As<sem::ForLoopStatement>(sem_stmt->Parent())) {
                return sem_fl->Declaration()->initializer == stmt;
            }
        }
        return false;
    }

    // Inserts an `IfDiscardReturn` after `stmt` if possible (i.e. `stmt` is not
    // in a for-loop init), otherwise falls back to HoistAndInsertBefore, hoisting
    // `sem_expr` to a let followed by an `IfDiscardReturn` before `stmt`.
    //
    // For example, if `stmt` is:
    //
    //    let r = f();
    //
    // This function will transform this to:
    //
    //    let r = f();
    //    if (tint_discard) {
    //      return;
    //    }
    const ast::Statement* TryInsertAfter(const ast::Statement* stmt,
                                         const sem::Expression* sem_expr) {
        // If `stmt` is the init of a for-loop, hoist and insert before instead.
        if (IsForLoopInitStatement(stmt)) {
            return HoistAndInsertBefore(stmt, sem_expr);
        }

        auto ip = utils::GetInsertionPoint(ctx, stmt);
        ctx.InsertAfter(ip.first->Declaration()->statements, ip.second, IfDiscardReturn(stmt));
        return nullptr;  // Don't replace current statement
    }

    // Replaces the input discard statement with either setting the module level
    // discard bool ("tint_discard = true"), or calling the discard function
    // ("tint_discard_func()"), followed by a default return statement.
    //
    // Replaces "discard;" with:
    //
    //    tint_discard = true;
    //    return;
    //
    // Or if `stmt` is a entry point function, replaces with:
    //
    //    tint_discard_func();
    //    return;
    //
    const ast::Statement* ReplaceDiscardStatement(const ast::DiscardStatement* stmt) {
        const ast::Statement* to_insert = nullptr;
        if (IsInEntryPointFunc(stmt)) {
            to_insert = CallDiscardFunc();
        } else {
            auto var_name = ModuleDiscardVarName();
            to_insert = b.Assign(var_name, true);
        }

        auto ip = utils::GetInsertionPoint(ctx, stmt);
        ctx.InsertBefore(ip.first->Declaration()->statements, ip.second, to_insert);
        return Return(stmt);
    }

    // Handle statement
    const ast::Statement* Statement(const ast::Statement* stmt) {
        return Switch(
            stmt,
            [&](const ast::DiscardStatement* s) -> const ast::Statement* {
                return ReplaceDiscardStatement(s);
            },
            [&](const ast::AssignmentStatement* s) -> const ast::Statement* {
                auto* sem_lhs = sem.Get(s->lhs);
                auto* sem_rhs = sem.Get(s->rhs);
                if (MayDiscard(sem_lhs)) {
                    if (MayDiscard(sem_rhs)) {
                        TINT_ICE(Transform, b.Diagnostics())
                            << "Unexpected: both sides of assignment statement may "
                               "discard. Make sure transform::PromoteSideEffectsToDecl "
                               "was run first.";
                    }
                    return TryInsertAfter(s, sem_lhs);
                } else if (MayDiscard(sem_rhs)) {
                    return TryInsertAfter(s, sem_rhs);
                }
                return nullptr;
            },
            [&](const ast::CallStatement* s) -> const ast::Statement* {
                auto* sem_expr = sem.Get(s->expr);
                if (!MayDiscard(sem_expr)) {
                    return nullptr;
                }
                return TryInsertAfter(s, sem_expr);
            },
            [&](const ast::ForLoopStatement* s) -> const ast::Statement* {
                if (MayDiscard(sem.Get(s->condition))) {
                    TINT_ICE(Transform, b.Diagnostics())
                        << "Unexpected ForLoopStatement condition that may discard. "
                           "Make sure transform::PromoteSideEffectsToDecl was run "
                           "first.";
                }
                return nullptr;
            },
            [&](const ast::IfStatement* s) -> const ast::Statement* {
                auto* sem_expr = sem.Get(s->condition);
                if (!MayDiscard(sem_expr)) {
                    return nullptr;
                }
                return HoistAndInsertBefore(s, sem_expr);
            },
            [&](const ast::ReturnStatement* s) -> const ast::Statement* {
                auto* sem_expr = sem.Get(s->value);
                if (!MayDiscard(sem_expr)) {
                    return nullptr;
                }
                return HoistAndInsertBefore(s, sem_expr);
            },
            [&](const ast::SwitchStatement* s) -> const ast::Statement* {
                auto* sem_expr = sem.Get(s->condition);
                if (!MayDiscard(sem_expr)) {
                    return nullptr;
                }
                return HoistAndInsertBefore(s, sem_expr);
            },
            [&](const ast::VariableDeclStatement* s) -> const ast::Statement* {
                auto* var = s->variable;
                if (!var->constructor) {
                    return nullptr;
                }
                auto* sem_expr = sem.Get(var->constructor);
                if (!MayDiscard(sem_expr)) {
                    return nullptr;
                }
                return TryInsertAfter(s, sem_expr);
            });
    }

  public:
    /// Constructor
    /// @param ctx_in the context
    explicit State(CloneContext& ctx_in) : ctx(ctx_in), b(*ctx_in.dst), sem(ctx_in.src->Sem()) {}

    /// Runs the transform
    void Run() {
        ctx.ReplaceAll([&](const ast::BlockStatement* block) -> const ast::Statement* {
            // Iterate block statements and replace them as needed.
            for (auto* stmt : block->statements) {
                if (auto* new_stmt = Statement(stmt)) {
                    ctx.Replace(stmt, new_stmt);
                }

                // Handle for loops, as they are the only other AST node that
                // contains statements outside of BlockStatements.
                if (auto* fl = stmt->As<ast::ForLoopStatement>()) {
                    if (auto* new_stmt = Statement(fl->initializer)) {
                        ctx.Replace(fl->initializer, new_stmt);
                    }
                    if (auto* new_stmt = Statement(fl->continuing)) {
                        // NOTE: Should never reach here as we cannot discard in a
                        // continuing block.
                        ctx.Replace(fl->continuing, new_stmt);
                    }
                }
            }

            return nullptr;
        });

        ctx.Clone();
    }
};

}  // namespace

UnwindDiscardFunctions::UnwindDiscardFunctions() = default;
UnwindDiscardFunctions::~UnwindDiscardFunctions() = default;

void UnwindDiscardFunctions::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    State state(ctx);
    state.Run();
}

bool UnwindDiscardFunctions::ShouldRun(const Program* program, const DataMap& /*data*/) const {
    auto& sem = program->Sem();
    for (auto* f : program->AST().Functions()) {
        if (sem.Get(f)->Behaviors().Contains(sem::Behavior::kDiscard)) {
            return true;
        }
    }
    return false;
}

}  // namespace tint::transform
