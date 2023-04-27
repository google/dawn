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

#include "src/tint/transform/canonicalize_entry_point_io.h"

#include <algorithm>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/ast/disable_validation_attribute.h"
#include "src/tint/builtin/builtin_value.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/function.h"
#include "src/tint/transform/unshadow.h"

using namespace tint::number_suffixes;  // NOLINT

TINT_INSTANTIATE_TYPEINFO(tint::transform::CanonicalizeEntryPointIO);
TINT_INSTANTIATE_TYPEINFO(tint::transform::CanonicalizeEntryPointIO::Config);

namespace tint::transform {

CanonicalizeEntryPointIO::CanonicalizeEntryPointIO() = default;
CanonicalizeEntryPointIO::~CanonicalizeEntryPointIO() = default;

namespace {

/// Info for a struct member
struct MemberInfo {
    /// The struct member item
    const ast::StructMember* member;
    /// The struct member location if provided
    std::optional<uint32_t> location;
};

/// FXC is sensitive to field order in structures, this is used by StructMemberComparator to ensure
/// that FXC is happy with the order of emitted fields.
uint32_t BuiltinOrder(builtin::BuiltinValue builtin) {
    switch (builtin) {
        case builtin::BuiltinValue::kPosition:
            return 1;
        case builtin::BuiltinValue::kVertexIndex:
            return 2;
        case builtin::BuiltinValue::kInstanceIndex:
            return 3;
        case builtin::BuiltinValue::kFrontFacing:
            return 4;
        case builtin::BuiltinValue::kFragDepth:
            return 5;
        case builtin::BuiltinValue::kLocalInvocationId:
            return 6;
        case builtin::BuiltinValue::kLocalInvocationIndex:
            return 7;
        case builtin::BuiltinValue::kGlobalInvocationId:
            return 8;
        case builtin::BuiltinValue::kWorkgroupId:
            return 9;
        case builtin::BuiltinValue::kNumWorkgroups:
            return 10;
        case builtin::BuiltinValue::kSampleIndex:
            return 11;
        case builtin::BuiltinValue::kSampleMask:
            return 12;
        case builtin::BuiltinValue::kPointSize:
            return 13;
        default:
            break;
    }
    return 0;
}

// Returns true if `attr` is a shader IO attribute.
bool IsShaderIOAttribute(const ast::Attribute* attr) {
    return attr->IsAnyOf<ast::BuiltinAttribute, ast::InterpolateAttribute, ast::InvariantAttribute,
                         ast::LocationAttribute>();
}

}  // namespace

/// PIMPL state for the transform
struct CanonicalizeEntryPointIO::State {
    /// OutputValue represents a shader result that the wrapper function produces.
    struct OutputValue {
        /// The name of the output value.
        std::string name;
        /// The type of the output value.
        ast::Type type;
        /// The shader IO attributes.
        utils::Vector<const ast::Attribute*, 8> attributes;
        /// The value itself.
        const ast::Expression* value;
        /// The output location.
        std::optional<uint32_t> location;
    };

    /// The clone context.
    CloneContext& ctx;
    /// The transform config.
    CanonicalizeEntryPointIO::Config const cfg;
    /// The entry point function (AST).
    const ast::Function* func_ast;
    /// The entry point function (SEM).
    const sem::Function* func_sem;

    /// The new entry point wrapper function's parameters.
    utils::Vector<const ast::Parameter*, 8> wrapper_ep_parameters;

    /// The members of the wrapper function's struct parameter.
    utils::Vector<MemberInfo, 8> wrapper_struct_param_members;
    /// The name of the wrapper function's struct parameter.
    Symbol wrapper_struct_param_name;
    /// The parameters that will be passed to the original function.
    utils::Vector<const ast::Expression*, 8> inner_call_parameters;
    /// The members of the wrapper function's struct return type.
    utils::Vector<MemberInfo, 8> wrapper_struct_output_members;
    /// The wrapper function output values.
    utils::Vector<OutputValue, 8> wrapper_output_values;
    /// The body of the wrapper function.
    utils::Vector<const ast::Statement*, 8> wrapper_body;
    /// Input names used by the entrypoint
    std::unordered_set<std::string> input_names;
    /// A map of cloned attribute to builtin value
    utils::Hashmap<const ast::BuiltinAttribute*, builtin::BuiltinValue, 16> builtin_attrs;

    /// Constructor
    /// @param context the clone context
    /// @param config the transform config
    /// @param function the entry point function
    State(CloneContext& context,
          const CanonicalizeEntryPointIO::Config& config,
          const ast::Function* function)
        : ctx(context), cfg(config), func_ast(function), func_sem(ctx.src->Sem().Get(function)) {}

