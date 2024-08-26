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

#include "src/tint/lang/glsl/writer/ast_raise/combine_samplers.h"

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/lang/glsl/writer/common/options.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/statement.h"

#include "src/tint/utils/containers/map.h"

TINT_INSTANTIATE_TYPEINFO(tint::glsl::writer::CombineSamplers);

namespace {

bool IsGlobal(const tint::sem::VariablePair& pair) {
    return tint::Is<tint::sem::GlobalVariable>(pair.first) &&
           (!pair.second || tint::Is<tint::sem::GlobalVariable>(pair.second));
}

}  // namespace

namespace tint::glsl::writer {

using namespace tint::core::number_suffixes;  // NOLINT
                                              //
/// PIMPL state for the transform
struct CombineSamplers::State {
    /// The source program
    const Program& src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    program::CloneContext ctx = {&b, &src, /* auto_clone_symbols */ true};

    /// The binding information
    const Bindings* bindings;

    /// Map from a texture/sampler pair to the corresponding combined sampler
    /// variable
    using CombinedTextureSamplerMap = std::unordered_map<sem::VariablePair, const ast::Variable*>;

    /// A map of all global texture/sampler variable pairs to the global
    /// combined sampler variable that will replace it.
    CombinedTextureSamplerMap global_combined_texture_samplers_;

    /// A map of all texture/sampler variable pairs that contain a function
    /// parameter to the combined sampler function paramter that will replace it.
    std::unordered_map<const sem::Function*, CombinedTextureSamplerMap>
        function_combined_texture_samplers_;

    /// Placeholder global samplers used when a function contains texture-only
    /// references (one comparison sampler, one regular). These are also used as
    /// temporary sampler parameters to the texture builtins to satisfy the WGSL
    /// resolver, but are then ignored and removed by the GLSL writer.
    const ast::Variable* placeholder_samplers_[2] = {};

    /// Group and binding attributes used by all combined sampler globals.
    /// Group 0 and binding 0 are used, with collisions disabled.
    /// @returns the newly-created attribute list
    auto Attributes() const {
        Vector<const ast::Attribute*, 3> attributes{ctx.dst->Group(0_a), ctx.dst->Binding(0_a)};
        attributes.Push(ctx.dst->Disable(ast::DisabledValidation::kBindingPointCollision));
        return attributes;
    }

    /// Constructor
    /// @param program the source program
    /// @param info the binding map information
    State(const Program& program, const Bindings* info) : src(program), bindings(info) {}

    /// Creates a combined sampler global variables.
    /// (Note this is actually a Texture node at the AST level, but it will be
    /// written as the corresponding sampler (eg., sampler2D) on GLSL output.)
    /// @param texture_var the texture (global) variable (must be non-null)
    /// @param sampler_var the sampler (global) variable
    /// @param name the default name to use (may be overridden by map lookup)
    /// @returns the newly-created global variable
    const ast::Variable* CreateCombinedGlobal(const sem::Variable* texture_var,
                                              const sem::Variable* sampler_var,
                                              std::string name) {
        binding::CombinedTextureSamplerPair st_pair;

        TINT_ASSERT(texture_var);
        st_pair.texture = *texture_var->As<sem::GlobalVariable>()->Attributes().binding_point;

        if (sampler_var) {
            st_pair.sampler = *sampler_var->As<sem::GlobalVariable>()->Attributes().binding_point;
        } else {
            st_pair.sampler = bindings->placeholder_sampler_bind_point;
        }

        auto it = bindings->sampler_texture_to_name.find(st_pair);
        if (it != bindings->sampler_texture_to_name.end()) {
            name = it->second;
        }

        ast::Type type = CreateCombinedASTTypeFor(texture_var, sampler_var);
        Symbol symbol = ctx.dst->Symbols().New(name);
        return ctx.dst->GlobalVar(symbol, type, Attributes());
    }

    /// Creates placeholder global sampler variables.
    /// @param kind the sampler kind to create for
    /// @returns the newly-created global variable
    const ast::Variable* CreatePlaceholder(core::type::SamplerKind kind) {
        ast::Type type = ctx.dst->ty.sampler(kind);
        const char* name = kind == core::type::SamplerKind::kComparisonSampler
                               ? "placeholder_comparison_sampler"
                               : "placeholder_sampler";
        Symbol symbol = ctx.dst->Symbols().New(name);
        return ctx.dst->GlobalVar(symbol, type, Attributes());
    }

