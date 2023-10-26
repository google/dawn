// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/glsl/writer/ast_raise/texture_builtins_from_uniform.h"

#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/module.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/variable.h"

#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/rtti/switch.h"

TINT_INSTANTIATE_TYPEINFO(tint::glsl::writer::TextureBuiltinsFromUniform);
TINT_INSTANTIATE_TYPEINFO(tint::glsl::writer::TextureBuiltinsFromUniform::Config);
TINT_INSTANTIATE_TYPEINFO(tint::glsl::writer::TextureBuiltinsFromUniform::Result);

namespace tint::glsl::writer {

namespace {

/// The member name of the texture builtin values.
constexpr std::string_view kTextureBuiltinValuesMemberNamePrefix = "texture_builtin_value_";

bool ShouldRun(const Program& program) {
    for (auto* fn : program.AST().Functions()) {
        if (auto* sem_fn = program.Sem().Get(fn)) {
            for (auto* builtin : sem_fn->DirectlyCalledBuiltins()) {
                // GLSL ES  has no native support for the counterpart of
                // textureNumLevels (textureQueryLevels) and textureNumSamples (textureSamples)
                if (builtin->Fn() == wgsl::BuiltinFn::kTextureNumLevels) {
                    return true;
                }
                if (builtin->Fn() == wgsl::BuiltinFn::kTextureNumSamples) {
                    return true;
                }
            }
        }
    }
    return false;
}

}  // namespace

TextureBuiltinsFromUniform::TextureBuiltinsFromUniform() = default;
TextureBuiltinsFromUniform::~TextureBuiltinsFromUniform() = default;

/// PIMPL state for the transform
struct TextureBuiltinsFromUniform::State {
    /// Constructor
    /// @param program the source program
    /// @param in the input transform data
    /// @param out the output transform data
    explicit State(const Program& program,
                   const ast::transform::DataMap& in,
                   ast::transform::DataMap& out)
        : src(program), inputs(in), outputs(out) {}

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    ApplyResult Run() {
        auto* cfg = inputs.Get<Config>();
        if (cfg == nullptr) {
            b.Diagnostics().add_error(
                diag::System::Transform,
                "missing transform data for " +
                    std::string(tint::TypeInfo::Of<TextureBuiltinsFromUniform>().name));
            return resolver::Resolve(b);
        }

        if (!ShouldRun(src)) {
            return SkipTransform;
        }

        // The dependency order declartions guaranteed that we traverse interested functions in the
        // following order:
        // 1. texture builtins
        // 2. user function directly calls texture builtins
        // 3. user function calls 2.
        // 4. user function calls 3.
        // ...
        // n. entry point function.
        for (auto* fn_decl : sem.Module()->DependencyOrderedDeclarations()) {
            if (auto* fn = sem.Get<sem::Function>(fn_decl)) {
                for (auto* call : fn->DirectCalls()) {
                    auto* call_expr = call->Declaration();

                    tint::Switch(
                        call->Target(),
                        [&](const sem::BuiltinFn* builtin) {
                            if (builtin->Fn() != wgsl::BuiltinFn::kTextureNumLevels &&
                                builtin->Fn() != wgsl::BuiltinFn::kTextureNumSamples) {
                                return;
                            }
                            if (auto* call_stmt =
                                    call->Stmt()->Declaration()->As<ast::CallStatement>()) {
                                if (call_stmt->expr == call->Declaration()) {
                                    // textureNumLevels() / textureNumSamples() is used as a
                                    // statement. The argument expression must be side-effect free,
                                    // so just drop the statement.
                                    RemoveStatement(ctx, call_stmt);
                                    return;
                                }
                            }

                            auto* texture_expr = call->Declaration()->args[0];
                            auto* texture_sem = sem.GetVal(texture_expr)->RootIdentifier();
                            TINT_ASSERT(texture_sem);

                            TextureBuiltinsFromUniformOptions::Field dataType =
                                GetFieldFromBuiltinFunctionType(builtin->Fn());

                            tint::Switch(
                                texture_sem,
                                [&](const sem::GlobalVariable* global) {
                                    // This texture variable is a global variable.
                                    auto binding = GetAndRecordGlobalBinding(global, dataType);
                                    // Record the call and binding to be replaced later.
                                    builtin_to_replace.Add(call_expr, binding);
                                },
                                [&](const sem::Variable* variable) {
                                    // This texture variable is a user function parameter.
                                    auto new_param =
                                        GetAndRecordFunctionParameter(fn, variable, dataType);
                                    // Record the call and new_param to be replaced later.
                                    builtin_to_replace.Add(call_expr, new_param);
                                },  //
                                TINT_ICE_ON_NO_MATCH);
                        },
                        [&](const sem::Function* user_fn) {
                            auto user_param_to_info = fn_to_data.Find(user_fn);
                            if (!user_param_to_info) {
                                // Uninterested function not calling texture builtins with function
                                // texture param.
                                return;
                            }
                            TINT_ASSERT(call->Arguments().Length() ==
                                        user_fn->Declaration()->params.Length());
                            for (size_t i = 0; i < call->Arguments().Length(); i++) {
                                auto param = user_fn->Declaration()->params[i];
                                auto info = user_param_to_info->Get(param);
                                if (info.has_value()) {
                                    auto* arg = call->Arguments()[i];
                                    auto* texture_sem = arg->RootIdentifier();
                                    auto& args = call_to_data.GetOrCreate(call_expr, [&] {
                                        return Vector<
                                            std::variant<BindingPoint, const ast::Parameter*>, 4>();
                                    });

                                    tint::Switch(
                                        texture_sem,
                                        [&](const sem::GlobalVariable* global) {
                                            // This texture variable is a global variable.
                                            auto binding =
                                                GetAndRecordGlobalBinding(global, info->field);
                                            // Record the binding to add to args.
                                            args.Push(binding);
                                        },
                                        [&](const sem::Variable* variable) {
                                            // This texture variable is a user function parameter.
                                            auto new_param = GetAndRecordFunctionParameter(
                                                fn, variable, info->field);
                                            // Record adding extra function parameter
                                            args.Push(new_param);
                                        },  //
                                        TINT_ICE_ON_NO_MATCH);
                                }
                            }
                        });
                }
            }
        }

        // If there's no interested texture builtin at all, skip the transform.
        if (bindpoint_to_data.empty()) {
            return SkipTransform;
        }

        // If any functions need extra params, add them now.
        if (!fn_to_data.IsEmpty()) {
            for (auto pair : fn_to_data) {
                auto* fn = pair.key;

                // Reorder the param to a vector to make sure params are in the correct order.
                Vector<const ast::Parameter*, 4> extra_params_in_order;
                extra_params_in_order.Resize(pair.value.Count());
                for (auto t_p : pair.value) {
                    TINT_ASSERT(t_p.value.extra_idx < extra_params_in_order.Length());
                    extra_params_in_order[t_p.value.extra_idx] = t_p.value.param;
                }

                for (auto p : extra_params_in_order) {
                    ctx.InsertBack(fn->Declaration()->params, p);
                }
            }
        }

        // Replace all interested texture builtin calls.
        for (auto pair : builtin_to_replace) {
            auto call = pair.key;
            if (std::holds_alternative<BindingPoint>(pair.value)) {
                // This texture is a global variable with binding point.
                // Read builtin value from uniform buffer.
                auto* builtin_value = GetUniformValue(std::get<BindingPoint>(pair.value));
                ctx.Replace(call, builtin_value);
            } else {
                // Otherwise this value comes from a function param
                auto* param = std::get<const ast::Parameter*>(pair.value);
                ctx.Replace(call, b.Expr(param));
            }
        }

        // Insert all extra args to interested function calls.
        for (auto pair : call_to_data) {
            auto call = pair.key;
            for (auto new_arg_info : pair.value) {
                if (std::holds_alternative<BindingPoint>(new_arg_info)) {
                    // This texture is a global variable with binding point.
                    // Read builtin value from uniform buffer.
                    auto* builtin_value = GetUniformValue(std::get<BindingPoint>(new_arg_info));
                    ctx.InsertBack(call->args, builtin_value);
                } else {
                    // Otherwise this value comes from a function param
                    auto* param = std::get<const ast::Parameter*>(new_arg_info);
                    ctx.InsertBack(call->args, b.Expr(param));
                }
            }
        }

        outputs.Add<Result>(bindpoint_to_data);

        ctx.Clone();
        return resolver::Resolve(b);
    }

