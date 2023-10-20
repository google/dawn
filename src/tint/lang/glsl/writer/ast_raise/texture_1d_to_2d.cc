// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/glsl/writer/ast_raise/texture_1d_to_2d.h"

#include <utility>

#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/type_expression.h"
#include "src/tint/utils/rtti/switch.h"

TINT_INSTANTIATE_TYPEINFO(tint::glsl::writer::Texture1DTo2D);

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::glsl::writer {

namespace {

bool ShouldRun(const Program& program) {
    for (auto* fn : program.AST().Functions()) {
        if (auto* sem_fn = program.Sem().Get(fn)) {
            for (auto* builtin : sem_fn->DirectlyCalledBuiltins()) {
                const auto& signature = builtin->Signature();
                auto texture = signature.Parameter(core::ParameterUsage::kTexture);
                if (texture) {
                    auto* tex = texture->Type()->As<core::type::Texture>();
                    if (tex->dim() == core::type::TextureDimension::k1d) {
                        return true;
                    }
                }
            }
        }
    }
    for (auto* var : program.AST().GlobalVariables()) {
        if (Switch(
                program.Sem().Get(var)->Type()->UnwrapRef(),
                [&](const core::type::SampledTexture* tex) {
                    return tex->dim() == core::type::TextureDimension::k1d;
                },
                [&](const core::type::StorageTexture* storage_tex) {
                    return storage_tex->dim() == core::type::TextureDimension::k1d;
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
    const Program& src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    program::CloneContext ctx = {&b, &src, /* auto_clone_symbols */ true};

    /// Constructor
    /// @param program the source program
    explicit State(const Program& program) : src(program) {}

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    ApplyResult Run() {
        auto& sem = src.Sem();

        if (!ShouldRun(src)) {
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
                [&](const core::type::SampledTexture* tex) -> const ast::Variable* {
                    if (tex->dim() == core::type::TextureDimension::k1d) {
                        auto type = ctx.dst->ty.sampled_texture(core::type::TextureDimension::k2d,
                                                                CreateASTTypeFor(ctx, tex->type()));
                        return create_var(v, type);
                    } else {
                        return nullptr;
                    }
                },
                [&](const core::type::StorageTexture* storage_tex) -> const ast::Variable* {
                    if (storage_tex->dim() == core::type::TextureDimension::k1d) {
                        auto type = ctx.dst->ty.storage_texture(core::type::TextureDimension::k2d,
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
            auto* builtin = call->Target()->As<sem::BuiltinFn>();
            if (!builtin) {
                return nullptr;
            }
            const auto& signature = builtin->Signature();
            auto* texture = signature.Parameter(core::ParameterUsage::kTexture);
            if (!texture) {
                return nullptr;
            }
            auto* tex = texture->Type()->As<core::type::Texture>();
            if (tex->dim() != core::type::TextureDimension::k1d) {
                return nullptr;
            }

            if (builtin->Fn() == wgsl::BuiltinFn::kTextureDimensions) {
                auto* new_call = ctx.CloneWithoutTransform(c);
                return ctx.dst->MemberAccessor(new_call, "x");
            }

            auto coords_index = signature.IndexOf(core::ParameterUsage::kCoords);
            if (coords_index == -1) {
                return nullptr;
            }

            tint::Vector<const ast::Expression*, 8> args;
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
        return resolver::Resolve(b);
    }
};

Texture1DTo2D::Texture1DTo2D() = default;

Texture1DTo2D::~Texture1DTo2D() = default;

ast::transform::Transform::ApplyResult Texture1DTo2D::Apply(const Program& src,
                                                            const ast::transform::DataMap&,
                                                            ast::transform::DataMap&) const {
    return State(src).Run();
}

}  // namespace tint::glsl::writer
