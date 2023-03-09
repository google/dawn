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

#include "src/tint/transform/texture_1d_to_2d.h"

#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/type_expression.h"
#include "src/tint/switch.h"
#include "src/tint/type/texture_dimension.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::Texture1DTo2D);

using namespace tint::number_suffixes;  // NOLINT

namespace tint::transform {

namespace {

bool ShouldRun(const Program* program) {
    for (auto* fn : program->AST().Functions()) {
        if (auto* sem_fn = program->Sem().Get(fn)) {
            for (auto* builtin : sem_fn->DirectlyCalledBuiltins()) {
                const auto& signature = builtin->Signature();
                auto texture = signature.Parameter(sem::ParameterUsage::kTexture);
                if (texture) {
                    auto* tex = texture->Type()->As<type::Texture>();
                    if (tex->dim() == type::TextureDimension::k1d) {
                        return true;
                    }
                }
            }
        }
    }
    for (auto* var : program->AST().GlobalVariables()) {
        if (Switch(
                program->Sem().Get(var)->Type()->UnwrapRef(),
                [&](const type::SampledTexture* tex) {
                    return tex->dim() == type::TextureDimension::k1d;
                },
                [&](const type::StorageTexture* storage_tex) {
                    return storage_tex->dim() == type::TextureDimension::k1d;
                })) {
            return true;
        }
    }
    return false;
}

}  // namespace

/// PIMPL state for the transform
struct Texture1DTo2D::State {
    /// The source program
    const Program* const src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    CloneContext ctx = {&b, src, /* auto_clone_symbols */ true};

    /// Constructor
    /// @param program the source program
    explicit State(const Program* program) : src(program) {}

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    ApplyResult Run() {
        auto& sem = src->Sem();

        if (!ShouldRun(ctx.src)) {
            return SkipTransform;
        }

        auto create_var = [&](const ast::Variable* v, ast::Type type) -> const ast::Variable* {
            if (v->As<ast::Parameter>()) {
                return ctx.dst->Param(ctx.Clone(v->name->symbol), type, ctx.Clone(v->attributes));
            } else {
                return ctx.dst->Var(ctx.Clone(v->name->symbol), type, ctx.Clone(v->attributes));
            }
        };

        ctx.ReplaceAll([&](const ast::Variable* v) -> const ast::Variable* {
            const ast::Variable* r = Switch(
                sem.Get(v)->Type()->UnwrapRef(),
                [&](const type::SampledTexture* tex) -> const ast::Variable* {
                    if (tex->dim() == type::TextureDimension::k1d) {
                        auto type = ctx.dst->ty.sampled_texture(type::TextureDimension::k2d,
                                                                CreateASTTypeFor(ctx, tex->type()));
                        return create_var(v, type);
                    } else {
                        return nullptr;
                    }
                },
                [&](const type::StorageTexture* storage_tex) -> const ast::Variable* {
                    if (storage_tex->dim() == type::TextureDimension::k1d) {
                        auto type = ctx.dst->ty.storage_texture(type::TextureDimension::k2d,
                                                                storage_tex->texel_format(),
                                                                storage_tex->access());
                        return create_var(v, type);
                    } else {
                        return nullptr;
                    }
                },
                [](Default) { return nullptr; });
            return r;
        });

        ctx.ReplaceAll([&](const ast::CallExpression* c) -> const ast::Expression* {
            auto* call = sem.Get(c)->UnwrapMaterialize()->As<sem::Call>();
            if (!call) {
                return nullptr;
            }
            auto* builtin = call->Target()->As<sem::Builtin>();
            if (!builtin) {
                return nullptr;
            }
            const auto& signature = builtin->Signature();
            auto* texture = signature.Parameter(sem::ParameterUsage::kTexture);
            if (!texture) {
                return nullptr;
            }
            auto* tex = texture->Type()->As<type::Texture>();
            if (tex->dim() != type::TextureDimension::k1d) {
                return nullptr;
            }

            if (builtin->Type() == builtin::Function::kTextureDimensions) {
                // If this textureDimensions() call is in a CallStatement, we can leave it
                // unmodified since the return value will be dropped on the floor anyway.
                if (call->Stmt()->Declaration()->Is<ast::CallStatement>()) {
                    return nullptr;
                }
                auto* new_call = ctx.CloneWithoutTransform(c);
                return ctx.dst->MemberAccessor(new_call, "x");
            }

            auto coords_index = signature.IndexOf(sem::ParameterUsage::kCoords);
            if (coords_index == -1) {
                return nullptr;
            }

            utils::Vector<const ast::Expression*, 8> args;
            int index = 0;
            for (auto* arg : c->args) {
                if (index == coords_index) {
                    auto* ctype = call->Arguments()[static_cast<size_t>(coords_index)]->Type();
                    auto* coords = c->args[static_cast<size_t>(coords_index)];

                    const ast::LiteralExpression* half = nullptr;
                    if (ctype->is_integer_scalar()) {
                        half = ctx.dst->Expr(0_a);
                    } else {
                        half = ctx.dst->Expr(0.5_a);
                    }
                    args.Push(
                        ctx.dst->vec(CreateASTTypeFor(ctx, ctype), 2u, ctx.Clone(coords), half));
                } else {
                    args.Push(ctx.Clone(arg));
                }
                index++;
            }
            return ctx.dst->Call(ctx.Clone(c->target), args);
        });

        ctx.Clone();
        return Program(std::move(b));
    }
};

Texture1DTo2D::Texture1DTo2D() = default;

Texture1DTo2D::~Texture1DTo2D() = default;

Transform::ApplyResult Texture1DTo2D::Apply(const Program* src, const DataMap&, DataMap&) const {
    return State(src).Run();
}

}  // namespace tint::transform
