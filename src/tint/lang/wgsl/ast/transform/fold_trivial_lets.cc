// Copyright 2023 The Tint Authors.
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

#include "src/tint/lang/wgsl/ast/transform/fold_trivial_lets.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/traverse_expressions.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/utils/containers/hashmap.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::FoldTrivialLets);

namespace tint::ast::transform {

/// PIMPL state for the transform.
struct FoldTrivialLets::State {
    /// The source program
    const Program* const src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    CloneContext ctx = {&b, src, /* auto_clone_symbols */ true};
    /// The semantic info.
    const sem::Info& sem = {ctx.src->Sem()};

    /// Constructor
    /// @param program the source program
    explicit State(const Program* program) : src(program) {}

    /// Process a block.
    /// @param block the block
    void ProcessBlock(const BlockStatement* block) {
        // PendingLet describes a let declaration that might be inlined.
        struct PendingLet {
            // The let declaration.
            const VariableDeclStatement* decl = nullptr;
            // The number of uses that have not yet been inlined.
            size_t remaining_uses = 0;
        };

        // A map from semantic variables to their PendingLet descriptors.
        Hashmap<const sem::Variable*, PendingLet, 16> pending_lets;

        // Helper that folds pending let declarations into `expr` if possible.
        auto fold_lets = [&](const Expression* expr) {
            TraverseExpressions(expr, [&](const IdentifierExpression* ident) {
                if (auto* user = sem.Get<sem::VariableUser>(ident)) {
                    auto itr = pending_lets.Find(user->Variable());
                    if (itr) {
                        TINT_ASSERT(itr->remaining_uses > 0);

                        // We found a reference to a pending let, so replace it with the inlined
                        // initializer expression.
                        ctx.Replace(ident, ctx.Clone(itr->decl->variable->initializer));

                        // Decrement the remaining uses count and remove the let declaration if this
                        // was the last remaining use.
                        if (--itr->remaining_uses == 0) {
                            ctx.Remove(block->statements, itr->decl);
                        }
                    }
                }
                return TraverseAction::Descend;
            });
        };

        // Loop over all statements in the block.
        for (auto* stmt : block->statements) {
            // Check for a let declarations.
            if (auto* decl = stmt->As<VariableDeclStatement>()) {
                if (auto* let = decl->variable->As<Let>()) {
                    // If the initializer doesn't have side effects, we might be able to inline it.
                    if (!sem.GetVal(let->initializer)->HasSideEffects()) {  //
                        auto num_users = sem.Get(let)->Users().Length();
                        if (let->initializer->Is<IdentifierExpression>()) {
                            // The initializer is a single identifier expression.
                            // We can fold it into multiple uses in the next non-let statement.
                            // We also fold previous pending lets into this one, but only if
                            // it's only used once (to avoid duplicating potentially complex
                            // expressions).
                            if (num_users == 1) {
                                fold_lets(let->initializer);
                            }
                            pending_lets.Add(sem.Get(let), PendingLet{decl, num_users});
                        } else {
                            // The initializer is something more complex, so we only want to inline
                            // it if it's only used once.
                            // We also fold previous pending lets into this one.
                            fold_lets(let->initializer);
                            if (num_users == 1) {
                                pending_lets.Add(sem.Get(let), PendingLet{decl, 1});
                            }
                        }
                        continue;
                    }
                }
            }

            // Fold pending let declarations into a select few places that are frequently generated
            // by the SPIR_V reader.
            if (auto* assign = stmt->As<AssignmentStatement>()) {
                // We can fold into the RHS of an assignment statement if the RHS and LHS
                // expressions have no side effects.
                if (!sem.GetVal(assign->lhs)->HasSideEffects() &&
                    !sem.GetVal(assign->rhs)->HasSideEffects()) {
                    fold_lets(assign->rhs);
                }
            } else if (auto* ifelse = stmt->As<IfStatement>()) {
                // We can fold into the condition of an if statement if the condition expression has
                // no side effects.
                if (!sem.GetVal(ifelse->condition)->HasSideEffects()) {
                    fold_lets(ifelse->condition);
                }
            }

            // Clear any remaining pending lets.
            // We do not try to fold lets beyond the first non-let statement.
            pending_lets.Clear();
        }
    }

    /// Runs the transform.
    /// @returns the new program
    ApplyResult Run() {
        // Process all blocks in the module.
        for (auto* node : src->ASTNodes().Objects()) {
            if (auto* block = node->As<BlockStatement>()) {
                ProcessBlock(block);
            }
        }
        ctx.Clone();
        return Program(std::move(b));
    }
};

FoldTrivialLets::FoldTrivialLets() = default;

FoldTrivialLets::~FoldTrivialLets() = default;

Transform::ApplyResult FoldTrivialLets::Apply(const Program* src, const DataMap&, DataMap&) const {
    return State(src).Run();
}

}  // namespace tint::ast::transform