    /// Clones the attributes from @p in and adds it to @p out. If @p in is a builtin attribute,
    /// then builtin_attrs is updated with the builtin information.
    /// @param in the attribute to clone
    /// @param out the output Attributes
    template <size_t N>
    void CloneAttribute(const ast::Attribute* in, utils::Vector<const ast::Attribute*, N>& out) {
        auto* cloned = ctx.Clone(in);
        out.Push(cloned);
        if (auto* builtin = in->As<ast::BuiltinAttribute>()) {
            builtin_attrs.Add(cloned->As<ast::BuiltinAttribute>(),
                              ctx.src->Sem().Get(builtin)->Value());
        }
    }

    /// Clones the shader IO attributes from @p in.
    /// @param in the attributes to clone
    /// @param do_interpolate whether to clone InterpolateAttribute
    /// @return the cloned attributes
    template <size_t N>
    auto CloneShaderIOAttributes(const utils::Vector<const ast::Attribute*, N> in,
                                 bool do_interpolate) {
        utils::Vector<const ast::Attribute*, N> out;
        for (auto* attr : in) {
            if (IsShaderIOAttribute(attr) &&
                (do_interpolate || !attr->template Is<ast::InterpolateAttribute>())) {
                CloneAttribute(attr, out);
            }
        }
        return out;
    }

    /// @param attr the input attribute
    /// @returns the builtin value of the attribute
    builtin::BuiltinValue BuiltinOf(const ast::BuiltinAttribute* attr) {
        if (attr->program_id == ctx.dst->ID()) {
            // attr belongs to the target program.
            // Obtain the builtin value from #builtin_attrs.
            if (auto b = builtin_attrs.Get(attr)) {
                return *b;
            }
        } else {
            // attr belongs to the source program.
            // Obtain the builtin value from the semantic info.
            return ctx.src->Sem().Get(attr)->Value();
        }
        TINT_ICE(Resolver, ctx.dst->Diagnostics())
            << "could not obtain builtin value from attribute";
        return builtin::BuiltinValue::kUndefined;
    }

    /// @param attrs the input attribute list
    /// @returns the builtin value if any of the attributes in @p attrs is a builtin attribute,
    /// otherwise builtin::BuiltinValue::kUndefined
    builtin::BuiltinValue BuiltinOf(utils::VectorRef<const ast::Attribute*> attrs) {
        if (auto* builtin = ast::GetAttribute<ast::BuiltinAttribute>(attrs)) {
            return BuiltinOf(builtin);
        }
        return builtin::BuiltinValue::kUndefined;
    }

    /// Create or return a symbol for the wrapper function's struct parameter.
    /// @returns the symbol for the struct parameter
    Symbol InputStructSymbol() {
        if (!wrapper_struct_param_name.IsValid()) {
            wrapper_struct_param_name = ctx.dst->Sym();
        }
        return wrapper_struct_param_name;
    }

