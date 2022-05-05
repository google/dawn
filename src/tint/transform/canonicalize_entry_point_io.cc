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

// Comparison function used to reorder struct members such that all members with
// location attributes appear first (ordered by location slot), followed by
// those with builtin attributes.
bool StructMemberComparator(const ast::StructMember* a, const ast::StructMember* b) {
    auto* a_loc = ast::GetAttribute<ast::LocationAttribute>(a->attributes);
    auto* b_loc = ast::GetAttribute<ast::LocationAttribute>(b->attributes);
    auto* a_blt = ast::GetAttribute<ast::BuiltinAttribute>(a->attributes);
    auto* b_blt = ast::GetAttribute<ast::BuiltinAttribute>(b->attributes);
    if (a_loc) {
        if (!b_loc) {
            // `a` has location attribute and `b` does not: `a` goes first.
            return true;
        }
        // Both have location attributes: smallest goes first.
        return a_loc->value < b_loc->value;
    } else {
        if (b_loc) {
            // `b` has location attribute and `a` does not: `b` goes first.
            return false;
        }
        // Both are builtins: order doesn't matter, just use enum value.
        return a_blt->builtin < b_blt->builtin;
    }
}

// Returns true if `attr` is a shader IO attribute.
bool IsShaderIOAttribute(const ast::Attribute* attr) {
    return attr->IsAnyOf<ast::BuiltinAttribute, ast::InterpolateAttribute, ast::InvariantAttribute,
                         ast::LocationAttribute>();
}

// Returns true if `attrs` contains a `sample_mask` builtin.
bool HasSampleMask(const ast::AttributeList& attrs) {
    auto* builtin = ast::GetAttribute<ast::BuiltinAttribute>(attrs);
    return builtin && builtin->builtin == ast::Builtin::kSampleMask;
}

}  // namespace

/// State holds the current transform state for a single entry point.
struct CanonicalizeEntryPointIO::State {
    /// OutputValue represents a shader result that the wrapper function produces.
    struct OutputValue {
        /// The name of the output value.
        std::string name;
        /// The type of the output value.
        const ast::Type* type;
        /// The shader IO attributes.
        ast::AttributeList attributes;
        /// The value itself.
        const ast::Expression* value;
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
    ast::VariableList wrapper_ep_parameters;
    /// The members of the wrapper function's struct parameter.
    ast::StructMemberList wrapper_struct_param_members;
    /// The name of the wrapper function's struct parameter.
    Symbol wrapper_struct_param_name;
    /// The parameters that will be passed to the original function.
    ast::ExpressionList inner_call_parameters;
    /// The members of the wrapper function's struct return type.
    ast::StructMemberList wrapper_struct_output_members;
    /// The wrapper function output values.
    std::vector<OutputValue> wrapper_output_values;
    /// The body of the wrapper function.
    ast::StatementList wrapper_body;
    /// Input names used by the entrypoint
    std::unordered_set<std::string> input_names;

    /// Constructor
    /// @param context the clone context
    /// @param config the transform config
    /// @param function the entry point function
    State(CloneContext& context,
          const CanonicalizeEntryPointIO::Config& config,
          const ast::Function* function)
        : ctx(context), cfg(config), func_ast(function), func_sem(ctx.src->Sem().Get(function)) {}

