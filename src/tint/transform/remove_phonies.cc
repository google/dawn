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

#include "src/tint/transform/remove_phonies.h"

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/ast/traverse_expressions.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/variable.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/scoped_assignment.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::RemovePhonies);

namespace tint::transform {
namespace {

using SinkSignature = std::vector<const type::Type*>;

}  // namespace

RemovePhonies::RemovePhonies() = default;

RemovePhonies::~RemovePhonies() = default;

Transform::ApplyResult RemovePhonies::Apply(const Program* src, const DataMap&, DataMap&) const {
    ProgramBuilder b;
    CloneContext ctx{&b, src, /* auto_clone_symbols */ true};

    auto& sem = src->Sem();

    utils::Hashmap<SinkSignature, Symbol, 8, utils::Hasher<SinkSignature>> sinks;

    bool made_changes = false;
    for (auto* node : src->ASTNodes().Objects()) {
        Switch(
            node,
            [&](const ast::AssignmentStatement* stmt) {
                if (stmt->lhs->Is<ast::PhonyExpression>()) {
                    made_changes = true;

                    std::vector<const ast::Expression*> side_effects;
                    if (!ast::TraverseExpressions(
                            stmt->rhs, b.Diagnostics(), [&](const ast::CallExpression* expr) {
                                // ast::CallExpression may map to a function or builtin call
                                // (both may have side-effects), or a value constructor or value
                                // conversion (both do not have side effects).
                                auto* call = sem.Get<sem::Call>(expr);
                                if (!call) {
                                    // Semantic node must be a Materialize, in which case the
                                    // expression was creation-time (compile time), so could not
                                    // have side effects. Just skip.
                                    return ast::TraverseAction::Skip;
                                }
                                if (call->Target()->IsAnyOf<sem::Function, sem::Builtin>() &&
                                    call->HasSideEffects()) {
                                    side_effects.push_back(expr);
                                    return ast::TraverseAction::Skip;
                                }
                                return ast::TraverseAction::Descend;
                            })) {
                        return;
                    }

                    if (side_effects.empty()) {
                        // Phony assignment with no side effects.
                        // Just remove it.
                        RemoveStatement(ctx, stmt);
                        return;
                    }

                    if (side_effects.size() == 1) {
                        if (auto* call_expr = side_effects[0]->As<ast::CallExpression>()) {
                            // Phony assignment with single call side effect.
                            auto* call = sem.Get(call_expr)->Unwrap()->As<sem::Call>();
                            if (call->Target()->MustUse()) {
                                // Replace phony assignment assignment to uniquely named let.
                                ctx.Replace<ast::Statement>(stmt, [&, call_expr] {  //
                                    auto name = b.Symbols().New("tint_phony");
                                    auto* rhs = ctx.Clone(call_expr);
                                    return b.Decl(b.Let(name, rhs));
                                });
                            } else {
                                // Replace phony assignment with call statement.
                                ctx.Replace(stmt, [&, call_expr] {  //
                                    return b.CallStmt(ctx.Clone(call_expr));
                                });
                            }
                            return;
                        }
                    }

                    // Phony assignment with multiple side effects.
                    // Generate a call to a placeholder function with the side
                    // effects as arguments.
                    ctx.Replace(stmt, [&, side_effects] {
                        SinkSignature sig;
                        for (auto* arg : side_effects) {
                            sig.push_back(sem.GetVal(arg)->Type()->UnwrapRef());
                        }
                        auto sink = sinks.GetOrCreate(sig, [&] {
                            auto name = b.Symbols().New("phony_sink");
                            utils::Vector<const ast::Parameter*, 8> params;
                            for (auto* ty : sig) {
                                auto ast_ty = CreateASTTypeFor(ctx, ty);
                                params.Push(b.Param("p" + std::to_string(params.Length()), ast_ty));
                            }
                            b.Func(name, params, b.ty.void_(), {});
                            return name;
                        });
                        utils::Vector<const ast::Expression*, 8> args;
                        for (auto* arg : side_effects) {
                            args.Push(ctx.Clone(arg));
                        }
                        return b.CallStmt(b.Call(sink, args));
                    });
                }
            },
            [&](const ast::CallStatement* stmt) {
                // Remove call statements to const value-returning functions.
                // TODO(crbug.com/tint/1637): Remove if `stmt->expr` has no side-effects.
                auto* sem_expr = sem.Get(stmt->expr);
                if ((sem_expr->ConstantValue() != nullptr) && !sem_expr->HasSideEffects()) {
                    made_changes = true;
                    ctx.Remove(sem.Get(stmt)->Block()->Declaration()->statements, stmt);
                }
            });
    }

    if (!made_changes) {
        return SkipTransform;
    }

    ctx.Clone();
    return Program(std::move(b));
}

}  // namespace tint::transform