    /// Add a shader input to the entry point.
    /// @param name the name of the shader input
    /// @param type the type of the shader input
    /// @param location the location if provided
    /// @param attrs the attributes to apply to the shader input
    /// @returns an expression which evaluates to the value of the shader input
    const ast::Expression* AddInput(std::string name,
                                    const type::Type* type,
                                    std::optional<uint32_t> location,
                                    utils::Vector<const ast::Attribute*, 8> attrs) {
        auto ast_type = CreateASTTypeFor(ctx, type);

        auto builtin_attr = BuiltinOf(attrs);

        if (cfg.shader_style == ShaderStyle::kSpirv || cfg.shader_style == ShaderStyle::kGlsl) {
            // Vulkan requires that integer user-defined fragment inputs are always decorated with
            // `Flat`. See:
            // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/StandaloneSpirv.html#VUID-StandaloneSpirv-Flat-04744
            // TODO(crbug.com/tint/1224): Remove this once a flat interpolation attribute is
            // required for integers.
            if (func_ast->PipelineStage() == ast::PipelineStage::kFragment &&
                type->is_integer_scalar_or_vector() &&
                !ast::HasAttribute<ast::InterpolateAttribute>(attrs) &&
                (ast::HasAttribute<ast::LocationAttribute>(attrs) ||
                 cfg.shader_style == ShaderStyle::kSpirv)) {
                attrs.Push(ctx.dst->Interpolate(builtin::InterpolationType::kFlat,
                                                builtin::InterpolationSampling::kUndefined));
            }

            // Disable validation for use of the `input` address space.
            attrs.Push(ctx.dst->Disable(ast::DisabledValidation::kIgnoreAddressSpace));

            // In GLSL, if it's a builtin, override the name with the
            // corresponding gl_ builtin name
            if (cfg.shader_style == ShaderStyle::kGlsl &&
                builtin_attr != builtin::BuiltinValue::kUndefined) {
                name = GLSLBuiltinToString(builtin_attr, func_ast->PipelineStage(),
                                           builtin::AddressSpace::kIn);
            }
            auto symbol = ctx.dst->Symbols().New(name);

            // Create the global variable and use its value for the shader input.
            const ast::Expression* value = ctx.dst->Expr(symbol);

            if (builtin_attr != builtin::BuiltinValue::kUndefined) {
                if (cfg.shader_style == ShaderStyle::kGlsl) {
                    value = FromGLSLBuiltin(builtin_attr, value, ast_type);
                } else if (builtin_attr == builtin::BuiltinValue::kSampleMask) {
                    // Vulkan requires the type of a SampleMask builtin to be an array.
                    // Declare it as array<u32, 1> and then load the first element.
                    ast_type = ctx.dst->ty.array(ast_type, 1_u);
                    value = ctx.dst->IndexAccessor(value, 0_i);
                }
            }
            ctx.dst->GlobalVar(symbol, ast_type, builtin::AddressSpace::kIn, std::move(attrs));
            return value;
        } else if (cfg.shader_style == ShaderStyle::kMsl &&
                   builtin_attr != builtin::BuiltinValue::kUndefined) {
            // If this input is a builtin and we are targeting MSL, then add it to the
            // parameter list and pass it directly to the inner function.
            Symbol symbol = input_names.emplace(name).second ? ctx.dst->Symbols().Register(name)
                                                             : ctx.dst->Symbols().New(name);
            wrapper_ep_parameters.Push(ctx.dst->Param(symbol, ast_type, std::move(attrs)));
            return ctx.dst->Expr(symbol);
        } else {
            // Otherwise, move it to the new structure member list.
            Symbol symbol = input_names.emplace(name).second ? ctx.dst->Symbols().Register(name)
                                                             : ctx.dst->Symbols().New(name);
            wrapper_struct_param_members.Push(
                {ctx.dst->Member(symbol, ast_type, std::move(attrs)), location});
            return ctx.dst->MemberAccessor(InputStructSymbol(), symbol);
        }
    }

    /// Add a shader output to the entry point.
    /// @param name the name of the shader output
    /// @param type the type of the shader output
    /// @param location the location if provided
    /// @param attrs the attributes to apply to the shader output
    /// @param value the value of the shader output
    void AddOutput(std::string name,
                   const type::Type* type,
                   std::optional<uint32_t> location,
                   utils::Vector<const ast::Attribute*, 8> attrs,
                   const ast::Expression* value) {
        auto builtin_attr = BuiltinOf(attrs);
        // Vulkan requires that integer user-defined vertex outputs are always decorated with
        // `Flat`.
        // TODO(crbug.com/tint/1224): Remove this once a flat interpolation attribute is required
        // for integers.
        if (cfg.shader_style == ShaderStyle::kSpirv &&
            func_ast->PipelineStage() == ast::PipelineStage::kVertex &&
            type->is_integer_scalar_or_vector() &&
            ast::HasAttribute<ast::LocationAttribute>(attrs) &&
            !ast::HasAttribute<ast::InterpolateAttribute>(attrs)) {
            attrs.Push(ctx.dst->Interpolate(builtin::InterpolationType::kFlat,
                                            builtin::InterpolationSampling::kUndefined));
        }

        // In GLSL, if it's a builtin, override the name with the
        // corresponding gl_ builtin name
        if (cfg.shader_style == ShaderStyle::kGlsl) {
            if (builtin_attr != builtin::BuiltinValue::kUndefined) {
                name = GLSLBuiltinToString(builtin_attr, func_ast->PipelineStage(),
                                           builtin::AddressSpace::kOut);
                value = ToGLSLBuiltin(builtin_attr, value, type);
            }
        }

        OutputValue output;
        output.name = name;
        output.type = CreateASTTypeFor(ctx, type);
        output.attributes = std::move(attrs);
        output.value = value;
        output.location = location;
        wrapper_output_values.Push(output);
    }