  private:
    /// The source program
    const Program& src;
    /// The transform inputs
    const ast::transform::DataMap& inputs;
    /// The transform outputs
    ast::transform::DataMap& outputs;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    program::CloneContext ctx = {&b, &src, /* auto_clone_symbols */ true};
    /// Alias to the semantic info in ctx.src
    const sem::Info& sem = src.Sem();

    /// The bindpoint to byte offset and field to pass out in transform result.
    /// For one texture type, it could only be passed into one of the
    /// textureNumLevels or textureNumSamples because their accepting param texture
    /// type is different. There cannot be a binding entry with both field type.
    /// Note: because this transform must be run before CombineSampler and BindingRemapper,
    /// the binding number here is before remapped.
    Result::BindingPointToFieldAndOffset bindpoint_to_data;

    struct FunctionExtraParamInfo {
        using Field = TextureBuiltinsFromUniformOptions::Field;
        // The kind of texture information this parameter holds.
        Field field = Field::TextureNumLevels;

        // The extra passed in param that corresponds to the texture param.
        const ast::Parameter* param = nullptr;

        // id of this extra param e.g. f(t0, foo, t1, e0, e1) e0 and e1 are extra params, their
        // extra_idx are 0 and 1. This is to help sort extra ids in the correct order.
        size_t extra_idx = 0;
    };