    /// Creates Identifier for a given texture and sampler variable pair.
    /// Depth textures with no samplers are turned into the corresponding
    /// f32 texture (e.g., texture_depth_2d -> texture_2d<f32>).
    /// @param texture the texture variable of interest
    /// @param sampler the texture variable of interest
    /// @returns the newly-created type
    ast::Type CreateCombinedASTTypeFor(const sem::Variable* texture, const sem::Variable* sampler) {
        const core::type::Type* texture_type = texture->Type()->UnwrapRef();
        const core::type::DepthTexture* depth = texture_type->As<core::type::DepthTexture>();
        if (depth && !sampler) {
            return ctx.dst->ty.sampled_texture(depth->Dim(), ctx.dst->ty.f32());
        }

        return CreateASTTypeFor(ctx, texture_type);
    }

    /// Insert a new texture/sampler pair into the combined samplers maps (global or local, as
    /// appropriate). If local, also add a function parameter to "params".
    /// @param pair the texture/sampler pair to insert
    /// @param fn the function scope in which to insert (if local)
    /// @param params the calling function's parameter list to modify (if local)
    void InsertPair(sem::VariablePair pair,
                    const sem::Function* fn,
                    tint::Vector<const ast::Parameter*, 8>* params) {
        const sem::Variable* texture_var = pair.first;
        const sem::Variable* sampler_var = pair.second;
        std::string name = texture_var->Declaration()->name->symbol.Name();
        if (sampler_var) {
            name += "_" + sampler_var->Declaration()->name->symbol.Name();
        }
        if (IsGlobal(pair)) {
            // Both texture and sampler are global; add a new global variable
            // to represent the combined sampler (if not already created).
            GetOrAdd(global_combined_texture_samplers_, pair,
                     [&] { return CreateCombinedGlobal(texture_var, sampler_var, name); });
        } else {
            // Either texture or sampler (or both) is a function parameter;
            // add a new function parameter to represent the combined sampler.
            ast::Type type = CreateCombinedASTTypeFor(texture_var, sampler_var);
            auto* var = ctx.dst->Param(ctx.dst->Symbols().New(name), type);
            params->Push(var);
            function_combined_texture_samplers_[fn][pair] = var;
        }
    }