    /// Process a non-struct parameter.
    /// This creates a new object for the shader input, moving the shader IO
    /// attributes to it. It also adds an expression to the list of parameters
    /// that will be passed to the original function.
    /// @param param the original function parameter
    void ProcessNonStructParameter(const sem::Parameter* param) {
        // Do not add interpolation attributes on vertex input
        bool do_interpolate = func_ast->PipelineStage() != ast::PipelineStage::kVertex;
        // Remove the shader IO attributes from the inner function parameter, and attach them to the
        // new object instead.
        utils::Vector<const ast::Attribute*, 8> attributes;
        for (auto* attr : param->Declaration()->attributes) {
            if (IsShaderIOAttribute(attr)) {
                ctx.Remove(param->Declaration()->attributes, attr);
                if ((do_interpolate || !attr->Is<ast::InterpolateAttribute>())) {
                    CloneAttribute(attr, attributes);
                }
            }
        }

        auto name = param->Declaration()->name->symbol.Name();
        auto* input_expr = AddInput(name, param->Type(), param->Location(), std::move(attributes));
        inner_call_parameters.Push(input_expr);
    }

    /// Process a struct parameter.
    /// This creates new objects for each struct member, moving the shader IO
    /// attributes to them. It also creates the structure that will be passed to
    /// the original function.
    /// @param param the original function parameter
    void ProcessStructParameter(const sem::Parameter* param) {
        // Do not add interpolation attributes on vertex input
        bool do_interpolate = func_ast->PipelineStage() != ast::PipelineStage::kVertex;

        auto* str = param->Type()->As<sem::Struct>();

        // Recreate struct members in the outer entry point and build an initializer
        // list to pass them through to the inner function.
        utils::Vector<const ast::Expression*, 8> inner_struct_values;
        for (auto* member : str->Members()) {
            if (TINT_UNLIKELY(member->Type()->Is<type::Struct>())) {
                TINT_ICE(Transform, ctx.dst->Diagnostics()) << "nested IO struct";
                continue;
            }

            auto name = member->Name().Name();

            auto attributes =
                CloneShaderIOAttributes(member->Declaration()->attributes, do_interpolate);
            auto* input_expr = AddInput(name, member->Type(), member->Attributes().location,
                                        std::move(attributes));
            inner_struct_values.Push(input_expr);
        }

        // Construct the original structure using the new shader input objects.
        inner_call_parameters.Push(
            ctx.dst->Call(ctx.Clone(param->Declaration()->type), inner_struct_values));
    }

    /// Process the entry point return type.
    /// This generates a list of output values that are returned by the original
    /// function.
    /// @param inner_ret_type the original function return type
    /// @param original_result the result object produced by the original function
    void ProcessReturnType(const type::Type* inner_ret_type, Symbol original_result) {
        // Do not add interpolation attributes on fragment output
        bool do_interpolate = func_ast->PipelineStage() != ast::PipelineStage::kFragment;
        if (auto* str = inner_ret_type->As<sem::Struct>()) {
            for (auto* member : str->Members()) {
                if (TINT_UNLIKELY(member->Type()->Is<type::Struct>())) {
                    TINT_ICE(Transform, ctx.dst->Diagnostics()) << "nested IO struct";
                    continue;
                }

                auto name = member->Name().Name();
                auto attributes =
                    CloneShaderIOAttributes(member->Declaration()->attributes, do_interpolate);

                // Extract the original structure member.
                AddOutput(name, member->Type(), member->Attributes().location,
                          std::move(attributes), ctx.dst->MemberAccessor(original_result, name));
            }
        } else if (!inner_ret_type->Is<type::Void>()) {
            auto attributes =
                CloneShaderIOAttributes(func_ast->return_type_attributes, do_interpolate);

            // Propagate the non-struct return value as is.
            AddOutput("value", func_sem->ReturnType(), func_sem->ReturnLocation(),
                      std::move(attributes), ctx.dst->Expr(original_result));
        }
    }

    /// Add a fixed sample mask to the wrapper function output.
    /// If there is already a sample mask, bitwise-and it with the fixed mask.
    /// Otherwise, create a new output value from the fixed mask.
    void AddFixedSampleMask() {
        // Check the existing output values for a sample mask builtin.
        for (auto& outval : wrapper_output_values) {
            if (BuiltinOf(outval.attributes) == builtin::BuiltinValue::kSampleMask) {
                // Combine the authored sample mask with the fixed mask.
                outval.value = ctx.dst->And(outval.value, u32(cfg.fixed_sample_mask));
                return;
            }
        }

        // No existing sample mask builtin was found, so create a new output value using the fixed
        // sample mask.
        auto* builtin = ctx.dst->Builtin(builtin::BuiltinValue::kSampleMask);
        builtin_attrs.Add(builtin, builtin::BuiltinValue::kSampleMask);
        AddOutput("fixed_sample_mask", ctx.dst->create<type::U32>(), std::nullopt, {builtin},
                  ctx.dst->Expr(u32(cfg.fixed_sample_mask)));
    }