    /// Store a map from function to a collection of extra params that need adding.
    /// The value of the map is made a map instead of a vector to make it easier to find the param.
    /// for call sites. e.g. fn f(t: texture_2d<f32>) -> u32 {
    ///   return textureNumLevels(t);
    /// }
    /// ->
    /// fn f(t : texture_2d<f32>, tint_symbol : u32) -> u32 {
    ///   return tint_symbol;
    /// }
    Hashmap<const sem::Function*, Hashmap<const ast::Parameter*, FunctionExtraParamInfo, 4>, 8>
        fn_to_data;

    /// For each callsite of the above functions, record a vector of extra call args that need
    /// inserting. e.g. f(tex)
    /// ->
    /// f(tex, internal_uniform.texture_builtin_value), if tex is from a global
    /// variable, store the BindingPoint. or f(tex, extra_param_tex), if tex is from a function
    /// param, store the texture function parameter pointer.
    Hashmap<const ast::CallExpression*,
            Vector<std::variant<BindingPoint, const ast::Parameter*>, 4>,
            8>
        call_to_data;

    /// Texture builtin calls to be replaced by either uniform values or function parameters.
    Hashmap<const ast::CallExpression*, std::variant<BindingPoint, const ast::Parameter*>, 8>
        builtin_to_replace;

    /// A map from global texture bindpoint to the symbol storing its builtin value in the uniform
    /// buffer struct.
    Hashmap<BindingPoint, Symbol, 16> bindpoint_to_syms;

    /// The internal uniform buffer
    const ast::Variable* ubo = nullptr;
    /// Get or create a UBO including u32 scalars for texture builtin values.
    /// @returns the symbol of the uniform buffer variable.
    Symbol GetUboSym() {
        if (ubo) {
            // Already created
            return ubo->name->symbol;
        }

        auto* cfg = inputs.Get<Config>();

        Vector<const ast::StructMember*, 16> new_members;
        new_members.Resize(bindpoint_to_data.size());
        for (auto it : bindpoint_to_data) {
            // Emit a u32 scalar for each binding that needs builtin value passed in.
            size_t i = it.second.second / sizeof(uint32_t);
            TINT_ASSERT(i < new_members.Length());
            // Append the vector index with the variable name to avoid unstable naming issue.
            auto sym = b.Symbols().New(std::string(kTextureBuiltinValuesMemberNamePrefix) +
                                       std::to_string(i));
            bindpoint_to_syms.Add(it.first, sym);
            new_members[i] = b.Member(sym, b.ty.u32());
        }

        // Find if there's any existing global variable using the same cfg->ubo_binding
        for (auto* var : src.AST().Globals<ast::Var>()) {
            if (var->HasBindingPoint()) {
                auto* global_sem = sem.Get<sem::GlobalVariable>(var);

                // The original binding point
                BindingPoint binding_point = *global_sem->BindingPoint();

                if (binding_point == cfg->ubo_binding) {
                    // This ubo_binding struct already exists.
                    // which should only be added by other *FromUniform transforms.
                    // Replace it with a new struct including the new_member.
                    // Then remove the old structure global declaration.

                    ubo = var->As<ast::Variable>();

                    auto* ty = global_sem->Type()->UnwrapRef();
                    auto* str = ty->As<sem::Struct>();
                    if (TINT_UNLIKELY(!str)) {
                        TINT_ICE()
                            << "existing ubo binding " << cfg->ubo_binding << " is not a struct.";
                        return ctx.Clone(ubo->name->symbol);
                    }

                    for (auto new_member : new_members) {
                        ctx.InsertBack(str->Declaration()->members, new_member);
                    }
                    return ctx.Clone(ubo->name->symbol);
                }
            }
        }

        auto* buffer_struct = b.Structure(b.Sym(), std::move(new_members));
        ubo = b.GlobalVar(b.Sym(), b.ty.Of(buffer_struct), core::AddressSpace::kUniform,
                          b.Group(core::AInt(cfg->ubo_binding.group)),
                          b.Binding(core::AInt(cfg->ubo_binding.binding)));
        return ubo->name->symbol;
    }

