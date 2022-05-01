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

#include "src/tint/transform/unshadow.h"

#include <memory>
#include <unordered_map>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::Unshadow);

namespace tint::transform {

/// The PIMPL state for the Unshadow transform
struct Unshadow::State {
    /// The clone context
    CloneContext& ctx;

    /// Constructor
    /// @param context the clone context
    explicit State(CloneContext& context) : ctx(context) {}

    /// Performs the transformation
    void Run() {
        auto& sem = ctx.src->Sem();

        // Maps a variable to its new name.
        std::unordered_map<const sem::Variable*, Symbol> renamed_to;

        auto rename = [&](const sem::Variable* var) -> const ast::Variable* {
            auto* decl = var->Declaration();
            auto name = ctx.src->Symbols().NameFor(decl->symbol);
            auto symbol = ctx.dst->Symbols().New(name);
            renamed_to.emplace(var, symbol);

            auto source = ctx.Clone(decl->source);
            auto* type = ctx.Clone(decl->type);
            auto* constructor = ctx.Clone(decl->constructor);
            auto attributes = ctx.Clone(decl->attributes);
            return ctx.dst->create<ast::Variable>(source, symbol, decl->declared_storage_class,
                                                  decl->declared_access, type, decl->is_const,
                                                  decl->is_overridable, constructor, attributes);
        };

        ctx.ReplaceAll([&](const ast::Variable* var) -> const ast::Variable* {
            if (auto* local = sem.Get<sem::LocalVariable>(var)) {
                if (local->Shadows()) {
                    return rename(local);
                }
            }
            if (auto* param = sem.Get<sem::Parameter>(var)) {
                if (param->Shadows()) {
                    return rename(param);
                }
            }
            return nullptr;
        });
        ctx.ReplaceAll(
            [&](const ast::IdentifierExpression* ident) -> const tint::ast::IdentifierExpression* {
                if (auto* user = sem.Get<sem::VariableUser>(ident)) {
                    auto it = renamed_to.find(user->Variable());
                    if (it != renamed_to.end()) {
                        return ctx.dst->Expr(it->second);
                    }
                }
                return nullptr;
            });
        ctx.Clone();
    }
};

Unshadow::Unshadow() = default;

Unshadow::~Unshadow() = default;

void Unshadow::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    State(ctx).Run();
}

}  // namespace tint::transform