    /// Add a point size builtin to the wrapper function output.
    void AddVertexPointSize() {
        // Create a new output value and assign it a literal 1.0 value.
        auto* builtin = ctx.dst->Builtin(builtin::BuiltinValue::kPointSize);
        builtin_attrs.Add(builtin, builtin::BuiltinValue::kPointSize);
        AddOutput("vertex_point_size", ctx.dst->create<type::F32>(), std::nullopt, {builtin},
                  ctx.dst->Expr(1_f));
    }

    /// Create an expression for gl_Position.[component]
    /// @param component the component of gl_Position to access
    /// @returns the new expression
    const ast::Expression* GLPosition(const char* component) {
        Symbol pos = ctx.dst->Symbols().Register("gl_Position");
        Symbol c = ctx.dst->Symbols().Register(component);
        return ctx.dst->MemberAccessor(ctx.dst->Expr(pos), c);
    }

    /// Comparison function used to reorder struct members such that all members with
    /// location attributes appear first (ordered by location slot), followed by
    /// those with builtin attributes.
    /// @param a a struct member
    /// @param b another struct member
    /// @returns true if a comes before b
    bool StructMemberComparator(const MemberInfo& a, const MemberInfo& b) {
        auto* a_loc = ast::GetAttribute<ast::LocationAttribute>(a.member->attributes);
        auto* b_loc = ast::GetAttribute<ast::LocationAttribute>(b.member->attributes);
        auto* a_blt = ast::GetAttribute<ast::BuiltinAttribute>(a.member->attributes);
        auto* b_blt = ast::GetAttribute<ast::BuiltinAttribute>(b.member->attributes);
        if (a_loc) {
            if (!b_loc) {
                // `a` has location attribute and `b` does not: `a` goes first.
                return true;
            }
            // Both have location attributes: smallest goes first.
            return a.location < b.location;
        } else {
            if (b_loc) {
                // `b` has location attribute and `a` does not: `b` goes first.
                return false;
            }
            // Both are builtins: order matters for FXC.
            auto builtin_a = BuiltinOf(a_blt);
            auto builtin_b = BuiltinOf(b_blt);
            return BuiltinOrder(builtin_a) < BuiltinOrder(builtin_b);
        }
    }
    /// Create the wrapper function's struct parameter and type objects.
    void CreateInputStruct() {
        // Sort the struct members to satisfy HLSL interfacing matching rules.
        std::sort(wrapper_struct_param_members.begin(), wrapper_struct_param_members.end(),
                  [&](auto& a, auto& b) { return StructMemberComparator(a, b); });

        utils::Vector<const ast::StructMember*, 8> members;
        for (auto& mem : wrapper_struct_param_members) {
            members.Push(mem.member);
        }

        // Create the new struct type.
        auto struct_name = ctx.dst->Sym();
        auto* in_struct = ctx.dst->create<ast::Struct>(ctx.dst->Ident(struct_name),
                                                       std::move(members), utils::Empty);
        ctx.InsertBefore(ctx.src->AST().GlobalDeclarations(), func_ast, in_struct);

        // Create a new function parameter using this struct type.
        auto* param = ctx.dst->Param(InputStructSymbol(), ctx.dst->ty(struct_name));
        wrapper_ep_parameters.Push(param);
    }