    /// Clones the shader IO attributes from `src`.
    /// @param src the attributes to clone
    /// @param do_interpolate whether to clone InterpolateAttribute
    /// @return the cloned attributes
    ast::AttributeList CloneShaderIOAttributes(const ast::AttributeList& src, bool do_interpolate) {
        ast::AttributeList new_attributes;
        for (auto* attr : src) {
            if (IsShaderIOAttribute(attr) &&
                (do_interpolate || !attr->Is<ast::InterpolateAttribute>())) {
                new_attributes.push_back(ctx.Clone(attr));
            }
        }
        return new_attributes;
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
    /// @param attributes the attributes to apply to the shader input
    /// @returns an expression which evaluates to the value of the shader input
    const ast::Expression* AddInput(std::string name,
                                    const sem::Type* type,
                                    ast::AttributeList attributes) {
        auto* ast_type = CreateASTTypeFor(ctx, type);
        if (cfg.shader_style == ShaderStyle::kSpirv || cfg.shader_style == ShaderStyle::kGlsl) {
            // Vulkan requires that integer user-defined fragment inputs are
            // always decorated with `Flat`.
            // TODO(crbug.com/tint/1224): Remove this once a flat interpolation
            // attribute is required for integers.
            if (type->is_integer_scalar_or_vector() &&
                ast::HasAttribute<ast::LocationAttribute>(attributes) &&
                !ast::HasAttribute<ast::InterpolateAttribute>(attributes) &&
                func_ast->PipelineStage() == ast::PipelineStage::kFragment) {
                attributes.push_back(ctx.dst->Interpolate(ast::InterpolationType::kFlat,
                                                          ast::InterpolationSampling::kNone));
            }

            // Disable validation for use of the `input` storage class.
            attributes.push_back(ctx.dst->Disable(ast::DisabledValidation::kIgnoreStorageClass));

            // In GLSL, if it's a builtin, override the name with the
            // corresponding gl_ builtin name
            auto* builtin = ast::GetAttribute<ast::BuiltinAttribute>(attributes);
            if (cfg.shader_style == ShaderStyle::kGlsl && builtin) {
                name = GLSLBuiltinToString(builtin->builtin, func_ast->PipelineStage(),
                                           ast::StorageClass::kInput);
            }
            auto symbol = ctx.dst->Symbols().New(name);

            // Create the global variable and use its value for the shader input.
            const ast::Expression* value = ctx.dst->Expr(symbol);

            if (builtin) {
                if (cfg.shader_style == ShaderStyle::kGlsl) {
                    value = FromGLSLBuiltin(builtin->builtin, value, ast_type);
                } else if (builtin->builtin == ast::Builtin::kSampleMask) {
                    // Vulkan requires the type of a SampleMask builtin to be an array.
                    // Declare it as array<u32, 1> and then load the first element.
                    ast_type = ctx.dst->ty.array(ast_type, 1_u);
                    value = ctx.dst->IndexAccessor(value, 0_i);
                }
            }
            ctx.dst->Global(symbol, ast_type, ast::StorageClass::kInput, std::move(attributes));
            return value;
        } else if (cfg.shader_style == ShaderStyle::kMsl &&
                   ast::HasAttribute<ast::BuiltinAttribute>(attributes)) {
            // If this input is a builtin and we are targeting MSL, then add it to the
            // parameter list and pass it directly to the inner function.
            Symbol symbol = input_names.emplace(name).second ? ctx.dst->Symbols().Register(name)
                                                             : ctx.dst->Symbols().New(name);
            wrapper_ep_parameters.push_back(
                ctx.dst->Param(symbol, ast_type, std::move(attributes)));
            return ctx.dst->Expr(symbol);
        } else {
            // Otherwise, move it to the new structure member list.
            Symbol symbol = input_names.emplace(name).second ? ctx.dst->Symbols().Register(name)
                                                             : ctx.dst->Symbols().New(name);
            wrapper_struct_param_members.push_back(
                ctx.dst->Member(symbol, ast_type, std::move(attributes)));
            return ctx.dst->MemberAccessor(InputStructSymbol(), symbol);
        }
    }

    /// Add a shader output to the entry point.
    /// @param name the name of the shader output
    /// @param type the type of the shader output
    /// @param attributes the attributes to apply to the shader output
    /// @param value the value of the shader output
    void AddOutput(std::string name,
                   const sem::Type* type,
                   ast::AttributeList attributes,
                   const ast::Expression* value) {
        // Vulkan requires that integer user-defined vertex outputs are
        // always decorated with `Flat`.
        // TODO(crbug.com/tint/1224): Remove this once a flat interpolation
        // attribute is required for integers.
        if (cfg.shader_style == ShaderStyle::kSpirv && type->is_integer_scalar_or_vector() &&
            ast::HasAttribute<ast::LocationAttribute>(attributes) &&
            !ast::HasAttribute<ast::InterpolateAttribute>(attributes) &&
            func_ast->PipelineStage() == ast::PipelineStage::kVertex) {
            attributes.push_back(ctx.dst->Interpolate(ast::InterpolationType::kFlat,
                                                      ast::InterpolationSampling::kNone));
        }

        // In GLSL, if it's a builtin, override the name with the
        // corresponding gl_ builtin name
        if (cfg.shader_style == ShaderStyle::kGlsl) {
            if (auto* b = ast::GetAttribute<ast::BuiltinAttribute>(attributes)) {
                name = GLSLBuiltinToString(b->builtin, func_ast->PipelineStage(),
                                           ast::StorageClass::kOutput);
                value = ToGLSLBuiltin(b->builtin, value, type);
            }
        }

        OutputValue output;
        output.name = name;
        output.type = CreateASTTypeFor(ctx, type);
        output.attributes = std::move(attributes);
        output.value = value;
        wrapper_output_values.push_back(output);
    }

    /// Process a non-struct parameter.
    /// This creates a new object for the shader input, moving the shader IO
    /// attributes to it. It also adds an expression to the list of parameters
    /// that will be passed to the original function.
    /// @param param the original function parameter
    void ProcessNonStructParameter(const sem::Parameter* param) {
        // Remove the shader IO attributes from the inner function parameter, and
        // attach them to the new object instead.
        ast::AttributeList attributes;
        for (auto* attr : param->Declaration()->attributes) {
            if (IsShaderIOAttribute(attr)) {
                ctx.Remove(param->Declaration()->attributes, attr);
                attributes.push_back(ctx.Clone(attr));
            }
        }

        auto name = ctx.src->Symbols().NameFor(param->Declaration()->symbol);
        auto* input_expr = AddInput(name, param->Type(), std::move(attributes));
        inner_call_parameters.push_back(input_expr);
    }

    /// Process a struct parameter.
    /// This creates new objects for each struct member, moving the shader IO
    /// attributes to them. It also creates the structure that will be passed to
    /// the original function.
    /// @param param the original function parameter
    void ProcessStructParameter(const sem::Parameter* param) {
        auto* str = param->Type()->As<sem::Struct>();

        // Recreate struct members in the outer entry point and build an initializer
        // list to pass them through to the inner function.
        ast::ExpressionList inner_struct_values;
        for (auto* member : str->Members()) {
            if (member->Type()->Is<sem::Struct>()) {
                TINT_ICE(Transform, ctx.dst->Diagnostics()) << "nested IO struct";
                continue;
            }

            auto* member_ast = member->Declaration();
            auto name = ctx.src->Symbols().NameFor(member_ast->symbol);

            // In GLSL, do not add interpolation attributes on vertex input
            bool do_interpolate = true;
            if (cfg.shader_style == ShaderStyle::kGlsl &&
                func_ast->PipelineStage() == ast::PipelineStage::kVertex) {
                do_interpolate = false;
            }
            auto attributes = CloneShaderIOAttributes(member_ast->attributes, do_interpolate);
            auto* input_expr = AddInput(name, member->Type(), std::move(attributes));
            inner_struct_values.push_back(input_expr);
        }

        // Construct the original structure using the new shader input objects.
        inner_call_parameters.push_back(
            ctx.dst->Construct(ctx.Clone(param->Declaration()->type), inner_struct_values));
    }

    /// Process the entry point return type.
    /// This generates a list of output values that are returned by the original
    /// function.
    /// @param inner_ret_type the original function return type
    /// @param original_result the result object produced by the original function
    void ProcessReturnType(const sem::Type* inner_ret_type, Symbol original_result) {
        bool do_interpolate = true;
        // In GLSL, do not add interpolation attributes on fragment output
        if (cfg.shader_style == ShaderStyle::kGlsl &&
            func_ast->PipelineStage() == ast::PipelineStage::kFragment) {
            do_interpolate = false;
        }
        if (auto* str = inner_ret_type->As<sem::Struct>()) {
            for (auto* member : str->Members()) {
                if (member->Type()->Is<sem::Struct>()) {
                    TINT_ICE(Transform, ctx.dst->Diagnostics()) << "nested IO struct";
                    continue;
                }

                auto* member_ast = member->Declaration();
                auto name = ctx.src->Symbols().NameFor(member_ast->symbol);
                auto attributes = CloneShaderIOAttributes(member_ast->attributes, do_interpolate);

                // Extract the original structure member.
                AddOutput(name, member->Type(), std::move(attributes),
                          ctx.dst->MemberAccessor(original_result, name));
            }
        } else if (!inner_ret_type->Is<sem::Void>()) {
            auto attributes =
                CloneShaderIOAttributes(func_ast->return_type_attributes, do_interpolate);

            // Propagate the non-struct return value as is.
            AddOutput("value", func_sem->ReturnType(), std::move(attributes),
                      ctx.dst->Expr(original_result));
        }
    }

    /// Add a fixed sample mask to the wrapper function output.
    /// If there is already a sample mask, bitwise-and it with the fixed mask.
    /// Otherwise, create a new output value from the fixed mask.
    void AddFixedSampleMask() {
        // Check the existing output values for a sample mask builtin.
        for (auto& outval : wrapper_output_values) {
            if (HasSampleMask(outval.attributes)) {
                // Combine the authored sample mask with the fixed mask.
                outval.value = ctx.dst->And(outval.value, u32(cfg.fixed_sample_mask));
                return;
            }
        }

        // No existing sample mask builtin was found, so create a new output value
        // using the fixed sample mask.
        AddOutput("fixed_sample_mask", ctx.dst->create<sem::U32>(),
                  {ctx.dst->Builtin(ast::Builtin::kSampleMask)},
                  ctx.dst->Expr(u32(cfg.fixed_sample_mask)));
    }

    /// Add a point size builtin to the wrapper function output.
    void AddVertexPointSize() {
        // Create a new output value and assign it a literal 1.0 value.
        AddOutput("vertex_point_size", ctx.dst->create<sem::F32>(),
                  {ctx.dst->Builtin(ast::Builtin::kPointSize)}, ctx.dst->Expr(1.f));
    }

    /// Create an expression for gl_Position.[component]
    /// @param component the component of gl_Position to access
    /// @returns the new expression
    const ast::Expression* GLPosition(const char* component) {
        Symbol pos = ctx.dst->Symbols().Register("gl_Position");
        Symbol c = ctx.dst->Symbols().Register(component);
        return ctx.dst->MemberAccessor(ctx.dst->Expr(pos), ctx.dst->Expr(c));
    }

    /// Create the wrapper function's struct parameter and type objects.
    void CreateInputStruct() {
        // Sort the struct members to satisfy HLSL interfacing matching rules.
        std::sort(wrapper_struct_param_members.begin(), wrapper_struct_param_members.end(),
                  StructMemberComparator);

        // Create the new struct type.
        auto struct_name = ctx.dst->Sym();
        auto* in_struct = ctx.dst->create<ast::Struct>(struct_name, wrapper_struct_param_members,
                                                       ast::AttributeList{});
        ctx.InsertBefore(ctx.src->AST().GlobalDeclarations(), func_ast, in_struct);

        // Create a new function parameter using this struct type.
        auto* param = ctx.dst->Param(InputStructSymbol(), ctx.dst->ty.type_name(struct_name));
        wrapper_ep_parameters.push_back(param);
    }

    /// Create and return the wrapper function's struct result object.
    /// @returns the struct type
    ast::Struct* CreateOutputStruct() {
        ast::StatementList assignments;

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
            member_names.insert(ctx.dst->Symbols().NameFor(name));

            wrapper_struct_output_members.push_back(
                ctx.dst->Member(name, outval.type, std::move(outval.attributes)));
            assignments.push_back(
                ctx.dst->Assign(ctx.dst->MemberAccessor(wrapper_result, name), outval.value));
        }

        // Sort the struct members to satisfy HLSL interfacing matching rules.
        std::sort(wrapper_struct_output_members.begin(), wrapper_struct_output_members.end(),
                  StructMemberComparator);

        // Create the new struct type.
        auto* out_struct = ctx.dst->create<ast::Struct>(
            ctx.dst->Sym(), wrapper_struct_output_members, ast::AttributeList{});
        ctx.InsertBefore(ctx.src->AST().GlobalDeclarations(), func_ast, out_struct);

        // Create the output struct object, assign its members, and return it.
        auto* result_object = ctx.dst->Var(wrapper_result, ctx.dst->ty.type_name(out_struct->name));
        wrapper_body.push_back(ctx.dst->Decl(result_object));
        wrapper_body.insert(wrapper_body.end(), assignments.begin(), assignments.end());
        wrapper_body.push_back(ctx.dst->Return(wrapper_result));

        return out_struct;
    }

