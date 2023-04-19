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

#include "src/tint/transform/combine_samplers.h"

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/program_builder.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/statement.h"

#include "src/tint/utils/map.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::CombineSamplers);
TINT_INSTANTIATE_TYPEINFO(tint::transform::CombineSamplers::BindingInfo);

namespace {

bool IsGlobal(const tint::sem::VariablePair& pair) {
    return pair.first->Is<tint::sem::GlobalVariable>() &&
           (!pair.second || pair.second->Is<tint::sem::GlobalVariable>());
}

}  // namespace

namespace tint::transform {

using namespace tint::number_suffixes;  // NOLINT

CombineSamplers::BindingInfo::BindingInfo(const BindingMap& map,
                                          const sem::BindingPoint& placeholder)
    : binding_map(map), placeholder_binding_point(placeholder) {}
CombineSamplers::BindingInfo::BindingInfo(const BindingInfo& other) = default;
CombineSamplers::BindingInfo::~BindingInfo() = default;

/// PIMPL state for the transform
struct CombineSamplers::State {
    /// The source program
    const Program* const src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    CloneContext ctx = {&b, src, /* auto_clone_symbols */ true};

    /// The binding info
    const BindingInfo* binding_info;

    /// Map from a texture/sampler pair to the corresponding combined sampler
    /// variable
    using CombinedTextureSamplerMap = std::unordered_map<sem::VariablePair, const ast::Variable*>;

    /// Use sem::BindingPoint without scope.
    using BindingPoint = sem::BindingPoint;

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
        utils::Vector<const ast::Attribute*, 3> attributes{ctx.dst->Group(0_a),
                                                           ctx.dst->Binding(0_a)};
        attributes.Push(ctx.dst->Disable(ast::DisabledValidation::kBindingPointCollision));
        return attributes;
    }

    /// Constructor
    /// @param program the source program
    /// @param info the binding map information
    State(const Program* program, const BindingInfo* info) : src(program), binding_info(info) {}

    /// Creates a combined sampler global variables.
    /// (Note this is actually a Texture node at the AST level, but it will be
    /// written as the corresponding sampler (eg., sampler2D) on GLSL output.)
    /// @param texture_var the texture (global) variable
    /// @param sampler_var the sampler (global) variable
    /// @param name the default name to use (may be overridden by map lookup)
    /// @returns the newly-created global variable
    const ast::Variable* CreateCombinedGlobal(const sem::Variable* texture_var,
                                              const sem::Variable* sampler_var,
                                              std::string name) {
        SamplerTexturePair bp_pair;
        bp_pair.texture_binding_point = *texture_var->As<sem::GlobalVariable>()->BindingPoint();
        bp_pair.sampler_binding_point =
            sampler_var ? *sampler_var->As<sem::GlobalVariable>()->BindingPoint()
                        : binding_info->placeholder_binding_point;
        auto it = binding_info->binding_map.find(bp_pair);
        if (it != binding_info->binding_map.end()) {
            name = it->second;
        }
        ast::Type type = CreateCombinedASTTypeFor(texture_var, sampler_var);
        Symbol symbol = ctx.dst->Symbols().New(name);
        return ctx.dst->GlobalVar(symbol, type, Attributes());
    }

    /// Creates placeholder global sampler variables.
    /// @param kind the sampler kind to create for
    /// @returns the newly-created global variable
    const ast::Variable* CreatePlaceholder(type::SamplerKind kind) {
        ast::Type type = ctx.dst->ty.sampler(kind);
        const char* name = kind == type::SamplerKind::kComparisonSampler
                               ? "placeholder_comparison_sampler"
                               : "placeholder_sampler";
        Symbol symbol = ctx.dst->Symbols().New(name);
        return ctx.dst->GlobalVar(symbol, type, Attributes());
    }

