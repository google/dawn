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

#include "src/tint/lang/wgsl/ast/transform/substitute_override.h"

#include <functional>
#include <utility>

#include "src/tint/lang/core/function.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/builtin.h"
#include "src/tint/lang/wgsl/sem/index_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/rtti/switch.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::SubstituteOverride);
TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::SubstituteOverride::Config);

namespace tint::ast::transform {
namespace {

bool ShouldRun(const Program* program) {
    for (auto* node : program->AST().GlobalVariables()) {
        if (node->Is<Override>()) {
            return true;
        }
    }
    return false;
}

}  // namespace

SubstituteOverride::SubstituteOverride() = default;

SubstituteOverride::~SubstituteOverride() = default;

Transform::ApplyResult SubstituteOverride::Apply(const Program* src,
                                                 const DataMap& config,
                                                 DataMap&) const {
    ProgramBuilder b;
    program::CloneContext ctx{&b, src, /* auto_clone_symbols */ true};

    const auto* data = config.Get<Config>();
    if (!data) {
        b.Diagnostics().add_error(diag::System::Transform, "Missing override substitution data");
        return resolver::Resolve(b);
    }

    if (!ShouldRun(ctx.src)) {
        return SkipTransform;
    }

    ctx.ReplaceAll([&](const Override* w) -> const Const* {
        auto* sem = ctx.src->Sem().Get(w);

        auto source = ctx.Clone(w->source);
        auto sym = ctx.Clone(w->name->symbol);
        Type ty = w->type ? ctx.Clone(w->type) : Type{};

        // No replacement provided, just clone the override node as a const.
        auto iter = data->map.find(sem->OverrideId());
        if (iter == data->map.end()) {
            if (!w->initializer) {
                b.Diagnostics().add_error(
                    diag::System::Transform,
                    "Initializer not provided for override, and override not overridden.");
                return nullptr;
            }
            return b.Const(source, sym, ty, ctx.Clone(w->initializer));
        }

        auto value = iter->second;
        auto* ctor = Switch(
            sem->Type(),
            [&](const type::Bool*) { return b.Expr(!std::equal_to<double>()(value, 0.0)); },
            [&](const type::I32*) { return b.Expr(i32(value)); },
            [&](const type::U32*) { return b.Expr(u32(value)); },
            [&](const type::F32*) { return b.Expr(f32(value)); },
            [&](const type::F16*) { return b.Expr(f16(value)); });

        if (!ctor) {
            b.Diagnostics().add_error(diag::System::Transform,
                                      "Failed to create override-expression");
            return nullptr;
        }

        return b.Const(source, sym, ty, ctor);
    });

    // Ensure that objects that are indexed with an override-expression are materialized.
    // If the object is not materialized, and the 'override' variable is turned to a 'const', the
    // resulting type of the index may change. See: crbug.com/tint/1697.
    ctx.ReplaceAll([&](const IndexAccessorExpression* expr) -> const IndexAccessorExpression* {
        if (auto* sem = src->Sem().Get(expr)) {
            if (auto* access = sem->UnwrapMaterialize()->As<sem::IndexAccessorExpression>()) {
                if (access->Object()->UnwrapMaterialize()->Type()->HoldsAbstract() &&
                    access->Index()->Stage() == core::EvaluationStage::kOverride) {
                    auto* obj = b.Call(core::str(core::Function::kTintMaterialize),
                                       ctx.Clone(expr->object));
                    return b.IndexAccessor(obj, ctx.Clone(expr->index));
                }
            }
        }
        return nullptr;
    });

    ctx.Clone();
    return resolver::Resolve(b);
}

SubstituteOverride::Config::Config() = default;

SubstituteOverride::Config::Config(const Config&) = default;

SubstituteOverride::Config::~Config() = default;

SubstituteOverride::Config& SubstituteOverride::Config::operator=(const Config&) = default;

}  // namespace tint::ast::transform
