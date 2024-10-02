/// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/lang/glsl/writer/ast_printer/ast_printer.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <set>
#include <utility>
#include <vector>

#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/constant/value.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/glsl/writer/ast_raise/combine_samplers.h"
#include "src/tint/lang/glsl/writer/ast_raise/pad_structs.h"
#include "src/tint/lang/glsl/writer/ast_raise/texture_1d_to_2d.h"
#include "src/tint/lang/glsl/writer/ast_raise/texture_builtins_from_uniform.h"
#include "src/tint/lang/glsl/writer/common/option_helpers.h"
#include "src/tint/lang/glsl/writer/common/options.h"
#include "src/tint/lang/glsl/writer/common/printer_support.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/id_attribute.h"
#include "src/tint/lang/wgsl/ast/internal_attribute.h"
#include "src/tint/lang/wgsl/ast/interpolate_attribute.h"
#include "src/tint/lang/wgsl/ast/transform/add_block_attribute.h"
#include "src/tint/lang/wgsl/ast/transform/add_empty_entry_point.h"
#include "src/tint/lang/wgsl/ast/transform/binding_remapper.h"
#include "src/tint/lang/wgsl/ast/transform/builtin_polyfill.h"
#include "src/tint/lang/wgsl/ast/transform/canonicalize_entry_point_io.h"
#include "src/tint/lang/wgsl/ast/transform/clamp_frag_depth.h"
#include "src/tint/lang/wgsl/ast/transform/demote_to_helper.h"
#include "src/tint/lang/wgsl/ast/transform/direct_variable_access.h"
#include "src/tint/lang/wgsl/ast/transform/disable_uniformity_analysis.h"
#include "src/tint/lang/wgsl/ast/transform/expand_compound_assignment.h"
#include "src/tint/lang/wgsl/ast/transform/fold_constants.h"
#include "src/tint/lang/wgsl/ast/transform/manager.h"
#include "src/tint/lang/wgsl/ast/transform/multiplanar_external_texture.h"
#include "src/tint/lang/wgsl/ast/transform/offset_first_index.h"
#include "src/tint/lang/wgsl/ast/transform/preserve_padding.h"
#include "src/tint/lang/wgsl/ast/transform/promote_initializers_to_let.h"
#include "src/tint/lang/wgsl/ast/transform/promote_side_effects_to_decl.h"
#include "src/tint/lang/wgsl/ast/transform/remove_continue_in_switch.h"
#include "src/tint/lang/wgsl/ast/transform/remove_phonies.h"
#include "src/tint/lang/wgsl/ast/transform/renamer.h"
#include "src/tint/lang/wgsl/ast/transform/robustness.h"
#include "src/tint/lang/wgsl/ast/transform/simplify_pointers.h"
#include "src/tint/lang/wgsl/ast/transform/single_entry_point.h"
#include "src/tint/lang/wgsl/ast/transform/std140.h"
#include "src/tint/lang/wgsl/ast/transform/unshadow.h"
#include "src/tint/lang/wgsl/ast/transform/zero_init_workgroup_memory.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/helpers/append_vector.h"
#include "src/tint/lang/wgsl/helpers/check_supported_extensions.h"
#include "src/tint/lang/wgsl/sem/block_statement.h"
#include "src/tint/lang/wgsl/sem/builtin_enum_expression.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/module.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/lang/wgsl/sem/switch_statement.h"
#include "src/tint/lang/wgsl/sem/value_constructor.h"
#include "src/tint/lang/wgsl/sem/value_conversion.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/string_stream.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::glsl::writer {
namespace {

const char kTempNamePrefix[] = "tint_tmp";

bool last_is_break(const ast::BlockStatement* stmts) {
    return tint::IsAnyOf<ast::BreakStatement>(stmts->Last());
}

bool IsRelational(tint::core::BinaryOp op) {
    return op == tint::core::BinaryOp::kEqual || op == tint::core::BinaryOp::kNotEqual ||
           op == tint::core::BinaryOp::kLessThan || op == tint::core::BinaryOp::kGreaterThan ||
           op == tint::core::BinaryOp::kLessThanEqual ||
           op == tint::core::BinaryOp::kGreaterThanEqual;
}

bool RequiresOESSampleVariables(tint::core::BuiltinValue builtin) {
    switch (builtin) {
        case tint::core::BuiltinValue::kSampleIndex:
        case tint::core::BuiltinValue::kSampleMask:
            return true;
        default:
            return false;
    }
}

}  // namespace

SanitizedResult::SanitizedResult() = default;
SanitizedResult::~SanitizedResult() = default;
SanitizedResult::SanitizedResult(SanitizedResult&&) = default;

SanitizedResult Sanitize(const Program& in,
                         const Options& options,
                         const std::string& entry_point) {
    ast::transform::Manager manager;
    ast::transform::DataMap data;

    manager.Add<ast::transform::FoldConstants>();

    manager.Add<ast::transform::DisableUniformityAnalysis>();

    // ExpandCompoundAssignment must come before BuiltinPolyfill
    manager.Add<ast::transform::ExpandCompoundAssignment>();

    if (!entry_point.empty()) {
        manager.Add<ast::transform::SingleEntryPoint>();
        data.Add<ast::transform::SingleEntryPoint::Config>(entry_point);
    }

    manager.Add<ast::transform::PreservePadding>();  // Must come before DirectVariableAccess

    manager.Add<ast::transform::Unshadow>();  // Must come before DirectVariableAccess

    manager.Add<ast::transform::PromoteSideEffectsToDecl>();

    if (!options.disable_robustness) {
        // Robustness must come after PromoteSideEffectsToDecl
        // Robustness must come before BuiltinPolyfill and CanonicalizeEntryPointIO
        manager.Add<ast::transform::Robustness>();
    }

    if (!options.disable_workgroup_init) {
        // ZeroInitWorkgroupMemory must come before CanonicalizeEntryPointIO as
        // ZeroInitWorkgroupMemory may inject new builtin parameters.
        manager.Add<ast::transform::ZeroInitWorkgroupMemory>();
    }

    manager.Add<ast::transform::RemovePhonies>();

    // TextureBuiltinsFromUniform must come before CombineSamplers to preserve texture binding point
    // info, instead of combined sampler binding point. As a result, TextureBuiltinsFromUniform also
    // comes before BindingRemapper so the binding point info it reflects is before remapping.
    manager.Add<TextureBuiltinsFromUniform>();
    data.Add<TextureBuiltinsFromUniform::Config>(
        options.bindings.texture_builtins_from_uniform.ubo_binding,
        options.bindings.texture_builtins_from_uniform.ubo_bindingpoint_ordering);

    tint::transform::multiplanar::BindingsMap multiplanar_map{};
    RemapperData remapper_data{};
    PopulateBindingInfo(options, remapper_data, multiplanar_map);

    data.Add<ast::transform::BindingRemapper::Remappings>(
        remapper_data, std::unordered_map<BindingPoint, core::Access>{},
        /* allow_collisions */ true);
    manager.Add<ast::transform::BindingRemapper>();

    // Note: it is more efficient for MultiplanarExternalTexture to come after Robustness
    // Must come before builtin polyfills
    data.Add<ast::transform::MultiplanarExternalTexture::NewBindingPoints>(
        multiplanar_map, /* allow collisions */ true);
    manager.Add<ast::transform::MultiplanarExternalTexture>();

    // Must be after multiplanar and must be before OffsetFirstindex
    manager.Add<ast::transform::AddBlockAttribute>();

    // This must come before ClampFragDepth as the AddBlockAttribute will change around the struct
    // that gets created for the push constants and we end up with the `inner` structure sitting at
    // the same offset we want to place the first_instance value.
    manager.Add<ast::transform::OffsetFirstIndex>();
    data.Add<ast::transform::OffsetFirstIndex::Config>(options.first_vertex_offset,
                                                       options.first_instance_offset);

    // ClampFragDepth must come before CanonicalizeEntryPointIO, or the assignments to FragDepth are
    // lost
    manager.Add<ast::transform::ClampFragDepth>();
    std::optional<ast::transform::ClampFragDepth::RangeOffsets> range_offsets;
    if (options.depth_range_offsets.has_value()) {
        range_offsets = {options.depth_range_offsets->min, options.depth_range_offsets->max};
    }
    data.Add<ast::transform::ClampFragDepth::Config>(range_offsets);

    // CanonicalizeEntryPointIO must come after Robustness
    manager.Add<ast::transform::CanonicalizeEntryPointIO>();
    data.Add<ast::transform::CanonicalizeEntryPointIO::Config>(
        ast::transform::CanonicalizeEntryPointIO::ShaderStyle::kGlsl);

    // DemoteToHelper must come after PromoteSideEffectsToDecl and ExpandCompoundAssignment.
    manager.Add<ast::transform::DemoteToHelper>();

    {  // Builtin polyfills
        ast::transform::BuiltinPolyfill::Builtins polyfills;
        polyfills.acosh = ast::transform::BuiltinPolyfill::Level::kRangeCheck;
        polyfills.atanh = ast::transform::BuiltinPolyfill::Level::kRangeCheck;
        polyfills.bgra8unorm = true;
        polyfills.bitshift_modulo = true;
        polyfills.conv_f32_to_iu32 = true;
        polyfills.count_leading_zeros = true;
        polyfills.count_trailing_zeros = true;
        polyfills.extract_bits = ast::transform::BuiltinPolyfill::Level::kClampParameters;
        polyfills.first_leading_bit = true;
        polyfills.first_trailing_bit = true;
        polyfills.insert_bits = ast::transform::BuiltinPolyfill::Level::kClampParameters;
        polyfills.int_div_mod = !options.disable_polyfill_integer_div_mod;
        polyfills.saturate = true;
        polyfills.texture_sample_base_clamp_to_edge_2d_f32 = true;
        polyfills.workgroup_uniform_load = true;
        polyfills.dot_4x8_packed = true;
        polyfills.pack_unpack_4x8 = true;
        polyfills.pack_4xu8_clamp = true;
        data.Add<ast::transform::BuiltinPolyfill::Config>(polyfills);
        manager.Add<ast::transform::BuiltinPolyfill>();  // Must come before DirectVariableAccess
    }

    manager.Add<ast::transform::DirectVariableAccess>();

    // Must come after builtin polyfills (specifically texture_sample_base_clamp_to_edge_2d_f32)
    data.Add<Bindings>(options.bindings);
    manager.Add<CombineSamplers>();

    // PadStructs must come after CanonicalizeEntryPointIO and CombineSamplers
    manager.Add<PadStructs>();

    // Promote initializers must come after binding polyfill
    manager.Add<ast::transform::PromoteInitializersToLet>();
    manager.Add<ast::transform::RemoveContinueInSwitch>();
    manager.Add<ast::transform::AddEmptyEntryPoint>();

    // Std140 must come after PromoteSideEffectsToDecl and before SimplifyPointers.
    manager.Add<ast::transform::Std140>();

    manager.Add<Texture1DTo2D>();

    manager.Add<ast::transform::SimplifyPointers>();

    SanitizedResult result;
    ast::transform::DataMap outputs;
    result.program = manager.Run(in, data, outputs);
    return result;
}

ASTPrinter::ASTPrinter(const Program& program, const Version& version)
    : builder_(ProgramBuilder::Wrap(program)), version_(version) {}

ASTPrinter::~ASTPrinter() = default;