    /// Create and return the wrapper function's struct result object.
    /// @returns the struct type
    ast::Struct* CreateOutputStruct() {
        utils::Vector<const ast::Statement*, 8> assignments;

        auto wrapper_result = ctx.dst->Symbols().New("wrapper_result");

        // Create the struct members and their corresponding assignment statements.
        std::unordered_set<std::string> member_names;
        for (auto& outval : wrapper_output_values) {
            // Use the original output name, unless that is already taken.
            Symbol name;
            if (member_names.count(outval.name)) {
                name = ctx.dst->Symbols().New(outval.name);
            } else {
                name = ctx.dst->Symbols().Register(outval.name);
            }
            member_names.insert(name.Name());

            wrapper_struct_output_members.Push({
                ctx.dst->Member(name, outval.type, std::move(outval.attributes)),
                outval.location,
            });
            assignments.Push(
                ctx.dst->Assign(ctx.dst->MemberAccessor(wrapper_result, name), outval.value));
        }

        // Sort the struct members to satisfy HLSL interfacing matching rules.
        std::sort(wrapper_struct_output_members.begin(), wrapper_struct_output_members.end(),
                  [&](auto& a, auto& b) { return StructMemberComparator(a, b); });

        utils::Vector<const ast::StructMember*, 8> members;
        for (auto& mem : wrapper_struct_output_members) {
            members.Push(mem.member);
        }

        // Create the new struct type.
        auto* out_struct = ctx.dst->create<ast::Struct>(ctx.dst->Ident(ctx.dst->Sym()),
                                                        std::move(members), utils::Empty);
        ctx.InsertBefore(ctx.src->AST().GlobalDeclarations(), func_ast, out_struct);

        // Create the output struct object, assign its members, and return it.
        auto* result_object = ctx.dst->Var(wrapper_result, ctx.dst->ty(out_struct->name->symbol));
        wrapper_body.Push(ctx.dst->Decl(result_object));
        for (auto* assignment : assignments) {
            wrapper_body.Push(assignment);
        }
        wrapper_body.Push(ctx.dst->Return(wrapper_result));

        return out_struct;
    }

    /// Create and assign the wrapper function's output variables.
    void CreateGlobalOutputVariables() {
        for (auto& outval : wrapper_output_values) {
            // Disable validation for use of the `output` address space.
            auto attributes = std::move(outval.attributes);
            attributes.Push(ctx.dst->Disable(ast::DisabledValidation::kIgnoreAddressSpace));

            // Create the global variable and assign it the output value.
            auto name = ctx.dst->Symbols().New(outval.name);
            ast::Type type = outval.type;
            const ast::Expression* lhs = ctx.dst->Expr(name);
            if (BuiltinOf(attributes) == builtin::BuiltinValue::kSampleMask) {
                // Vulkan requires the type of a SampleMask builtin to be an array.
                // Declare it as array<u32, 1> and then store to the first element.
                type = ctx.dst->ty.array(type, 1_u);
                lhs = ctx.dst->IndexAccessor(lhs, 0_i);
            }
            ctx.dst->GlobalVar(name, type, builtin::AddressSpace::kOut, std::move(attributes));
            wrapper_body.Push(ctx.dst->Assign(lhs, outval.value));
        }
    }

    // Recreate the original function without entry point attributes and call it.
    /// @returns the inner function call expression
    const ast::CallExpression* CallInnerFunction() {
        Symbol inner_name;
        if (cfg.shader_style == ShaderStyle::kGlsl) {
            // In GLSL, clone the original entry point name, as the wrapper will be
            // called "main".
            inner_name = ctx.Clone(func_ast->name->symbol);
        } else {
            // Add a suffix to the function name, as the wrapper function will take
            // the original entry point name.
            auto ep_name = func_ast->name->symbol.Name();
            inner_name = ctx.dst->Symbols().New(ep_name + "_inner");
        }

        // Clone everything, dropping the function and return type attributes.
        // The parameter attributes will have already been stripped during
        // processing.
        auto* inner_function =
            ctx.dst->create<ast::Function>(ctx.dst->Ident(inner_name), ctx.Clone(func_ast->params),
                                           ctx.Clone(func_ast->return_type),
                                           ctx.Clone(func_ast->body), utils::Empty, utils::Empty);
        ctx.Replace(func_ast, inner_function);

        // Call the function.
        return ctx.dst->Call(inner_function->name->symbol, inner_call_parameters);
    }

