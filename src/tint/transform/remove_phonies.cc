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

struct SinkSignature {
    std::vector<const sem::Type*> types;

    bool operator==(const SinkSignature& other) const {
        if (types.size() != other.types.size()) {
            return false;
        }
        for (size_t i = 0; i < types.size(); i++) {
            if (types[i] != other.types[i]) {
                return false;
            }
        }
        return true;
    }

    struct Hasher {
        /// @param sig the CallTargetSignature to hash
        /// @return the hash value
        std::size_t operator()(const SinkSignature& sig) const {
            size_t hash = tint::utils::Hash(sig.types.size());
            for (auto* ty : sig.types) {
                tint::utils::HashCombine(&hash, ty);
            }
            return hash;
        }
    };
};

}  // namespace

RemovePhonies::RemovePhonies() = default;

RemovePhonies::~RemovePhonies() = default;

bool RemovePhonies::ShouldRun(const Program* program, const DataMap&) const {
    for (auto* node : program->ASTNodes().Objects()) {
        if (node->Is<ast::PhonyExpression>()) {
            return true;
        }
    }
    return false;
}

void RemovePhonies::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    auto& sem = ctx.src->Sem();

    std::unordered_map<SinkSignature, Symbol, SinkSignature::Hasher> sinks;

    for (auto* node : ctx.src->ASTNodes().Objects()) {
        if (auto* stmt = node->As<ast::AssignmentStatement>()) {
            if (stmt->lhs->Is<ast::PhonyExpression>()) {
                std::vector<const ast::Expression*> side_effects;
                if (!ast::TraverseExpressions(
                        stmt->rhs, ctx.dst->Diagnostics(), [&](const ast::CallExpression* call) {
                            // ast::CallExpression may map to a function or builtin call
                            // (both may have side-effects), or a type constructor or
                            // type conversion (both do not have side effects).
                            if (sem.Get(call)->Target()->IsAnyOf<sem::Function, sem::Builtin>()) {
                                side_effects.push_back(call);
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
                    continue;
                }

                if (side_effects.size() == 1) {
                    if (auto* call = side_effects[0]->As<ast::CallExpression>()) {
                        // Phony assignment with single call side effect.
                        // Replace phony assignment with call.
                        ctx.Replace(stmt, [&, call] { return ctx.dst->CallStmt(ctx.Clone(call)); });
                        continue;
                    }
                }

                // Phony assignment with multiple side effects.
                // Generate a call to a placeholder function with the side
                // effects as arguments.
                ctx.Replace(stmt, [&, side_effects] {
                    SinkSignature sig;
                    for (auto* arg : side_effects) {
                        sig.types.push_back(sem.Get(arg)->Type()->UnwrapRef());
                    }
                    auto sink = utils::GetOrCreate(sinks, sig, [&] {
                        auto name = ctx.dst->Symbols().New("phony_sink");
                        ast::VariableList params;
                        for (auto* ty : sig.types) {
                            auto* ast_ty = CreateASTTypeFor(ctx, ty);
                            params.push_back(
                                ctx.dst->Param("p" + std::to_string(params.size()), ast_ty));
                        }
                        ctx.dst->Func(name, params, ctx.dst->ty.void_(), {});
                        return name;
                    });
                    ast::ExpressionList args;
                    for (auto* arg : side_effects) {
                        args.push_back(ctx.Clone(arg));
                    }
                    return ctx.dst->CallStmt(ctx.dst->Call(sink, args));
                });
            }
        }
    }

    ctx.Clone();
}

}  // namespace tint::transform