    /// Creates ast::Identifier for a given texture and sampler variable pair.
    /// Depth textures with no samplers are turned into the corresponding
    /// f32 texture (e.g., texture_depth_2d -> texture_2d<f32>).
    /// @param texture the texture variable of interest
    /// @param sampler the texture variable of interest
    /// @returns the newly-created type
    ast::Type CreateCombinedASTTypeFor(const sem::Variable* texture, const sem::Variable* sampler) {
        const type::Type* texture_type = texture->Type()->UnwrapRef();
        const type::DepthTexture* depth = texture_type->As<type::DepthTexture>();
        if (depth && !sampler) {
            return ctx.dst->ty.sampled_texture(depth->dim(), ctx.dst->ty.f32());
        } else {
            return CreateASTTypeFor(ctx, texture_type);
        }
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
            if (tint::utils::IsAnyOf<type::Texture, type::Sampler>(type) &&
                !type->Is<type::StorageTexture>()) {
                ctx.Remove(ctx.src->AST().GlobalDeclarations(), global);
            } else if (auto binding_point = global_sem->BindingPoint()) {
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
                auto pairs = fn->TextureSamplerPairs();
                if (pairs.IsEmpty()) {
                    return nullptr;
                }
                utils::Vector<const ast::Parameter*, 8> params;
                for (auto pair : fn->TextureSamplerPairs()) {
                    const sem::Variable* texture_var = pair.first;
                    const sem::Variable* sampler_var = pair.second;
                    std::string name = texture_var->Declaration()->name->symbol.Name();
                    if (sampler_var) {
                        name += "_" + sampler_var->Declaration()->name->symbol.Name();
                    }
                    if (IsGlobal(pair)) {
                        // Both texture and sampler are global; add a new global variable
                        // to represent the combined sampler (if not already created).
                        utils::GetOrCreate(global_combined_texture_samplers_, pair, [&] {
                            return CreateCombinedGlobal(texture_var, sampler_var, name);
                        });
                    } else {
                        // Either texture or sampler (or both) is a function parameter;
                        // add a new function parameter to represent the combined sampler.
                        ast::Type type = CreateCombinedASTTypeFor(texture_var, sampler_var);
                        auto* var = ctx.dst->Param(ctx.dst->Symbols().New(name), type);
                        params.Push(var);
                        function_combined_texture_samplers_[fn][pair] = var;
                    }
                }
                // Filter out separate textures and samplers from the original
                // function signature.
                for (auto* param : fn->Parameters()) {
                    if (!param->Type()->IsAnyOf<type::Texture, type::Sampler>()) {
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
                utils::Vector<const ast::Expression*, 8> args;
                // Replace all texture builtin calls.
                if (auto* builtin = call->Target()->As<sem::Builtin>()) {
                    const auto& signature = builtin->Signature();
                    auto sampler_index = signature.IndexOf(sem::ParameterUsage::kSampler);
                    auto texture_index = signature.IndexOf(sem::ParameterUsage::kTexture);
                    if (texture_index == -1) {
                        return nullptr;
                    }
                    const sem::ValueExpression* texture =
                        call->Arguments()[static_cast<size_t>(texture_index)];
                    // We don't want to combine storage textures with anything, since
                    // they never have associated samplers in GLSL.
                    if (texture->Type()->UnwrapRef()->Is<type::StorageTexture>()) {
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
                        if (type->Is<type::Texture>()) {
                            const ast::Variable* var =
                                IsGlobal(new_pair)
                                    ? global_combined_texture_samplers_[new_pair]
                                    : function_combined_texture_samplers_[call->Stmt()->Function()]
                                                                         [new_pair];
                            args.Push(ctx.dst->Expr(var->name->symbol));
                        } else if (auto* sampler_type = type->As<type::Sampler>()) {
                            type::SamplerKind kind = sampler_type->kind();
                            int index = (kind == type::SamplerKind::kSampler) ? 0 : 1;
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
                    if (builtin->Type() == builtin::Function::kTextureLoad &&
                        texture_var->Type()->UnwrapRef()->Is<type::DepthTexture>() &&
                        !call->Stmt()->Declaration()->Is<ast::CallStatement>()) {
                        value = ctx.dst->MemberAccessor(value, "x");
                    }
                    return value;
                }
                // Replace all function calls.
                if (auto* callee = call->Target()->As<sem::Function>()) {
                    for (auto pair : callee->TextureSamplerPairs()) {
                        // Global pairs used by the callee do not require a function
                        // parameter at the call site.
                        if (IsGlobal(pair)) {
                            continue;
                        }
                        const sem::Variable* texture_var = pair.first;
                        const sem::Variable* sampler_var = pair.second;
                        if (auto* param = texture_var->As<sem::Parameter>()) {
                            const sem::ValueExpression* texture = call->Arguments()[param->Index()];
                            texture_var =
                                texture->UnwrapLoad()->As<sem::VariableUser>()->Variable();
                        }
                        if (sampler_var) {
                            if (auto* param = sampler_var->As<sem::Parameter>()) {
                                const sem::ValueExpression* sampler =
                                    call->Arguments()[param->Index()];
                                sampler_var =
                                    sampler->UnwrapLoad()->As<sem::VariableUser>()->Variable();
                            }
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
                        auto* arg = ctx.dst->Expr(var->name->symbol);
                        args.Push(arg);
                    }
                    // Append all of the remaining non-texture and non-sampler
                    // parameters.
                    for (auto* arg : expr->args) {
                        if (!ctx.src->TypeOf(arg)
                                 ->UnwrapRef()
                                 ->IsAnyOf<type::Texture, type::Sampler>()) {
                            args.Push(ctx.Clone(arg));
                        }
                    }
                    return ctx.dst->Call(ctx.Clone(expr->target), args);
                }
            }
            return nullptr;
        });

        ctx.Clone();
        return Program(std::move(b));
    }
};

CombineSamplers::CombineSamplers() = default;

CombineSamplers::~CombineSamplers() = default;

Transform::ApplyResult CombineSamplers::Apply(const Program* src,
                                              const DataMap& inputs,
                                              DataMap&) const {
    auto* binding_info = inputs.Get<BindingInfo>();
    if (!binding_info) {
        ProgramBuilder b;
        b.Diagnostics().add_error(diag::System::Transform,
                                  "missing transform data for " + std::string(TypeInfo().name));
        return Program(std::move(b));
    }

    return State(src, binding_info).Run();
}

}  // namespace tint::transform