    /// Process the entry point function.
    void Process() {
        bool needs_fixed_sample_mask = false;
        bool needs_vertex_point_size = false;
        if (func_ast->PipelineStage() == ast::PipelineStage::kFragment &&
            cfg.fixed_sample_mask != 0xFFFFFFFF) {
            needs_fixed_sample_mask = true;
        }
        if (func_ast->PipelineStage() == ast::PipelineStage::kVertex &&
            cfg.emit_vertex_point_size) {
            needs_vertex_point_size = true;
        }

        // Exit early if there is no shader IO to handle.
        if (func_sem->Parameters().Length() == 0 && func_sem->ReturnType()->Is<type::Void>() &&
            !needs_fixed_sample_mask && !needs_vertex_point_size &&
            cfg.shader_style != ShaderStyle::kGlsl) {
            return;
        }

        // Process the entry point parameters, collecting those that need to be
        // aggregated into a single structure.
        if (!func_sem->Parameters().IsEmpty()) {
            for (auto* param : func_sem->Parameters()) {
                if (param->Type()->Is<type::Struct>()) {
                    ProcessStructParameter(param);
                } else {
                    ProcessNonStructParameter(param);
                }
            }

            // Create a structure parameter for the outer entry point if necessary.
            if (!wrapper_struct_param_members.IsEmpty()) {
                CreateInputStruct();
            }
        }

        // Recreate the original function and call it.
        auto* call_inner = CallInnerFunction();

        // Process the return type, and start building the wrapper function body.
        std::function<ast::Type()> wrapper_ret_type = [&] { return ctx.dst->ty.void_(); };
        if (func_sem->ReturnType()->Is<type::Void>()) {
            // The function call is just a statement with no result.
            wrapper_body.Push(ctx.dst->CallStmt(call_inner));
        } else {
            // Capture the result of calling the original function.
            auto* inner_result = ctx.dst->Let(ctx.dst->Symbols().New("inner_result"), call_inner);
            wrapper_body.Push(ctx.dst->Decl(inner_result));

            // Process the original return type to determine the outputs that the
            // outer function needs to produce.
            ProcessReturnType(func_sem->ReturnType(), inner_result->name->symbol);
        }

        // Add a fixed sample mask, if necessary.
        if (needs_fixed_sample_mask) {
            AddFixedSampleMask();
        }

        // Add the pointsize builtin, if necessary.
        if (needs_vertex_point_size) {
            AddVertexPointSize();
        }

        // Produce the entry point outputs, if necessary.
        if (!wrapper_output_values.IsEmpty()) {
            if (cfg.shader_style == ShaderStyle::kSpirv || cfg.shader_style == ShaderStyle::kGlsl) {
                CreateGlobalOutputVariables();
            } else {
                auto* output_struct = CreateOutputStruct();
                wrapper_ret_type = [&, output_struct] {
                    return ctx.dst->ty(output_struct->name->symbol);
                };
            }
        }

        if (cfg.shader_style == ShaderStyle::kGlsl &&
            func_ast->PipelineStage() == ast::PipelineStage::kVertex) {
            auto* pos_y = GLPosition("y");
            auto* negate_pos_y =
                ctx.dst->create<ast::UnaryOpExpression>(ast::UnaryOp::kNegation, GLPosition("y"));
            wrapper_body.Push(ctx.dst->Assign(pos_y, negate_pos_y));

            auto* two_z = ctx.dst->Mul(ctx.dst->Expr(2_f), GLPosition("z"));
            auto* fixed_z = ctx.dst->Sub(two_z, GLPosition("w"));
            wrapper_body.Push(ctx.dst->Assign(GLPosition("z"), fixed_z));
        }

        // Create the wrapper entry point function.
        // For GLSL, use "main", otherwise take the name of the original
        // entry point function.
        Symbol name;
        if (cfg.shader_style == ShaderStyle::kGlsl) {
            name = ctx.dst->Symbols().New("main");
        } else {
            name = ctx.Clone(func_ast->name->symbol);
        }

        auto* wrapper_func = ctx.dst->create<ast::Function>(
            ctx.dst->Ident(name), wrapper_ep_parameters, ctx.dst->ty(wrapper_ret_type()),
            ctx.dst->Block(wrapper_body), ctx.Clone(func_ast->attributes), utils::Empty);
        ctx.InsertAfter(ctx.src->AST().GlobalDeclarations(), func_ast, wrapper_func);
    }

    /// Retrieve the gl_ string corresponding to a builtin.
    /// @param builtin the builtin
    /// @param stage the current pipeline stage
    /// @param address_space the address space (input or output)
    /// @returns the gl_ string corresponding to that builtin
    const char* GLSLBuiltinToString(builtin::BuiltinValue builtin,
                                    ast::PipelineStage stage,
                                    builtin::AddressSpace address_space) {
        switch (builtin) {
            case builtin::BuiltinValue::kPosition:
                switch (stage) {
                    case ast::PipelineStage::kVertex:
                        return "gl_Position";
                    case ast::PipelineStage::kFragment:
                        return "gl_FragCoord";
                    default:
                        return "";
                }
            case builtin::BuiltinValue::kVertexIndex:
                return "gl_VertexID";
            case builtin::BuiltinValue::kInstanceIndex:
                return "gl_InstanceID";
            case builtin::BuiltinValue::kFrontFacing:
                return "gl_FrontFacing";
            case builtin::BuiltinValue::kFragDepth:
                return "gl_FragDepth";
            case builtin::BuiltinValue::kLocalInvocationId:
                return "gl_LocalInvocationID";
            case builtin::BuiltinValue::kLocalInvocationIndex:
                return "gl_LocalInvocationIndex";
            case builtin::BuiltinValue::kGlobalInvocationId:
                return "gl_GlobalInvocationID";
            case builtin::BuiltinValue::kNumWorkgroups:
                return "gl_NumWorkGroups";
            case builtin::BuiltinValue::kWorkgroupId:
                return "gl_WorkGroupID";
            case builtin::BuiltinValue::kSampleIndex:
                return "gl_SampleID";
            case builtin::BuiltinValue::kSampleMask:
                if (address_space == builtin::AddressSpace::kIn) {
                    return "gl_SampleMaskIn";
                } else {
                    return "gl_SampleMask";
                }
            default:
                return "";
        }
    }