    /// Get the expression of retrieving the builtin value from the uniform buffer.
    /// @param binding of the global variable.
    /// @returns an expression of the builtin value.
    const ast::Expression* GetUniformValue(const BindingPoint& binding) {
        auto iter = bindpoint_to_data.find(binding);
        TINT_ASSERT(iter != bindpoint_to_data.end());

        // Make sure GetUboSym() is called first to initialize the uniform buffer struct.
        auto ubo_sym = GetUboSym();
        // Load the builtin value from the UBO.
        auto member_sym = bindpoint_to_syms.Get(binding);
        TINT_ASSERT(member_sym.has_value());
        auto* builtin_value = b.MemberAccessor(ubo_sym, *member_sym);

        return builtin_value;
    }

    /// Get and return the binding of the global texture variable. Record in bindpoint_to_data if
    /// first visited.
    /// @param global global variable of the texture variable.
    /// @param field type of the interested builtin function data related to this texture.
    /// @returns binding of the global variable.
    BindingPoint GetAndRecordGlobalBinding(const sem::GlobalVariable* global,
                                           TextureBuiltinsFromUniformOptions::Field field) {
        auto binding = global->BindingPoint().value();
        auto iter = bindpoint_to_data.find(binding);
        if (iter == bindpoint_to_data.end()) {
            // First visit, recording the binding.
            uint32_t index = static_cast<uint32_t>(bindpoint_to_data.size());
            bindpoint_to_data.emplace(
                binding,
                Result::FieldAndOffset{field, index * static_cast<uint32_t>(sizeof(uint32_t))});
        }
        return binding;
    }

    /// Find which function param is the given texture variable.
    /// Add a new u32 param relates to this texture param. Record in fn_to_data if first visited.
    /// @param fn the current function scope.
    /// @param var the texture variable.
    /// @param field type of the interested builtin function data related to this texture.
    /// @returns the new u32 function parameter.
    const ast::Parameter* GetAndRecordFunctionParameter(
        const sem::Function* fn,
        const sem::Variable* var,
        TextureBuiltinsFromUniformOptions::Field field) {
        auto& param_to_info = fn_to_data.GetOrCreate(
            fn, [&] { return Hashmap<const ast::Parameter*, FunctionExtraParamInfo, 4>(); });

        const ast::Parameter* param = nullptr;
        for (auto p : fn->Declaration()->params) {
            if (p->As<ast::Variable>() == var->Declaration()) {
                param = p;
                break;
            }
        }
        TINT_ASSERT(param);
        // Get or record a new u32 param to this function if first visited.
        auto entry = param_to_info.Get(param);
        if (entry.has_value()) {
            return entry->param;
        }
        const ast::Parameter* new_param = b.Param(b.Sym(), b.ty.u32());
        size_t idx = param_to_info.Count();
        param_to_info.Add(param, FunctionExtraParamInfo{field, new_param, idx});
        return new_param;
    }

    /// Get the uniform options field for the builtin function.
    /// @param type of the builtin function
    /// @returns corresponding TextureBuiltinsFromUniformOptions::Field for the builtin
    static TextureBuiltinsFromUniformOptions::Field GetFieldFromBuiltinFunctionType(
        wgsl::BuiltinFn type) {
        switch (type) {
            case wgsl::BuiltinFn::kTextureNumLevels:
                return TextureBuiltinsFromUniformOptions::Field::TextureNumLevels;
            case wgsl::BuiltinFn::kTextureNumSamples:
                return TextureBuiltinsFromUniformOptions::Field::TextureNumSamples;
            default:
                TINT_UNREACHABLE() << "unsupported builtin function type " << type;
        }
        return TextureBuiltinsFromUniformOptions::Field::TextureNumLevels;
    }
};

ast::transform::Transform::ApplyResult TextureBuiltinsFromUniform::Apply(
    const Program& src,
    const ast::transform::DataMap& inputs,
    ast::transform::DataMap& outputs) const {
    return State{src, inputs, outputs}.Run();
}

TextureBuiltinsFromUniform::Config::Config(BindingPoint ubo_bp) : ubo_binding(ubo_bp) {}
TextureBuiltinsFromUniform::Config::Config(const Config&) = default;
TextureBuiltinsFromUniform::Config& TextureBuiltinsFromUniform::Config::operator=(const Config&) =
    default;
TextureBuiltinsFromUniform::Config::~Config() = default;

TextureBuiltinsFromUniform::Result::Result(BindingPointToFieldAndOffset bindpoint_to_data_in)
    : bindpoint_to_data(std::move(bindpoint_to_data_in)) {}
TextureBuiltinsFromUniform::Result::Result(const Result&) = default;
TextureBuiltinsFromUniform::Result::~Result() = default;

}  // namespace tint::glsl::writer
