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

CombineSamplers::BindingInfo::BindingInfo(const BindingMap& map,
                                          const sem::BindingPoint& placeholder)
    : binding_map(map), placeholder_binding_point(placeholder) {}
CombineSamplers::BindingInfo::BindingInfo(const BindingInfo& other) = default;
CombineSamplers::BindingInfo::~BindingInfo() = default;

/// The PIMPL state for the CombineSamplers transform
struct CombineSamplers::State {
    /// The clone context
    CloneContext& ctx;

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
    ast::AttributeList Attributes() const {
        auto attributes = ctx.dst->GroupAndBinding(0, 0);
        attributes.push_back(ctx.dst->Disable(ast::DisabledValidation::kBindingPointCollision));
        return attributes;
    }

    /// Constructor
    /// @param context the clone context
    /// @param info the binding map information
    State(CloneContext& context, const BindingInfo* info) : ctx(context), binding_info(info) {}

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
        bp_pair.texture_binding_point = texture_var->As<sem::GlobalVariable>()->BindingPoint();
        bp_pair.sampler_binding_point = sampler_var
                                            ? sampler_var->As<sem::GlobalVariable>()->BindingPoint()
                                            : binding_info->placeholder_binding_point;
        auto it = binding_info->binding_map.find(bp_pair);
        if (it != binding_info->binding_map.end()) {
            name = it->second;
        }
        const ast::Type* type = CreateCombinedASTTypeFor(texture_var, sampler_var);
        Symbol symbol = ctx.dst->Symbols().New(name);
        return ctx.dst->Global(symbol, type, Attributes());
    }

    /// Creates placeholder global sampler variables.
    /// @param kind the sampler kind to create for
    /// @returns the newly-created global variable
    const ast::Variable* CreatePlaceholder(ast::SamplerKind kind) {
        const ast::Type* type = ctx.dst->ty.sampler(kind);
        const char* name = kind == ast::SamplerKind::kComparisonSampler
                               ? "placeholder_comparison_sampler"
                               : "placeholder_sampler";
        Symbol symbol = ctx.dst->Symbols().New(name);
        return ctx.dst->Global(symbol, type, Attributes());
    }

    /// Creates ast::Type for a given texture and sampler variable pair.
    /// Depth textures with no samplers are turned into the corresponding
    /// f32 texture (e.g., texture_depth_2d -> texture_2d<f32>).
    /// @param texture the texture variable of interest
    /// @param sampler the texture variable of interest
    /// @returns the newly-created type
    const ast::Type* CreateCombinedASTTypeFor(const sem::Variable* texture,
                                              const sem::Variable* sampler) {
        const sem::Type* texture_type = texture->Type()->UnwrapRef();
        const sem::DepthTexture* depth = texture_type->As<sem::DepthTexture>();
        if (depth && !sampler) {
            return ctx.dst->create<ast::SampledTexture>(depth->dim(), ctx.dst->create<ast::F32>());
        } else {
            return CreateASTTypeFor(ctx, texture_type);
        }
    }

    /// Performs the transformation
    void Run() {
        auto& sem = ctx.src->Sem();

        // Remove all texture and sampler global variables. These will be replaced
        // by combined samplers.
        for (auto* var : ctx.src->AST().GlobalVariables()) {
            auto* type = sem.Get(var->type);
            if (type && type->IsAnyOf<sem::Texture, sem::Sampler>() &&
                !type->Is<sem::StorageTexture>()) {
                ctx.Remove(ctx.src->AST().GlobalDeclarations(), var);
            } else if (auto binding_point = var->BindingPoint()) {
                if (binding_point.group->value == 0 && binding_point.binding->value == 0) {
                    auto* attribute =
                        ctx.dst->Disable(ast::DisabledValidation::kBindingPointCollision);
                    ctx.InsertFront(var->attributes, attribute);
                }
            }
        }

        // Rewrite all function signatures to use combined samplers, and remove
        // separate textures & samplers. Create new combined globals where found.
        ctx.ReplaceAll([&](const ast::Function* src) -> const ast::Function* {
            if (auto* func = sem.Get(src)) {
                auto pairs = func->TextureSamplerPairs();
                if (pairs.empty()) {
                    return nullptr;
                }
                ast::VariableList params;
                for (auto pair : func->TextureSamplerPairs()) {
                    const sem::Variable* texture_var = pair.first;
                    const sem::Variable* sampler_var = pair.second;
                    std::string name =
                        ctx.src->Symbols().NameFor(texture_var->Declaration()->symbol);
                    if (sampler_var) {
                        name +=
                            "_" + ctx.src->Symbols().NameFor(sampler_var->Declaration()->symbol);
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
                        const ast::Type* type = CreateCombinedASTTypeFor(texture_var, sampler_var);
                        const ast::Variable* var =
                            ctx.dst->Param(ctx.dst->Symbols().New(name), type);
                        params.push_back(var);
                        function_combined_texture_samplers_[func][pair] = var;
                    }
                }
                // Filter out separate textures and samplers from the original
                // function signature.
                for (auto* var : src->params) {
                    if (!sem.Get(var->type)->IsAnyOf<sem::Texture, sem::Sampler>()) {
                        params.push_back(ctx.Clone(var));
                    }
                }
                // Create a new function signature that differs only in the parameter
                // list.
                auto symbol = ctx.Clone(src->symbol);
                auto* return_type = ctx.Clone(src->return_type);
                auto* body = ctx.Clone(src->body);
                auto attributes = ctx.Clone(src->attributes);
                auto return_type_attributes = ctx.Clone(src->return_type_attributes);
                return ctx.dst->create<ast::Function>(symbol, params, return_type, body,
                                                      std::move(attributes),
                                                      std::move(return_type_attributes));
            }
            return nullptr;
        });

        // Replace all function call expressions containing texture or
        // sampler parameters to use the current function's combined samplers or
        // the combined global samplers, as appropriate.
        ctx.ReplaceAll([&](const ast::CallExpression* expr) -> const ast::Expression* {
            if (auto* call = sem.Get(expr)) {
                ast::ExpressionList args;
                // Replace all texture builtin calls.
                if (auto* builtin = call->Target()->As<sem::Builtin>()) {
                    const auto& signature = builtin->Signature();
                    int sampler_index = signature.IndexOf(sem::ParameterUsage::kSampler);
                    int texture_index = signature.IndexOf(sem::ParameterUsage::kTexture);
                    if (texture_index == -1) {
                        return nullptr;
                    }
                    const sem::Expression* texture = call->Arguments()[texture_index];
                    // We don't want to combine storage textures with anything, since
                    // they never have associated samplers in GLSL.
                    if (texture->Type()->UnwrapRef()->Is<sem::StorageTexture>()) {
                        return nullptr;
                    }
                    const sem::Expression* sampler =
                        sampler_index != -1 ? call->Arguments()[sampler_index] : nullptr;
                    auto* texture_var = texture->As<sem::VariableUser>()->Variable();
                    auto* sampler_var =
                        sampler ? sampler->As<sem::VariableUser>()->Variable() : nullptr;
                    sem::VariablePair new_pair(texture_var, sampler_var);
                    for (auto* arg : expr->args) {
                        auto* type = ctx.src->TypeOf(arg)->UnwrapRef();
                        if (type->Is<sem::Texture>()) {
                            const ast::Variable* var =
                                IsGlobal(new_pair)
                                    ? global_combined_texture_samplers_[new_pair]
                                    : function_combined_texture_samplers_[call->Stmt()->Function()]
                                                                         [new_pair];
                            args.push_back(ctx.dst->Expr(var->symbol));
                        } else if (auto* sampler_type = type->As<sem::Sampler>()) {
                            ast::SamplerKind kind = sampler_type->kind();
                            int index = (kind == ast::SamplerKind::kSampler) ? 0 : 1;
                            const ast::Variable*& p = placeholder_samplers_[index];
                            if (!p) {
                                p = CreatePlaceholder(kind);
                            }
                            args.push_back(ctx.dst->Expr(p->symbol));
                        } else {
                            args.push_back(ctx.Clone(arg));
                        }
                    }
                    const ast::Expression* value =
                        ctx.dst->Call(ctx.Clone(expr->target.name), args);
                    if (builtin->Type() == sem::BuiltinType::kTextureLoad &&
                        texture_var->Type()->UnwrapRef()->Is<sem::DepthTexture>() &&
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
                            const sem::Expression* texture = call->Arguments()[param->Index()];
                            texture_var = texture->As<sem::VariableUser>()->Variable();
                        }
                        if (sampler_var) {
                            if (auto* param = sampler_var->As<sem::Parameter>()) {
                                const sem::Expression* sampler = call->Arguments()[param->Index()];
                                sampler_var = sampler->As<sem::VariableUser>()->Variable();
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
                        auto* arg = ctx.dst->Expr(var->symbol);
                        args.push_back(arg);
                    }
                    // Append all of the remaining non-texture and non-sampler
                    // parameters.
                    for (auto* arg : expr->args) {
                        if (!ctx.src->TypeOf(arg)
                                 ->UnwrapRef()
                                 ->IsAnyOf<sem::Texture, sem::Sampler>()) {
                            args.push_back(ctx.Clone(arg));
                        }
                    }
                    return ctx.dst->Call(ctx.Clone(expr->target.name), args);
                }
            }
            return nullptr;
        });

        ctx.Clone();
    }
};

CombineSamplers::CombineSamplers() = default;

CombineSamplers::~CombineSamplers() = default;

void CombineSamplers::Run(CloneContext& ctx, const DataMap& inputs, DataMap&) const {
    auto* binding_info = inputs.Get<BindingInfo>();
    if (!binding_info) {
        ctx.dst->Diagnostics().add_error(
            diag::System::Transform, "missing transform data for " + std::string(TypeInfo().name));
        return;
    }

    State(ctx, binding_info).Run();
}

}  // namespace tint::transform
