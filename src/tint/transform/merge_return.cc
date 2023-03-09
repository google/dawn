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

#include "src/tint/transform/merge_return.h"

#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/statement.h"
#include "src/tint/switch.h"
#include "src/tint/utils/scoped_assignment.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::MergeReturn);

using namespace tint::number_suffixes;  // NOLINT

namespace tint::transform {

namespace {

/// Returns `true` if `stmt` has the behavior `behavior`.
bool HasBehavior(const Program* program, const ast::Statement* stmt, sem::Behavior behavior) {
    return program->Sem().Get(stmt)->Behaviors().Contains(behavior);
}

/// Returns `true` if `func` needs to be transformed.
bool NeedsTransform(const Program* program, const ast::Function* func) {
    // Entry points and intrinsic declarations never need transforming.
    if (func->IsEntryPoint() || func->body == nullptr) {
        return false;
    }

    // Avoid transforming functions that only have a single exit point.
    // TODO(jrprice): Alternatively, use the uniformity analysis to decide which
    // functions need to be transformed.
    for (auto* s : func->body->statements) {
        // Find the first statement that has contains the Return behavior.
        if (HasBehavior(program, s, sem::Behavior::kReturn)) {
            // If this statement is itself a return, it will be the only exit point,
            // so no need to apply the transform to the function.
            if (s->Is<ast::ReturnStatement>()) {
                return false;
            } else {
                // Apply the transform in all other cases.
                return true;
            }
        }
    }
    return false;
}

}  // namespace

MergeReturn::MergeReturn() = default;

MergeReturn::~MergeReturn() = default;

namespace {

/// Internal class used to during the transform.
class State {
  private:
    /// The clone context.
    CloneContext& ctx;

    /// The program builder.
    ProgramBuilder& b;

    /// The function.
    const ast::Function* function;

    /// The symbol for the return flag variable.
    Symbol flag;

    /// The symbol for the return value variable.
    Symbol retval;

    /// Tracks whether we are currently inside a loop or switch statement.
    bool is_in_loop_or_switch = false;

  public:
    /// Constructor
    /// @param context the clone context
    State(CloneContext& context, const ast::Function* func)
        : ctx(context), b(*ctx.dst), function(func) {}

    /// Process a statement (recursively).
    void ProcessStatement(const ast::Statement* stmt) {
        if (stmt == nullptr || !HasBehavior(ctx.src, stmt, sem::Behavior::kReturn)) {
            return;
        }

        Switch(
            stmt, [&](const ast::BlockStatement* block) { ProcessBlock(block); },
            [&](const ast::CaseStatement* c) { ProcessStatement(c->body); },
            [&](const ast::ForLoopStatement* f) {
                TINT_SCOPED_ASSIGNMENT(is_in_loop_or_switch, true);
                ProcessStatement(f->body);
            },
            [&](const ast::IfStatement* i) {
                ProcessStatement(i->body);
                ProcessStatement(i->else_statement);
            },
            [&](const ast::LoopStatement* l) {
                TINT_SCOPED_ASSIGNMENT(is_in_loop_or_switch, true);
                ProcessStatement(l->body);
            },
            [&](const ast::ReturnStatement* r) {
                utils::Vector<const ast::Statement*, 3> stmts;
                // Set the return flag to signal that we have hit a return.
                stmts.Push(b.Assign(b.Expr(flag), true));
                if (r->value) {
                    // Set the return value if necessary.
                    stmts.Push(b.Assign(b.Expr(retval), ctx.Clone(r->value)));
                }
                if (is_in_loop_or_switch) {
                    // If we are in a loop or switch statement, break out of it.
                    stmts.Push(b.Break());
                }
                ctx.Replace(r, b.Block(std::move(stmts)));
            },
            [&](const ast::SwitchStatement* s) {
                TINT_SCOPED_ASSIGNMENT(is_in_loop_or_switch, true);
                for (auto* c : s->body) {
                    ProcessStatement(c);
                }
            },
            [&](const ast::WhileStatement* w) {
                TINT_SCOPED_ASSIGNMENT(is_in_loop_or_switch, true);
                ProcessStatement(w->body);
            },
            [&](Default) { TINT_ICE(Transform, b.Diagnostics()) << "unhandled statement type"; });
    }

    void ProcessBlock(const ast::BlockStatement* block) {
        // We will rebuild the contents of the block statement.
        // We may introduce conditionals around statements that follow a statement with the
        // `Return` behavior, so build a stack of statement lists that represent the new
        // (potentially nested) conditional blocks.
        utils::Vector<utils::Vector<const ast::Statement*, 8>, 8> new_stmts({{}});

        // Insert variables for the return flag and return value at the top of the function.
        if (block == function->body) {
            flag = b.Symbols().New("tint_return_flag");
            new_stmts[0].Push(b.Decl(b.Var(flag, b.ty.bool_())));

            if (function->return_type) {
                retval = b.Symbols().New("tint_return_value");
                new_stmts[0].Push(b.Decl(b.Var(retval, ctx.Clone(function->return_type))));
            }
        }

        for (auto* s : block->statements) {
            // Process the statement and add it to the current block.
            ProcessStatement(s);
            new_stmts.Back().Push(ctx.Clone(s));

            // Check if the statement is or contains a return statement.
            // We need to make sure any statements that follow this one do not get executed if the
            // return flag has been set.
            if (HasBehavior(ctx.src, s, sem::Behavior::kReturn)) {
                if (is_in_loop_or_switch) {
                    // We're in a loop/switch, and so we would have inserted a `break`.
                    // If we've just come out of a loop/switch statement, we need to `break` again.
                    if (s->IsAnyOf<ast::LoopStatement, ast::ForLoopStatement,
                                   ast::SwitchStatement>()) {
                        // If the loop only has the 'Return' behavior, we can just unconditionally
                        // break. Otherwise check the return flag.
                        if (HasBehavior(ctx.src, s, sem::Behavior::kNext)) {
                            new_stmts.Back().Push(
                                b.If(b.Expr(flag), b.Block(utils::Vector{b.Break()})));
                        } else {
                            new_stmts.Back().Push(b.Break());
                        }
                    }
                } else {
                    // Create a new list for any subsequent statements, which we will wrap in a
                    // conditional block.
                    new_stmts.Push({});
                }
            }
        }

        // Descend the stack of new block statements, wrapping them in conditionals.
        while (new_stmts.Length() > 1) {
            const ast::IfStatement* i = nullptr;
            if (new_stmts.Back().Length() > 0) {
                i = b.If(b.Not(b.Expr(flag)), b.Block(new_stmts.Back()));
            }
            new_stmts.Pop();
            if (i) {
                new_stmts.Back().Push(i);
            }
        }

        // Insert the final return statement at the end of the function body.
        if (block == function->body && retval.IsValid()) {
            new_stmts[0].Push(b.Return(b.Expr(retval)));
        }

        ctx.Replace(block, b.Block(new_stmts[0]));
    }
};

}  // namespace

Transform::ApplyResult MergeReturn::Apply(const Program* src, const DataMap&, DataMap&) const {
    ProgramBuilder b;
    CloneContext ctx{&b, src, /* auto_clone_symbols */ true};

    bool made_changes = false;

    for (auto* func : ctx.src->AST().Functions()) {
        if (!NeedsTransform(ctx.src, func)) {
            continue;
        }

        State state(ctx, func);
        state.ProcessStatement(func->body);
        made_changes = true;
    }

    if (!made_changes) {
        return SkipTransform;
    }

    ctx.Clone();
    return Program(std::move(b));
}

}  // namespace tint::transform