    /// Create and assign the wrapper function's output variables.
    void CreateGlobalOutputVariables() {
        for (auto& outval : wrapper_output_values) {
            // Disable validation for use of the `output` storage class.
            ast::AttributeList attributes = std::move(outval.attributes);
            attributes.push_back(ctx.dst->Disable(ast::DisabledValidation::kIgnoreStorageClass));

            // Create the global variable and assign it the output value.
            auto name = ctx.dst->Symbols().New(outval.name);
            auto* type = outval.type;
            const ast::Expression* lhs = ctx.dst->Expr(name);
            if (HasSampleMask(attributes)) {
                // Vulkan requires the type of a SampleMask builtin to be an array.
                // Declare it as array<u32, 1> and then store to the first element.
                type = ctx.dst->ty.array(type, 1_u);
                lhs = ctx.dst->IndexAccessor(lhs, 0_i);
            }
            ctx.dst->Global(name, type, ast::StorageClass::kOutput, std::move(attributes));
            wrapper_body.push_back(ctx.dst->Assign(lhs, outval.value));
        }
    }

    // Recreate the original function without entry point attributes and call it.
    /// @returns the inner function call expression
    const ast::CallExpression* CallInnerFunction() {
        Symbol inner_name;
        if (cfg.shader_style == ShaderStyle::kGlsl) {
            // In GLSL, clone the original entry point name, as the wrapper will be
            // called "main".
            inner_name = ctx.Clone(func_ast->symbol);
        } else {
            // Add a suffix to the function name, as the wrapper function will take
            // the original entry point name.
            auto ep_name = ctx.src->Symbols().NameFor(func_ast->symbol);
            inner_name = ctx.dst->Symbols().New(ep_name + "_inner");
        }

        // Clone everything, dropping the function and return type attributes.
        // The parameter attributes will have already been stripped during
        // processing.
        auto* inner_function = ctx.dst->create<ast::Function>(
            inner_name, ctx.Clone(func_ast->params), ctx.Clone(func_ast->return_type),
            ctx.Clone(func_ast->body), ast::AttributeList{}, ast::AttributeList{});
        ctx.Replace(func_ast, inner_function);

        // Call the function.
        return ctx.dst->Call(inner_function->symbol, inner_call_parameters);
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
        if (func_sem->Parameters().size() == 0 && func_sem->ReturnType()->Is<sem::Void>() &&
            !needs_fixed_sample_mask && !needs_vertex_point_size &&
            cfg.shader_style != ShaderStyle::kGlsl) {
            return;
        }

        // Process the entry point parameters, collecting those that need to be
        // aggregated into a single structure.
        if (!func_sem->Parameters().empty()) {
            for (auto* param : func_sem->Parameters()) {
                if (param->Type()->Is<sem::Struct>()) {
                    ProcessStructParameter(param);
                } else {
                    ProcessNonStructParameter(param);
                }
            }

            // Create a structure parameter for the outer entry point if necessary.
            if (!wrapper_struct_param_members.empty()) {
                CreateInputStruct();
            }
        }

        // Recreate the original function and call it.
        auto* call_inner = CallInnerFunction();

        // Process the return type, and start building the wrapper function body.
        std::function<const ast::Type*()> wrapper_ret_type = [&] { return ctx.dst->ty.void_(); };
        if (func_sem->ReturnType()->Is<sem::Void>()) {
            // The function call is just a statement with no result.
            wrapper_body.push_back(ctx.dst->CallStmt(call_inner));
        } else {
            // Capture the result of calling the original function.
            auto* inner_result =
                ctx.dst->Let(ctx.dst->Symbols().New("inner_result"), nullptr, call_inner);
            wrapper_body.push_back(ctx.dst->Decl(inner_result));

            // Process the original return type to determine the outputs that the
            // outer function needs to produce.
            ProcessReturnType(func_sem->ReturnType(), inner_result->symbol);
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
        if (!wrapper_output_values.empty()) {
            if (cfg.shader_style == ShaderStyle::kSpirv || cfg.shader_style == ShaderStyle::kGlsl) {
                CreateGlobalOutputVariables();
            } else {
                auto* output_struct = CreateOutputStruct();
                wrapper_ret_type = [&, output_struct] {
                    return ctx.dst->ty.type_name(output_struct->name);
                };
            }
        }

        if (cfg.shader_style == ShaderStyle::kGlsl &&
            func_ast->PipelineStage() == ast::PipelineStage::kVertex) {
            auto* pos_y = GLPosition("y");
            auto* negate_pos_y =
                ctx.dst->create<ast::UnaryOpExpression>(ast::UnaryOp::kNegation, GLPosition("y"));
            wrapper_body.push_back(ctx.dst->Assign(pos_y, negate_pos_y));

            auto* two_z = ctx.dst->Mul(ctx.dst->Expr(2.0f), GLPosition("z"));
            auto* fixed_z = ctx.dst->Sub(two_z, GLPosition("w"));
            wrapper_body.push_back(ctx.dst->Assign(GLPosition("z"), fixed_z));
        }

        // Create the wrapper entry point function.
        // For GLSL, use "main", otherwise take the name of the original
        // entry point function.
        Symbol name;
        if (cfg.shader_style == ShaderStyle::kGlsl) {
            name = ctx.dst->Symbols().New("main");
        } else {
            name = ctx.Clone(func_ast->symbol);
        }

        auto* wrapper_func = ctx.dst->create<ast::Function>(
            name, wrapper_ep_parameters, wrapper_ret_type(), ctx.dst->Block(wrapper_body),
            ctx.Clone(func_ast->attributes), ast::AttributeList{});
        ctx.InsertAfter(ctx.src->AST().GlobalDeclarations(), func_ast, wrapper_func);
    }

    /// Retrieve the gl_ string corresponding to a builtin.
    /// @param builtin the builtin
    /// @param stage the current pipeline stage
    /// @param storage_class the storage class (input or output)
    /// @returns the gl_ string corresponding to that builtin
    const char* GLSLBuiltinToString(ast::Builtin builtin,
                                    ast::PipelineStage stage,
                                    ast::StorageClass storage_class) {
        switch (builtin) {
            case ast::Builtin::kPosition:
                switch (stage) {
                    case ast::PipelineStage::kVertex:
                        return "gl_Position";
                    case ast::PipelineStage::kFragment:
                        return "gl_FragCoord";
                    default:
                        return "";
                }
            case ast::Builtin::kVertexIndex:
                return "gl_VertexID";
            case ast::Builtin::kInstanceIndex:
                return "gl_InstanceID";
            case ast::Builtin::kFrontFacing:
                return "gl_FrontFacing";
            case ast::Builtin::kFragDepth:
                return "gl_FragDepth";
            case ast::Builtin::kLocalInvocationId:
                return "gl_LocalInvocationID";
            case ast::Builtin::kLocalInvocationIndex:
                return "gl_LocalInvocationIndex";
            case ast::Builtin::kGlobalInvocationId:
                return "gl_GlobalInvocationID";
            case ast::Builtin::kNumWorkgroups:
                return "gl_NumWorkGroups";
            case ast::Builtin::kWorkgroupId:
                return "gl_WorkGroupID";
            case ast::Builtin::kSampleIndex:
                return "gl_SampleID";
            case ast::Builtin::kSampleMask:
                if (storage_class == ast::StorageClass::kInput) {
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
    const ast::Expression* FromGLSLBuiltin(ast::Builtin builtin,
                                           const ast::Expression* value,
                                           const ast::Type*& ast_type) {
        switch (builtin) {
            case ast::Builtin::kVertexIndex:
            case ast::Builtin::kInstanceIndex:
            case ast::Builtin::kSampleIndex:
                // GLSL uses i32 for these, so bitcast to u32.
                value = ctx.dst->Bitcast(ast_type, value);
                ast_type = ctx.dst->ty.i32();
                break;
            case ast::Builtin::kSampleMask:
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
    const ast::Expression* ToGLSLBuiltin(ast::Builtin builtin,
                                         const ast::Expression* value,
                                         const sem::Type*& type) {
        switch (builtin) {
            case ast::Builtin::kVertexIndex:
            case ast::Builtin::kInstanceIndex:
            case ast::Builtin::kSampleIndex:
            case ast::Builtin::kSampleMask:
                type = ctx.dst->create<sem::I32>();
                value = ctx.dst->Bitcast(CreateASTTypeFor(ctx, type), value);
                break;
            default:
                break;
        }
        return value;
    }
};

void CanonicalizeEntryPointIO::Run(CloneContext& ctx, const DataMap& inputs, DataMap&) const {
    auto* cfg = inputs.Get<Config>();
    if (cfg == nullptr) {
        ctx.dst->Diagnostics().add_error(
            diag::System::Transform, "missing transform data for " + std::string(TypeInfo().name));
        return;
    }

    // Remove entry point IO attributes from struct declarations.
    // New structures will be created for each entry point, as necessary.
    for (auto* ty : ctx.src->AST().TypeDecls()) {
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

    for (auto* func_ast : ctx.src->AST().Functions()) {
        if (!func_ast->IsEntryPoint()) {
            continue;
        }

        State state(ctx, *cfg, func_ast);
        state.Process();
    }

    ctx.Clone();
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