    /// For a given texture, find any texture/sampler pair with a non-null sampler in the given
    /// function scope.
    /// @param texture_var the texture variable of interest
    /// @param fn the function scope in which to search
    /// @returns the full pair, if found
    const sem::VariablePair* FindFullTextureSamplerPair(const sem::Variable* texture_var,
                                                        const sem::Function* fn) {
        for (auto pairIter = fn->TextureSamplerPairs().begin();
             pairIter != fn->TextureSamplerPairs().end(); pairIter++) {
            if (pairIter->first == texture_var && pairIter->second) {
                return pairIter;
            }
        }
        return nullptr;
    }

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    ApplyResult Run() {
        auto& sem = ctx.src->Sem();

        // Remove all texture and sampler global variables. These will be replaced
        // by combined samplers.
        for (auto* global : ctx.src->AST().GlobalVariables()) {
            auto* global_sem = sem.Get(global)->As<sem::GlobalVariable>();
            auto* type = ctx.src->TypeOf(global->type);
            if (tint::IsAnyOf<core::type::Texture, core::type::Sampler>(type) &&
                !type->Is<core::type::StorageTexture>()) {
                ctx.Remove(ctx.src->AST().GlobalDeclarations(), global);
            } else if (auto binding_point = global_sem->Attributes().binding_point) {
                if (binding_point->group == 0 && binding_point->binding == 0) {
                    auto* attribute =
                        ctx.dst->Disable(ast::DisabledValidation::kBindingPointCollision);
                    ctx.InsertFront(global->attributes, attribute);
                }
            }
        }

        // Rewrite all function signatures to use combined samplers, and remove
        // separate textures & samplers. Create new combined globals where found.
        ctx.ReplaceAll([&](const ast::Function* ast_fn) -> const ast::Function* {
            if (auto* fn = sem.Get(ast_fn)) {
                Vector<const ast::Parameter*, 8> params;
                for (auto pair : fn->TextureSamplerPairs()) {
                    if (!pair.second) {
                        continue;
                    }
                    InsertPair(pair, fn, &params);
                }
                for (auto pair : fn->TextureSamplerPairs()) {
                    if (pair.second) {
                        continue;
                    }
                    // Look for another pair with a non-null sampler.
                    // NOTE: this is O(N^2) in the number of pairs, since
                    // FindFullTextureSamplerPair() also loops over all pairs. If this proves
                    // problematic, it could be optimized.
                    if (const sem::VariablePair* fullPair =
                            FindFullTextureSamplerPair(pair.first, fn)) {
                        if (IsGlobal(pair)) {
                            global_combined_texture_samplers_[pair] =
                                global_combined_texture_samplers_[*fullPair];
                        } else {
                            auto* var = function_combined_texture_samplers_[fn][*fullPair];
                            function_combined_texture_samplers_[fn][pair] = var;
                        }
                    } else {
                        InsertPair(pair, fn, &params);
                    }
                }
                // Filter out separate textures and samplers from the original
                // function signature.
                for (auto* param : fn->Parameters()) {
                    if (!param->Type()->IsAnyOf<core::type::Texture, core::type::Sampler>()) {
                        params.Push(ctx.Clone(param->Declaration()));
                    }
                }
                // Create a new function signature that differs only in the parameter
                // list.
                auto name = ctx.Clone(ast_fn->name);
                auto return_type = ctx.Clone(ast_fn->return_type);
                auto* body = ctx.Clone(ast_fn->body);
                auto attributes = ctx.Clone(ast_fn->attributes);
                auto return_type_attributes = ctx.Clone(ast_fn->return_type_attributes);
                return ctx.dst->create<ast::Function>(name, params, return_type, body,
                                                      std::move(attributes),
                                                      std::move(return_type_attributes));
            }
            return nullptr;
        });

        // Replace all function call expressions containing texture or
        // sampler parameters to use the current function's combined samplers or
        // the combined global samplers, as appropriate.
        ctx.ReplaceAll([&](const ast::CallExpression* expr) -> const ast::Expression* {
            if (auto* call = sem.Get(expr)->UnwrapMaterialize()->As<sem::Call>()) {
                Vector<const ast::Expression*, 8> args;
                // Replace all texture builtin calls.
                if (auto* builtin = call->Target()->As<sem::BuiltinFn>()) {
                    const auto& signature = builtin->Signature();
                    auto sampler_index = signature.IndexOf(core::ParameterUsage::kSampler);
                    auto texture_index = signature.IndexOf(core::ParameterUsage::kTexture);
                    if (texture_index == -1) {
                        return nullptr;
                    }

                    const sem::ValueExpression* texture =
                        call->Arguments()[static_cast<size_t>(texture_index)];
                    // We don't want to combine storage textures with anything, since
                    // they never have associated samplers in GLSL.
                    if (texture->Type()->UnwrapRef()->Is<core::type::StorageTexture>()) {
                        return nullptr;
                    }

                    const sem::ValueExpression* sampler =
                        sampler_index != -1 ? call->Arguments()[static_cast<size_t>(sampler_index)]
                                            : nullptr;
                    auto* texture_var = texture->UnwrapLoad()->As<sem::VariableUser>()->Variable();
                    auto* sampler_var =
                        sampler ? sampler->UnwrapLoad()->As<sem::VariableUser>()->Variable()
                                : nullptr;
                    sem::VariablePair new_pair(texture_var, sampler_var);
                    for (auto* arg : expr->args) {
                        auto* type = ctx.src->TypeOf(arg)->UnwrapRef();
                        if (type->Is<core::type::Texture>()) {
                            const ast::Variable* var =
                                IsGlobal(new_pair)
                                    ? global_combined_texture_samplers_[new_pair]
                                    : function_combined_texture_samplers_[call->Stmt()->Function()]
                                                                         [new_pair];
                            args.Push(ctx.dst->Expr(var->name->symbol));
                        } else if (auto* sampler_type = type->As<core::type::Sampler>()) {
                            core::type::SamplerKind kind = sampler_type->Kind();
                            int index = (kind == core::type::SamplerKind::kSampler) ? 0 : 1;
                            const ast::Variable*& p = placeholder_samplers_[index];
                            if (!p) {
                                p = CreatePlaceholder(kind);
                            }
                            args.Push(ctx.dst->Expr(p->name->symbol));
                        } else {
                            args.Push(ctx.Clone(arg));
                        }
                    }

                    const ast::Expression* value = ctx.dst->Call(ctx.Clone(expr->target), args);
                    if (builtin->Fn() == wgsl::BuiltinFn::kTextureLoad &&
                        texture_var->Type()->UnwrapRef()->Is<core::type::DepthTexture>() &&
                        !call->Stmt()->Declaration()->Is<ast::CallStatement>()) {
                        value = ctx.dst->MemberAccessor(value, "x");
                    }
                    return value;
                }

                // Replace all function calls.
                if (auto* callee = call->Target()->As<sem::Function>()) {
                    auto make_arg = [&](const sem::Variable* texture_var,
                                        const sem::Variable* sampler_var) {
                        if (auto* param = tint::As<sem::Parameter>(texture_var)) {
                            const sem::ValueExpression* texture = call->Arguments()[param->Index()];
                            texture_var =
                                texture->UnwrapLoad()->As<sem::VariableUser>()->Variable();
                        }
                        if (auto* param = tint::As<sem::Parameter>(sampler_var)) {
                            const sem::ValueExpression* sampler = call->Arguments()[param->Index()];
                            sampler_var =
                                sampler->UnwrapLoad()->As<sem::VariableUser>()->Variable();
                        }
                        sem::VariablePair new_pair(texture_var, sampler_var);
                        // If both texture and sampler are (now) global, pass that
                        // global variable to the callee. Otherwise use the caller's
                        // function parameter for this pair.
                        const ast::Variable* var =
                            IsGlobal(new_pair)
                                ? global_combined_texture_samplers_[new_pair]
                                : function_combined_texture_samplers_[call->Stmt()->Function()]
                                                                     [new_pair];
                        return ctx.dst->Expr(var->name->symbol);
                    };

                    for (auto pair : callee->TextureSamplerPairs()) {
                        if (!pair.second) {
                            continue;
                        }
                        // Global pairs used by the callee do not require a function
                        // parameter at the call site.
                        if (IsGlobal(pair)) {
                            continue;
                        }

                        args.Push(make_arg(pair.first, pair.second));
                    }
                    for (auto pair : callee->TextureSamplerPairs()) {
                        if (pair.second) {
                            continue;
                        }
                        // Global pairs used by the callee do not require a function
                        // parameter at the call site.
                        if (IsGlobal(pair)) {
                            continue;
                        }
                        // Texture-only pairs do not require a function parameter if they've been
                        // replaced by a real pair.
                        if (FindFullTextureSamplerPair(pair.first, callee)) {
                            continue;
                        }

                        args.Push(make_arg(pair.first, nullptr));
                    }
                    // Append all of the remaining non-texture and non-sampler
                    // parameters.
                    for (auto* arg : expr->args) {
                        if (!ctx.src->TypeOf(arg)
                                 ->UnwrapRef()
                                 ->IsAnyOf<core::type::Texture, core::type::Sampler>()) {
                            args.Push(ctx.Clone(arg));
                        }
                    }
                    return ctx.dst->Call(ctx.Clone(expr->target), args);
                }
            }
            return nullptr;
        });

        ctx.Clone();
        return resolver::Resolve(b);
    }
};

CombineSamplers::CombineSamplers() = default;

CombineSamplers::~CombineSamplers() = default;

ast::transform::Transform::ApplyResult CombineSamplers::Apply(const Program& src,
                                                              const ast::transform::DataMap& inputs,
                                                              ast::transform::DataMap&) const {
    auto* info = inputs.Get<Bindings>();
    if (!info) {
        ProgramBuilder b;
        b.Diagnostics().AddError(Source{}) << "missing transform data for " << TypeInfo().name;
        return resolver::Resolve(b);
    }

    return State(src, info).Run();
}

}  // namespace tint::glsl::writer