    /// Convert a given GLSL builtin value to the corresponding WGSL value.
    /// @param builtin the builtin variable
    /// @param value the value to convert
    /// @param ast_type (inout) the incoming WGSL and outgoing GLSL types
    /// @returns an expression representing the GLSL builtin converted to what
    /// WGSL expects
    const ast::Expression* FromGLSLBuiltin(builtin::BuiltinValue builtin,
                                           const ast::Expression* value,
                                           ast::Type& ast_type) {
        switch (builtin) {
            case builtin::BuiltinValue::kVertexIndex:
            case builtin::BuiltinValue::kInstanceIndex:
            case builtin::BuiltinValue::kSampleIndex:
                // GLSL uses i32 for these, so bitcast to u32.
                value = ctx.dst->Bitcast(ast_type, value);
                ast_type = ctx.dst->ty.i32();
                break;
            case builtin::BuiltinValue::kSampleMask:
                // gl_SampleMask is an array of i32. Retrieve the first element and
                // bitcast it to u32.
                value = ctx.dst->IndexAccessor(value, 0_i);
                value = ctx.dst->Bitcast(ast_type, value);
                ast_type = ctx.dst->ty.array(ctx.dst->ty.i32(), 1_u);
                break;
            default:
                break;
        }
        return value;
    }

    /// Convert a given WGSL value to the type expected when assigning to a
    /// GLSL builtin.
    /// @param builtin the builtin variable
    /// @param value the value to convert
    /// @param type (out) the type to which the value was converted
    /// @returns the converted value which can be assigned to the GLSL builtin
    const ast::Expression* ToGLSLBuiltin(builtin::BuiltinValue builtin,
                                         const ast::Expression* value,
                                         const type::Type*& type) {
        switch (builtin) {
            case builtin::BuiltinValue::kVertexIndex:
            case builtin::BuiltinValue::kInstanceIndex:
            case builtin::BuiltinValue::kSampleIndex:
            case builtin::BuiltinValue::kSampleMask:
                type = ctx.dst->create<type::I32>();
                value = ctx.dst->Bitcast(CreateASTTypeFor(ctx, type), value);
                break;
            default:
                break;
        }
        return value;
    }
};

Transform::ApplyResult CanonicalizeEntryPointIO::Apply(const Program* src,
                                                       const DataMap& inputs,
                                                       DataMap&) const {
    ProgramBuilder b;
    CloneContext ctx{&b, src, /* auto_clone_symbols */ true};

    auto* cfg = inputs.Get<Config>();
    if (cfg == nullptr) {
        b.Diagnostics().add_error(diag::System::Transform,
                                  "missing transform data for " + std::string(TypeInfo().name));
        return Program(std::move(b));
    }

    // Remove entry point IO attributes from struct declarations.
    // New structures will be created for each entry point, as necessary.
    for (auto* ty : src->AST().TypeDecls()) {
        if (auto* struct_ty = ty->As<ast::Struct>()) {
            for (auto* member : struct_ty->members) {
                for (auto* attr : member->attributes) {
                    if (IsShaderIOAttribute(attr)) {
                        ctx.Remove(member->attributes, attr);
                    }
                }
            }
        }
    }

    for (auto* func_ast : src->AST().Functions()) {
        if (!func_ast->IsEntryPoint()) {
            continue;
        }

        State state(ctx, *cfg, func_ast);
        state.Process();
    }

    ctx.Clone();
    return Program(std::move(b));
}

CanonicalizeEntryPointIO::Config::Config(ShaderStyle style,
                                         uint32_t sample_mask,
                                         bool emit_point_size)
    : shader_style(style),
      fixed_sample_mask(sample_mask),
      emit_vertex_point_size(emit_point_size) {}

CanonicalizeEntryPointIO::Config::Config(const Config&) = default;
CanonicalizeEntryPointIO::Config::~Config() = default;

}  // namespace tint::transform