bool ASTPrinter::Generate() {
    if (!tint::wgsl::CheckSupportedExtensions(
            "GLSL", builder_.AST(), diagnostics_,
            Vector{
                wgsl::Extension::kChromiumDisableUniformityAnalysis,
                wgsl::Extension::kChromiumExperimentalPushConstant,
                wgsl::Extension::kChromiumInternalGraphite,
                wgsl::Extension::kF16,
                wgsl::Extension::kDualSourceBlending,
            })) {
        return false;
    }

    {
        auto out = Line();
        out << "#version " << version_.major_version << version_.minor_version << "0";
        if (version_.IsES()) {
            out << " es";
        }
    }

    auto helpers_insertion_point = current_buffer_->lines.size();

    Line();

    auto* mod = builder_.Sem().Module();
    for (auto* decl : mod->DependencyOrderedDeclarations()) {
        if (decl->IsAnyOf<ast::Alias, ast::ConstAssert, ast::DiagnosticDirective>()) {
            continue;  // These are not emitted.
        }

        Switch(
            decl,  //
            [&](const ast::Variable* global) { return EmitGlobalVariable(global); },
            [&](const ast::Struct* str) {
                auto* sem = builder_.Sem().Get(str);
                bool has_rt_arr = false;
                if (auto* arr = sem->Members().Back()->Type()->As<core::type::Array>()) {
                    has_rt_arr = arr->Count()->Is<core::type::RuntimeArrayCount>();
                }
                bool is_block =
                    ast::HasAttribute<ast::transform::AddBlockAttribute::BlockAttribute>(
                        str->attributes) &&
                    !sem->UsedAs(core::AddressSpace::kPushConstant);
                if (!has_rt_arr && !is_block) {
                    EmitStructType(current_buffer_, sem);
                }
            },
            [&](const ast::Function* func) {
                if (func->IsEntryPoint()) {
                    EmitEntryPointFunction(func);
                } else {
                    EmitFunction(func);
                }
            },
            [&](const ast::Enable* enable) {
                // Record the required extension for generating extension directive later
                RecordExtension(enable);
            },  //
            [&](const ast::Requires*) {
                // Do nothing for requiring language features in GLSL.
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    TextBuffer extensions;

    if (version_.IsES() && requires_oes_sample_variables_) {
        extensions.Append("#extension GL_OES_sample_variables : require");
    }

    if (requires_f16_extension_) {
        extensions.Append("#extension GL_AMD_gpu_shader_half_float : require");
    }

    if (requires_dual_source_blending_extension_) {
        extensions.Append("#extension GL_EXT_blend_func_extended : require");
    }

    auto indent = current_buffer_->current_indent;

    if (!extensions.lines.empty()) {
        current_buffer_->Insert(extensions, helpers_insertion_point, indent);
        helpers_insertion_point += extensions.lines.size();
    }

    if (version_.IsES() && requires_default_precision_qualifier_) {
        current_buffer_->Insert("precision highp float;", helpers_insertion_point++, indent);
        current_buffer_->Insert("precision highp int;", helpers_insertion_point++, indent);
    }

    if (!helpers_.lines.empty()) {
        current_buffer_->Insert("", helpers_insertion_point++, indent);
        current_buffer_->Insert(helpers_, helpers_insertion_point, indent);
        helpers_insertion_point += helpers_.lines.size();
    }

    return !diagnostics_.ContainsErrors();
}

void ASTPrinter::RecordExtension(const ast::Enable* enable) {
    // Deal with extension node here, recording it within the generator for later emition.

    if (enable->HasExtension(wgsl::Extension::kF16)) {
        requires_f16_extension_ = true;
    }

    if (enable->HasExtension(wgsl::Extension::kDualSourceBlending)) {
        requires_dual_source_blending_extension_ = true;
    }
}

void ASTPrinter::EmitIndexAccessor(StringStream& out, const ast::IndexAccessorExpression* expr) {
    EmitExpression(out, expr->object);
    out << "[";
    EmitExpression(out, expr->index);
    out << "]";
}

void ASTPrinter::EmitBitcastCall(StringStream& out, const ast::CallExpression* call) {
    auto* arg = call->args[0];
    auto* src_type = TypeOf(arg)->UnwrapRef();
    auto* dst_type = TypeOf(call);

    if (!dst_type->IsIntegerScalarOrVector() && !dst_type->IsFloatScalarOrVector()) {
        diagnostics_.AddError(Source{})
            << "Unable to do bitcast to type " << dst_type->FriendlyName();
        return;
    }

    // Handle identity bitcast.
    if (src_type == dst_type) {
        return EmitExpression(out, arg);
    }

    // Use packFloat2x16 and unpackFloat2x16 for f16 types.
    if (src_type->DeepestElement()->Is<core::type::F16>()) {
        // Source type must be vec2<f16> or vec4<f16>, since type f16 and vec3<f16> can only have
        // identity bitcast.
        auto* src_vec = src_type->As<core::type::Vector>();
        TINT_ASSERT(src_vec);
        TINT_ASSERT(((src_vec->Width() == 2u) || (src_vec->Width() == 4u)));
        std::string fn =
            GetOrAdd(bitcast_funcs_, BinaryOperandType{{src_type, dst_type}}, [&]() -> std::string {
                TextBuffer b;
                TINT_DEFER(helpers_.Append(b));

                auto fn_name = UniqueIdentifier("tint_bitcast_from_f16");
                {
                    auto decl = Line(&b);
                    EmitTypeAndName(decl, dst_type, core::AddressSpace::kUndefined,
                                    core::Access::kUndefined, fn_name);
                    {
                        ScopedParen sp(decl);
                        EmitTypeAndName(decl, src_type, core::AddressSpace::kUndefined,
                                        core::Access::kUndefined, "src");
                    }
                    decl << " {";
                }
                {
                    ScopedIndent si(&b);
                    switch (src_vec->Width()) {
                        case 2: {
                            Line(&b) << "uint r = packFloat2x16(src);";
                            break;
                        }
                        case 4: {
                            Line(&b)
                                << "uvec2 r = uvec2(packFloat2x16(src.xy), packFloat2x16(src.zw));";
                            break;
                        }
                    }
                    auto s = Line(&b);
                    s << "return ";
                    if (dst_type->IsFloatScalarOrVector()) {
                        s << "uintBitsToFloat";
                    } else {
                        EmitType(s, dst_type, core::AddressSpace::kUndefined,
                                 core::Access::kReadWrite, "");
                    }
                    s << "(r);";
                }
                Line(&b) << "}";
                return fn_name;
            });
        // Call the helper
        out << fn;
        {
            ScopedParen sp(out);
            EmitExpression(out, arg);
        }
    } else if (dst_type->DeepestElement()->Is<core::type::F16>()) {
        // Destination type must be vec2<f16> or vec4<f16>.
        auto* dst_vec = dst_type->As<core::type::Vector>();
        TINT_ASSERT(dst_vec);
        TINT_ASSERT(((dst_vec->Width() == 2u) || (dst_vec->Width() == 4u)));
        std::string fn =
            GetOrAdd(bitcast_funcs_, BinaryOperandType{{src_type, dst_type}}, [&]() -> std::string {
                TextBuffer b;
                TINT_DEFER(helpers_.Append(b));

                auto fn_name = UniqueIdentifier("tint_bitcast_to_f16");
                {
                    auto decl = Line(&b);
                    EmitTypeAndName(decl, dst_type, core::AddressSpace::kUndefined,
                                    core::Access::kUndefined, fn_name);
                    {
                        ScopedParen sp(decl);
                        EmitTypeAndName(decl, src_type, core::AddressSpace::kUndefined,
                                        core::Access::kUndefined, "src");
                    }
                    decl << " {";
                }
                {
                    ScopedIndent si(&b);
                    if (auto src_vec = src_type->As<core::type::Vector>()) {
                        // Source vector type must be vec2<f32/i32/u32>, destination type vec4<f16>.
                        TINT_ASSERT(
                            (src_vec->DeepestElement()
                                 ->IsAnyOf<core::type::I32, core::type::U32, core::type::F32>()));
                        TINT_ASSERT((src_vec->Width() == 2u));
                        {
                            auto s = Line(&b);
                            s << "uvec2 r = ";
                            if (src_type->IsFloatScalarOrVector()) {
                                s << "floatBitsToUint";
                            } else {
                                s << "uvec2";
                            }
                            s << "(src);";
                        }
                        Line(&b) << "f16vec2 v_xy = unpackFloat2x16(r.x);";
                        Line(&b) << "f16vec2 v_zw = unpackFloat2x16(r.y);";
                        Line(&b) << "return f16vec4(v_xy.x, v_xy.y, v_zw.x, v_zw.y);";
                    } else {
                        // Source scalar type must be f32/i32/u32, destination type vec2<f16>.
                        TINT_ASSERT(
                            (src_type
                                 ->IsAnyOf<core::type::I32, core::type::U32, core::type::F32>()));
                        {
                            auto s = Line(&b);
                            s << "uint r = ";
                            if (src_type->IsFloatScalarOrVector()) {
                                s << "floatBitsToUint";
                            } else {
                                s << "uint";
                            }
                            s << "(src);";
                        }
                        Line(&b) << "return unpackFloat2x16(r);";
                    }
                }
                Line(&b) << "}";
                return fn_name;
            });
        // Call the helper
        out << fn;
        {
            ScopedParen sp(out);
            EmitExpression(out, arg);
        }
    } else {
        if (src_type->IsFloatScalarOrVector() && dst_type->IsSignedIntegerScalarOrVector()) {
            out << "floatBitsToInt";
        } else if (src_type->IsFloatScalarOrVector() &&
                   dst_type->IsUnsignedIntegerScalarOrVector()) {
            out << "floatBitsToUint";
        } else if (src_type->IsSignedIntegerScalarOrVector() && dst_type->IsFloatScalarOrVector()) {
            out << "intBitsToFloat";
        } else if (src_type->IsUnsignedIntegerScalarOrVector() &&
                   dst_type->IsFloatScalarOrVector()) {
            out << "uintBitsToFloat";
        } else {
            EmitType(out, dst_type, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
        }
        ScopedParen sp(out);
        EmitExpression(out, arg);
    }
}

void ASTPrinter::EmitAssign(const ast::AssignmentStatement* stmt) {
    auto out = Line();
    EmitExpression(out, stmt->lhs);
    out << " = ";
    EmitExpression(out, stmt->rhs);
    out << ";";
}

void ASTPrinter::EmitVectorRelational(StringStream& out, const ast::BinaryExpression* expr) {
    switch (expr->op) {
        case core::BinaryOp::kEqual:
            out << "equal";
            break;
        case core::BinaryOp::kNotEqual:
            out << "notEqual";
            break;
        case core::BinaryOp::kLessThan:
            out << "lessThan";
            break;
        case core::BinaryOp::kGreaterThan:
            out << "greaterThan";
            break;
        case core::BinaryOp::kLessThanEqual:
            out << "lessThanEqual";
            break;
        case core::BinaryOp::kGreaterThanEqual:
            out << "greaterThanEqual";
            break;
        default:
            break;
    }
    ScopedParen sp(out);
    EmitExpression(out, expr->lhs);
    out << ", ";
    EmitExpression(out, expr->rhs);
}

void ASTPrinter::EmitBitwiseBoolOp(StringStream& out, const ast::BinaryExpression* expr) {
    auto* bool_type = TypeOf(expr->lhs)->UnwrapRef();
    auto* uint_type = BoolTypeToUint(bool_type);

    // Cast result to bool scalar or vector type.
    EmitType(out, bool_type, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
    ScopedParen outerCastParen(out);
    // Cast LHS to uint scalar or vector type.
    EmitType(out, uint_type, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
    {
        ScopedParen innerCastParen(out);
        // Emit LHS.
        EmitExpression(out, expr->lhs);
    }
    // Emit operator.
    if (expr->op == core::BinaryOp::kAnd) {
        out << " & ";
    } else if (DAWN_LIKELY(expr->op == core::BinaryOp::kOr)) {
        out << " | ";
    } else {
        TINT_ICE() << "unexpected binary op: " << expr->op;
    }

    // Cast RHS to uint scalar or vector type.
    EmitType(out, uint_type, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
    {
        ScopedParen innerCastParen(out);
        // Emit RHS.
        EmitExpression(out, expr->rhs);
    }
}

void ASTPrinter::EmitFloatModulo(StringStream& out, const ast::BinaryExpression* expr) {
    std::string fn;
    auto* ret_ty = TypeOf(expr)->UnwrapRef();
    auto* lhs_ty = TypeOf(expr->lhs)->UnwrapRef();
    auto* rhs_ty = TypeOf(expr->rhs)->UnwrapRef();
    fn = tint::GetOrAdd(float_modulo_funcs_, BinaryOperandType{{lhs_ty, rhs_ty}},
                        [&]() -> std::string {
                            TextBuffer b;
                            TINT_DEFER(helpers_.Append(b));

                            auto fn_name = UniqueIdentifier("tint_float_modulo");
                            std::vector<std::string> parameter_names;
                            {
                                auto decl = Line(&b);
                                EmitTypeAndName(decl, ret_ty, core::AddressSpace::kUndefined,
                                                core::Access::kUndefined, fn_name);
                                {
                                    ScopedParen sp(decl);
                                    const auto* ty = TypeOf(expr->lhs)->UnwrapRef();
                                    EmitTypeAndName(decl, ty, core::AddressSpace::kUndefined,
                                                    core::Access::kUndefined, "lhs");
                                    decl << ", ";
                                    ty = TypeOf(expr->rhs)->UnwrapRef();
                                    EmitTypeAndName(decl, ty, core::AddressSpace::kUndefined,
                                                    core::Access::kUndefined, "rhs");
                                }
                                decl << " {";
                            }
                            {
                                ScopedIndent si(&b);
                                Line(&b) << "return (lhs - rhs * trunc(lhs / rhs));";
                            }
                            Line(&b) << "}";
                            Line(&b);
                            return fn_name;
                        });

    // Call the helper
    out << fn;
    {
        ScopedParen sp(out);
        EmitExpression(out, expr->lhs);
        out << ", ";
        EmitExpression(out, expr->rhs);
    }
}

void ASTPrinter::EmitBinary(StringStream& out, const ast::BinaryExpression* expr) {
    if (IsRelational(expr->op) && !TypeOf(expr->lhs)->UnwrapRef()->Is<core::type::Scalar>()) {
        EmitVectorRelational(out, expr);
        return;
    }

    if (expr->op == core::BinaryOp::kLogicalAnd || expr->op == core::BinaryOp::kLogicalOr) {
        auto name = UniqueIdentifier(kTempNamePrefix);

        {
            auto pre = Line();
            pre << "bool " << name << " = ";
            EmitExpression(pre, expr->lhs);
            pre << ";";
        }

        if (expr->op == core::BinaryOp::kLogicalOr) {
            Line() << "if (!" << name << ") {";
        } else {
            Line() << "if (" << name << ") {";
        }

        {
            ScopedIndent si(this);
            auto pre = Line();
            pre << name << " = ";
            EmitExpression(pre, expr->rhs);
            pre << ";";
        }

        Line() << "}";

        out << "(" << name << ")";
        return;
    }

    if ((expr->op == core::BinaryOp::kAnd || expr->op == core::BinaryOp::kOr) &&
        TypeOf(expr->lhs)->UnwrapRef()->IsBoolScalarOrVector()) {
        EmitBitwiseBoolOp(out, expr);
        return;
    }

    if (expr->op == core::BinaryOp::kModulo &&
        (TypeOf(expr->lhs)->UnwrapRef()->IsFloatScalarOrVector() ||
         TypeOf(expr->rhs)->UnwrapRef()->IsFloatScalarOrVector())) {
        EmitFloatModulo(out, expr);
        return;
    }

    ScopedParen sp(out);
    EmitExpression(out, expr->lhs);
    out << " ";

    switch (expr->op) {
        case core::BinaryOp::kAnd:
            out << "&";
            break;
        case core::BinaryOp::kOr:
            out << "|";
            break;
        case core::BinaryOp::kXor:
            out << "^";
            break;
        case core::BinaryOp::kLogicalAnd:
        case core::BinaryOp::kLogicalOr: {
            // These are both handled above.
            TINT_UNREACHABLE();
        }
        case core::BinaryOp::kEqual:
            out << "==";
            break;
        case core::BinaryOp::kNotEqual:
            out << "!=";
            break;
        case core::BinaryOp::kLessThan:
            out << "<";
            break;
        case core::BinaryOp::kGreaterThan:
            out << ">";
            break;
        case core::BinaryOp::kLessThanEqual:
            out << "<=";
            break;
        case core::BinaryOp::kGreaterThanEqual:
            out << ">=";
            break;
        case core::BinaryOp::kShiftLeft:
            out << "<<";
            break;
        case core::BinaryOp::kShiftRight:
            out << R"(>>)";
            break;

        case core::BinaryOp::kAdd:
            out << "+";
            break;
        case core::BinaryOp::kSubtract:
            out << "-";
            break;
        case core::BinaryOp::kMultiply:
            out << "*";
            break;
        case core::BinaryOp::kDivide:
            out << "/";
            break;
        case core::BinaryOp::kModulo:
            out << "%";
            break;
    }
    out << " ";
    EmitExpression(out, expr->rhs);
}

void ASTPrinter::EmitStatements(VectorRef<const ast::Statement*> stmts) {
    for (auto* s : stmts) {
        EmitStatement(s);
    }
}

void ASTPrinter::EmitStatementsWithIndent(VectorRef<const ast::Statement*> stmts) {
    ScopedIndent si(this);
    EmitStatements(stmts);
}

void ASTPrinter::EmitBlock(const ast::BlockStatement* stmt) {
    Line() << "{";
    EmitStatementsWithIndent(stmt->statements);
    Line() << "}";
}

void ASTPrinter::EmitBreak(const ast::BreakStatement*) {
    Line() << "break;";
}

void ASTPrinter::EmitBreakIf(const ast::BreakIfStatement* b) {
    auto out = Line();
    out << "if (";
    EmitExpression(out, b->condition);
    out << ") { break; }";
}

void ASTPrinter::EmitCall(StringStream& out, const ast::CallExpression* expr) {
    auto* call = builder_.Sem().Get<sem::Call>(expr);
    Switch(
        call->Target(),  //
        [&](const sem::Function* fn) { EmitFunctionCall(out, call, fn); },
        [&](const sem::BuiltinFn* builtin) { EmitBuiltinCall(out, call, builtin); },
        [&](const sem::ValueConversion* conv) { EmitValueConversion(out, call, conv); },
        [&](const sem::ValueConstructor* ctor) { EmitValueConstructor(out, call, ctor); },
        TINT_ICE_ON_NO_MATCH);
}

void ASTPrinter::EmitFunctionCall(StringStream& out,
                                  const sem::Call* call,
                                  const sem::Function* fn) {
    const auto& args = call->Arguments();
    auto* ident = fn->Declaration()->name;

    out << ident->symbol.Name();
    ScopedParen sp(out);

    bool first = true;
    for (auto* arg : args) {
        if (!first) {
            out << ", ";
        }
        first = false;

        EmitExpression(out, arg->Declaration());
    }
}

void ASTPrinter::EmitBuiltinCall(StringStream& out,
                                 const sem::Call* call,
                                 const sem::BuiltinFn* builtin) {
    auto* expr = call->Declaration();
    if (builtin->IsTexture()) {
        EmitTextureCall(out, call, builtin);
    } else if (builtin->Fn() == wgsl::BuiltinFn::kCountOneBits) {
        EmitCountOneBitsCall(out, expr);
    } else if (builtin->Fn() == wgsl::BuiltinFn::kSelect) {
        EmitSelectCall(out, expr, builtin);
    } else if (builtin->Fn() == wgsl::BuiltinFn::kBitcast) {
        EmitBitcastCall(out, expr);
    } else if (builtin->Fn() == wgsl::BuiltinFn::kDot) {
        EmitDotCall(out, expr, builtin);
    } else if (builtin->Fn() == wgsl::BuiltinFn::kModf) {
        EmitModfCall(out, expr, builtin);
    } else if (builtin->Fn() == wgsl::BuiltinFn::kFrexp) {
        EmitFrexpCall(out, expr, builtin);
    } else if (builtin->Fn() == wgsl::BuiltinFn::kDegrees) {
        EmitDegreesCall(out, expr, builtin);
    } else if (builtin->Fn() == wgsl::BuiltinFn::kRadians) {
        EmitRadiansCall(out, expr, builtin);
    } else if (builtin->Fn() == wgsl::BuiltinFn::kQuantizeToF16) {
        EmitQuantizeToF16Call(out, expr, builtin);
    } else if (builtin->Fn() == wgsl::BuiltinFn::kArrayLength) {
        EmitArrayLength(out, expr);
    } else if (builtin->Fn() == wgsl::BuiltinFn::kExtractBits) {
        EmitExtractBits(out, expr);
    } else if (builtin->Fn() == wgsl::BuiltinFn::kInsertBits) {
        EmitInsertBits(out, expr);
    } else if (builtin->Fn() == wgsl::BuiltinFn::kFma && version_.IsES()) {
        EmitEmulatedFMA(out, expr);
    } else if (builtin->Fn() == wgsl::BuiltinFn::kAbs &&
               TypeOf(expr->args[0])->UnwrapRef()->IsUnsignedIntegerScalarOrVector()) {
        // GLSL does not support abs() on unsigned arguments. However, it's a no-op.
        EmitExpression(out, expr->args[0]);
    } else if ((builtin->Fn() == wgsl::BuiltinFn::kAny || builtin->Fn() == wgsl::BuiltinFn::kAll) &&
               TypeOf(expr->args[0])->UnwrapRef()->Is<core::type::Scalar>()) {
        // GLSL does not support any() or all() on scalar arguments. It's a no-op.
        EmitExpression(out, expr->args[0]);
    } else if (builtin->IsBarrier()) {
        EmitBarrierCall(out, builtin);
    } else if (builtin->IsAtomic()) {
        EmitWorkgroupAtomicCall(out, expr, builtin);
    } else {
        auto name = generate_builtin_name(builtin);
        if (name.empty()) {
            return;
        }

        out << name;
        ScopedParen sp(out);

        bool first = true;
        for (auto* arg : call->Arguments()) {
            if (!first) {
                out << ", ";
            }
            first = false;

            EmitExpression(out, arg->Declaration());
        }
    }
}

void ASTPrinter::EmitValueConversion(StringStream& out,
                                     const sem::Call* call,
                                     const sem::ValueConversion* conv) {
    EmitType(out, conv->Target(), core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
    ScopedParen sp(out);
    EmitExpression(out, call->Arguments()[0]->Declaration());
}

void ASTPrinter::EmitValueConstructor(StringStream& out,
                                      const sem::Call* call,
                                      const sem::ValueConstructor* ctor) {
    auto* type = ctor->ReturnType();

    // If the value constructor is empty then we need to construct with the zero value for all
    // components.
    if (call->Arguments().IsEmpty()) {
        EmitZeroValue(out, type);
        return;
    }

    EmitType(out, type, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
    ScopedParen sp(out);

    bool first = true;
    for (auto* arg : call->Arguments()) {
        if (!first) {
            out << ", ";
        }
        first = false;

        EmitExpression(out, arg->Declaration());
    }
}

void ASTPrinter::EmitWorkgroupAtomicCall(StringStream& out,
                                         const ast::CallExpression* expr,
                                         const sem::BuiltinFn* builtin) {
    auto call = [&](const char* name) {
        out << name;
        {
            ScopedParen sp(out);
            for (size_t i = 0; i < expr->args.Length(); i++) {
                auto* arg = expr->args[i];
                if (i > 0) {
                    out << ", ";
                }
                EmitExpression(out, arg);
            }
        }
        return;
    };

    switch (builtin->Fn()) {
        case wgsl::BuiltinFn::kAtomicLoad: {
            // GLSL does not have an atomicLoad, so we emulate it with
            // atomicOr using 0 as the OR value
            out << "atomicOr";
            {
                ScopedParen sp(out);
                EmitExpression(out, expr->args[0]);
                out << ", 0";
                if (builtin->ReturnType()->Is<core::type::U32>()) {
                    out << "u";
                }
            }
            return;
        }
        case wgsl::BuiltinFn::kAtomicCompareExchangeWeak: {
            EmitStructType(&helpers_, builtin->ReturnType()->As<core::type::Struct>());

            auto* dest = expr->args[0];
            auto* compare_value = expr->args[1];
            auto* value = expr->args[2];

            std::string result = UniqueIdentifier("atomic_compare_result");

            {
                auto pre = Line();
                EmitTypeAndName(pre, builtin->ReturnType(), core::AddressSpace::kUndefined,
                                core::Access::kUndefined, result);
                pre << ";";
            }
            {
                auto pre = Line();
                pre << result << ".old_value = atomicCompSwap";
                {
                    ScopedParen sp(pre);
                    EmitExpression(pre, dest);
                    pre << ", ";
                    EmitExpression(pre, compare_value);
                    pre << ", ";
                    EmitExpression(pre, value);
                }
                pre << ";";
            }
            {
                auto pre = Line();
                pre << result << ".exchanged = " << result << ".old_value == ";
                EmitExpression(pre, compare_value);
                pre << ";";
            }

            out << result;
            return;
        }

        case wgsl::BuiltinFn::kAtomicAdd:
            call("atomicAdd");
            return;

        case wgsl::BuiltinFn::kAtomicSub:
            out << "atomicAdd";
            {
                ScopedParen sp(out);
                TINT_ASSERT(expr->args.Length() == 2);
                EmitExpression(out, expr->args[0]);
                out << ", -";
                {
                    ScopedParen argSP(out);
                    EmitExpression(out, expr->args[1]);
                }
            }
            return;

        case wgsl::BuiltinFn::kAtomicMax:
            call("atomicMax");
            return;

        case wgsl::BuiltinFn::kAtomicMin:
            call("atomicMin");
            return;

        case wgsl::BuiltinFn::kAtomicAnd:
            call("atomicAnd");
            return;

        case wgsl::BuiltinFn::kAtomicOr:
            call("atomicOr");
            return;

        case wgsl::BuiltinFn::kAtomicXor:
            call("atomicXor");
            return;

        case wgsl::BuiltinFn::kAtomicExchange:
        case wgsl::BuiltinFn::kAtomicStore:
            // GLSL does not have an atomicStore, so we emulate it with
            // atomicExchange.
            call("atomicExchange");
            return;

        default:
            break;
    }

    TINT_UNREACHABLE() << "unsupported atomic builtin: " << builtin->Fn();
}

void ASTPrinter::EmitArrayLength(StringStream& out, const ast::CallExpression* expr) {
    out << "uint(";
    EmitExpression(out, expr->args[0]);
    out << ".length())";
}

void ASTPrinter::EmitExtractBits(StringStream& out, const ast::CallExpression* expr) {
    out << "bitfieldExtract(";
    EmitExpression(out, expr->args[0]);
    out << ", int(";
    EmitExpression(out, expr->args[1]);
    out << "), int(";
    EmitExpression(out, expr->args[2]);
    out << "))";
}

void ASTPrinter::EmitInsertBits(StringStream& out, const ast::CallExpression* expr) {
    out << "bitfieldInsert(";
    EmitExpression(out, expr->args[0]);
    out << ", ";
    EmitExpression(out, expr->args[1]);
    out << ", int(";
    EmitExpression(out, expr->args[2]);
    out << "), int(";
    EmitExpression(out, expr->args[3]);
    out << "))";
}

void ASTPrinter::EmitEmulatedFMA(StringStream& out, const ast::CallExpression* expr) {
    out << "((";
    EmitExpression(out, expr->args[0]);
    out << ") * (";
    EmitExpression(out, expr->args[1]);
    out << ") + (";
    EmitExpression(out, expr->args[2]);
    out << "))";
}

void ASTPrinter::EmitCountOneBitsCall(StringStream& out, const ast::CallExpression* expr) {
    // GLSL's bitCount returns an integer type, so cast it to the appropriate
    // unsigned type.
    EmitType(out, TypeOf(expr)->UnwrapRef(), core::AddressSpace::kUndefined,
             core::Access::kReadWrite, "");
    out << "(bitCount(";
    EmitExpression(out, expr->args[0]);
    out << "))";
}

void ASTPrinter::EmitSelectCall(StringStream& out,
                                const ast::CallExpression* expr,
                                const sem::BuiltinFn* builtin) {
    auto* expr_false = expr->args[0];
    auto* expr_true = expr->args[1];
    auto* expr_cond = expr->args[2];

    out << "mix";
    ScopedParen paren(out);

    EmitExpression(out, expr_false);
    out << ", ";
    EmitExpression(out, expr_true);
    out << ", ";

    auto* p0_ty = builtin->Parameters()[0]->Type();
    auto* p2_ty = builtin->Parameters()[2]->Type();

    // If the value types are vectors, but the condition is a single bool, splat the bool into a
    // vector of equivalent size.
    if (p0_ty->Is<core::type::Vector>() && !p2_ty->Is<core::type::Vector>()) {
        auto* vec = p0_ty->As<core::type::Vector>();
        out << "bvec" << vec->Width();
        ScopedParen cast_paren(out);
        EmitExpression(out, expr_cond);
    } else {
        EmitExpression(out, expr_cond);
    }
}

void ASTPrinter::EmitDotCall(StringStream& out,
                             const ast::CallExpression* expr,
                             const sem::BuiltinFn* builtin) {
    auto* vec_ty = builtin->Parameters()[0]->Type()->As<core::type::Vector>();
    std::string fn = "dot";
    if (vec_ty->Type()->IsIntegerScalar()) {
        // GLSL does not have a builtin for dot() with integer vector types.
        // Generate the helper function if it hasn't been created already
        fn = tint::GetOrAdd(int_dot_funcs_, vec_ty, [&]() -> std::string {
            TextBuffer b;
            TINT_DEFER(helpers_.Append(b));

            auto fn_name = UniqueIdentifier("tint_int_dot");

            std::string v;
            {
                StringStream s;
                EmitType(s, vec_ty->Type(), core::AddressSpace::kUndefined, core::Access::kRead,
                         "");
                v = s.str();
            }
            {  // (u)int tint_int_dot([i|u]vecN a, [i|u]vecN b) {
                auto l = Line(&b);
                EmitType(l, vec_ty->Type(), core::AddressSpace::kUndefined, core::Access::kRead,
                         "");
                l << " " << fn_name << "(";
                EmitType(l, vec_ty, core::AddressSpace::kUndefined, core::Access::kRead, "");
                l << " a, ";
                EmitType(l, vec_ty, core::AddressSpace::kUndefined, core::Access::kRead, "");
                l << " b) {";
            }
            {
                auto l = Line(&b);
                l << "  return ";
                for (uint32_t i = 0; i < vec_ty->Width(); i++) {
                    if (i > 0) {
                        l << " + ";
                    }
                    l << "a[" << i << "]*b[" << i << "]";
                }
                l << ";";
            }
            Line(&b) << "}";
            return fn_name;
        });
    }

    out << fn;
    ScopedParen sp(out);

    EmitExpression(out, expr->args[0]);
    out << ", ";
    EmitExpression(out, expr->args[1]);
}

void ASTPrinter::EmitModfCall(StringStream& out,
                              const ast::CallExpression* expr,
                              const sem::BuiltinFn* builtin) {
    TINT_ASSERT(expr->args.Length() == 1);
    CallBuiltinHelper(
        out, expr, builtin, [&](TextBuffer* b, const std::vector<std::string>& params) {
            // Emit the builtin return type unique to this overload. This does not
            // exist in the AST, so it will not be generated in Generate().
            EmitStructType(&helpers_, builtin->ReturnType()->As<core::type::Struct>());

            {
                auto l = Line(b);
                EmitType(l, builtin->ReturnType(), core::AddressSpace::kUndefined,
                         core::Access::kUndefined, "");
                l << " result;";
            }
            Line(b) << "result.fract = modf(" << params[0] << ", result.whole);";
            Line(b) << "return result;";
        });
}

void ASTPrinter::EmitFrexpCall(StringStream& out,
                               const ast::CallExpression* expr,
                               const sem::BuiltinFn* builtin) {
    TINT_ASSERT(expr->args.Length() == 1);
    CallBuiltinHelper(
        out, expr, builtin, [&](TextBuffer* b, const std::vector<std::string>& params) {
            // Emit the builtin return type unique to this overload. This does not
            // exist in the AST, so it will not be generated in Generate().
            EmitStructType(&helpers_, builtin->ReturnType()->As<core::type::Struct>());

            {
                auto l = Line(b);
                EmitType(l, builtin->ReturnType(), core::AddressSpace::kUndefined,
                         core::Access::kUndefined, "");
                l << " result;";
            }
            Line(b) << "result.fract = frexp(" << params[0] << ", result.exp);";
            Line(b) << "return result;";
        });
}

void ASTPrinter::EmitDegreesCall(StringStream& out,
                                 const ast::CallExpression* expr,
                                 const sem::BuiltinFn* builtin) {
    auto* return_elem_type = builtin->ReturnType()->DeepestElement();
    const std::string suffix = Is<core::type::F16>(return_elem_type) ? "hf" : "f";
    CallBuiltinHelper(out, expr, builtin,
                      [&](TextBuffer* b, const std::vector<std::string>& params) {
                          Line(b) << "return " << params[0] << " * " << std::setprecision(20)
                                  << sem::kRadToDeg << suffix << ";";
                      });
}

void ASTPrinter::EmitRadiansCall(StringStream& out,
                                 const ast::CallExpression* expr,
                                 const sem::BuiltinFn* builtin) {
    auto* return_elem_type = builtin->ReturnType()->DeepestElement();
    const std::string suffix = Is<core::type::F16>(return_elem_type) ? "hf" : "f";
    CallBuiltinHelper(out, expr, builtin,
                      [&](TextBuffer* b, const std::vector<std::string>& params) {
                          Line(b) << "return " << params[0] << " * " << std::setprecision(20)
                                  << sem::kDegToRad << suffix << ";";
                      });
}

void ASTPrinter::EmitQuantizeToF16Call(StringStream& out,
                                       const ast::CallExpression* expr,
                                       const sem::BuiltinFn* builtin) {
    // Emulate by casting to f16 and back again.
    CallBuiltinHelper(
        out, expr, builtin, [&](TextBuffer* b, const std::vector<std::string>& params) {
            const auto v = params[0];
            if (auto* vec = builtin->ReturnType()->As<core::type::Vector>()) {
                switch (vec->Width()) {
                    case 2: {
                        Line(b) << "return unpackHalf2x16(packHalf2x16(" << v << "));";
                        return;
                    }
                    case 3: {
                        Line(b) << "return vec3(";
                        Line(b) << "  unpackHalf2x16(packHalf2x16(" << v << ".xy)),";
                        Line(b) << "  unpackHalf2x16(packHalf2x16(" << v << ".zz)).x);";
                        return;
                    }
                    default: {
                        Line(b) << "return vec4(";
                        Line(b) << "  unpackHalf2x16(packHalf2x16(" << v << ".xy)),";
                        Line(b) << "  unpackHalf2x16(packHalf2x16(" << v << ".zw)));";
                        return;
                    }
                }
            }
            Line(b) << "return unpackHalf2x16(packHalf2x16(vec2(" << v << "))).x;";
        });
}

void ASTPrinter::EmitBarrierCall(StringStream& out, const sem::BuiltinFn* builtin) {
    // TODO(crbug.com/tint/661): Combine sequential barriers to a single
    // instruction.
    if (builtin->Fn() == wgsl::BuiltinFn::kWorkgroupBarrier) {
        out << "barrier()";
    } else if (builtin->Fn() == wgsl::BuiltinFn::kStorageBarrier) {
        out << "{ memoryBarrierBuffer(); barrier(); }";
    } else if (builtin->Fn() == wgsl::BuiltinFn::kTextureBarrier) {
        out << "{ memoryBarrierImage(); barrier(); }";
    } else {
        TINT_UNREACHABLE() << "unexpected barrier builtin type " << builtin->Fn();
    }
}

const ast::Expression* ASTPrinter::CreateF32Zero(const sem::Statement* stmt) {
    auto* zero = builder_.Expr(0_f);
    auto* f32 = builder_.create<core::type::F32>();
    auto* sem_zero = builder_.create<sem::ValueExpression>(
        zero, f32, core::EvaluationStage::kRuntime, stmt, /* constant_value */ nullptr,
        /* has_side_effects */ false);
    builder_.Sem().Add(zero, sem_zero);
    return zero;
}

void ASTPrinter::EmitTextureCall(StringStream& out,
                                 const sem::Call* call,
                                 const sem::BuiltinFn* builtin) {
    using Usage = core::ParameterUsage;

    auto& signature = builtin->Signature();
    auto* expr = call->Declaration();
    auto arguments = expr->args;

    // Returns the argument with the given usage
    auto arg = [&](Usage usage) {
        auto idx = signature.IndexOf(usage);
        return (idx >= 0) ? arguments[static_cast<size_t>(idx)] : nullptr;
    };

    auto* texture = arg(Usage::kTexture);
    if (DAWN_UNLIKELY(!texture)) {
        TINT_ICE() << "missing texture argument";
    }

    auto* texture_type = TypeOf(texture)->UnwrapRef()->As<core::type::Texture>();

    auto emit_signed_int_type = [&](const core::type::Type* ty) {
        uint32_t width = ty->Elements().count;
        if (width > 1) {
            out << "ivec" << width;
        } else {
            out << "int";
        }
    };

    auto emit_unsigned_int_type = [&](const core::type::Type* ty) {
        uint32_t width = ty->Elements().count;
        if (width > 1) {
            out << "uvec" << width;
        } else {
            out << "uint";
        }
    };

    auto emit_expr_as_signed = [&](const ast::Expression* e) {
        auto* ty = TypeOf(e)->UnwrapRef();
        if (!ty->IsUnsignedIntegerScalarOrVector()) {
            EmitExpression(out, e);
            return;
        }
        emit_signed_int_type(ty);
        ScopedParen sp(out);
        EmitExpression(out, e);
        return;
    };

    switch (builtin->Fn()) {
        case wgsl::BuiltinFn::kTextureDimensions: {
            // textureDimensions() returns an unsigned scalar / vector in WGSL.
            // textureSize() / imageSize() returns a signed scalar / vector in GLSL.
            // Cast.
            emit_unsigned_int_type(call->Type());
            ScopedParen sp(out);

            if (texture_type->Is<core::type::StorageTexture>()) {
                out << "imageSize(";
            } else {
                out << "textureSize(";
            }
            EmitExpression(out, texture);

            // The LOD parameter is mandatory on textureSize() for non-multisampled
            // textures.
            if (!texture_type->Is<core::type::StorageTexture>() &&
                !texture_type->Is<core::type::MultisampledTexture>() &&
                !texture_type->Is<core::type::DepthMultisampledTexture>()) {
                out << ", ";
                if (auto* level_arg = arg(Usage::kLevel)) {
                    emit_expr_as_signed(level_arg);
                } else {
                    out << "0";
                }
            }
            out << ")";
            // textureSize() on array samplers returns the array size in the
            // final component, so strip it out.
            if (texture_type->Dim() == core::type::TextureDimension::k2dArray ||
                texture_type->Dim() == core::type::TextureDimension::kCubeArray) {
                out << ".xy";
            }
            return;
        }
        case wgsl::BuiltinFn::kTextureNumLayers: {
            // textureNumLayers() returns an unsigned scalar in WGSL.
            // textureSize() / imageSize() returns a signed scalar / vector in GLSL.
            // Cast.
            out << "uint";
            ScopedParen sp(out);

            if (texture_type->Is<core::type::StorageTexture>()) {
                out << "imageSize(";
            } else {
                out << "textureSize(";
            }
            // textureSize() on sampler2dArray returns the array size in the
            // final component, so return it
            EmitExpression(out, texture);

            // The LOD parameter is mandatory on textureSize() for non-multisampled
            // textures.
            if (!texture_type->Is<core::type::StorageTexture>() &&
                !texture_type->Is<core::type::MultisampledTexture>() &&
                !texture_type->Is<core::type::DepthMultisampledTexture>()) {
                out << ", ";
                if (auto* level_arg = arg(Usage::kLevel)) {
                    emit_expr_as_signed(level_arg);
                } else {
                    out << "0";
                }
            }
            out << ").z";
            return;
        }
        case wgsl::BuiltinFn::kTextureNumLevels: {
            // textureNumLevels() returns an unsigned scalar in WGSL.
            // textureQueryLevels() returns a signed scalar in GLSL.
            // Cast.
            out << "uint";
            ScopedParen sp(out);

            out << "textureQueryLevels(";
            EmitExpression(out, texture);
            out << ")";
            return;
        }
        case wgsl::BuiltinFn::kTextureNumSamples: {
            // textureNumSamples() returns an unsigned scalar in WGSL.
            // textureSamples() returns a signed scalar in GLSL.
            // Cast.
            out << "uint";
            ScopedParen sp(out);

            out << "textureSamples(";
            EmitExpression(out, texture);
            out << ")";
            return;
        }
        default:
            break;
    }

    uint32_t glsl_ret_width = 4u;
    bool append_depth_ref_to_coords = true;
    bool is_depth = texture_type->Is<core::type::DepthTexture>();
    bool is_array = IsTextureArray(texture_type->Dim());

    switch (builtin->Fn()) {
        case wgsl::BuiltinFn::kTextureSample:
        case wgsl::BuiltinFn::kTextureSampleBias:
            out << "texture";
            if (is_depth) {
                glsl_ret_width = 1u;
            }
            break;
        case wgsl::BuiltinFn::kTextureSampleLevel:
            out << "textureLod";
            if (is_depth) {
                glsl_ret_width = 1u;
            }
            break;
        case wgsl::BuiltinFn::kTextureGather:
        case wgsl::BuiltinFn::kTextureGatherCompare:
            out << "textureGather";
            append_depth_ref_to_coords = false;
            break;
        case wgsl::BuiltinFn::kTextureSampleGrad:
            out << "textureGrad";
            break;
        case wgsl::BuiltinFn::kTextureSampleCompare:
        case wgsl::BuiltinFn::kTextureSampleCompareLevel:
            out << "texture";
            glsl_ret_width = 1;
            break;
        case wgsl::BuiltinFn::kTextureLoad:
            if (texture_type->Is<core::type::StorageTexture>()) {
                out << "imageLoad";
            } else {
                out << "texelFetch";
            }
            break;
        case wgsl::BuiltinFn::kTextureStore:
            out << "imageStore";
            break;
        default:
            TINT_ICE() << "Unhandled texture builtin '" << std::string(builtin->str()) << "'";
    }

    bool supplyDerivativesAsGradients = false;
    if (builtin->Signature().IndexOf(core::ParameterUsage::kOffset) >= 0) {
        if ((builtin->Fn() == wgsl::BuiltinFn::kTextureSample ||
             builtin->Fn() == wgsl::BuiltinFn::kTextureSampleCompare) &&
            is_depth && is_array) {
            out << "Grad";
            supplyDerivativesAsGradients = true;
        }
        out << "Offset";
    }

    out << "(";
    EmitExpression(out, texture);
    out << ", ";

    auto* param_coords = arg(Usage::kCoords);
    if (DAWN_UNLIKELY(!param_coords)) {
        TINT_ICE() << "missing coords argument";
    }

    if (auto* array_index = arg(Usage::kArrayIndex)) {
        // Array index needs to be appended to the coordinates.
        param_coords =
            tint::wgsl::AppendVector(&builder_, param_coords, array_index)->Declaration();
    }

    // GLSL requires Dref to be appended to the coordinates, *unless* it's
    // samplerCubeArrayShadow, in which case it will be handled as a separate
    // parameter.
    if (texture_type->Dim() == core::type::TextureDimension::kCubeArray) {
        append_depth_ref_to_coords = false;
    }

    if (is_depth && append_depth_ref_to_coords) {
        auto* depth_ref = arg(Usage::kDepthRef);
        if (!depth_ref) {
            // Sampling a depth texture in GLSL always requires a depth reference, so
            // append zero here.
            depth_ref = CreateF32Zero(builder_.Sem().Get(param_coords)->Stmt());
        }
        param_coords = tint::wgsl::AppendVector(&builder_, param_coords, depth_ref)->Declaration();
    }

    emit_expr_as_signed(param_coords);

    if (supplyDerivativesAsGradients) {
        auto* coords = arg(Usage::kCoords);
        out << ", dFdx(";
        EmitExpression(out, coords);
        out << "), dFdy(";
        EmitExpression(out, coords);
        out << ")";
    }
    for (auto usage : {Usage::kLevel, Usage::kDdx, Usage::kDdy, Usage::kSampleIndex}) {
        if (auto* e = arg(usage)) {
            out << ", ";
            if (usage == Usage::kLevel && is_depth) {
                // WGSL's textureSampleLevel() "level" param is i32 for depth textures,
                // whereas GLSL's textureLod() "lod" param is always float, so cast it.
                out << "float(";
                EmitExpression(out, e);
                out << ")";
            } else {
                emit_expr_as_signed(e);
            }
        }
    }

    if (auto* e = arg(Usage::kValue)) {
        out << ", ";
        EmitExpression(out, e);
    }

    // GLSL's textureGather always requires a refZ parameter.
    if (is_depth && builtin->Fn() == wgsl::BuiltinFn::kTextureGather) {
        out << ", 0.0";
    }

    // [1] samplerCubeArrayShadow requires a separate depthRef parameter
    if (is_depth && !append_depth_ref_to_coords) {
        if (auto* e = arg(Usage::kDepthRef)) {
            out << ", ";
            EmitExpression(out, e);
        } else if (builtin->Fn() == wgsl::BuiltinFn::kTextureSample) {
            out << ", 0.0f";
        }
    }

    for (auto usage : {Usage::kOffset, Usage::kComponent, Usage::kBias}) {
        if (auto* e = arg(usage)) {
            out << ", ";
            emit_expr_as_signed(e);
        }
    }

    out << ")";

    if (builtin->ReturnType()->Is<core::type::Void>()) {
        return;
    }
    // If the builtin return type does not match the number of elements of the
    // GLSL builtin, we need to swizzle the expression to generate the correct
    // number of components.
    uint32_t wgsl_ret_width = 1;
    if (auto* vec = builtin->ReturnType()->As<core::type::Vector>()) {
        wgsl_ret_width = vec->Width();
    }
    if (wgsl_ret_width < glsl_ret_width) {
        out << ".";
        for (uint32_t i = 0; i < wgsl_ret_width; i++) {
            out << "xyz"[i];
        }
    }
    if (DAWN_UNLIKELY(wgsl_ret_width > glsl_ret_width)) {
        TINT_ICE() << "WGSL return width (" << wgsl_ret_width
                   << ") is wider than GLSL return width (" << glsl_ret_width << ") for "
                   << builtin->Fn();
    }
}

std::string ASTPrinter::generate_builtin_name(const sem::BuiltinFn* builtin) {
    switch (builtin->Fn()) {
        case wgsl::BuiltinFn::kAbs:
        case wgsl::BuiltinFn::kAcos:
        case wgsl::BuiltinFn::kAcosh:
        case wgsl::BuiltinFn::kAll:
        case wgsl::BuiltinFn::kAny:
        case wgsl::BuiltinFn::kAsin:
        case wgsl::BuiltinFn::kAsinh:
        case wgsl::BuiltinFn::kAtan:
        case wgsl::BuiltinFn::kAtanh:
        case wgsl::BuiltinFn::kCeil:
        case wgsl::BuiltinFn::kClamp:
        case wgsl::BuiltinFn::kCos:
        case wgsl::BuiltinFn::kCosh:
        case wgsl::BuiltinFn::kCross:
        case wgsl::BuiltinFn::kDeterminant:
        case wgsl::BuiltinFn::kDistance:
        case wgsl::BuiltinFn::kDot:
        case wgsl::BuiltinFn::kExp:
        case wgsl::BuiltinFn::kExp2:
        case wgsl::BuiltinFn::kFloor:
        case wgsl::BuiltinFn::kFrexp:
        case wgsl::BuiltinFn::kLdexp:
        case wgsl::BuiltinFn::kLength:
        case wgsl::BuiltinFn::kLog:
        case wgsl::BuiltinFn::kLog2:
        case wgsl::BuiltinFn::kMax:
        case wgsl::BuiltinFn::kMin:
        case wgsl::BuiltinFn::kModf:
        case wgsl::BuiltinFn::kNormalize:
        case wgsl::BuiltinFn::kPow:
        case wgsl::BuiltinFn::kReflect:
        case wgsl::BuiltinFn::kRefract:
        case wgsl::BuiltinFn::kRound:
        case wgsl::BuiltinFn::kSign:
        case wgsl::BuiltinFn::kSin:
        case wgsl::BuiltinFn::kSinh:
        case wgsl::BuiltinFn::kSqrt:
        case wgsl::BuiltinFn::kStep:
        case wgsl::BuiltinFn::kTan:
        case wgsl::BuiltinFn::kTanh:
        case wgsl::BuiltinFn::kTranspose:
        case wgsl::BuiltinFn::kTrunc:
            return builtin->str();
        case wgsl::BuiltinFn::kAtan2:
            return "atan";
        case wgsl::BuiltinFn::kCountOneBits:
            return "bitCount";
        case wgsl::BuiltinFn::kDpdx:
            return "dFdx";
        case wgsl::BuiltinFn::kDpdxCoarse:
            if (version_.IsES()) {
                return "dFdx";
            }
            return "dFdxCoarse";
        case wgsl::BuiltinFn::kDpdxFine:
            if (version_.IsES()) {
                return "dFdx";
            }
            return "dFdxFine";
        case wgsl::BuiltinFn::kDpdy:
            return "dFdy";
        case wgsl::BuiltinFn::kDpdyCoarse:
            if (version_.IsES()) {
                return "dFdy";
            }
            return "dFdyCoarse";
        case wgsl::BuiltinFn::kDpdyFine:
            if (version_.IsES()) {
                return "dFdy";
            }
            return "dFdyFine";
        case wgsl::BuiltinFn::kFaceForward:
            return "faceforward";
        case wgsl::BuiltinFn::kFract:
            return "fract";
        case wgsl::BuiltinFn::kFma:
            return "fma";
        case wgsl::BuiltinFn::kFwidth:
        case wgsl::BuiltinFn::kFwidthCoarse:
        case wgsl::BuiltinFn::kFwidthFine:
            return "fwidth";
        case wgsl::BuiltinFn::kInverseSqrt:
            return "inversesqrt";
        case wgsl::BuiltinFn::kMix:
            return "mix";
        case wgsl::BuiltinFn::kPack2X16Float:
            return "packHalf2x16";
        case wgsl::BuiltinFn::kPack2X16Snorm:
            return "packSnorm2x16";
        case wgsl::BuiltinFn::kPack2X16Unorm:
            return "packUnorm2x16";
        case wgsl::BuiltinFn::kPack4X8Snorm:
            return "packSnorm4x8";
        case wgsl::BuiltinFn::kPack4X8Unorm:
            return "packUnorm4x8";
        case wgsl::BuiltinFn::kReverseBits:
            return "bitfieldReverse";
        case wgsl::BuiltinFn::kSmoothstep:
            return "smoothstep";
        case wgsl::BuiltinFn::kUnpack2X16Float:
            return "unpackHalf2x16";
        case wgsl::BuiltinFn::kUnpack2X16Snorm:
            return "unpackSnorm2x16";
        case wgsl::BuiltinFn::kUnpack2X16Unorm:
            return "unpackUnorm2x16";
        case wgsl::BuiltinFn::kUnpack4X8Snorm:
            return "unpackSnorm4x8";
        case wgsl::BuiltinFn::kUnpack4X8Unorm:
            return "unpackUnorm4x8";
        default:
            diagnostics_.AddError(Source{}) << "Unknown builtin method: " << builtin;
    }

    return "";
}

void ASTPrinter::EmitCase(const ast::CaseStatement* stmt) {
    auto* sem = builder_.Sem().Get<sem::CaseStatement>(stmt);
    for (auto* selector : sem->Selectors()) {
        auto out = Line();

        if (selector->IsDefault()) {
            out << "default";
        } else {
            out << "case ";
            EmitConstant(out, selector->Value());
        }
        out << ":";
        if (selector == sem->Selectors().back()) {
            out << " {";
        }
    }

    {
        ScopedIndent si(this);
        EmitStatements(stmt->body->statements);
        if (!last_is_break(stmt->body)) {
            Line() << "break;";
        }
    }

    Line() << "}";
}

void ASTPrinter::EmitContinue(const ast::ContinueStatement*) {
    if (emit_continuing_) {
        emit_continuing_();
    }
    Line() << "continue;";
}

void ASTPrinter::EmitDiscard(const ast::DiscardStatement*) {
    // TODO(dsinclair): Verify this is correct when the discard semantics are
    // defined for WGSL (https://github.com/gpuweb/gpuweb/issues/361)
    Line() << "discard;";
}

void ASTPrinter::EmitExpression(StringStream& out, const ast::Expression* expr) {
    if (auto* sem = builder_.Sem().GetVal(expr)) {
        if (auto* constant = sem->ConstantValue()) {
            EmitConstant(out, constant);
            return;
        }
    }
    Switch(
        expr,  //
        [&](const ast::IndexAccessorExpression* a) { EmitIndexAccessor(out, a); },
        [&](const ast::BinaryExpression* b) { EmitBinary(out, b); },
        [&](const ast::CallExpression* c) { EmitCall(out, c); },
        [&](const ast::IdentifierExpression* i) { EmitIdentifier(out, i); },
        [&](const ast::LiteralExpression* l) { EmitLiteral(out, l); },
        [&](const ast::MemberAccessorExpression* m) { EmitMemberAccessor(out, m); },
        [&](const ast::UnaryOpExpression* u) { EmitUnaryOp(out, u); },  //
        TINT_ICE_ON_NO_MATCH);
}

void ASTPrinter::EmitIdentifier(StringStream& out, const ast::IdentifierExpression* expr) {
    out << expr->identifier->symbol.Name();
}

void ASTPrinter::EmitIf(const ast::IfStatement* stmt) {
    {
        auto out = Line();
        out << "if (";
        EmitExpression(out, stmt->condition);
        out << ") {";
    }
    EmitStatementsWithIndent(stmt->body->statements);

    if (stmt->else_statement) {
        Line() << "} else {";
        if (auto* block = stmt->else_statement->As<ast::BlockStatement>()) {
            EmitStatementsWithIndent(block->statements);
        } else {
            EmitStatementsWithIndent(Vector{stmt->else_statement});
        }
    }
    Line() << "}";
}

void ASTPrinter::EmitFunction(const ast::Function* func) {
    auto* sem = builder_.Sem().Get(func);

    if (ast::HasAttribute<ast::InternalAttribute>(func->attributes)) {
        // An internal function. Do not emit.
        return;
    }

    {
        auto out = Line();
        auto name = func->name->symbol.Name();
        EmitType(out, sem->ReturnType(), core::AddressSpace::kUndefined, core::Access::kReadWrite,
                 "");
        out << " " << name << "(";

        bool first = true;
        for (auto* v : sem->Parameters()) {
            if (!first) {
                out << ", ";
            }
            first = false;

            auto const* type = v->Type();

            if (auto* ptr = type->As<core::type::Pointer>()) {
                // Transform pointer parameters in to `inout` parameters.
                // The WGSL spec is highly restrictive in what can be passed in pointer
                // parameters, which allows for this transformation. See:
                // https://gpuweb.github.io/gpuweb/wgsl/#function-restriction
                out << "inout ";
                type = ptr->StoreType();
            }

            // Note: WGSL only allows for AddressSpace::kUndefined on parameters, however
            // the sanitizer transforms generates load / store functions for storage
            // or uniform buffers. These functions have a buffer parameter with
            // AddressSpace::kStorage or AddressSpace::kUniform. This is required to
            // correctly translate the parameter to a [RW]ByteAddressBuffer for
            // storage buffers and a uint4[N] for uniform buffers.
            EmitTypeAndName(out, type, v->AddressSpace(), v->Access(),
                            v->Declaration()->name->symbol.Name());
        }
        out << ") {";
    }

    EmitStatementsWithIndent(func->body->statements);

    Line() << "}";
    Line();
}

void ASTPrinter::EmitGlobalVariable(const ast::Variable* global) {
    Switch(
        global,  //
        [&](const ast::Var* var) {
            auto* sem = builder_.Sem().Get<sem::GlobalVariable>(global);
            switch (sem->AddressSpace()) {
                case core::AddressSpace::kUniform:
                    EmitUniformVariable(var, sem);
                    return;
                case core::AddressSpace::kStorage:
                    EmitStorageVariable(var, sem);
                    return;
                case core::AddressSpace::kHandle:
                    EmitHandleVariable(var, sem);
                    return;
                case core::AddressSpace::kPrivate:
                    EmitPrivateVariable(sem);
                    return;
                case core::AddressSpace::kWorkgroup:
                    EmitWorkgroupVariable(sem);
                    return;
                case core::AddressSpace::kIn:
                case core::AddressSpace::kOut:
                    EmitIOVariable(sem);
                    return;
                case core::AddressSpace::kPushConstant:
                    EmitPushConstant(sem);
                    return;
                default: {
                    TINT_ICE() << "unhandled address space " << sem->AddressSpace();
                }
            }
        },
        [&](const ast::Let* let) { EmitProgramConstVariable(let); },
        [&](const ast::Override*) {
            // Override is removed with SubstituteOverride
            diagnostics_.AddError(Source{})
                << "override-expressions should have been removed with the "
                   "SubstituteOverride transform";
        },
        [&](const ast::Const*) {
            // Constants are embedded at their use
        },  //
        TINT_ICE_ON_NO_MATCH);
}

void ASTPrinter::EmitUniformVariable(const ast::Var* var, const sem::GlobalVariable* sem) {
    auto* type = sem->Type()->UnwrapRef();
    auto* str = type->As<core::type::Struct>();
    if (DAWN_UNLIKELY(!str)) {
        TINT_ICE() << "storage variable must be of struct type";
    }
    auto bp = *sem->Attributes().binding_point;
    {
        auto out = Line();
        out << "layout(binding = " << bp.binding << ", std140";
        out << ") uniform " << UniqueIdentifier(StructName(str) + "_ubo") << " {";
    }
    EmitStructMembers(current_buffer_, str);
    auto name = var->name->symbol.Name();
    Line() << "} " << name << ";";
    Line();
}

void ASTPrinter::EmitStorageVariable(const ast::Var* var, const sem::GlobalVariable* sem) {
    auto* type = sem->Type()->UnwrapRef();
    auto* str = type->As<core::type::Struct>();
    if (DAWN_UNLIKELY(!str)) {
        TINT_ICE() << "storage variable must be of struct type";
    }
    auto bp = *sem->Attributes().binding_point;
    Line() << "layout(binding = " << bp.binding << ", std430) buffer "
           << UniqueIdentifier(StructName(str) + "_ssbo") << " {";
    EmitStructMembers(current_buffer_, str);
    auto name = var->name->symbol.Name();
    Line() << "} " << name << ";";
    Line();
}

void ASTPrinter::EmitHandleVariable(const ast::Var* var, const sem::GlobalVariable* sem) {
    auto out = Line();

    auto name = var->name->symbol.Name();
    auto* type = sem->Type()->UnwrapRef();
    if (type->Is<core::type::Sampler>()) {
        // GLSL ignores Sampler variables.
        return;
    }

    if (auto* storage = type->As<core::type::StorageTexture>()) {
        auto bp = *sem->Attributes().binding_point;
        out << "layout(binding = " << bp.binding << ", ";
        switch (storage->TexelFormat()) {
            case core::TexelFormat::kBgra8Unorm:
                TINT_ICE() << "bgra8unorm should have been polyfilled to rgba8unorm";
            case core::TexelFormat::kR32Uint:
                out << "r32ui";
                break;
            case core::TexelFormat::kR32Sint:
                out << "r32i";
                break;
            case core::TexelFormat::kR32Float:
                out << "r32f";
                break;
            case core::TexelFormat::kRgba8Unorm:
                out << "rgba8";
                break;
            case core::TexelFormat::kRgba8Snorm:
                out << "rgba8_snorm";
                break;
            case core::TexelFormat::kRgba8Uint:
                out << "rgba8ui";
                break;
            case core::TexelFormat::kRgba8Sint:
                out << "rgba8i";
                break;
            case core::TexelFormat::kRg32Uint:
                out << "rg32ui";
                break;
            case core::TexelFormat::kRg32Sint:
                out << "rg32i";
                break;
            case core::TexelFormat::kRg32Float:
                out << "rg32f";
                break;
            case core::TexelFormat::kRgba16Uint:
                out << "rgba16ui";
                break;
            case core::TexelFormat::kRgba16Sint:
                out << "rgba16i";
                break;
            case core::TexelFormat::kRgba16Float:
                out << "rgba16f";
                break;
            case core::TexelFormat::kRgba32Uint:
                out << "rgba32ui";
                break;
            case core::TexelFormat::kRgba32Sint:
                out << "rgba32i";
                break;
            case core::TexelFormat::kRgba32Float:
                out << "rgba32f";
                break;
            case core::TexelFormat::kR8Unorm:
                out << "r8";
                break;
            case core::TexelFormat::kUndefined:
                TINT_ICE() << "invalid texel format";
        }
        out << ") ";
    }
    EmitTypeAndName(out, type, sem->AddressSpace(), sem->Access(), name);
    out << ";";
}

void ASTPrinter::EmitPrivateVariable(const sem::Variable* var) {
    auto* decl = var->Declaration();
    auto out = Line();

    auto name = decl->name->symbol.Name();
    auto* type = var->Type()->UnwrapRef();
    EmitTypeAndName(out, type, var->AddressSpace(), var->Access(), name);

    out << " = ";
    if (auto* initializer = decl->initializer) {
        EmitExpression(out, initializer);
    } else {
        EmitZeroValue(out, var->Type()->UnwrapRef());
    }
    out << ";";
}

void ASTPrinter::EmitWorkgroupVariable(const sem::Variable* var) {
    auto* decl = var->Declaration();
    auto out = Line();

    out << "shared ";

    auto name = decl->name->symbol.Name();
    auto* type = var->Type()->UnwrapRef();
    EmitTypeAndName(out, type, var->AddressSpace(), var->Access(), name);

    if (auto* initializer = decl->initializer) {
        out << " = ";
        EmitExpression(out, initializer);
    }

    out << ";";
}

void ASTPrinter::EmitIOVariable(const sem::GlobalVariable* var) {
    auto* decl = var->Declaration();

    if (auto* attr = ast::GetAttribute<ast::BuiltinAttribute>(decl->attributes)) {
        // Use of gl_SampleID requires the GL_OES_sample_variables extension
        if (RequiresOESSampleVariables(attr->builtin)) {
            requires_oes_sample_variables_ = true;
        }
        // Do not emit builtin (gl_) variables.
        return;
    }

    auto out = Line();
    EmitAttributes(out, var);
    EmitInterpolationQualifiers(out, decl->attributes);

    auto name = decl->name->symbol.Name();
    auto* type = var->Type()->UnwrapRef();
    EmitTypeAndName(out, type, var->AddressSpace(), var->Access(), name);

    if (auto* initializer = decl->initializer) {
        out << " = ";
        EmitExpression(out, initializer);
    }
    out << ";";
}

void ASTPrinter::EmitPushConstant(const sem::GlobalVariable* var) {
    auto* decl = var->Declaration();

    auto out = Line();

    auto name = decl->name->symbol.Name();
    auto* type = var->Type()->UnwrapRef();
    out << "layout(location=0) ";
    EmitTypeAndName(out, type, var->AddressSpace(), var->Access(), name);
    out << ";";
}

void ASTPrinter::EmitInterpolationQualifiers(StringStream& out,
                                             VectorRef<const ast::Attribute*> attributes) {
    for (auto* attr : attributes) {
        if (auto* interpolate = attr->As<ast::InterpolateAttribute>()) {
            switch (interpolate->interpolation.type) {
                case core::InterpolationType::kPerspective:
                case core::InterpolationType::kLinear:
                case core::InterpolationType::kUndefined:
                    break;
                case core::InterpolationType::kFlat:
                    out << "flat ";
                    break;
            }

            switch (interpolate->interpolation.sampling) {
                case core::InterpolationSampling::kCentroid:
                    out << "centroid ";
                    break;
                case core::InterpolationSampling::kSample:
                case core::InterpolationSampling::kCenter:
                case core::InterpolationSampling::kFirst:
                case core::InterpolationSampling::kEither:
                case core::InterpolationSampling::kUndefined:
                    break;
            }
        }
    }
}

void ASTPrinter::EmitAttributes(StringStream& out, const sem::GlobalVariable* var) {
    auto& attrs = var->Attributes();

    bool first = true;
    if (attrs.location.has_value()) {
        out << (first ? "layout(" : ", ");
        out << "location = " << std::to_string(attrs.location.value());
        first = false;
    }
    if (attrs.blend_src.has_value()) {
        out << ", index = " << std::to_string(attrs.blend_src.value());
    }
    if (!first) {
        out << ") ";
    }
}

void ASTPrinter::EmitEntryPointFunction(const ast::Function* func) {
    auto* func_sem = builder_.Sem().Get(func);

    if (func->PipelineStage() == ast::PipelineStage::kFragment) {
        requires_default_precision_qualifier_ = true;
    }

    if (func->PipelineStage() == ast::PipelineStage::kCompute) {
        auto out = Line();
        // Emit the layout(local_size) attributes.
        auto wgsize = func_sem->WorkgroupSize();
        out << "layout(";
        for (size_t i = 0; i < 3; i++) {
            if (i > 0) {
                out << ", ";
            }
            out << "local_size_" << (i == 0 ? "x" : i == 1 ? "y" : "z") << " = ";

            if (!wgsize[i].has_value()) {
                diagnostics_.AddError(Source{})
                    << "override-expressions should have been removed with the SubstituteOverride "
                       "transform";
                return;
            }
            out << std::to_string(wgsize[i].value());
        }
        out << ") in;";
    }

    // Emit original entry point signature
    {
        auto out = Line();
        EmitTypeAndName(out, func_sem->ReturnType(), core::AddressSpace::kUndefined,
                        core::Access::kUndefined, func->name->symbol.Name());
        out << "(";

        bool first = true;

        // Emit entry point parameters.
        for (auto* var : func->params) {
            auto* sem = builder_.Sem().Get(var);
            auto* type = sem->Type();
            if (DAWN_UNLIKELY(!type->Is<core::type::Struct>())) {
                // ICE likely indicates that the CanonicalizeEntryPointIO transform was
                // not run, or a builtin parameter was added after it was run.
                TINT_ICE() << "Unsupported non-struct entry point parameter";
            }

            if (!first) {
                out << ", ";
            }
            first = false;

            EmitTypeAndName(out, type, sem->AddressSpace(), sem->Access(),
                            var->name->symbol.Name());
        }

        out << ") {";
    }

    // Emit original entry point function body
    {
        ScopedIndent si(this);
        if (func->PipelineStage() == ast::PipelineStage::kVertex) {
            Line() << "gl_PointSize = 1.0;";
        }

        EmitStatements(func->body->statements);

        if (!Is<ast::ReturnStatement>(func->body->Last())) {
            ast::ReturnStatement ret(GenerationID{}, ast::NodeID{}, Source{});
            EmitStatement(&ret);
        }
    }

    Line() << "}";
}

void ASTPrinter::EmitConstant(StringStream& out, const core::constant::Value* constant) {
    Switch(
        constant->Type(),  //
        [&](const core::type::Bool*) { out << (constant->ValueAs<AInt>() ? "true" : "false"); },
        [&](const core::type::F32*) { PrintF32(out, constant->ValueAs<f32>()); },
        [&](const core::type::F16*) { PrintF16(out, constant->ValueAs<f16>()); },
        [&](const core::type::I32*) { PrintI32(out, constant->ValueAs<i32>()); },
        [&](const core::type::U32*) { out << constant->ValueAs<AInt>() << "u"; },
        [&](const core::type::Vector* v) {
            EmitType(out, v, core::AddressSpace::kUndefined, core::Access::kUndefined, "");

            ScopedParen sp(out);

            if (auto* splat = constant->As<core::constant::Splat>()) {
                EmitConstant(out, splat->el);
                return;
            }

            for (size_t i = 0; i < v->Width(); i++) {
                if (i > 0) {
                    out << ", ";
                }
                EmitConstant(out, constant->Index(i));
            }
        },
        [&](const core::type::Matrix* m) {
            EmitType(out, m, core::AddressSpace::kUndefined, core::Access::kUndefined, "");

            ScopedParen sp(out);

            for (size_t column_idx = 0; column_idx < m->Columns(); column_idx++) {
                if (column_idx > 0) {
                    out << ", ";
                }
                EmitConstant(out, constant->Index(column_idx));
            }
        },
        [&](const core::type::Array* a) {
            EmitType(out, a, core::AddressSpace::kUndefined, core::Access::kUndefined, "");

            ScopedParen sp(out);

            auto count = a->ConstantCount();
            if (!count) {
                diagnostics_.AddError(Source{}) << core::type::Array::kErrExpectedConstantCount;
                return;
            }

            for (size_t i = 0; i < count; i++) {
                if (i > 0) {
                    out << ", ";
                }
                EmitConstant(out, constant->Index(i));
            }
        },
        [&](const core::type::Struct* s) {
            EmitStructType(&helpers_, s);

            out << StructName(s);

            ScopedParen sp(out);

            for (size_t i = 0; i < s->Members().Length(); i++) {
                if (i > 0) {
                    out << ", ";
                }
                EmitConstant(out, constant->Index(i));
            }
        },  //
        TINT_ICE_ON_NO_MATCH);
}

void ASTPrinter::EmitLiteral(StringStream& out, const ast::LiteralExpression* lit) {
    Switch(
        lit,  //
        [&](const ast::BoolLiteralExpression* l) { out << (l->value ? "true" : "false"); },
        [&](const ast::FloatLiteralExpression* l) {
            if (l->suffix == ast::FloatLiteralExpression::Suffix::kH) {
                PrintF16(out, static_cast<float>(l->value));
            } else {
                PrintF32(out, static_cast<float>(l->value));
            }
        },
        [&](const ast::IntLiteralExpression* i) {
            switch (i->suffix) {
                case ast::IntLiteralExpression::Suffix::kNone:
                case ast::IntLiteralExpression::Suffix::kI: {
                    PrintI32(out, static_cast<int32_t>(i->value));
                    return;
                }
                case ast::IntLiteralExpression::Suffix::kU: {
                    out << i->value << "u";
                    return;
                }
            }
            diagnostics_.AddError(Source{}) << "unknown integer literal suffix type";
        },  //
        TINT_ICE_ON_NO_MATCH);
}

void ASTPrinter::EmitZeroValue(StringStream& out, const core::type::Type* type) {
    if (type->Is<core::type::Bool>()) {
        out << "false";
    } else if (type->Is<core::type::F32>()) {
        out << "0.0f";
    } else if (type->Is<core::type::F16>()) {
        out << "0.0hf";
    } else if (type->Is<core::type::I32>()) {
        out << "0";
    } else if (type->Is<core::type::U32>()) {
        out << "0u";
    } else if (auto* vec = type->As<core::type::Vector>()) {
        EmitType(out, type, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
        ScopedParen sp(out);
        for (uint32_t i = 0; i < vec->Width(); i++) {
            if (i != 0) {
                out << ", ";
            }
            EmitZeroValue(out, vec->Type());
        }
    } else if (auto* mat = type->As<core::type::Matrix>()) {
        EmitType(out, type, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
        ScopedParen sp(out);
        for (uint32_t i = 0; i < (mat->Rows() * mat->Columns()); i++) {
            if (i != 0) {
                out << ", ";
            }
            EmitZeroValue(out, mat->Type());
        }
    } else if (auto* str = type->As<core::type::Struct>()) {
        EmitType(out, type, core::AddressSpace::kUndefined, core::Access::kUndefined, "");
        bool first = true;
        ScopedParen sp(out);
        for (auto* member : str->Members()) {
            if (!first) {
                out << ", ";
            } else {
                first = false;
            }
            EmitZeroValue(out, member->Type());
        }
    } else if (auto* arr = type->As<core::type::Array>()) {
        EmitType(out, type, core::AddressSpace::kUndefined, core::Access::kUndefined, "");
        ScopedParen sp(out);

        auto count = arr->ConstantCount();
        if (!count) {
            diagnostics_.AddError(Source{}) << core::type::Array::kErrExpectedConstantCount;
            return;
        }

        for (uint32_t i = 0; i < count; i++) {
            if (i != 0) {
                out << ", ";
            }
            EmitZeroValue(out, arr->ElemType());
        }
    } else {
        diagnostics_.AddError(Source{})
            << "Invalid type for zero emission: " << type->FriendlyName();
    }
}

void ASTPrinter::EmitLoop(const ast::LoopStatement* stmt) {
    auto emit_continuing = [this, stmt] {
        if (stmt->continuing && !stmt->continuing->Empty()) {
            EmitBlock(stmt->continuing);
        }
    };

    TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);
    Line() << "while (true) {";
    {
        ScopedIndent si(this);
        EmitStatements(stmt->body->statements);
        emit_continuing_();
    }
    Line() << "}";
}

void ASTPrinter::EmitForLoop(const ast::ForLoopStatement* stmt) {
    // Nest a for loop with a new block. In HLSL the initializer scope is not
    // nested by the for-loop, so we may get variable redefinitions.
    Line() << "{";
    IncrementIndent();
    TINT_DEFER({
        DecrementIndent();
        Line() << "}";
    });

    TextBuffer init_buf;
    if (auto* init = stmt->initializer) {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &init_buf);
        EmitStatement(init);
    }

    TextBuffer cond_pre;
    StringStream cond_buf;
    if (auto* cond = stmt->condition) {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &cond_pre);
        EmitExpression(cond_buf, cond);
    }

    TextBuffer cont_buf;
    if (auto* cont = stmt->continuing) {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &cont_buf);
        EmitStatement(cont);
    }

    // If the for-loop has a multi-statement conditional and / or continuing, then
    // we cannot emit this as a regular for-loop in HLSL. Instead we need to
    // generate a `while(true)` loop.
    bool emit_as_loop = cond_pre.lines.size() > 0 || cont_buf.lines.size() > 1;

    // If the for-loop has multi-statement initializer, or is going to be emitted
    // as a `while(true)` loop, then declare the initializer statement(s) before
    // the loop.
    if (init_buf.lines.size() > 1 || (stmt->initializer && emit_as_loop)) {
        current_buffer_->Append(init_buf);
        init_buf.lines.clear();  // Don't emit the initializer again in the 'for'
    }

    if (emit_as_loop) {
        auto emit_continuing = [&] { current_buffer_->Append(cont_buf); };

        TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);
        Line() << "while (true) {";
        IncrementIndent();
        TINT_DEFER({
            DecrementIndent();
            Line() << "}";
        });

        if (stmt->condition) {
            current_buffer_->Append(cond_pre);
            Line() << "if (!(" << cond_buf.str() << ")) { break; }";
        }

        EmitStatements(stmt->body->statements);
        emit_continuing_();
    } else {
        // For-loop can be generated.
        {
            auto out = Line();
            out << "for";
            {
                ScopedParen sp(out);

                if (!init_buf.lines.empty()) {
                    out << init_buf.lines[0].content << " ";
                } else {
                    out << "; ";
                }

                out << cond_buf.str() << "; ";

                if (!cont_buf.lines.empty()) {
                    out << tint::TrimSuffix(cont_buf.lines[0].content, ";");
                }
            }
            out << " {";
        }
        {
            auto emit_continuing = [] { return true; };
            TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);
            EmitStatementsWithIndent(stmt->body->statements);
        }
        Line() << "}";
    }
}

void ASTPrinter::EmitWhile(const ast::WhileStatement* stmt) {
    TextBuffer cond_pre;
    StringStream cond_buf;
    {
        auto* cond = stmt->condition;
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &cond_pre);
        EmitExpression(cond_buf, cond);
    }

    auto emit_continuing = [&] {};
    TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);

    // If the whilehas a multi-statement conditional, then we cannot emit this
    // as a regular while in GLSL. Instead we need to generate a `while(true)` loop.
    bool emit_as_loop = cond_pre.lines.size() > 0;
    if (emit_as_loop) {
        Line() << "while (true) {";
        IncrementIndent();
        TINT_DEFER({
            DecrementIndent();
            Line() << "}";
        });

        current_buffer_->Append(cond_pre);
        Line() << "if (!(" << cond_buf.str() << ")) { break; }";

        EmitStatements(stmt->body->statements);
    } else {
        // While can be generated.
        {
            auto out = Line();
            out << "while";
            {
                ScopedParen sp(out);
                out << cond_buf.str();
            }
            out << " {";
        }
        EmitStatementsWithIndent(stmt->body->statements);
        Line() << "}";
    }
}

void ASTPrinter::EmitMemberAccessor(StringStream& out, const ast::MemberAccessorExpression* expr) {
    EmitExpression(out, expr->object);
    out << ".";

    auto* sem = builder_.Sem().Get(expr)->UnwrapLoad();

    Switch(
        sem,
        [&](const sem::Swizzle*) {
            // Swizzles output the name directly
            out << expr->member->symbol.Name();
        },
        [&](const sem::StructMemberAccess* member_access) {
            out << member_access->Member()->Name().Name();
        },  //
        TINT_ICE_ON_NO_MATCH);
}

void ASTPrinter::EmitReturn(const ast::ReturnStatement* stmt) {
    if (stmt->value) {
        auto out = Line();
        out << "return ";
        EmitExpression(out, stmt->value);
        out << ";";
    } else {
        Line() << "return;";
    }
}

void ASTPrinter::EmitStatement(const ast::Statement* stmt) {
    Switch(
        stmt,  //
        [&](const ast::AssignmentStatement* a) { EmitAssign(a); },
        [&](const ast::BlockStatement* b) { EmitBlock(b); },
        [&](const ast::BreakStatement* b) { EmitBreak(b); },
        [&](const ast::BreakIfStatement* b) { EmitBreakIf(b); },
        [&](const ast::CallStatement* c) {
            auto out = Line();
            EmitCall(out, c->expr);
            out << ";";
        },
        [&](const ast::ContinueStatement* c) { EmitContinue(c); },
        [&](const ast::DiscardStatement* d) { EmitDiscard(d); },
        [&](const ast::IfStatement* i) { EmitIf(i); },
        [&](const ast::LoopStatement* l) { EmitLoop(l); },
        [&](const ast::ForLoopStatement* l) { EmitForLoop(l); },
        [&](const ast::WhileStatement* l) { EmitWhile(l); },
        [&](const ast::ReturnStatement* r) { EmitReturn(r); },
        [&](const ast::SwitchStatement* s) { EmitSwitch(s); },
        [&](const ast::VariableDeclStatement* v) {
            Switch(
                v->variable,  //
                [&](const ast::Var* var) { EmitVar(var); },
                [&](const ast::Let* let) { EmitLet(let); },
                [&](const ast::Const*) {
                    // Constants are embedded at their use
                },  //
                TINT_ICE_ON_NO_MATCH);
        },
        [&](const ast::ConstAssert*) {
            // Not emitted
        },  //
        TINT_ICE_ON_NO_MATCH);
}

void ASTPrinter::EmitSwitch(const ast::SwitchStatement* stmt) {
    {  // switch(expr) {
        auto out = Line();
        out << "switch(";
        EmitExpression(out, stmt->condition);
        out << ") {";
    }

    {
        ScopedIndent si(this);
        for (auto* s : stmt->body) {
            EmitCase(s);
        }
    }

    Line() << "}";
}

void ASTPrinter::EmitType(StringStream& out,
                          const core::type::Type* type,
                          core::AddressSpace address_space,
                          core::Access access,
                          const std::string& name,
                          bool* name_printed /* = nullptr */) {
    if (name_printed) {
        *name_printed = false;
    }
    switch (address_space) {
        case core::AddressSpace::kIn: {
            out << "in ";
            break;
        }
        case core::AddressSpace::kOut: {
            out << "out ";
            break;
        }
        case core::AddressSpace::kUniform:
        case core::AddressSpace::kPushConstant:
        case core::AddressSpace::kHandle: {
            out << "uniform ";
            break;
        }
        default:
            break;
    }

    if (auto* ary = type->As<core::type::Array>()) {
        const core::type::Type* base_type = ary;
        std::vector<uint32_t> sizes;
        while (auto* arr = base_type->As<core::type::Array>()) {
            if (arr->Count()->Is<core::type::RuntimeArrayCount>()) {
                sizes.push_back(0);
            } else {
                auto count = arr->ConstantCount();
                if (!count) {
                    diagnostics_.AddError(Source{}) << core::type::Array::kErrExpectedConstantCount;
                    return;
                }
                sizes.push_back(count.value());
            }

            base_type = arr->ElemType();
        }
        EmitType(out, base_type, address_space, access, "");
        if (!name.empty()) {
            out << " " << name;
            if (name_printed) {
                *name_printed = true;
            }
        }
        for (uint32_t size : sizes) {
            if (size > 0) {
                out << "[" << size << "]";
            } else {
                out << "[]";
            }
        }
    } else if (type->Is<core::type::Bool>()) {
        out << "bool";
    } else if (type->Is<core::type::F32>()) {
        out << "float";
    } else if (type->Is<core::type::F16>()) {
        out << "float16_t";
    } else if (type->Is<core::type::I32>()) {
        out << "int";
    } else if (auto* mat = type->As<core::type::Matrix>()) {
        TINT_ASSERT((mat->Type()->IsAnyOf<core::type::F32, core::type::F16>()));
        if (mat->Type()->Is<core::type::F16>()) {
            out << "f16";
        }
        out << "mat" << mat->Columns();
        if (mat->Rows() != mat->Columns()) {
            out << "x" << mat->Rows();
        }
    } else if (DAWN_UNLIKELY(type->Is<core::type::Pointer>())) {
        TINT_ICE() << "Attempting to emit pointer type. These should have been removed with the "
                      "SimplifyPointers transform";
    } else if (type->Is<core::type::Sampler>()) {
    } else if (auto* str = type->As<core::type::Struct>()) {
        out << StructName(str);
    } else if (auto* tex = type->As<core::type::Texture>()) {
        if (DAWN_UNLIKELY(tex->Is<core::type::ExternalTexture>())) {
            TINT_ICE() << "Multiplanar external texture transform was not run.";
        }

        auto* storage = tex->As<core::type::StorageTexture>();
        auto* ms = tex->As<core::type::MultisampledTexture>();
        auto* depth_ms = tex->As<core::type::DepthMultisampledTexture>();
        auto* sampled = tex->As<core::type::SampledTexture>();

        out << "highp ";

        if (storage) {
            switch (storage->Access()) {
                case core::Access::kRead:
                    out << "readonly ";
                    break;
                case core::Access::kWrite:
                    out << "writeonly ";
                    break;
                case core::Access::kReadWrite: {
                    if (version_.IsES()) {
                        // ESSL 3.1 SPEC (chapter 4.9, Memory Access Qualifiers):
                        // Except for image variables qualified with the format qualifiers r32f,
                        // r32i, and r32ui, image variables must specify either memory qualifier
                        // readonly or the memory qualifier writeonly.
                        switch (storage->TexelFormat()) {
                            case core::TexelFormat::kR32Float:
                            case core::TexelFormat::kR32Sint:
                            case core::TexelFormat::kR32Uint:
                                break;
                            default: {
                                // TODO(dawn:1972): Fix the tests that contain read-write storage
                                // textures with illegal formats.
                                out << "writeonly ";
                                break;
                            }
                        }
                    }
                    break;
                }
                default:
                    TINT_UNREACHABLE() << "unexpected storage texture access " << storage->Access();
            }
        }
        auto* subtype = sampled   ? sampled->Type()
                        : storage ? storage->Type()
                        : ms      ? ms->Type()
                                  : nullptr;
        if (!subtype || subtype->Is<core::type::F32>()) {
        } else if (subtype->Is<core::type::I32>()) {
            out << "i";
        } else if (DAWN_LIKELY(subtype->Is<core::type::U32>())) {
            out << "u";
        } else {
            TINT_ICE() << "Unsupported texture type";
        }

        out << (storage ? "image" : "sampler");

        switch (tex->Dim()) {
            case core::type::TextureDimension::k1d:
                out << "1D";
                break;
            case core::type::TextureDimension::k2d:
                out << ((ms || depth_ms) ? "2DMS" : "2D");
                break;
            case core::type::TextureDimension::k2dArray:
                out << ((ms || depth_ms) ? "2DMSArray" : "2DArray");
                break;
            case core::type::TextureDimension::k3d:
                out << "3D";
                break;
            case core::type::TextureDimension::kCube:
                out << "Cube";
                break;
            case core::type::TextureDimension::kCubeArray:
                out << "CubeArray";
                break;
            default:
                TINT_UNREACHABLE() << "unexpected TextureDimension " << tex->Dim();
        }
        if (tex->Is<core::type::DepthTexture>()) {
            out << "Shadow";
        }
    } else if (type->Is<core::type::U32>()) {
        out << "uint";
    } else if (auto* vec = type->As<core::type::Vector>()) {
        auto width = vec->Width();
        if (vec->Type()->Is<core::type::F32>() && width >= 1 && width <= 4) {
            out << "vec" << width;
        } else if (vec->Type()->Is<core::type::F16>() && width >= 1 && width <= 4) {
            out << "f16vec" << width;
        } else if (vec->Type()->Is<core::type::I32>() && width >= 1 && width <= 4) {
            out << "ivec" << width;
        } else if (vec->Type()->Is<core::type::U32>() && width >= 1 && width <= 4) {
            out << "uvec" << width;
        } else if (vec->Type()->Is<core::type::Bool>() && width >= 1 && width <= 4) {
            out << "bvec" << width;
        } else {
            out << "vector<";
            EmitType(out, vec->Type(), address_space, access, "");
            out << ", " << width << ">";
        }
    } else if (auto* atomic = type->As<core::type::Atomic>()) {
        EmitType(out, atomic->Type(), address_space, access, name);
    } else if (type->Is<core::type::Void>()) {
        out << "void";
    } else {
        diagnostics_.AddError(Source{}) << "unknown type in EmitType";
    }
}

void ASTPrinter::EmitTypeAndName(StringStream& out,
                                 const core::type::Type* type,
                                 core::AddressSpace address_space,
                                 core::Access access,
                                 const std::string& name) {
    bool printed_name = false;
    EmitType(out, type, address_space, access, name, &printed_name);
    if (!name.empty() && !printed_name) {
        out << " " << name;
    }
}

void ASTPrinter::EmitStructType(TextBuffer* b, const core::type::Struct* str) {
    auto it = emitted_structs_.emplace(str);
    if (!it.second) {
        return;
    }

    auto address_space_uses = str->AddressSpaceUsage();
    Line(b) << "struct " << StructName(str) << " {";
    EmitStructMembers(b, str);
    Line(b) << "};";
    Line(b);
}

void ASTPrinter::EmitStructMembers(TextBuffer* b, const core::type::Struct* str) {
    ScopedIndent si(b);
    for (auto* mem : str->Members()) {
        auto name = mem->Name().Name();
        auto* ty = mem->Type();

        auto out = Line(b);
        EmitTypeAndName(out, ty, core::AddressSpace::kUndefined, core::Access::kReadWrite, name);
        out << ";";
    }
}

void ASTPrinter::EmitUnaryOp(StringStream& out, const ast::UnaryOpExpression* expr) {
    switch (expr->op) {
        case core::UnaryOp::kIndirection:
        case core::UnaryOp::kAddressOf:
            EmitExpression(out, expr->expr);
            return;
        case core::UnaryOp::kComplement:
            out << "~";
            break;
        case core::UnaryOp::kNot:
            if (TypeOf(expr)->UnwrapRef()->Is<core::type::Scalar>()) {
                out << "!";
            } else {
                out << "not";
            }
            break;
        case core::UnaryOp::kNegation:
            out << "-";
            break;
    }

    ScopedParen sp(out);
    EmitExpression(out, expr->expr);
}

void ASTPrinter::EmitVar(const ast::Var* var) {
    auto* sem = builder_.Sem().Get(var);
    auto* type = sem->Type()->UnwrapRef();

    auto out = Line();
    EmitTypeAndName(out, type, sem->AddressSpace(), sem->Access(), var->name->symbol.Name());

    out << " = ";

    if (var->initializer) {
        EmitExpression(out, var->initializer);
    } else {
        EmitZeroValue(out, type);
    }
    out << ";";
}

void ASTPrinter::EmitLet(const ast::Let* let) {
    auto* sem = builder_.Sem().Get(let);
    auto* type = sem->Type()->UnwrapRef();

    auto out = Line();
    // TODO(senorblanco): handle const
    EmitTypeAndName(out, type, core::AddressSpace::kUndefined, core::Access::kUndefined,
                    let->name->symbol.Name());

    out << " = ";
    EmitExpression(out, let->initializer);
    out << ";";
}

void ASTPrinter::EmitProgramConstVariable(const ast::Variable* var) {
    auto* sem = builder_.Sem().Get(var);
    auto* type = sem->Type();

    auto out = Line();
    out << "const ";
    EmitTypeAndName(out, type, core::AddressSpace::kUndefined, core::Access::kUndefined,
                    var->name->symbol.Name());
    out << " = ";
    EmitExpression(out, var->initializer);
    out << ";";
}

template <typename F>
void ASTPrinter::CallBuiltinHelper(StringStream& out,
                                   const ast::CallExpression* call,
                                   const sem::BuiltinFn* builtin,
                                   F&& build) {
    // Generate the helper function if it hasn't been created already
    auto fn = tint::GetOrAdd(builtins_, builtin, [&]() -> std::string {
        TextBuffer b;
        TINT_DEFER(helpers_.Append(b));

        auto fn_name = UniqueIdentifier(std::string("tint_") + wgsl::str(builtin->Fn()));
        std::vector<std::string> parameter_names;
        {
            auto decl = Line(&b);
            EmitTypeAndName(decl, builtin->ReturnType(), core::AddressSpace::kUndefined,
                            core::Access::kUndefined, fn_name);
            {
                ScopedParen sp(decl);
                for (auto* param : builtin->Parameters()) {
                    if (!parameter_names.empty()) {
                        decl << ", ";
                    }
                    auto param_name = "param_" + std::to_string(parameter_names.size());
                    const auto* ty = param->Type();
                    if (auto* ptr = ty->As<core::type::Pointer>()) {
                        decl << "inout ";
                        ty = ptr->StoreType();
                    }
                    EmitTypeAndName(decl, ty, core::AddressSpace::kUndefined,
                                    core::Access::kUndefined, param_name);
                    parameter_names.emplace_back(std::move(param_name));
                }
            }
            decl << " {";
        }
        {
            ScopedIndent si(&b);
            build(&b, parameter_names);
        }
        Line(&b) << "}";
        Line(&b);
        return fn_name;
    });

    // Call the helper
    out << fn;
    {
        ScopedParen sp(out);
        bool first = true;
        for (auto* arg : call->args) {
            if (!first) {
                out << ", ";
            }
            first = false;
            EmitExpression(out, arg);
        }
    }
}

core::type::Type* ASTPrinter::BoolTypeToUint(const core::type::Type* type) {
    auto* u32 = builder_.create<core::type::U32>();
    if (type->Is<core::type::Bool>()) {
        return u32;
    } else if (auto* vec = type->As<core::type::Vector>()) {
        return builder_.create<core::type::Vector>(u32, vec->Width());
    } else {
        return nullptr;
    }
}

std::string ASTPrinter::StructName(const core::type::Struct* s) {
    auto name = s->Name().Name();
    if (HasPrefix(name, "__")) {
        name = tint::GetOrAdd(builtin_struct_names_, s,
                              [&] { return UniqueIdentifier(name.substr(2)); });
    }
    return name;
}

std::string ASTPrinter::UniqueIdentifier(const std::string& prefix /* = "" */) {
    return builder_.Symbols().New(prefix).Name();
}

}  // namespace tint::glsl::writer
