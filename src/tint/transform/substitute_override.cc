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

#include "src/tint/transform/substitute_override.h"

#include <functional>

#include "src/tint/program_builder.h"
#include "src/tint/sem/builtin.h"
#include "src/tint/sem/index_accessor_expression.h"
#include "src/tint/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::SubstituteOverride);
TINT_INSTANTIATE_TYPEINFO(tint::transform::SubstituteOverride::Config);

namespace tint::transform {

SubstituteOverride::SubstituteOverride() = default;

SubstituteOverride::~SubstituteOverride() = default;

bool SubstituteOverride::ShouldRun(const Program* program, const DataMap&) const {
    for (auto* node : program->AST().GlobalVariables()) {
        if (node->Is<ast::Override>()) {
            return true;
        }
    }
    return false;
}

void SubstituteOverride::Run(CloneContext& ctx, const DataMap& config, DataMap&) const {
    const auto* data = config.Get<Config>();
    if (!data) {
        ctx.dst->Diagnostics().add_error(diag::System::Transform,
                                         "Missing override substitution data");
        return;
    }

    ctx.ReplaceAll([&](const ast::Override* w) -> const ast::Const* {
        auto* sem = ctx.src->Sem().Get(w);

        auto src = ctx.Clone(w->source);
        auto sym = ctx.Clone(w->symbol);
        auto* ty = ctx.Clone(w->type);

        // No replacement provided, just clone the override node as a const.
        auto iter = data->map.find(sem->OverrideId());
        if (iter == data->map.end()) {
            if (!w->constructor) {
                ctx.dst->Diagnostics().add_error(
                    diag::System::Transform,
                    "Initializer not provided for override, and override not overridden.");
                return nullptr;
            }
            return ctx.dst->Const(src, sym, ty, ctx.Clone(w->constructor));
        }

        auto value = iter->second;
        auto* ctor = Switch(
            sem->Type(),
            [&](const sem::Bool*) { return ctx.dst->Expr(!std::equal_to<double>()(value, 0.0)); },
            [&](const sem::I32*) { return ctx.dst->Expr(i32(value)); },
            [&](const sem::U32*) { return ctx.dst->Expr(u32(value)); },
            [&](const sem::F32*) { return ctx.dst->Expr(f32(value)); },
            [&](const sem::F16*) { return ctx.dst->Expr(f16(value)); });

        if (!ctor) {
            ctx.dst->Diagnostics().add_error(diag::System::Transform,
                                             "Failed to create override-expression");
            return nullptr;
        }

        return ctx.dst->Const(src, sym, ty, ctor);
    });

    // Ensure that objects that are indexed with an override-expression are materialized.
    // If the object is not materialized, and the 'override' variable is turned to a 'const', the
    // resulting type of the index may change. See: crbug.com/tint/1697.
    ctx.ReplaceAll(
        [&](const ast::IndexAccessorExpression* expr) -> const ast::IndexAccessorExpression* {
            if (auto* sem = ctx.src->Sem().Get(expr)) {
                if (auto* access = sem->UnwrapMaterialize()->As<sem::IndexAccessorExpression>()) {
                    if (access->Object()->UnwrapMaterialize()->Type()->HoldsAbstract() &&
                        access->Index()->Stage() == sem::EvaluationStage::kOverride) {
                        auto& b = *ctx.dst;
                        auto* obj = b.Call(sem::str(sem::BuiltinType::kTintMaterialize),
                                           ctx.Clone(expr->object));
                        return b.IndexAccessor(obj, ctx.Clone(expr->index));
                    }
                }
            }
            return nullptr;
        });

    ctx.Clone();
}

SubstituteOverride::Config::Config() = default;

SubstituteOverride::Config::Config(const Config&) = default;

SubstituteOverride::Config::~Config() = default;

SubstituteOverride::Config& SubstituteOverride::Config::operator=(const Config&) = default;

}  // namespace tint::transform
