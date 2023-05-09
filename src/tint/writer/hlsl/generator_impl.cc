/// Copyright 2020 The Tint Authors.
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

#include "src/tint/writer/hlsl/generator_impl.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iomanip>
#include <set>
#include <utility>
#include <vector>

#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/internal_attribute.h"
#include "src/tint/ast/interpolate_attribute.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/constant/value.h"
#include "src/tint/debug.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/module.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/struct.h"
#include "src/tint/sem/switch_statement.h"
#include "src/tint/sem/value_constructor.h"
#include "src/tint/sem/value_conversion.h"
#include "src/tint/sem/variable.h"
#include "src/tint/switch.h"
#include "src/tint/transform/add_empty_entry_point.h"
#include "src/tint/transform/array_length_from_uniform.h"
#include "src/tint/transform/binding_remapper.h"
#include "src/tint/transform/builtin_polyfill.h"
#include "src/tint/transform/calculate_array_length.h"
#include "src/tint/transform/canonicalize_entry_point_io.h"
#include "src/tint/transform/decompose_memory_access.h"
#include "src/tint/transform/demote_to_helper.h"
#include "src/tint/transform/direct_variable_access.h"
#include "src/tint/transform/disable_uniformity_analysis.h"
#include "src/tint/transform/expand_compound_assignment.h"
#include "src/tint/transform/localize_struct_array_assignment.h"
#include "src/tint/transform/manager.h"
#include "src/tint/transform/multiplanar_external_texture.h"
#include "src/tint/transform/num_workgroups_from_uniform.h"
#include "src/tint/transform/promote_initializers_to_let.h"
#include "src/tint/transform/promote_side_effects_to_decl.h"
#include "src/tint/transform/remove_continue_in_switch.h"
#include "src/tint/transform/remove_phonies.h"
#include "src/tint/transform/robustness.h"
#include "src/tint/transform/simplify_pointers.h"
#include "src/tint/transform/truncate_interstage_variables.h"
#include "src/tint/transform/unshadow.h"
#include "src/tint/transform/vectorize_scalar_matrix_initializers.h"
#include "src/tint/transform/zero_init_workgroup_memory.h"
#include "src/tint/type/array.h"
#include "src/tint/type/atomic.h"
#include "src/tint/type/depth_multisampled_texture.h"
#include "src/tint/type/depth_texture.h"
#include "src/tint/type/multisampled_texture.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/storage_texture.h"
#include "src/tint/type/texture_dimension.h"
#include "src/tint/utils/compiler_macros.h"
#include "src/tint/utils/defer.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/scoped_assignment.h"
#include "src/tint/utils/string.h"
#include "src/tint/utils/string_stream.h"
#include "src/tint/writer/append_vector.h"
#include "src/tint/writer/check_supported_extensions.h"
#include "src/tint/writer/float_to_string.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::hlsl {
namespace {

const char kTempNamePrefix[] = "tint_tmp";

const char* image_format_to_rwtexture_type(builtin::TexelFormat image_format) {
    switch (image_format) {
        case builtin::TexelFormat::kBgra8Unorm:
        case builtin::TexelFormat::kRgba8Unorm:
        case builtin::TexelFormat::kRgba8Snorm:
        case builtin::TexelFormat::kRgba16Float:
        case builtin::TexelFormat::kR32Float:
        case builtin::TexelFormat::kRg32Float:
        case builtin::TexelFormat::kRgba32Float:
            return "float4";
        case builtin::TexelFormat::kRgba8Uint:
        case builtin::TexelFormat::kRgba16Uint:
        case builtin::TexelFormat::kR32Uint:
        case builtin::TexelFormat::kRg32Uint:
        case builtin::TexelFormat::kRgba32Uint:
            return "uint4";
        case builtin::TexelFormat::kRgba8Sint:
        case builtin::TexelFormat::kRgba16Sint:
        case builtin::TexelFormat::kR32Sint:
        case builtin::TexelFormat::kRg32Sint:
        case builtin::TexelFormat::kRgba32Sint:
            return "int4";
        default:
            return nullptr;
    }
}

void PrintF32(utils::StringStream& out, float value) {
    if (std::isinf(value)) {
        out << "0.0f " << (value >= 0 ? "/* inf */" : "/* -inf */");
    } else if (std::isnan(value)) {
        out << "0.0f /* nan */";
    } else {
        out << FloatToString(value) << "f";
    }
}

void PrintF16(utils::StringStream& out, float value) {
    if (std::isinf(value)) {
        out << "0.0h " << (value >= 0 ? "/* inf */" : "/* -inf */");
    } else if (std::isnan(value)) {
        out << "0.0h /* nan */";
    } else {
        out << FloatToString(value) << "h";
    }
}

// Helper for writing " : register(RX, spaceY)", where R is the register, X is
// the binding point binding value, and Y is the binding point group value.
struct RegisterAndSpace {
    RegisterAndSpace(char r, sem::BindingPoint bp) : reg(r), binding_point(bp) {}

    const char reg;
    sem::BindingPoint const binding_point;
};

utils::StringStream& operator<<(utils::StringStream& s, const RegisterAndSpace& rs) {
    s << " : register(" << rs.reg << rs.binding_point.binding;
    // Omit the space if it's 0, as it's the default.
    // SM 5.0 doesn't support spaces, so we don't emit them if group is 0 for better compatibility.
    if (rs.binding_point.group == 0) {
        s << ")";
    } else {
        s << ", space" << rs.binding_point.group << ")";
    }
    return s;
}

}  // namespace

SanitizedResult::SanitizedResult() = default;
SanitizedResult::~SanitizedResult() = default;
SanitizedResult::SanitizedResult(SanitizedResult&&) = default;

SanitizedResult Sanitize(const Program* in, const Options& options) {
    transform::Manager manager;
    transform::DataMap data;

    manager.Add<transform::DisableUniformityAnalysis>();

    // ExpandCompoundAssignment must come before BuiltinPolyfill
    manager.Add<transform::ExpandCompoundAssignment>();

    manager.Add<transform::Unshadow>();  // Must come before DirectVariableAccess

    // LocalizeStructArrayAssignment must come after:
    // * SimplifyPointers, because it assumes assignment to arrays in structs are
    // done directly, not indirectly.
    // TODO(crbug.com/tint/1340): See if we can get rid of the duplicate
    // SimplifyPointers transform. Can't do it right now because
    // LocalizeStructArrayAssignment introduces pointers.
    manager.Add<transform::SimplifyPointers>();
    manager.Add<transform::LocalizeStructArrayAssignment>();

    manager.Add<transform::PromoteSideEffectsToDecl>();

    if (!options.disable_robustness) {
        // Robustness must come after PromoteSideEffectsToDecl
        // Robustness must come before BuiltinPolyfill and CanonicalizeEntryPointIO
        manager.Add<transform::Robustness>();
    }

    // Note: it is more efficient for MultiplanarExternalTexture to come after Robustness
    data.Add<transform::MultiplanarExternalTexture::NewBindingPoints>(
        options.external_texture_options.bindings_map);
    manager.Add<transform::MultiplanarExternalTexture>();

    // BindingRemapper must come after MultiplanarExternalTexture
    manager.Add<transform::BindingRemapper>();
    data.Add<transform::BindingRemapper::Remappings>(
        options.binding_remapper_options.binding_points,
        options.binding_remapper_options.access_controls,
        options.binding_remapper_options.allow_collisions);

    {  // Builtin polyfills
        transform::BuiltinPolyfill::Builtins polyfills;
        polyfills.acosh = transform::BuiltinPolyfill::Level::kFull;
        polyfills.asinh = true;
        polyfills.atanh = transform::BuiltinPolyfill::Level::kFull;
        polyfills.bitshift_modulo = true;
        polyfills.clamp_int = true;
        // TODO(crbug.com/tint/1449): Some of these can map to HLSL's `firstbitlow`
        // and `firstbithigh`.
        polyfills.conv_f32_to_iu32 = true;
        polyfills.count_leading_zeros = true;
        polyfills.count_trailing_zeros = true;
        polyfills.extract_bits = transform::BuiltinPolyfill::Level::kFull;
        polyfills.first_leading_bit = true;
        polyfills.first_trailing_bit = true;
        polyfills.insert_bits = transform::BuiltinPolyfill::Level::kFull;
        polyfills.int_div_mod = true;
        polyfills.precise_float_mod = true;
        polyfills.reflect_vec2_f32 = options.polyfill_reflect_vec2_f32;
        polyfills.texture_sample_base_clamp_to_edge_2d_f32 = true;
        polyfills.workgroup_uniform_load = true;
        data.Add<transform::BuiltinPolyfill::Config>(polyfills);
        manager.Add<transform::BuiltinPolyfill>();  // Must come before DirectVariableAccess
    }

    manager.Add<transform::DirectVariableAccess>();

    if (!options.disable_workgroup_init) {
        // ZeroInitWorkgroupMemory must come before CanonicalizeEntryPointIO as
        // ZeroInitWorkgroupMemory may inject new builtin parameters.
        manager.Add<transform::ZeroInitWorkgroupMemory>();
    }

    // CanonicalizeEntryPointIO must come after Robustness
    manager.Add<transform::CanonicalizeEntryPointIO>();

    if (options.truncate_interstage_variables) {
        // When interstage_locations is empty, it means there's no user-defined interstage variables
        // being used in the next stage. Still, HLSL compiler register mismatch could happen, if
        // there's builtin inputs used in the next stage. So we still run
        // TruncateInterstageVariables transform.

        // TruncateInterstageVariables itself will skip when interstage_locations matches exactly
        // with the current stage output.

        // Build the config for internal TruncateInterstageVariables transform.
        transform::TruncateInterstageVariables::Config truncate_interstage_variables_cfg;
        truncate_interstage_variables_cfg.interstage_locations =
            std::move(options.interstage_locations);
        manager.Add<transform::TruncateInterstageVariables>();
        data.Add<transform::TruncateInterstageVariables::Config>(
            std::move(truncate_interstage_variables_cfg));
    }

    // NumWorkgroupsFromUniform must come after CanonicalizeEntryPointIO, as it
    // assumes that num_workgroups builtins only appear as struct members and are
    // only accessed directly via member accessors.
    manager.Add<transform::NumWorkgroupsFromUniform>();
    manager.Add<transform::VectorizeScalarMatrixInitializers>();
    manager.Add<transform::SimplifyPointers>();
    manager.Add<transform::RemovePhonies>();

    // Build the config for the internal ArrayLengthFromUniform transform.
    auto& array_length_from_uniform = options.array_length_from_uniform;
    transform::ArrayLengthFromUniform::Config array_length_from_uniform_cfg(
        array_length_from_uniform.ubo_binding);
    array_length_from_uniform_cfg.bindpoint_to_size_index =
        array_length_from_uniform.bindpoint_to_size_index;

    // DemoteToHelper must come after CanonicalizeEntryPointIO, PromoteSideEffectsToDecl, and
    // ExpandCompoundAssignment.
    // TODO(crbug.com/tint/1752): This is only necessary when FXC is being used.
    manager.Add<transform::DemoteToHelper>();

    // ArrayLengthFromUniform must come after SimplifyPointers as it assumes that the form of the
    // array length argument is &var.array.
    manager.Add<transform::ArrayLengthFromUniform>();
    data.Add<transform::ArrayLengthFromUniform::Config>(std::move(array_length_from_uniform_cfg));
    // DecomposeMemoryAccess must come after:
    // * SimplifyPointers, as we cannot take the address of calls to
    //   DecomposeMemoryAccess::Intrinsic and we need to fold away the address-of and dereferences
    //   of `*(&(intrinsic_load()))` expressions.
    // * RemovePhonies, as phonies can be assigned a pointer to a
    //   non-constructible buffer, or dynamic array, which DMA cannot cope with.
    manager.Add<transform::DecomposeMemoryAccess>();
    // CalculateArrayLength must come after DecomposeMemoryAccess, as
    // DecomposeMemoryAccess special-cases the arrayLength() intrinsic, which
    // will be transformed by CalculateArrayLength
    manager.Add<transform::CalculateArrayLength>();
    manager.Add<transform::PromoteInitializersToLet>();

    manager.Add<transform::RemoveContinueInSwitch>();

    manager.Add<transform::AddEmptyEntryPoint>();

    data.Add<transform::CanonicalizeEntryPointIO::Config>(
        transform::CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    data.Add<transform::NumWorkgroupsFromUniform::Config>(options.root_constant_binding_point);

    auto out = manager.Run(in, data);

    SanitizedResult result;
    result.program = std::move(out.program);
    if (auto* res = out.data.Get<transform::ArrayLengthFromUniform::Result>()) {
        result.used_array_length_from_uniform_indices = std::move(res->used_size_indices);
    }
    return result;
}

GeneratorImpl::GeneratorImpl(const Program* program) : TextGenerator(program) {}

GeneratorImpl::~GeneratorImpl() = default;

bool GeneratorImpl::Generate() {
    if (!CheckSupportedExtensions("HLSL", program_->AST(), diagnostics_,
                                  utils::Vector{
                                      builtin::Extension::kChromiumDisableUniformityAnalysis,
                                      builtin::Extension::kChromiumExperimentalDp4A,
                                      builtin::Extension::kChromiumExperimentalFullPtrParameters,
                                      builtin::Extension::kChromiumExperimentalPushConstant,
                                      builtin::Extension::kF16,
                                  })) {
        return false;
    }

    const utils::TypeInfo* last_kind = nullptr;
    size_t last_padding_line = 0;

    auto* mod = builder_.Sem().Module();
    for (auto* decl : mod->DependencyOrderedDeclarations()) {
        if (decl->IsAnyOf<ast::Alias, ast::DiagnosticDirective, ast::Enable, ast::ConstAssert>()) {
            continue;  // These are not emitted.
        }

        // Emit a new line between declarations if the type of declaration has
        // changed, or we're about to emit a function
        auto* kind = &decl->TypeInfo();
        if (current_buffer_->lines.size() != last_padding_line) {
            if (last_kind && (last_kind != kind || decl->Is<ast::Function>())) {
                line();
                last_padding_line = current_buffer_->lines.size();
            }
        }
        last_kind = kind;

        bool ok = Switch(
            decl,
            [&](const ast::Variable* global) {  //
                return EmitGlobalVariable(global);
            },
            [&](const ast::Struct* str) {
                auto* ty = builder_.Sem().Get(str);
                auto address_space_uses = ty->AddressSpaceUsage();
                if (address_space_uses.size() !=
                    (address_space_uses.count(builtin::AddressSpace::kStorage) +
                     address_space_uses.count(builtin::AddressSpace::kUniform))) {
                    // The structure is used as something other than a storage buffer or
                    // uniform buffer, so it needs to be emitted.
                    // Storage buffer are read and written to via a ByteAddressBuffer
                    // instead of true structure.
                    // Structures used as uniform buffer are read from an array of
                    // vectors instead of true structure.
                    return EmitStructType(current_buffer_, ty);
                }
                return true;
            },
            [&](const ast::Function* func) {
                if (func->IsEntryPoint()) {
                    return EmitEntryPointFunction(func);
                }
                return EmitFunction(func);
            },
            [&](Default) {
                TINT_ICE(Writer, diagnostics_)
                    << "unhandled module-scope declaration: " << decl->TypeInfo().name;
                return false;
            });

        if (!ok) {
            return false;
        }
    }

    if (!helpers_.lines.empty()) {
        current_buffer_->Insert(helpers_, 0, 0);
    }

    return true;
}

bool GeneratorImpl::EmitDynamicVectorAssignment(const ast::AssignmentStatement* stmt,
                                                const type::Vector* vec) {
    auto name = utils::GetOrCreate(dynamic_vector_write_, vec, [&]() -> std::string {
        std::string fn;
        {
            utils::StringStream ss;
            if (!EmitType(ss, vec, tint::builtin::AddressSpace::kUndefined,
                          builtin::Access::kUndefined, "")) {
                return "";
            }
            fn = UniqueIdentifier("set_" + ss.str());
        }
        {
            auto out = line(&helpers_);
            out << "void " << fn << "(inout ";
            if (!EmitTypeAndName(out, vec, builtin::AddressSpace::kUndefined,
                                 builtin::Access::kUndefined, "vec")) {
                return "";
            }
            out << ", int idx, ";
            if (!EmitTypeAndName(out, vec->type(), builtin::AddressSpace::kUndefined,
                                 builtin::Access::kUndefined, "val")) {
                return "";
            }
            out << ") {";
        }
        {
            ScopedIndent si(&helpers_);
            auto out = line(&helpers_);
            switch (vec->Width()) {
                case 2:
                    out << "vec = (idx.xx == int2(0, 1)) ? val.xx : vec;";
                    break;
                case 3:
                    out << "vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;";
                    break;
                case 4:
                    out << "vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;";
                    break;
                default:
                    TINT_UNREACHABLE(Writer, diagnostics_)
                        << "invalid vector size " << vec->Width();
                    break;
            }
        }
        line(&helpers_) << "}";
        line(&helpers_);
        return fn;
    });

    if (name.empty()) {
        return false;
    }

    auto* ast_access_expr = stmt->lhs->As<ast::IndexAccessorExpression>();

    auto out = line();
    out << name << "(";
    if (!EmitExpression(out, ast_access_expr->object)) {
        return false;
    }
    out << ", ";
    if (!EmitExpression(out, ast_access_expr->index)) {
        return false;
    }
    out << ", ";
    if (!EmitExpression(out, stmt->rhs)) {
        return false;
    }
    out << ");";

    return true;
}

bool GeneratorImpl::EmitDynamicMatrixVectorAssignment(const ast::AssignmentStatement* stmt,
                                                      const type::Matrix* mat) {
    auto name = utils::GetOrCreate(dynamic_matrix_vector_write_, mat, [&]() -> std::string {
        std::string fn;
        {
            utils::StringStream ss;
            if (!EmitType(ss, mat, tint::builtin::AddressSpace::kUndefined,
                          builtin::Access::kUndefined, "")) {
                return "";
            }
            fn = UniqueIdentifier("set_vector_" + ss.str());
        }
        {
            auto out = line(&helpers_);
            out << "void " << fn << "(inout ";
            if (!EmitTypeAndName(out, mat, builtin::AddressSpace::kUndefined,
                                 builtin::Access::kUndefined, "mat")) {
                return "";
            }
            out << ", int col, ";
            if (!EmitTypeAndName(out, mat->ColumnType(), builtin::AddressSpace::kUndefined,
                                 builtin::Access::kUndefined, "val")) {
                return "";
            }
            out << ") {";
        }
        {
            ScopedIndent si(&helpers_);
            line(&helpers_) << "switch (col) {";
            {
                ScopedIndent si2(&helpers_);
                for (uint32_t i = 0; i < mat->columns(); ++i) {
                    line(&helpers_) << "case " << i << ": mat[" << i << "] = val; break;";
                }
            }
            line(&helpers_) << "}";
        }
        line(&helpers_) << "}";
        line(&helpers_);
        return fn;
    });

    if (name.empty()) {
        return false;
    }

    auto* ast_access_expr = stmt->lhs->As<ast::IndexAccessorExpression>();

    auto out = line();
    out << name << "(";
    if (!EmitExpression(out, ast_access_expr->object)) {
        return false;
    }
    out << ", ";
    if (!EmitExpression(out, ast_access_expr->index)) {
        return false;
    }
    out << ", ";
    if (!EmitExpression(out, stmt->rhs)) {
        return false;
    }
    out << ");";

    return true;
}

bool GeneratorImpl::EmitDynamicMatrixScalarAssignment(const ast::AssignmentStatement* stmt,
                                                      const type::Matrix* mat) {
    auto* lhs_row_access = stmt->lhs->As<ast::IndexAccessorExpression>();
    auto* lhs_col_access = lhs_row_access->object->As<ast::IndexAccessorExpression>();

    auto name = utils::GetOrCreate(dynamic_matrix_scalar_write_, mat, [&]() -> std::string {
        std::string fn;
        {
            utils::StringStream ss;
            if (!EmitType(ss, mat, tint::builtin::AddressSpace::kUndefined,
                          builtin::Access::kUndefined, "")) {
                return "";
            }
            fn = UniqueIdentifier("set_scalar_" + ss.str());
        }
        {
            auto out = line(&helpers_);
            out << "void " << fn << "(inout ";
            if (!EmitTypeAndName(out, mat, builtin::AddressSpace::kUndefined,
                                 builtin::Access::kUndefined, "mat")) {
                return "";
            }
            out << ", int col, int row, ";
            if (!EmitTypeAndName(out, mat->type(), builtin::AddressSpace::kUndefined,
                                 builtin::Access::kUndefined, "val")) {
                return "";
            }
            out << ") {";
        }
        {
            ScopedIndent si(&helpers_);
            line(&helpers_) << "switch (col) {";
            {
                ScopedIndent si2(&helpers_);
                for (uint32_t i = 0; i < mat->columns(); ++i) {
                    line(&helpers_) << "case " << i << ":";
                    {
                        auto vec_name = "mat[" + std::to_string(i) + "]";
                        ScopedIndent si3(&helpers_);
                        {
                            auto out = line(&helpers_);
                            switch (mat->rows()) {
                                case 2:
                                    out << vec_name
                                        << " = (row.xx == int2(0, 1)) ? val.xx : " << vec_name
                                        << ";";
                                    break;
                                case 3:
                                    out << vec_name
                                        << " = (row.xxx == int3(0, 1, 2)) ? val.xxx : " << vec_name
                                        << ";";
                                    break;
                                case 4:
                                    out << vec_name
                                        << " = (row.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : "
                                        << vec_name << ";";
                                    break;
                                default: {
                                    auto* vec = TypeOf(lhs_row_access->object)
                                                    ->UnwrapRef()
                                                    ->As<type::Vector>();
                                    TINT_UNREACHABLE(Writer, diagnostics_)
                                        << "invalid vector size " << vec->Width();
                                    break;
                                }
                            }
                        }
                        line(&helpers_) << "break;";
                    }
                }
            }
            line(&helpers_) << "}";
        }
        line(&helpers_) << "}";
        line(&helpers_);
        return fn;
    });

    if (name.empty()) {
        return false;
    }

    auto out = line();
    out << name << "(";
    if (!EmitExpression(out, lhs_col_access->object)) {
        return false;
    }
    out << ", ";
    if (!EmitExpression(out, lhs_col_access->index)) {
        return false;
    }
    out << ", ";
    if (!EmitExpression(out, lhs_row_access->index)) {
        return false;
    }
    out << ", ";
    if (!EmitExpression(out, stmt->rhs)) {
        return false;
    }
    out << ");";

    return true;
}

bool GeneratorImpl::EmitIndexAccessor(utils::StringStream& out,
                                      const ast::IndexAccessorExpression* expr) {
    if (!EmitExpression(out, expr->object)) {
        return false;
    }
    out << "[";

    if (!EmitExpression(out, expr->index)) {
        return false;
    }
    out << "]";

    return true;
}

bool GeneratorImpl::EmitBitcast(utils::StringStream& out, const ast::BitcastExpression* expr) {
    auto* type = TypeOf(expr);
    if (auto* vec = type->UnwrapRef()->As<type::Vector>()) {
        type = vec->type();
    }

    if (!type->is_integer_scalar() && !type->is_float_scalar()) {
        diagnostics_.add_error(diag::System::Writer,
                               "Unable to do bitcast to type " + type->FriendlyName());
        return false;
    }

    out << "as";
    if (!EmitType(out, type, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "")) {
        return false;
    }
    out << "(";
    if (!EmitExpression(out, expr->expr)) {
        return false;
    }
    out << ")";
    return true;
}

bool GeneratorImpl::EmitAssign(const ast::AssignmentStatement* stmt) {
    if (auto* lhs_access = stmt->lhs->As<ast::IndexAccessorExpression>()) {
        // BUG(crbug.com/tint/1333): work around assignment of scalar to matrices
        // with at least one dynamic index
        if (auto* lhs_sub_access = lhs_access->object->As<ast::IndexAccessorExpression>()) {
            if (auto* mat = TypeOf(lhs_sub_access->object)->UnwrapRef()->As<type::Matrix>()) {
                auto* rhs_row_idx_sem = builder_.Sem().GetVal(lhs_access->index);
                auto* rhs_col_idx_sem = builder_.Sem().GetVal(lhs_sub_access->index);
                if (!rhs_row_idx_sem->ConstantValue() || !rhs_col_idx_sem->ConstantValue()) {
                    return EmitDynamicMatrixScalarAssignment(stmt, mat);
                }
            }
        }
        // BUG(crbug.com/tint/1333): work around assignment of vector to matrices
        // with dynamic indices
        const auto* lhs_access_type = TypeOf(lhs_access->object)->UnwrapRef();
        if (auto* mat = lhs_access_type->As<type::Matrix>()) {
            auto* lhs_index_sem = builder_.Sem().GetVal(lhs_access->index);
            if (!lhs_index_sem->ConstantValue()) {
                return EmitDynamicMatrixVectorAssignment(stmt, mat);
            }
        }
        // BUG(crbug.com/tint/534): work around assignment to vectors with dynamic
        // indices
        if (auto* vec = lhs_access_type->As<type::Vector>()) {
            auto* rhs_sem = builder_.Sem().GetVal(lhs_access->index);
            if (!rhs_sem->ConstantValue()) {
                return EmitDynamicVectorAssignment(stmt, vec);
            }
        }
    }

    auto out = line();
    if (!EmitExpression(out, stmt->lhs)) {
        return false;
    }
    out << " = ";
    if (!EmitExpression(out, stmt->rhs)) {
        return false;
    }
    out << ";";
    return true;
}

bool GeneratorImpl::EmitBinary(utils::StringStream& out, const ast::BinaryExpression* expr) {
    if (expr->op == ast::BinaryOp::kLogicalAnd || expr->op == ast::BinaryOp::kLogicalOr) {
        auto name = UniqueIdentifier(kTempNamePrefix);

        {
            auto pre = line();
            pre << "bool " << name << " = ";
            if (!EmitExpression(pre, expr->lhs)) {
                return false;
            }
            pre << ";";
        }

        if (expr->op == ast::BinaryOp::kLogicalOr) {
            line() << "if (!" << name << ") {";
        } else {
            line() << "if (" << name << ") {";
        }

        {
            ScopedIndent si(this);
            auto pre = line();
            pre << name << " = ";
            if (!EmitExpression(pre, expr->rhs)) {
                return false;
            }
            pre << ";";
        }

        line() << "}";

        out << "(" << name << ")";
        return true;
    }

    auto* lhs_type = TypeOf(expr->lhs)->UnwrapRef();
    auto* rhs_type = TypeOf(expr->rhs)->UnwrapRef();
    // Multiplying by a matrix requires the use of `mul` in order to get the
    // type of multiply we desire.
    if (expr->op == ast::BinaryOp::kMultiply &&
        ((lhs_type->Is<type::Vector>() && rhs_type->Is<type::Matrix>()) ||
         (lhs_type->Is<type::Matrix>() && rhs_type->Is<type::Vector>()) ||
         (lhs_type->Is<type::Matrix>() && rhs_type->Is<type::Matrix>()))) {
        // Matrices are transposed, so swap LHS and RHS.
        out << "mul(";
        if (!EmitExpression(out, expr->rhs)) {
            return false;
        }
        out << ", ";
        if (!EmitExpression(out, expr->lhs)) {
            return false;
        }
        out << ")";

        return true;
    }

    ScopedParen sp(out);

    if (!EmitExpression(out, expr->lhs)) {
        return false;
    }
    out << " ";

    switch (expr->op) {
        case ast::BinaryOp::kAnd:
            out << "&";
            break;
        case ast::BinaryOp::kOr:
            out << "|";
            break;
        case ast::BinaryOp::kXor:
            out << "^";
            break;
        case ast::BinaryOp::kLogicalAnd:
        case ast::BinaryOp::kLogicalOr: {
            // These are both handled above.
            TINT_UNREACHABLE(Writer, diagnostics_);
            return false;
        }
        case ast::BinaryOp::kEqual:
            out << "==";
            break;
        case ast::BinaryOp::kNotEqual:
            out << "!=";
            break;
        case ast::BinaryOp::kLessThan:
            out << "<";
            break;
        case ast::BinaryOp::kGreaterThan:
            out << ">";
            break;
        case ast::BinaryOp::kLessThanEqual:
            out << "<=";
            break;
        case ast::BinaryOp::kGreaterThanEqual:
            out << ">=";
            break;
        case ast::BinaryOp::kShiftLeft:
            out << "<<";
            break;
        case ast::BinaryOp::kShiftRight:
            // TODO(dsinclair): MSL is based on C++14, and >> in C++14 has
            // implementation-defined behaviour for negative LHS.  We may have to
            // generate extra code to implement WGSL-specified behaviour for negative
            // LHS.
            out << R"(>>)";
            break;

        case ast::BinaryOp::kAdd:
            out << "+";
            break;
        case ast::BinaryOp::kSubtract:
            out << "-";
            break;
        case ast::BinaryOp::kMultiply:
            out << "*";
            break;
        case ast::BinaryOp::kDivide:
            out << "/";
            break;
        case ast::BinaryOp::kModulo:
            out << "%";
            break;
        case ast::BinaryOp::kNone:
            diagnostics_.add_error(diag::System::Writer, "missing binary operation type");
            return false;
    }
    out << " ";

    if (!EmitExpression(out, expr->rhs)) {
        return false;
    }

    return true;
}

bool GeneratorImpl::EmitStatements(utils::VectorRef<const ast::Statement*> stmts) {
    for (auto* s : stmts) {
        if (!EmitStatement(s)) {
            return false;
        }
    }
    return true;
}

bool GeneratorImpl::EmitStatementsWithIndent(utils::VectorRef<const ast::Statement*> stmts) {
    ScopedIndent si(this);
    return EmitStatements(stmts);
}

bool GeneratorImpl::EmitBlock(const ast::BlockStatement* stmt) {
    line() << "{";
    if (!EmitStatementsWithIndent(stmt->statements)) {
        return false;
    }
    line() << "}";
    return true;
}

bool GeneratorImpl::EmitBreak(const ast::BreakStatement*) {
    line() << "break;";
    return true;
}

bool GeneratorImpl::EmitBreakIf(const ast::BreakIfStatement* b) {
    auto out = line();
    out << "if (";
    if (!EmitExpression(out, b->condition)) {
        return false;
    }
    out << ") { break; }";
    return true;
}

bool GeneratorImpl::EmitCall(utils::StringStream& out, const ast::CallExpression* expr) {
    auto* call = builder_.Sem().Get<sem::Call>(expr);
    auto* target = call->Target();
    return Switch(
        target,  //
        [&](const sem::Function* func) { return EmitFunctionCall(out, call, func); },
        [&](const sem::Builtin* builtin) { return EmitBuiltinCall(out, call, builtin); },
        [&](const sem::ValueConversion* conv) { return EmitValueConversion(out, call, conv); },
        [&](const sem::ValueConstructor* ctor) { return EmitValueConstructor(out, call, ctor); },
        [&](Default) {
            TINT_ICE(Writer, diagnostics_) << "unhandled call target: " << target->TypeInfo().name;
            return false;
        });
}

bool GeneratorImpl::EmitFunctionCall(utils::StringStream& out,
                                     const sem::Call* call,
                                     const sem::Function* func) {
    auto* expr = call->Declaration();

    if (ast::HasAttribute<transform::CalculateArrayLength::BufferSizeIntrinsic>(
            func->Declaration()->attributes)) {
        // Special function generated by the CalculateArrayLength transform for
        // calling X.GetDimensions(Y)
        if (!EmitExpression(out, call->Arguments()[0]->Declaration())) {
            return false;
        }
        out << ".GetDimensions(";
        if (!EmitExpression(out, call->Arguments()[1]->Declaration())) {
            return false;
        }
        out << ")";
        return true;
    }

    if (auto* intrinsic = ast::GetAttribute<transform::DecomposeMemoryAccess::Intrinsic>(
            func->Declaration()->attributes)) {
        switch (intrinsic->address_space) {
            case builtin::AddressSpace::kUniform:
                return EmitUniformBufferAccess(out, expr, intrinsic);
            case builtin::AddressSpace::kStorage:
                if (!intrinsic->IsAtomic()) {
                    return EmitStorageBufferAccess(out, expr, intrinsic);
                }
                break;
            default:
                TINT_UNREACHABLE(Writer, diagnostics_)
                    << "unsupported DecomposeMemoryAccess::Intrinsic address space:"
                    << intrinsic->address_space;
                return false;
        }
    }

    out << func->Declaration()->name->symbol.Name() << "(";

    bool first = true;
    for (auto* arg : call->Arguments()) {
        if (!first) {
            out << ", ";
        }
        first = false;

        if (!EmitExpression(out, arg->Declaration())) {
            return false;
        }
    }

    out << ")";
    return true;
}

bool GeneratorImpl::EmitBuiltinCall(utils::StringStream& out,
                                    const sem::Call* call,
                                    const sem::Builtin* builtin) {
    const auto type = builtin->Type();

    auto* expr = call->Declaration();
    if (builtin->IsTexture()) {
        return EmitTextureCall(out, call, builtin);
    }
    if (type == builtin::Function::kSelect) {
        return EmitSelectCall(out, expr);
    }
    if (type == builtin::Function::kModf) {
        return EmitModfCall(out, expr, builtin);
    }
    if (type == builtin::Function::kFrexp) {
        return EmitFrexpCall(out, expr, builtin);
    }
    if (type == builtin::Function::kDegrees) {
        return EmitDegreesCall(out, expr, builtin);
    }
    if (type == builtin::Function::kRadians) {
        return EmitRadiansCall(out, expr, builtin);
    }
    if (type == builtin::Function::kSign) {
        return EmitSignCall(out, call, builtin);
    }
    if (type == builtin::Function::kQuantizeToF16) {
        return EmitQuantizeToF16Call(out, expr, builtin);
    }
    if (type == builtin::Function::kTrunc) {
        return EmitTruncCall(out, expr, builtin);
    }
    if (builtin->IsDataPacking()) {
        return EmitDataPackingCall(out, expr, builtin);
    }
    if (builtin->IsDataUnpacking()) {
        return EmitDataUnpackingCall(out, expr, builtin);
    }
    if (builtin->IsBarrier()) {
        return EmitBarrierCall(out, builtin);
    }
    if (builtin->IsAtomic()) {
        return EmitWorkgroupAtomicCall(out, expr, builtin);
    }
    if (builtin->IsDP4a()) {
        return EmitDP4aCall(out, expr, builtin);
    }

    auto name = generate_builtin_name(builtin);
    if (name.empty()) {
        return false;
    }

    // Handle single argument builtins that only accept and return uint (not int overload). We need
    // to explicitly cast the return value (we also cast the arg for good measure). See
    // crbug.com/tint/1550
    if (type == builtin::Function::kCountOneBits || type == builtin::Function::kReverseBits) {
        auto* arg = call->Arguments()[0];
        if (arg->Type()->UnwrapRef()->is_signed_integer_scalar_or_vector()) {
            out << "asint(" << name << "(asuint(";
            if (!EmitExpression(out, arg->Declaration())) {
                return false;
            }
            out << ")))";
            return true;
        }
    }

    out << name << "(";

    bool first = true;
    for (auto* arg : call->Arguments()) {
        if (!first) {
            out << ", ";
        }
        first = false;

        if (!EmitExpression(out, arg->Declaration())) {
            return false;
        }
    }

    out << ")";

    return true;
}

bool GeneratorImpl::EmitValueConversion(utils::StringStream& out,
                                        const sem::Call* call,
                                        const sem::ValueConversion* conv) {
    if (!EmitType(out, conv->Target(), builtin::AddressSpace::kUndefined,
                  builtin::Access::kReadWrite, "")) {
        return false;
    }
    out << "(";

    if (!EmitExpression(out, call->Arguments()[0]->Declaration())) {
        return false;
    }

    out << ")";
    return true;
}

bool GeneratorImpl::EmitValueConstructor(utils::StringStream& out,
                                         const sem::Call* call,
                                         const sem::ValueConstructor* ctor) {
    auto* type = call->Type();

    // If the value constructor arguments are empty then we need to construct with the zero value
    // for all components.
    if (call->Arguments().IsEmpty()) {
        return EmitZeroValue(out, type);
    }

    // Single parameter matrix initializers must be identity initializer.
    // It could also be conversions between f16 and f32 matrix when f16 is properly supported.
    if (type->Is<type::Matrix>() && call->Arguments().Length() == 1) {
        if (!ctor->Parameters()[0]->Type()->UnwrapRef()->is_float_matrix()) {
            TINT_UNREACHABLE(Writer, diagnostics_)
                << "found a single-parameter matrix initializer that is not identity initializer";
            return false;
        }
    }

    bool brackets = type->IsAnyOf<type::Array, type::Struct>();

    // For single-value vector initializers, swizzle the scalar to the right
    // vector dimension using .x
    const bool is_single_value_vector_init = type->is_scalar_vector() &&
                                             call->Arguments().Length() == 1 &&
                                             ctor->Parameters()[0]->Type()->is_scalar();

    if (brackets) {
        out << "{";
    } else {
        if (!EmitType(out, type, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite,
                      "")) {
            return false;
        }
        out << "(";
    }

    if (is_single_value_vector_init) {
        out << "(";
    }

    bool first = true;
    for (auto* e : call->Arguments()) {
        if (!first) {
            out << ", ";
        }
        first = false;

        if (!EmitExpression(out, e->Declaration())) {
            return false;
        }
    }

    if (is_single_value_vector_init) {
        out << ")." << std::string(type->As<type::Vector>()->Width(), 'x');
    }

    out << (brackets ? "}" : ")");
    return true;
}

bool GeneratorImpl::EmitUniformBufferAccess(
    utils::StringStream& out,
    const ast::CallExpression* expr,
    const transform::DecomposeMemoryAccess::Intrinsic* intrinsic) {
    auto const buffer = intrinsic->Buffer()->identifier->symbol.Name();
    auto* const offset = expr->args[0];

    // offset in bytes
    uint32_t scalar_offset_bytes = 0;
    // offset in uint (4 bytes)
    uint32_t scalar_offset_index = 0;
    // expression to calculate offset in bytes
    std::string scalar_offset_bytes_expr;
    // expression to calculate offset in uint, by dividing scalar_offset_bytes_expr by 4
    std::string scalar_offset_index_expr;
    // expression to calculate offset in uint, independently
    std::string scalar_offset_index_unified_expr;

    // If true, use scalar_offset_index, otherwise use scalar_offset_index_expr
    bool scalar_offset_constant = false;

    if (auto* val = builder_.Sem().GetVal(offset)->ConstantValue()) {
        TINT_ASSERT(Writer, val->Type()->Is<type::U32>());
        scalar_offset_bytes = static_cast<uint32_t>(val->ValueAs<AInt>());
        scalar_offset_index = scalar_offset_bytes / 4;  // bytes -> scalar index
        scalar_offset_constant = true;
    }

    // If true, scalar_offset_bytes or scalar_offset_bytes_expr should be used, otherwise only use
    // scalar_offset_index or scalar_offset_index_unified_expr. Currently only loading f16 scalar
    // require using offset in bytes.
    const bool need_offset_in_bytes =
        intrinsic->type == transform::DecomposeMemoryAccess::Intrinsic::DataType::kF16;

    if (!scalar_offset_constant) {
        // UBO offset not compile-time known.
        // Calculate the scalar offset into a temporary.
        if (need_offset_in_bytes) {
            scalar_offset_bytes_expr = UniqueIdentifier("scalar_offset_bytes");
            scalar_offset_index_expr = UniqueIdentifier("scalar_offset_index");
            {
                auto pre = line();
                pre << "const uint " << scalar_offset_bytes_expr << " = (";
                if (!EmitExpression(pre, offset)) {
                    return false;
                }
                pre << ");";
            }
            line() << "const uint " << scalar_offset_index_expr << " = " << scalar_offset_bytes_expr
                   << " / 4;";
        } else {
            scalar_offset_index_unified_expr = UniqueIdentifier("scalar_offset");
            auto pre = line();
            pre << "const uint " << scalar_offset_index_unified_expr << " = (";
            if (!EmitExpression(pre, offset)) {
                return false;
            }
            pre << ") / 4;";
        }
    }

    const char swizzle[] = {'x', 'y', 'z', 'w'};

    using Op = transform::DecomposeMemoryAccess::Intrinsic::Op;
    using DataType = transform::DecomposeMemoryAccess::Intrinsic::DataType;
    switch (intrinsic->op) {
        case Op::kLoad: {
            auto cast = [&](const char* to, auto&& load) {
                out << to << "(";
                auto result = load();
                out << ")";
                return result;
            };
            auto load_u32_to = [&](utils::StringStream& target) {
                target << buffer;
                if (scalar_offset_constant) {
                    target << "[" << (scalar_offset_index / 4) << "]."
                           << swizzle[scalar_offset_index & 3];
                } else {
                    target << "[" << scalar_offset_index_unified_expr << " / 4]["
                           << scalar_offset_index_unified_expr << " % 4]";
                }
                return true;
            };
            auto load_u32 = [&] { return load_u32_to(out); };
            // Has a minimum alignment of 8 bytes, so is either .xy or .zw
            auto load_vec2_u32_to = [&](utils::StringStream& target) {
                if (scalar_offset_constant) {
                    target << buffer << "[" << (scalar_offset_index / 4) << "]"
                           << ((scalar_offset_index & 2) == 0 ? ".xy" : ".zw");
                } else {
                    std::string ubo_load = UniqueIdentifier("ubo_load");
                    {
                        auto pre = line();
                        pre << "uint4 " << ubo_load << " = " << buffer << "["
                            << scalar_offset_index_unified_expr << " / 4];";
                    }
                    target << "((" << scalar_offset_index_unified_expr << " & 2) ? " << ubo_load
                           << ".zw : " << ubo_load << ".xy)";
                }
                return true;
            };
            auto load_vec2_u32 = [&] { return load_vec2_u32_to(out); };
            // vec4 has a minimum alignment of 16 bytes, easiest case
            auto load_vec4_u32 = [&] {
                out << buffer;
                if (scalar_offset_constant) {
                    out << "[" << (scalar_offset_index / 4) << "]";
                } else {
                    out << "[" << scalar_offset_index_unified_expr << " / 4]";
                }
                return true;
            };
            // vec3 has a minimum alignment of 16 bytes, so is just a .xyz swizzle
            auto load_vec3_u32 = [&] {
                if (!load_vec4_u32()) {
                    return false;
                }
                out << ".xyz";
                return true;
            };
            auto load_scalar_f16 = [&] {
                // offset bytes = 4k,   ((buffer[index].x) & 0xFFFF)
                // offset bytes = 4k+2, ((buffer[index].x >> 16) & 0xFFFF)
                out << "float16_t(f16tof32(((" << buffer;
                if (scalar_offset_constant) {
                    out << "[" << (scalar_offset_index / 4) << "]."
                        << swizzle[scalar_offset_index & 3];
                    // WGSL spec ensure little endian memory layout.
                    if (scalar_offset_bytes % 4 == 0) {
                        out << ") & 0xFFFF)";
                    } else {
                        out << " >> 16) & 0xFFFF)";
                    }
                } else {
                    out << "[" << scalar_offset_index_expr << " / 4][" << scalar_offset_index_expr
                        << " % 4] >> (" << scalar_offset_bytes_expr
                        << " % 4 == 0 ? 0 : 16)) & 0xFFFF)";
                }
                out << "))";
                return true;
            };
            auto load_vec2_f16 = [&] {
                // vec2<f16> is aligned to 4 bytes
                // Preclude code load the vec2<f16> data as a uint:
                //     uint ubo_load = buffer[id0][id1];
                // Loading code convert it to vec2<f16>:
                //     vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)),
                //     float16_t(f16tof32(ubo_load >> 16)))
                std::string ubo_load = UniqueIdentifier("ubo_load");
                {
                    auto pre = line();
                    // Load the 4 bytes f16 vector as an uint
                    pre << "uint " << ubo_load << " = ";
                    if (!load_u32_to(pre)) {
                        return false;
                    }
                    pre << ";";
                }
                out << "vector<float16_t, 2>(float16_t(f16tof32(" << ubo_load
                    << " & 0xFFFF)), float16_t(f16tof32(" << ubo_load << " >> 16)))";
                return true;
            };
            auto load_vec3_f16 = [&] {
                // vec3<f16> is aligned to 8 bytes
                // Preclude code load the vec3<f16> data as uint2 and convert its elements to
                // float16_t:
                //     uint2 ubo_load = buffer[id0].xy;
                //     /* The low 8 bits of two uint are the x and z elements of vec3<f16> */
                //     vector<float16_t> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load &
                //     0xFFFF));
                //     /* The high 8 bits of first uint is the y element of vec3<f16> */
                //     float16_t ubo_load_y = f16tof32(ubo_load[0] >> 16);
                // Loading code convert it to vec3<f16>:
                //     vector<float16_t, 3>(ubo_load_xz[0], ubo_load_y, ubo_load_xz[1])
                std::string ubo_load = UniqueIdentifier("ubo_load");
                std::string ubo_load_xz = UniqueIdentifier(ubo_load + "_xz");
                std::string ubo_load_y = UniqueIdentifier(ubo_load + "_y");
                {
                    auto pre = line();
                    // Load the 8 bytes uint2 with the f16 vector at lower 6 bytes
                    pre << "uint2 " << ubo_load << " = ";
                    if (!load_vec2_u32_to(pre)) {
                        return false;
                    }
                    pre << ";";
                }
                {
                    auto pre = line();
                    pre << "vector<float16_t, 2> " << ubo_load_xz
                        << " = vector<float16_t, 2>(f16tof32(" << ubo_load << " & 0xFFFF));";
                }
                {
                    auto pre = line();
                    pre << "float16_t " << ubo_load_y << " = f16tof32(" << ubo_load
                        << "[0] >> 16);";
                }
                out << "vector<float16_t, 3>(" << ubo_load_xz << "[0], " << ubo_load_y << ", "
                    << ubo_load_xz << "[1])";
                return true;
            };
            auto load_vec4_f16 = [&] {
                // vec4<f16> is aligned to 8 bytes
                // Preclude code load the vec4<f16> data as uint2 and convert its elements to
                // float16_t:
                //     uint2 ubo_load = buffer[id0].xy;
                //     /* The low 8 bits of two uint are the x and z elements of vec4<f16> */
                //     vector<float16_t> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load &
                //     0xFFFF));
                //     /* The high 8 bits of two uint are the y and w elements of vec4<f16> */
                //     vector<float16_t, 2> ubo_load_yw = vector<float16_t, 2>(f16tof32(ubo_load >>
                //     16));
                // Loading code convert it to vec4<f16>:
                //     vector<float16_t, 4>(ubo_load_xz[0], ubo_load_yw[0], ubo_load_xz[1],
                //     ubo_load_yw[1])
                std::string ubo_load = UniqueIdentifier("ubo_load");
                std::string ubo_load_xz = UniqueIdentifier(ubo_load + "_xz");
                std::string ubo_load_yw = UniqueIdentifier(ubo_load + "_yw");
                {
                    auto pre = line();
                    // Load the 8 bytes f16 vector as an uint2
                    pre << "uint2 " << ubo_load << " = ";
                    if (!load_vec2_u32_to(pre)) {
                        return false;
                    }
                    pre << ";";
                }
                {
                    auto pre = line();
                    pre << "vector<float16_t, 2> " << ubo_load_xz
                        << " = vector<float16_t, 2>(f16tof32(" << ubo_load << " & 0xFFFF));";
                }
                {
                    auto pre = line();
                    pre << "vector<float16_t, 2> " << ubo_load_yw
                        << " = vector<float16_t, 2>(f16tof32(" << ubo_load << " >> 16));";
                }
                out << "vector<float16_t, 4>(" << ubo_load_xz << "[0], " << ubo_load_yw << "[0], "
                    << ubo_load_xz << "[1], " << ubo_load_yw << "[1])";
                return true;
            };
            switch (intrinsic->type) {
                case DataType::kU32:
                    return load_u32();
                case DataType::kF32:
                    return cast("asfloat", load_u32);
                case DataType::kI32:
                    return cast("asint", load_u32);
                case DataType::kF16:
                    return load_scalar_f16();
                case DataType::kVec2U32:
                    return load_vec2_u32();
                case DataType::kVec2F32:
                    return cast("asfloat", load_vec2_u32);
                case DataType::kVec2I32:
                    return cast("asint", load_vec2_u32);
                case DataType::kVec2F16:
                    return load_vec2_f16();
                case DataType::kVec3U32:
                    return load_vec3_u32();
                case DataType::kVec3F32:
                    return cast("asfloat", load_vec3_u32);
                case DataType::kVec3I32:
                    return cast("asint", load_vec3_u32);
                case DataType::kVec3F16:
                    return load_vec3_f16();
                case DataType::kVec4U32:
                    return load_vec4_u32();
                case DataType::kVec4F32:
                    return cast("asfloat", load_vec4_u32);
                case DataType::kVec4I32:
                    return cast("asint", load_vec4_u32);
                case DataType::kVec4F16:
                    return load_vec4_f16();
            }
            TINT_UNREACHABLE(Writer, diagnostics_)
                << "unsupported DecomposeMemoryAccess::Intrinsic::DataType: "
                << static_cast<int>(intrinsic->type);
            return false;
        }
        default:
            break;
    }
    TINT_UNREACHABLE(Writer, diagnostics_)
        << "unsupported DecomposeMemoryAccess::Intrinsic::Op: " << static_cast<int>(intrinsic->op);
    return false;
}

bool GeneratorImpl::EmitStorageBufferAccess(
    utils::StringStream& out,
    const ast::CallExpression* expr,
    const transform::DecomposeMemoryAccess::Intrinsic* intrinsic) {
    auto const buffer = intrinsic->Buffer()->identifier->symbol.Name();
    auto* const offset = expr->args[0];
    auto* const value = expr->args[1];

    using Op = transform::DecomposeMemoryAccess::Intrinsic::Op;
    using DataType = transform::DecomposeMemoryAccess::Intrinsic::DataType;
    switch (intrinsic->op) {
        case Op::kLoad: {
            auto load = [&](const char* cast, int n) {
                if (cast) {
                    out << cast << "(";
                }
                out << buffer << ".Load";
                if (n > 1) {
                    out << n;
                }
                ScopedParen sp(out);
                if (!EmitExpression(out, offset)) {
                    return false;
                }
                if (cast) {
                    out << ")";
                }
                return true;
            };
            // Templated load used for f16 types, requires SM6.2 or higher and DXC
            // Used by loading f16 types, e.g. for f16 type, set type parameter to "float16_t"
            // to emit `buffer.Load<float16_t>(offset)`.
            auto templated_load = [&](const char* type) {
                out << buffer << ".Load<" << type << ">";  // templated load
                ScopedParen sp(out);
                if (!EmitExpression(out, offset)) {
                    return false;
                }
                return true;
            };
            switch (intrinsic->type) {
                case DataType::kU32:
                    return load(nullptr, 1);
                case DataType::kF32:
                    return load("asfloat", 1);
                case DataType::kI32:
                    return load("asint", 1);
                case DataType::kF16:
                    return templated_load("float16_t");
                case DataType::kVec2U32:
                    return load(nullptr, 2);
                case DataType::kVec2F32:
                    return load("asfloat", 2);
                case DataType::kVec2I32:
                    return load("asint", 2);
                case DataType::kVec2F16:
                    return templated_load("vector<float16_t, 2> ");
                case DataType::kVec3U32:
                    return load(nullptr, 3);
                case DataType::kVec3F32:
                    return load("asfloat", 3);
                case DataType::kVec3I32:
                    return load("asint", 3);
                case DataType::kVec3F16:
                    return templated_load("vector<float16_t, 3> ");
                case DataType::kVec4U32:
                    return load(nullptr, 4);
                case DataType::kVec4F32:
                    return load("asfloat", 4);
                case DataType::kVec4I32:
                    return load("asint", 4);
                case DataType::kVec4F16:
                    return templated_load("vector<float16_t, 4> ");
            }
            TINT_UNREACHABLE(Writer, diagnostics_)
                << "unsupported DecomposeMemoryAccess::Intrinsic::DataType: "
                << static_cast<int>(intrinsic->type);
            return false;
        }

        case Op::kStore: {
            auto store = [&](int n) {
                out << buffer << ".Store";
                if (n > 1) {
                    out << n;
                }
                ScopedParen sp1(out);
                if (!EmitExpression(out, offset)) {
                    return false;
                }
                out << ", asuint";
                ScopedParen sp2(out);
                if (!EmitExpression(out, value)) {
                    return false;
                }
                return true;
            };
            // Templated stored used for f16 types, requires SM6.2 or higher and DXC
            // Used by storing f16 types, e.g. for f16 type, set type parameter to "float16_t"
            // to emit `buffer.Store<float16_t>(offset)`.
            auto templated_store = [&](const char* type) {
                out << buffer << ".Store<" << type << ">";  // templated store
                ScopedParen sp1(out);
                if (!EmitExpression(out, offset)) {
                    return false;
                }
                out << ", ";
                if (!EmitExpression(out, value)) {
                    return false;
                }
                return true;
            };
            switch (intrinsic->type) {
                case DataType::kU32:
                    return store(1);
                case DataType::kF32:
                    return store(1);
                case DataType::kI32:
                    return store(1);
                case DataType::kF16:
                    return templated_store("float16_t");
                case DataType::kVec2U32:
                    return store(2);
                case DataType::kVec2F32:
                    return store(2);
                case DataType::kVec2I32:
                    return store(2);
                case DataType::kVec2F16:
                    return templated_store("vector<float16_t, 2> ");
                case DataType::kVec3U32:
                    return store(3);
                case DataType::kVec3F32:
                    return store(3);
                case DataType::kVec3I32:
                    return store(3);
                case DataType::kVec3F16:
                    return templated_store("vector<float16_t, 3> ");
                case DataType::kVec4U32:
                    return store(4);
                case DataType::kVec4F32:
                    return store(4);
                case DataType::kVec4I32:
                    return store(4);
                case DataType::kVec4F16:
                    return templated_store("vector<float16_t, 4> ");
            }
            TINT_UNREACHABLE(Writer, diagnostics_)
                << "unsupported DecomposeMemoryAccess::Intrinsic::DataType: "
                << static_cast<int>(intrinsic->type);
            return false;
        }
        default:
            // Break out to error case below
            // Note that atomic intrinsics are generated as functions.
            break;
    }

    TINT_UNREACHABLE(Writer, diagnostics_)
        << "unsupported DecomposeMemoryAccess::Intrinsic::Op: " << static_cast<int>(intrinsic->op);
    return false;
}

bool GeneratorImpl::EmitStorageAtomicIntrinsic(
    const ast::Function* func,
    const transform::DecomposeMemoryAccess::Intrinsic* intrinsic) {
    using Op = transform::DecomposeMemoryAccess::Intrinsic::Op;

    const sem::Function* sem_func = builder_.Sem().Get(func);
    auto* result_ty = sem_func->ReturnType();
    const auto name = func->name->symbol.Name();
    auto& buf = *current_buffer_;

    auto const buffer = intrinsic->Buffer()->identifier->symbol.Name();

    auto rmw = [&](const char* hlsl) -> bool {
        {
            auto fn = line(&buf);
            if (!EmitTypeAndName(fn, result_ty, builtin::AddressSpace::kUndefined,
                                 builtin::Access::kUndefined, name)) {
                return false;
            }
            fn << "(uint offset, ";
            if (!EmitTypeAndName(fn, result_ty, builtin::AddressSpace::kUndefined,
                                 builtin::Access::kUndefined, "value")) {
                return false;
            }
            fn << ") {";
        }

        buf.IncrementIndent();
        TINT_DEFER({
            buf.DecrementIndent();
            line(&buf) << "}";
            line(&buf);
        });

        {
            auto l = line(&buf);
            if (!EmitTypeAndName(l, result_ty, builtin::AddressSpace::kUndefined,
                                 builtin::Access::kUndefined, "original_value")) {
                return false;
            }
            l << " = 0;";
        }
        {
            auto l = line(&buf);
            l << buffer << "." << hlsl << "(offset, ";
            if (intrinsic->op == Op::kAtomicSub) {
                l << "-";
            }
            l << "value, original_value);";
        }
        line(&buf) << "return original_value;";
        return true;
    };

    switch (intrinsic->op) {
        case Op::kAtomicAdd:
            return rmw("InterlockedAdd");

        case Op::kAtomicSub:
            // Use add with the operand negated.
            return rmw("InterlockedAdd");

        case Op::kAtomicMax:
            return rmw("InterlockedMax");

        case Op::kAtomicMin:
            return rmw("InterlockedMin");

        case Op::kAtomicAnd:
            return rmw("InterlockedAnd");

        case Op::kAtomicOr:
            return rmw("InterlockedOr");

        case Op::kAtomicXor:
            return rmw("InterlockedXor");

        case Op::kAtomicExchange:
            return rmw("InterlockedExchange");

        case Op::kAtomicLoad: {
            // HLSL does not have an InterlockedLoad, so we emulate it with
            // InterlockedOr using 0 as the OR value
            {
                auto fn = line(&buf);
                if (!EmitTypeAndName(fn, result_ty, builtin::AddressSpace::kUndefined,
                                     builtin::Access::kUndefined, name)) {
                    return false;
                }
                fn << "(uint offset) {";
            }

            buf.IncrementIndent();
            TINT_DEFER({
                buf.DecrementIndent();
                line(&buf) << "}";
                line(&buf);
            });

            {
                auto l = line(&buf);
                if (!EmitTypeAndName(l, result_ty, builtin::AddressSpace::kUndefined,
                                     builtin::Access::kUndefined, "value")) {
                    return false;
                }
                l << " = 0;";
            }

            line(&buf) << buffer << ".InterlockedOr(offset, 0, value);";
            line(&buf) << "return value;";
            return true;
        }
        case Op::kAtomicStore: {
            auto* const value_ty = sem_func->Parameters()[1]->Type()->UnwrapRef();
            // HLSL does not have an InterlockedStore, so we emulate it with
            // InterlockedExchange and discard the returned value
            {
                auto fn = line(&buf);
                fn << "void " << name << "(uint offset, ";
                if (!EmitTypeAndName(fn, value_ty, builtin::AddressSpace::kUndefined,
                                     builtin::Access::kUndefined, "value")) {
                    return false;
                }
                fn << ") {";
            }

            buf.IncrementIndent();
            TINT_DEFER({
                buf.DecrementIndent();
                line(&buf) << "}";
                line(&buf);
            });

            {
                auto l = line(&buf);
                if (!EmitTypeAndName(l, value_ty, builtin::AddressSpace::kUndefined,
                                     builtin::Access::kUndefined, "ignored")) {
                    return false;
                }
                l << ";";
            }
            line(&buf) << buffer << ".InterlockedExchange(offset, value, ignored);";
            return true;
        }
        case Op::kAtomicCompareExchangeWeak: {
            auto* const value_ty = sem_func->Parameters()[1]->Type()->UnwrapRef();
            // NOTE: We don't need to emit the return type struct here as DecomposeMemoryAccess
            // already added it to the AST, and it should have already been emitted by now.
            {
                auto fn = line(&buf);
                if (!EmitTypeAndName(fn, result_ty, builtin::AddressSpace::kUndefined,
                                     builtin::Access::kUndefined, name)) {
                    return false;
                }
                fn << "(uint offset, ";
                if (!EmitTypeAndName(fn, value_ty, builtin::AddressSpace::kUndefined,
                                     builtin::Access::kUndefined, "compare")) {
                    return false;
                }
                fn << ", ";
                if (!EmitTypeAndName(fn, value_ty, builtin::AddressSpace::kUndefined,
                                     builtin::Access::kUndefined, "value")) {
                    return false;
                }
                fn << ") {";
            }

            buf.IncrementIndent();
            TINT_DEFER({
                buf.DecrementIndent();
                line(&buf) << "}";
                line(&buf);
            });

            {  // T result = {0};
                auto l = line(&buf);
                if (!EmitTypeAndName(l, result_ty, builtin::AddressSpace::kUndefined,
                                     builtin::Access::kUndefined, "result")) {
                    return false;
                }
                l << "=";
                if (!EmitZeroValue(l, result_ty)) {
                    return false;
                }
                l << ";";
            }

            line(&buf) << buffer
                       << ".InterlockedCompareExchange(offset, compare, value, result.old_value);";
            line(&buf) << "result.exchanged = result.old_value == compare;";
            line(&buf) << "return result;";

            return true;
        }
        default:
            break;
    }

    TINT_UNREACHABLE(Writer, diagnostics_)
        << "unsupported atomic DecomposeMemoryAccess::Intrinsic::Op: "
        << static_cast<int>(intrinsic->op);
    return false;
}

bool GeneratorImpl::EmitWorkgroupAtomicCall(utils::StringStream& out,
                                            const ast::CallExpression* expr,
                                            const sem::Builtin* builtin) {
    std::string result = UniqueIdentifier("atomic_result");

    if (!builtin->ReturnType()->Is<type::Void>()) {
        auto pre = line();
        if (!EmitTypeAndName(pre, builtin->ReturnType(), builtin::AddressSpace::kUndefined,
                             builtin::Access::kUndefined, result)) {
            return false;
        }
        pre << " = ";
        if (!EmitZeroValue(pre, builtin->ReturnType())) {
            return false;
        }
        pre << ";";
    }

    auto call = [&](const char* name) {
        auto pre = line();
        pre << name;

        {
            ScopedParen sp(pre);
            for (size_t i = 0; i < expr->args.Length(); i++) {
                auto* arg = expr->args[i];
                if (i > 0) {
                    pre << ", ";
                }
                if (i == 1 && builtin->Type() == builtin::Function::kAtomicSub) {
                    // Sub uses InterlockedAdd with the operand negated.
                    pre << "-";
                }
                if (!EmitExpression(pre, arg)) {
                    return false;
                }
            }

            pre << ", " << result;
        }

        pre << ";";

        out << result;
        return true;
    };

    switch (builtin->Type()) {
        case builtin::Function::kAtomicLoad: {
            // HLSL does not have an InterlockedLoad, so we emulate it with
            // InterlockedOr using 0 as the OR value
            auto pre = line();
            pre << "InterlockedOr";
            {
                ScopedParen sp(pre);
                if (!EmitExpression(pre, expr->args[0])) {
                    return false;
                }
                pre << ", 0, " << result;
            }
            pre << ";";

            out << result;
            return true;
        }
        case builtin::Function::kAtomicStore: {
            // HLSL does not have an InterlockedStore, so we emulate it with
            // InterlockedExchange and discard the returned value
            {  // T result = 0;
                auto pre = line();
                auto* value_ty = builtin->Parameters()[1]->Type()->UnwrapRef();
                if (!EmitTypeAndName(pre, value_ty, builtin::AddressSpace::kUndefined,
                                     builtin::Access::kUndefined, result)) {
                    return false;
                }
                pre << " = ";
                if (!EmitZeroValue(pre, value_ty)) {
                    return false;
                }
                pre << ";";
            }

            out << "InterlockedExchange";
            {
                ScopedParen sp(out);
                if (!EmitExpression(out, expr->args[0])) {
                    return false;
                }
                out << ", ";
                if (!EmitExpression(out, expr->args[1])) {
                    return false;
                }
                out << ", " << result;
            }
            return true;
        }
        case builtin::Function::kAtomicCompareExchangeWeak: {
            if (!EmitStructType(&helpers_, builtin->ReturnType()->As<type::Struct>())) {
                return false;
            }

            auto* dest = expr->args[0];
            auto* compare_value = expr->args[1];
            auto* value = expr->args[2];

            std::string compare = UniqueIdentifier("atomic_compare_value");

            {  // T compare_value = <compare_value>;
                auto pre = line();
                if (!EmitTypeAndName(pre, TypeOf(compare_value)->UnwrapRef(),
                                     builtin::AddressSpace::kUndefined, builtin::Access::kUndefined,
                                     compare)) {
                    return false;
                }
                pre << " = ";
                if (!EmitExpression(pre, compare_value)) {
                    return false;
                }
                pre << ";";
            }

            {  // InterlockedCompareExchange(dst, compare, value, result.old_value);
                auto pre = line();
                pre << "InterlockedCompareExchange";
                {
                    ScopedParen sp(pre);
                    if (!EmitExpression(pre, dest)) {
                        return false;
                    }
                    pre << ", " << compare << ", ";
                    if (!EmitExpression(pre, value)) {
                        return false;
                    }
                    pre << ", " << result << ".old_value";
                }
                pre << ";";
            }

            // result.exchanged = result.old_value == compare;
            line() << result << ".exchanged = " << result << ".old_value == " << compare << ";";

            out << result;
            return true;
        }

        case builtin::Function::kAtomicAdd:
        case builtin::Function::kAtomicSub:
            return call("InterlockedAdd");

        case builtin::Function::kAtomicMax:
            return call("InterlockedMax");

        case builtin::Function::kAtomicMin:
            return call("InterlockedMin");

        case builtin::Function::kAtomicAnd:
            return call("InterlockedAnd");

        case builtin::Function::kAtomicOr:
            return call("InterlockedOr");

        case builtin::Function::kAtomicXor:
            return call("InterlockedXor");

        case builtin::Function::kAtomicExchange:
            return call("InterlockedExchange");

        default:
            break;
    }

    TINT_UNREACHABLE(Writer, diagnostics_) << "unsupported atomic builtin: " << builtin->Type();
    return false;
}

bool GeneratorImpl::EmitSelectCall(utils::StringStream& out, const ast::CallExpression* expr) {
    auto* expr_false = expr->args[0];
    auto* expr_true = expr->args[1];
    auto* expr_cond = expr->args[2];
    ScopedParen paren(out);
    if (!EmitExpression(out, expr_cond)) {
        return false;
    }

    out << " ? ";

    if (!EmitExpression(out, expr_true)) {
        return false;
    }

    out << " : ";

    if (!EmitExpression(out, expr_false)) {
        return false;
    }

    return true;
}

bool GeneratorImpl::EmitModfCall(utils::StringStream& out,
                                 const ast::CallExpression* expr,
                                 const sem::Builtin* builtin) {
    return CallBuiltinHelper(
        out, expr, builtin, [&](TextBuffer* b, const std::vector<std::string>& params) {
            auto* ty = builtin->Parameters()[0]->Type();
            auto in = params[0];

            std::string width;
            if (auto* vec = ty->As<type::Vector>()) {
                width = std::to_string(vec->Width());
            }

            // Emit the builtin return type unique to this overload. This does not
            // exist in the AST, so it will not be generated in Generate().
            if (!EmitStructType(&helpers_, builtin->ReturnType()->As<type::Struct>())) {
                return false;
            }

            {
                auto l = line(b);
                if (!EmitType(l, builtin->ReturnType(), builtin::AddressSpace::kUndefined,
                              builtin::Access::kUndefined, "")) {
                    return false;
                }
                l << " result;";
            }
            line(b) << "result.fract = modf(" << params[0] << ", result.whole);";
            line(b) << "return result;";
            return true;
        });
}

bool GeneratorImpl::EmitFrexpCall(utils::StringStream& out,
                                  const ast::CallExpression* expr,
                                  const sem::Builtin* builtin) {
    return CallBuiltinHelper(
        out, expr, builtin, [&](TextBuffer* b, const std::vector<std::string>& params) {
            auto* ty = builtin->Parameters()[0]->Type();
            auto in = params[0];

            std::string width;
            if (auto* vec = ty->As<type::Vector>()) {
                width = std::to_string(vec->Width());
            }

            // Emit the builtin return type unique to this overload. This does not
            // exist in the AST, so it will not be generated in Generate().
            if (!EmitStructType(&helpers_, builtin->ReturnType()->As<type::Struct>())) {
                return false;
            }

            std::string member_type;
            if (Is<type::F16>(type::Type::DeepestElementOf(ty))) {
                member_type = width.empty() ? "float16_t" : ("vector<float16_t, " + width + ">");
            } else {
                member_type = "float" + width;
            }

            line(b) << member_type << " exp;";
            line(b) << member_type << " fract = sign(" << in << ") * frexp(" << in << ", exp);";
            {
                auto l = line(b);
                if (!EmitType(l, builtin->ReturnType(), builtin::AddressSpace::kUndefined,
                              builtin::Access::kUndefined, "")) {
                    return false;
                }
                l << " result = {fract, int" << width << "(exp)};";
            }
            line(b) << "return result;";
            return true;
        });
}

bool GeneratorImpl::EmitDegreesCall(utils::StringStream& out,
                                    const ast::CallExpression* expr,
                                    const sem::Builtin* builtin) {
    return CallBuiltinHelper(out, expr, builtin,
                             [&](TextBuffer* b, const std::vector<std::string>& params) {
                                 line(b) << "return " << params[0] << " * " << std::setprecision(20)
                                         << sem::kRadToDeg << ";";
                                 return true;
                             });
}

bool GeneratorImpl::EmitRadiansCall(utils::StringStream& out,
                                    const ast::CallExpression* expr,
                                    const sem::Builtin* builtin) {
    return CallBuiltinHelper(out, expr, builtin,
                             [&](TextBuffer* b, const std::vector<std::string>& params) {
                                 line(b) << "return " << params[0] << " * " << std::setprecision(20)
                                         << sem::kDegToRad << ";";
                                 return true;
                             });
}

// The HLSL `sign` method always returns an `int` result (scalar or vector). In WGSL the result is
// expected to be the same type as the argument. This injects a cast to the expected WGSL result
// type after the call to `sign`.
bool GeneratorImpl::EmitSignCall(utils::StringStream& out,
                                 const sem::Call* call,
                                 const sem::Builtin*) {
    auto* arg = call->Arguments()[0];
    if (!EmitType(out, arg->Type(), builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite,
                  "")) {
        return false;
    }
    out << "(sign(";
    if (!EmitExpression(out, arg->Declaration())) {
        return false;
    }
    out << "))";
    return true;
}

bool GeneratorImpl::EmitQuantizeToF16Call(utils::StringStream& out,
                                          const ast::CallExpression* expr,
                                          const sem::Builtin* builtin) {
    // Cast to f16 and back
    std::string width;
    if (auto* vec = builtin->ReturnType()->As<type::Vector>()) {
        width = std::to_string(vec->Width());
    }
    out << "f16tof32(f32tof16"
        << "(";
    if (!EmitExpression(out, expr->args[0])) {
        return false;
    }
    out << "))";
    return true;
}

bool GeneratorImpl::EmitTruncCall(utils::StringStream& out,
                                  const ast::CallExpression* expr,
                                  const sem::Builtin* builtin) {
    // HLSL's trunc is broken for very large/small float values.
    // See crbug.com/tint/1883
    return CallBuiltinHelper(  //
        out, expr, builtin, [&](TextBuffer* b, const std::vector<std::string>& params) {
            // value < 0 ? ceil(value) : floor(value)
            line(b) << "return " << params[0] << " < 0 ? ceil(" << params[0] << ") : floor("
                    << params[0] << ");";
            return true;
        });
}

bool GeneratorImpl::EmitDataPackingCall(utils::StringStream& out,
                                        const ast::CallExpression* expr,
                                        const sem::Builtin* builtin) {
    return CallBuiltinHelper(
        out, expr, builtin, [&](TextBuffer* b, const std::vector<std::string>& params) {
            uint32_t dims = 2;
            bool is_signed = false;
            uint32_t scale = 65535;
            if (builtin->Type() == builtin::Function::kPack4X8Snorm ||
                builtin->Type() == builtin::Function::kPack4X8Unorm) {
                dims = 4;
                scale = 255;
            }
            if (builtin->Type() == builtin::Function::kPack4X8Snorm ||
                builtin->Type() == builtin::Function::kPack2X16Snorm) {
                is_signed = true;
                scale = (scale - 1) / 2;
            }
            switch (builtin->Type()) {
                case builtin::Function::kPack4X8Snorm:
                case builtin::Function::kPack4X8Unorm:
                case builtin::Function::kPack2X16Snorm:
                case builtin::Function::kPack2X16Unorm: {
                    {
                        auto l = line(b);
                        l << (is_signed ? "" : "u") << "int" << dims
                          << " i = " << (is_signed ? "" : "u") << "int" << dims << "(round(clamp("
                          << params[0] << ", " << (is_signed ? "-1.0" : "0.0") << ", 1.0) * "
                          << scale << ".0))";
                        if (is_signed) {
                            l << " & " << (dims == 4 ? "0xff" : "0xffff");
                        }
                        l << ";";
                    }
                    {
                        auto l = line(b);
                        l << "return ";
                        if (is_signed) {
                            l << "asuint";
                        }
                        l << "(i.x | i.y << " << (32 / dims);
                        if (dims == 4) {
                            l << " | i.z << 16 | i.w << 24";
                        }
                        l << ");";
                    }
                    break;
                }
                case builtin::Function::kPack2X16Float: {
                    line(b) << "uint2 i = f32tof16(" << params[0] << ");";
                    line(b) << "return i.x | (i.y << 16);";
                    break;
                }
                default:
                    diagnostics_.add_error(diag::System::Writer,
                                           "Internal error: unhandled data packing builtin");
                    return false;
            }

            return true;
        });
}

bool GeneratorImpl::EmitDataUnpackingCall(utils::StringStream& out,
                                          const ast::CallExpression* expr,
                                          const sem::Builtin* builtin) {
    return CallBuiltinHelper(
        out, expr, builtin, [&](TextBuffer* b, const std::vector<std::string>& params) {
            uint32_t dims = 2;
            bool is_signed = false;
            uint32_t scale = 65535;
            if (builtin->Type() == builtin::Function::kUnpack4X8Snorm ||
                builtin->Type() == builtin::Function::kUnpack4X8Unorm) {
                dims = 4;
                scale = 255;
            }
            if (builtin->Type() == builtin::Function::kUnpack4X8Snorm ||
                builtin->Type() == builtin::Function::kUnpack2X16Snorm) {
                is_signed = true;
                scale = (scale - 1) / 2;
            }
            switch (builtin->Type()) {
                case builtin::Function::kUnpack4X8Snorm:
                case builtin::Function::kUnpack2X16Snorm: {
                    line(b) << "int j = int(" << params[0] << ");";
                    {  // Perform sign extension on the converted values.
                        auto l = line(b);
                        l << "int" << dims << " i = int" << dims << "(";
                        if (dims == 2) {
                            l << "j << 16, j) >> 16";
                        } else {
                            l << "j << 24, j << 16, j << 8, j) >> 24";
                        }
                        l << ";";
                    }
                    line(b) << "return clamp(float" << dims << "(i) / " << scale << ".0, "
                            << (is_signed ? "-1.0" : "0.0") << ", 1.0);";
                    break;
                }
                case builtin::Function::kUnpack4X8Unorm:
                case builtin::Function::kUnpack2X16Unorm: {
                    line(b) << "uint j = " << params[0] << ";";
                    {
                        auto l = line(b);
                        l << "uint" << dims << " i = uint" << dims << "(";
                        l << "j & " << (dims == 2 ? "0xffff" : "0xff") << ", ";
                        if (dims == 4) {
                            l << "(j >> " << (32 / dims) << ") & 0xff, (j >> 16) & 0xff, j >> 24";
                        } else {
                            l << "j >> " << (32 / dims);
                        }
                        l << ");";
                    }
                    line(b) << "return float" << dims << "(i) / " << scale << ".0;";
                    break;
                }
                case builtin::Function::kUnpack2X16Float:
                    line(b) << "uint i = " << params[0] << ";";
                    line(b) << "return f16tof32(uint2(i & 0xffff, i >> 16));";
                    break;
                default:
                    diagnostics_.add_error(diag::System::Writer,
                                           "Internal error: unhandled data packing builtin");
                    return false;
            }

            return true;
        });
}

bool GeneratorImpl::EmitDP4aCall(utils::StringStream& out,
                                 const ast::CallExpression* expr,
                                 const sem::Builtin* builtin) {
    // TODO(crbug.com/tint/1497): support the polyfill version of DP4a functions.
    return CallBuiltinHelper(
        out, expr, builtin, [&](TextBuffer* b, const std::vector<std::string>& params) {
            std::string functionName;
            switch (builtin->Type()) {
                case builtin::Function::kDot4I8Packed:
                    line(b) << "int accumulator = 0;";
                    functionName = "dot4add_i8packed";
                    break;
                case builtin::Function::kDot4U8Packed:
                    line(b) << "uint accumulator = 0u;";
                    functionName = "dot4add_u8packed";
                    break;
                default:
                    diagnostics_.add_error(diag::System::Writer,
                                           "Internal error: unhandled DP4a builtin");
                    return false;
            }
            line(b) << "return " << functionName << "(" << params[0] << ", " << params[1]
                    << ", accumulator);";

            return true;
        });
}

bool GeneratorImpl::EmitBarrierCall(utils::StringStream& out, const sem::Builtin* builtin) {
    // TODO(crbug.com/tint/661): Combine sequential barriers to a single
    // instruction.
    if (builtin->Type() == builtin::Function::kWorkgroupBarrier) {
        out << "GroupMemoryBarrierWithGroupSync()";
    } else if (builtin->Type() == builtin::Function::kStorageBarrier) {
        out << "DeviceMemoryBarrierWithGroupSync()";
    } else {
        TINT_UNREACHABLE(Writer, diagnostics_)
            << "unexpected barrier builtin type " << builtin::str(builtin->Type());
        return false;
    }
    return true;
}

bool GeneratorImpl::EmitTextureCall(utils::StringStream& out,
                                    const sem::Call* call,
                                    const sem::Builtin* builtin) {
    using Usage = sem::ParameterUsage;

    auto& signature = builtin->Signature();
    auto* expr = call->Declaration();
    auto arguments = expr->args;

    // Returns the argument with the given usage
    auto arg = [&](Usage usage) {
        int idx = signature.IndexOf(usage);
        return (idx >= 0) ? arguments[static_cast<size_t>(idx)] : nullptr;
    };

    auto* texture = arg(Usage::kTexture);
    if (TINT_UNLIKELY(!texture)) {
        TINT_ICE(Writer, diagnostics_) << "missing texture argument";
        return false;
    }

    auto* texture_type = TypeOf(texture)->UnwrapRef()->As<type::Texture>();

    switch (builtin->Type()) {
        case builtin::Function::kTextureDimensions:
        case builtin::Function::kTextureNumLayers:
        case builtin::Function::kTextureNumLevels:
        case builtin::Function::kTextureNumSamples: {
            // All of these builtins use the GetDimensions() method on the texture
            bool is_ms =
                texture_type->IsAnyOf<type::MultisampledTexture, type::DepthMultisampledTexture>();
            int num_dimensions = 0;
            std::string swizzle;

            switch (builtin->Type()) {
                case builtin::Function::kTextureDimensions:
                    switch (texture_type->dim()) {
                        case type::TextureDimension::kNone:
                            TINT_ICE(Writer, diagnostics_) << "texture dimension is kNone";
                            return false;
                        case type::TextureDimension::k1d:
                            num_dimensions = 1;
                            break;
                        case type::TextureDimension::k2d:
                            num_dimensions = is_ms ? 3 : 2;
                            swizzle = is_ms ? ".xy" : "";
                            break;
                        case type::TextureDimension::k2dArray:
                            num_dimensions = is_ms ? 4 : 3;
                            swizzle = ".xy";
                            break;
                        case type::TextureDimension::k3d:
                            num_dimensions = 3;
                            break;
                        case type::TextureDimension::kCube:
                            num_dimensions = 2;
                            break;
                        case type::TextureDimension::kCubeArray:
                            num_dimensions = 3;
                            swizzle = ".xy";
                            break;
                    }
                    break;
                case builtin::Function::kTextureNumLayers:
                    switch (texture_type->dim()) {
                        default:
                            TINT_ICE(Writer, diagnostics_) << "texture dimension is not arrayed";
                            return false;
                        case type::TextureDimension::k2dArray:
                            num_dimensions = is_ms ? 4 : 3;
                            swizzle = ".z";
                            break;
                        case type::TextureDimension::kCubeArray:
                            num_dimensions = 3;
                            swizzle = ".z";
                            break;
                    }
                    break;
                case builtin::Function::kTextureNumLevels:
                    switch (texture_type->dim()) {
                        default:
                            TINT_ICE(Writer, diagnostics_)
                                << "texture dimension does not support mips";
                            return false;
                        case type::TextureDimension::k1d:
                            num_dimensions = 2;
                            swizzle = ".y";
                            break;
                        case type::TextureDimension::k2d:
                        case type::TextureDimension::kCube:
                            num_dimensions = 3;
                            swizzle = ".z";
                            break;
                        case type::TextureDimension::k2dArray:
                        case type::TextureDimension::k3d:
                        case type::TextureDimension::kCubeArray:
                            num_dimensions = 4;
                            swizzle = ".w";
                            break;
                    }
                    break;
                case builtin::Function::kTextureNumSamples:
                    switch (texture_type->dim()) {
                        default:
                            TINT_ICE(Writer, diagnostics_)
                                << "texture dimension does not support multisampling";
                            return false;
                        case type::TextureDimension::k2d:
                            num_dimensions = 3;
                            swizzle = ".z";
                            break;
                        case type::TextureDimension::k2dArray:
                            num_dimensions = 4;
                            swizzle = ".w";
                            break;
                    }
                    break;
                default:
                    TINT_ICE(Writer, diagnostics_) << "unexpected builtin";
                    return false;
            }

            auto* level_arg = arg(Usage::kLevel);

            if (level_arg) {
                // `NumberOfLevels` is a non-optional argument if `MipLevel` was passed.
                // Increment the number of dimensions for the temporary vector to
                // accommodate this.
                num_dimensions++;

                // If the swizzle was empty, the expression will evaluate to the whole
                // vector. As we've grown the vector by one element, we now need to
                // swizzle to keep the result expression equivalent.
                if (swizzle.empty()) {
                    static constexpr const char* swizzles[] = {"", ".x", ".xy", ".xyz"};
                    swizzle = swizzles[num_dimensions - 1];
                }
            }

            if (TINT_UNLIKELY(num_dimensions > 4)) {
                TINT_ICE(Writer, diagnostics_) << "Texture query builtin temporary vector has "
                                               << num_dimensions << " dimensions";
                return false;
            }

            // Declare a variable to hold the queried texture info
            auto dims = UniqueIdentifier(kTempNamePrefix);
            if (num_dimensions == 1) {
                line() << "uint " << dims << ";";
            } else {
                line() << "uint" << num_dimensions << " " << dims << ";";
            }

            {  // texture.GetDimensions(...)
                auto pre = line();
                if (!EmitExpression(pre, texture)) {
                    return false;
                }
                pre << ".GetDimensions(";

                if (level_arg) {
                    if (!EmitExpression(pre, level_arg)) {
                        return false;
                    }
                    pre << ", ";
                } else if (builtin->Type() == builtin::Function::kTextureNumLevels) {
                    pre << "0, ";
                }

                if (num_dimensions == 1) {
                    pre << dims;
                } else {
                    static constexpr char xyzw[] = {'x', 'y', 'z', 'w'};
                    if (TINT_UNLIKELY(num_dimensions < 0 || num_dimensions > 4)) {
                        TINT_ICE(Writer, diagnostics_)
                            << "vector dimensions are " << num_dimensions;
                        return false;
                    }
                    for (int i = 0; i < num_dimensions; i++) {
                        if (i > 0) {
                            pre << ", ";
                        }
                        pre << dims << "." << xyzw[i];
                    }
                }

                pre << ");";
            }

            // The out parameters of the GetDimensions() call is now in temporary
            // `dims` variable. This may be packed with other data, so the final
            // expression may require a swizzle.
            out << dims << swizzle;
            return true;
        }
        default:
            break;
    }

    if (!EmitExpression(out, texture)) {
        return false;
    }

    // If pack_level_in_coords is true, then the mip level will be appended as the
    // last value of the coordinates argument. If the WGSL builtin overload does
    // not have a level parameter and pack_level_in_coords is true, then a zero
    // mip level will be inserted.
    bool pack_level_in_coords = false;

    uint32_t hlsl_ret_width = 4u;

    switch (builtin->Type()) {
        case builtin::Function::kTextureSample:
            out << ".Sample(";
            break;
        case builtin::Function::kTextureSampleBias:
            out << ".SampleBias(";
            break;
        case builtin::Function::kTextureSampleLevel:
            out << ".SampleLevel(";
            break;
        case builtin::Function::kTextureSampleGrad:
            out << ".SampleGrad(";
            break;
        case builtin::Function::kTextureSampleCompare:
            out << ".SampleCmp(";
            hlsl_ret_width = 1;
            break;
        case builtin::Function::kTextureSampleCompareLevel:
            out << ".SampleCmpLevelZero(";
            hlsl_ret_width = 1;
            break;
        case builtin::Function::kTextureLoad:
            out << ".Load(";
            // Multisampled textures do not support mip-levels.
            if (!texture_type->Is<type::MultisampledTexture>()) {
                pack_level_in_coords = true;
            }
            break;
        case builtin::Function::kTextureGather:
            out << ".Gather";
            if (builtin->Parameters()[0]->Usage() == sem::ParameterUsage::kComponent) {
                switch (call->Arguments()[0]->ConstantValue()->ValueAs<AInt>()) {
                    case 0:
                        out << "Red";
                        break;
                    case 1:
                        out << "Green";
                        break;
                    case 2:
                        out << "Blue";
                        break;
                    case 3:
                        out << "Alpha";
                        break;
                }
            }
            out << "(";
            break;
        case builtin::Function::kTextureGatherCompare:
            out << ".GatherCmp(";
            break;
        case builtin::Function::kTextureStore:
            out << "[";
            break;
        default:
            diagnostics_.add_error(diag::System::Writer,
                                   "Internal compiler error: Unhandled texture builtin '" +
                                       std::string(builtin->str()) + "'");
            return false;
    }

    if (auto* sampler = arg(Usage::kSampler)) {
        if (!EmitExpression(out, sampler)) {
            return false;
        }
        out << ", ";
    }

    auto* param_coords = arg(Usage::kCoords);
    if (TINT_UNLIKELY(!param_coords)) {
        TINT_ICE(Writer, diagnostics_) << "missing coords argument";
        return false;
    }

    auto emit_vector_appended_with_i32_zero = [&](const ast::Expression* vector) {
        auto* i32 = builder_.create<type::I32>();
        auto* zero = builder_.Expr(0_i);
        auto* stmt = builder_.Sem().Get(vector)->Stmt();
        builder_.Sem().Add(zero, builder_.create<sem::ValueExpression>(
                                     zero, i32, sem::EvaluationStage::kRuntime, stmt,
                                     /* constant_value */ nullptr,
                                     /* has_side_effects */ false));
        auto* packed = AppendVector(&builder_, vector, zero);
        return EmitExpression(out, packed->Declaration());
    };

    auto emit_vector_appended_with_level = [&](const ast::Expression* vector) {
        if (auto* level = arg(Usage::kLevel)) {
            auto* packed = AppendVector(&builder_, vector, level);
            return EmitExpression(out, packed->Declaration());
        }
        return emit_vector_appended_with_i32_zero(vector);
    };

    if (auto* array_index = arg(Usage::kArrayIndex)) {
        // Array index needs to be appended to the coordinates.
        auto* packed = AppendVector(&builder_, param_coords, array_index);
        if (pack_level_in_coords) {
            // Then mip level needs to be appended to the coordinates.
            if (!emit_vector_appended_with_level(packed->Declaration())) {
                return false;
            }
        } else {
            if (!EmitExpression(out, packed->Declaration())) {
                return false;
            }
        }
    } else if (pack_level_in_coords) {
        // Mip level needs to be appended to the coordinates.
        if (!emit_vector_appended_with_level(param_coords)) {
            return false;
        }
    } else {
        if (!EmitExpression(out, param_coords)) {
            return false;
        }
    }

    for (auto usage : {Usage::kDepthRef, Usage::kBias, Usage::kLevel, Usage::kDdx, Usage::kDdy,
                       Usage::kSampleIndex, Usage::kOffset}) {
        if (usage == Usage::kLevel && pack_level_in_coords) {
            continue;  // mip level already packed in coordinates.
        }
        if (auto* e = arg(usage)) {
            out << ", ";
            if (!EmitExpression(out, e)) {
                return false;
            }
        }
    }

    if (builtin->Type() == builtin::Function::kTextureStore) {
        out << "] = ";
        if (!EmitExpression(out, arg(Usage::kValue))) {
            return false;
        }
    } else {
        out << ")";

        // If the builtin return type does not match the number of elements of the
        // HLSL builtin, we need to swizzle the expression to generate the correct
        // number of components.
        uint32_t wgsl_ret_width = 1;
        if (auto* vec = builtin->ReturnType()->As<type::Vector>()) {
            wgsl_ret_width = vec->Width();
        }
        if (wgsl_ret_width < hlsl_ret_width) {
            out << ".";
            for (uint32_t i = 0; i < wgsl_ret_width; i++) {
                out << "xyz"[i];
            }
        }
        if (TINT_UNLIKELY(wgsl_ret_width > hlsl_ret_width)) {
            TINT_ICE(Writer, diagnostics_)
                << "WGSL return width (" << wgsl_ret_width << ") is wider than HLSL return width ("
                << hlsl_ret_width << ") for " << builtin->Type();
            return false;
        }
    }

    return true;
}

std::string GeneratorImpl::generate_builtin_name(const sem::Builtin* builtin) {
    switch (builtin->Type()) {
        case builtin::Function::kAbs:
        case builtin::Function::kAcos:
        case builtin::Function::kAll:
        case builtin::Function::kAny:
        case builtin::Function::kAsin:
        case builtin::Function::kAtan:
        case builtin::Function::kAtan2:
        case builtin::Function::kCeil:
        case builtin::Function::kClamp:
        case builtin::Function::kCos:
        case builtin::Function::kCosh:
        case builtin::Function::kCross:
        case builtin::Function::kDeterminant:
        case builtin::Function::kDistance:
        case builtin::Function::kDot:
        case builtin::Function::kExp:
        case builtin::Function::kExp2:
        case builtin::Function::kFloor:
        case builtin::Function::kFrexp:
        case builtin::Function::kLdexp:
        case builtin::Function::kLength:
        case builtin::Function::kLog:
        case builtin::Function::kLog2:
        case builtin::Function::kMax:
        case builtin::Function::kMin:
        case builtin::Function::kModf:
        case builtin::Function::kNormalize:
        case builtin::Function::kPow:
        case builtin::Function::kReflect:
        case builtin::Function::kRefract:
        case builtin::Function::kRound:
        case builtin::Function::kSaturate:
        case builtin::Function::kSin:
        case builtin::Function::kSinh:
        case builtin::Function::kSqrt:
        case builtin::Function::kStep:
        case builtin::Function::kTan:
        case builtin::Function::kTanh:
        case builtin::Function::kTranspose:
            return builtin->str();
        case builtin::Function::kCountOneBits:  // uint
            return "countbits";
        case builtin::Function::kDpdx:
            return "ddx";
        case builtin::Function::kDpdxCoarse:
            return "ddx_coarse";
        case builtin::Function::kDpdxFine:
            return "ddx_fine";
        case builtin::Function::kDpdy:
            return "ddy";
        case builtin::Function::kDpdyCoarse:
            return "ddy_coarse";
        case builtin::Function::kDpdyFine:
            return "ddy_fine";
        case builtin::Function::kFaceForward:
            return "faceforward";
        case builtin::Function::kFract:
            return "frac";
        case builtin::Function::kFma:
            return "mad";
        case builtin::Function::kFwidth:
        case builtin::Function::kFwidthCoarse:
        case builtin::Function::kFwidthFine:
            return "fwidth";
        case builtin::Function::kInverseSqrt:
            return "rsqrt";
        case builtin::Function::kMix:
            return "lerp";
        case builtin::Function::kReverseBits:  // uint
            return "reversebits";
        case builtin::Function::kSmoothstep:
            return "smoothstep";
        default:
            diagnostics_.add_error(diag::System::Writer,
                                   "Unknown builtin method: " + std::string(builtin->str()));
    }

    return "";
}

bool GeneratorImpl::EmitCase(const ast::SwitchStatement* s, size_t case_idx) {
    auto* stmt = s->body[case_idx];
    auto* sem = builder_.Sem().Get<sem::CaseStatement>(stmt);
    for (auto* selector : sem->Selectors()) {
        auto out = line();
        if (selector->IsDefault()) {
            out << "default";
        } else {
            out << "case ";
            if (!EmitConstant(out, selector->Value(), /* is_variable_initializer */ false)) {
                return false;
            }
        }
        out << ":";
        if (selector == sem->Selectors().back()) {
            out << " {";
        }
    }

    increment_indent();
    TINT_DEFER({
        decrement_indent();
        line() << "}";
    });

    // Emit the case statement
    if (!EmitStatements(stmt->body->statements)) {
        return false;
    }

    if (!tint::utils::IsAnyOf<ast::BreakStatement>(stmt->body->Last())) {
        line() << "break;";
    }

    return true;
}

bool GeneratorImpl::EmitContinue(const ast::ContinueStatement*) {
    if (!emit_continuing_ || !emit_continuing_()) {
        return false;
    }
    line() << "continue;";
    return true;
}

bool GeneratorImpl::EmitDiscard(const ast::DiscardStatement*) {
    // TODO(dsinclair): Verify this is correct when the discard semantics are
    // defined for WGSL (https://github.com/gpuweb/gpuweb/issues/361)
    line() << "discard;";
    return true;
}

bool GeneratorImpl::EmitExpression(utils::StringStream& out, const ast::Expression* expr) {
    if (auto* sem = builder_.Sem().GetVal(expr)) {
        if (auto* constant = sem->ConstantValue()) {
            bool is_variable_initializer = false;
            if (auto* stmt = sem->Stmt()) {
                if (auto* decl = As<ast::VariableDeclStatement>(stmt->Declaration())) {
                    is_variable_initializer = decl->variable->initializer == expr;
                }
            }
            return EmitConstant(out, constant, is_variable_initializer);
        }
    }
    return Switch(
        expr,  //
        [&](const ast::IndexAccessorExpression* a) { return EmitIndexAccessor(out, a); },
        [&](const ast::BinaryExpression* b) { return EmitBinary(out, b); },
        [&](const ast::BitcastExpression* b) { return EmitBitcast(out, b); },
        [&](const ast::CallExpression* c) { return EmitCall(out, c); },
        [&](const ast::IdentifierExpression* i) { return EmitIdentifier(out, i); },
        [&](const ast::LiteralExpression* l) { return EmitLiteral(out, l); },
        [&](const ast::MemberAccessorExpression* m) { return EmitMemberAccessor(out, m); },
        [&](const ast::UnaryOpExpression* u) { return EmitUnaryOp(out, u); },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer, "unknown expression type: " +
                                                             std::string(expr->TypeInfo().name));
            return false;
        });
}

bool GeneratorImpl::EmitIdentifier(utils::StringStream& out,
                                   const ast::IdentifierExpression* expr) {
    out << expr->identifier->symbol.Name();
    return true;
}

bool GeneratorImpl::EmitIf(const ast::IfStatement* stmt) {
    {
        auto out = line();
        out << "if (";
        if (!EmitExpression(out, stmt->condition)) {
            return false;
        }
        out << ") {";
    }

    if (!EmitStatementsWithIndent(stmt->body->statements)) {
        return false;
    }

    if (stmt->else_statement) {
        line() << "} else {";
        if (auto* block = stmt->else_statement->As<ast::BlockStatement>()) {
            if (!EmitStatementsWithIndent(block->statements)) {
                return false;
            }
        } else {
            if (!EmitStatementsWithIndent(utils::Vector{stmt->else_statement})) {
                return false;
            }
        }
    }
    line() << "}";

    return true;
}

bool GeneratorImpl::EmitFunction(const ast::Function* func) {
    auto* sem = builder_.Sem().Get(func);

    // Emit storage atomic helpers
    if (auto* intrinsic =
            ast::GetAttribute<transform::DecomposeMemoryAccess::Intrinsic>(func->attributes)) {
        if (intrinsic->address_space == builtin::AddressSpace::kStorage && intrinsic->IsAtomic()) {
            if (!EmitStorageAtomicIntrinsic(func, intrinsic)) {
                return false;
            }
        }
        return true;
    }

    if (ast::HasAttribute<ast::InternalAttribute>(func->attributes)) {
        // An internal function. Do not emit.
        return true;
    }

    {
        auto out = line();
        auto name = func->name->symbol.Name();
        // If the function returns an array, then we need to declare a typedef for
        // this.
        if (sem->ReturnType()->Is<type::Array>()) {
            auto typedef_name = UniqueIdentifier(name + "_ret");
            auto pre = line();
            pre << "typedef ";
            if (!EmitTypeAndName(pre, sem->ReturnType(), builtin::AddressSpace::kUndefined,
                                 builtin::Access::kReadWrite, typedef_name)) {
                return false;
            }
            pre << ";";
            out << typedef_name;
        } else {
            if (!EmitType(out, sem->ReturnType(), builtin::AddressSpace::kUndefined,
                          builtin::Access::kReadWrite, "")) {
                return false;
            }
        }

        out << " " << name << "(";

        bool first = true;

        for (auto* v : sem->Parameters()) {
            if (!first) {
                out << ", ";
            }
            first = false;

            auto const* type = v->Type();
            auto address_space = builtin::AddressSpace::kUndefined;
            auto access = builtin::Access::kUndefined;

            if (auto* ptr = type->As<type::Pointer>()) {
                type = ptr->StoreType();
                switch (ptr->AddressSpace()) {
                    case builtin::AddressSpace::kStorage:
                    case builtin::AddressSpace::kUniform:
                        // Not allowed by WGSL, but is used by certain transforms (e.g. DMA) to pass
                        // storage buffers and uniform buffers down into transform-generated
                        // functions. In this situation we want to generate the parameter without an
                        // 'inout', using the address space and access from the pointer.
                        address_space = ptr->AddressSpace();
                        access = ptr->Access();
                        break;
                    default:
                        // Transform regular WGSL pointer parameters in to `inout` parameters.
                        out << "inout ";
                }
            }

            // Note: WGSL only allows for AddressSpace::kUndefined on parameters, however
            // the sanitizer transforms generates load / store functions for storage
            // or uniform buffers. These functions have a buffer parameter with
            // AddressSpace::kStorage or AddressSpace::kUniform. This is required to
            // correctly translate the parameter to a [RW]ByteAddressBuffer for
            // storage buffers and a uint4[N] for uniform buffers.
            if (!EmitTypeAndName(out, type, address_space, access,
                                 v->Declaration()->name->symbol.Name())) {
                return false;
            }
        }
        out << ") {";
    }

    if (sem->DiscardStatement() && !sem->ReturnType()->Is<type::Void>()) {
        // BUG(crbug.com/tint/1081): work around non-void functions with discard
        // failing compilation sometimes
        if (!EmitFunctionBodyWithDiscard(func)) {
            return false;
        }
    } else {
        if (!EmitStatementsWithIndent(func->body->statements)) {
            return false;
        }
    }

    line() << "}";

    return true;
}

bool GeneratorImpl::EmitFunctionBodyWithDiscard(const ast::Function* func) {
    // FXC sometimes fails to compile functions that discard with 'Not all control
    // paths return a value'. We work around this by wrapping the function body
    // within an "if (true) { <body> } return <default return type obj>;" so that
    // there is always an (unused) return statement.

    auto* sem = builder_.Sem().Get(func);
    TINT_ASSERT(Writer, sem->DiscardStatement() && !sem->ReturnType()->Is<type::Void>());

    ScopedIndent si(this);
    line() << "if (true) {";

    if (!EmitStatementsWithIndent(func->body->statements)) {
        return false;
    }

    line() << "}";

    // Return an unused result that matches the type of the return value
    auto name = builder_.Symbols().New("unused").Name();
    {
        auto out = line();
        if (!EmitTypeAndName(out, sem->ReturnType(), builtin::AddressSpace::kUndefined,
                             builtin::Access::kReadWrite, name)) {
            return false;
        }
        out << ";";
    }
    line() << "return " << name << ";";

    return true;
}

bool GeneratorImpl::EmitGlobalVariable(const ast::Variable* global) {
    return Switch(
        global,  //
        [&](const ast::Var* var) {
            auto* sem = builder_.Sem().Get(global);
            switch (sem->AddressSpace()) {
                case builtin::AddressSpace::kUniform:
                    return EmitUniformVariable(var, sem);
                case builtin::AddressSpace::kStorage:
                    return EmitStorageVariable(var, sem);
                case builtin::AddressSpace::kHandle:
                    return EmitHandleVariable(var, sem);
                case builtin::AddressSpace::kPrivate:
                    return EmitPrivateVariable(sem);
                case builtin::AddressSpace::kWorkgroup:
                    return EmitWorkgroupVariable(sem);
                case builtin::AddressSpace::kPushConstant:
                    diagnostics_.add_error(
                        diag::System::Writer,
                        "unhandled address space " + utils::ToString(sem->AddressSpace()));
                    return false;
                default: {
                    TINT_ICE(Writer, diagnostics_)
                        << "unhandled address space " << sem->AddressSpace();
                    return false;
                }
            }
        },
        [&](const ast::Override*) {
            // Override is removed with SubstituteOverride
            diagnostics_.add_error(diag::System::Writer,
                                   "override-expressions should have been removed with the "
                                   "SubstituteOverride transform");
            return false;
        },
        [&](const ast::Const*) {
            return true;  // Constants are embedded at their use
        },
        [&](Default) {
            TINT_ICE(Writer, diagnostics_)
                << "unhandled global variable type " << global->TypeInfo().name;

            return false;
        });
}

bool GeneratorImpl::EmitUniformVariable(const ast::Var* var, const sem::Variable* sem) {
    auto binding_point = *sem->As<sem::GlobalVariable>()->BindingPoint();
    auto* type = sem->Type()->UnwrapRef();
    auto name = var->name->symbol.Name();
    line() << "cbuffer cbuffer_" << name << RegisterAndSpace('b', binding_point) << " {";

    {
        ScopedIndent si(this);
        auto out = line();
        if (!EmitTypeAndName(out, type, builtin::AddressSpace::kUniform, sem->Access(), name)) {
            return false;
        }
        out << ";";
    }

    line() << "};";

    return true;
}

bool GeneratorImpl::EmitStorageVariable(const ast::Var* var, const sem::Variable* sem) {
    auto* type = sem->Type()->UnwrapRef();
    auto out = line();
    if (!EmitTypeAndName(out, type, builtin::AddressSpace::kStorage, sem->Access(),
                         var->name->symbol.Name())) {
        return false;
    }

    auto* global_sem = sem->As<sem::GlobalVariable>();
    out << RegisterAndSpace(sem->Access() == builtin::Access::kRead ? 't' : 'u',
                            *global_sem->BindingPoint())
        << ";";

    return true;
}

bool GeneratorImpl::EmitHandleVariable(const ast::Var* var, const sem::Variable* sem) {
    auto* unwrapped_type = sem->Type()->UnwrapRef();
    auto out = line();

    auto name = var->name->symbol.Name();
    auto* type = sem->Type()->UnwrapRef();
    if (!EmitTypeAndName(out, type, sem->AddressSpace(), sem->Access(), name)) {
        return false;
    }

    const char* register_space = nullptr;

    if (unwrapped_type->Is<type::Texture>()) {
        register_space = "t";
        if (unwrapped_type->Is<type::StorageTexture>()) {
            register_space = "u";
        }
    } else if (unwrapped_type->Is<type::Sampler>()) {
        register_space = "s";
    }

    if (register_space) {
        auto bp = sem->As<sem::GlobalVariable>()->BindingPoint();
        out << " : register(" << register_space << bp->binding;
        // Omit the space if it's 0, as it's the default.
        // SM 5.0 doesn't support spaces, so we don't emit them if group is 0 for better
        // compatibility.
        if (bp->group == 0) {
            out << ")";
        } else {
            out << ", space" << bp->group << ")";
        }
    }

    out << ";";
    return true;
}

bool GeneratorImpl::EmitPrivateVariable(const sem::Variable* var) {
    auto* decl = var->Declaration();
    auto out = line();

    out << "static ";

    auto name = decl->name->symbol.Name();
    auto* type = var->Type()->UnwrapRef();
    if (!EmitTypeAndName(out, type, var->AddressSpace(), var->Access(), name)) {
        return false;
    }

    out << " = ";
    if (auto* initializer = decl->initializer) {
        if (!EmitExpression(out, initializer)) {
            return false;
        }
    } else {
        if (!EmitZeroValue(out, var->Type()->UnwrapRef())) {
            return false;
        }
    }

    out << ";";
    return true;
}

bool GeneratorImpl::EmitWorkgroupVariable(const sem::Variable* var) {
    auto* decl = var->Declaration();
    auto out = line();

    out << "groupshared ";

    auto name = decl->name->symbol.Name();
    auto* type = var->Type()->UnwrapRef();
    if (!EmitTypeAndName(out, type, var->AddressSpace(), var->Access(), name)) {
        return false;
    }

    if (auto* initializer = decl->initializer) {
        out << " = ";
        if (!EmitExpression(out, initializer)) {
            return false;
        }
    }

    out << ";";
    return true;
}

std::string GeneratorImpl::builtin_to_attribute(builtin::BuiltinValue builtin) const {
    switch (builtin) {
        case builtin::BuiltinValue::kPosition:
            return "SV_Position";
        case builtin::BuiltinValue::kVertexIndex:
            return "SV_VertexID";
        case builtin::BuiltinValue::kInstanceIndex:
            return "SV_InstanceID";
        case builtin::BuiltinValue::kFrontFacing:
            return "SV_IsFrontFace";
        case builtin::BuiltinValue::kFragDepth:
            return "SV_Depth";
        case builtin::BuiltinValue::kLocalInvocationId:
            return "SV_GroupThreadID";
        case builtin::BuiltinValue::kLocalInvocationIndex:
            return "SV_GroupIndex";
        case builtin::BuiltinValue::kGlobalInvocationId:
            return "SV_DispatchThreadID";
        case builtin::BuiltinValue::kWorkgroupId:
            return "SV_GroupID";
        case builtin::BuiltinValue::kSampleIndex:
            return "SV_SampleIndex";
        case builtin::BuiltinValue::kSampleMask:
            return "SV_Coverage";
        default:
            break;
    }
    return "";
}

std::string GeneratorImpl::interpolation_to_modifiers(
    builtin::InterpolationType type,
    builtin::InterpolationSampling sampling) const {
    std::string modifiers;
    switch (type) {
        case builtin::InterpolationType::kPerspective:
            modifiers += "linear ";
            break;
        case builtin::InterpolationType::kLinear:
            modifiers += "noperspective ";
            break;
        case builtin::InterpolationType::kFlat:
            modifiers += "nointerpolation ";
            break;
        case builtin::InterpolationType::kUndefined:
            break;
    }
    switch (sampling) {
        case builtin::InterpolationSampling::kCentroid:
            modifiers += "centroid ";
            break;
        case builtin::InterpolationSampling::kSample:
            modifiers += "sample ";
            break;
        case builtin::InterpolationSampling::kCenter:
        case builtin::InterpolationSampling::kUndefined:
            break;
    }
    return modifiers;
}

bool GeneratorImpl::EmitEntryPointFunction(const ast::Function* func) {
    auto* func_sem = builder_.Sem().Get(func);

    {
        auto out = line();
        if (func->PipelineStage() == ast::PipelineStage::kCompute) {
            // Emit the workgroup_size attribute.
            auto wgsize = func_sem->WorkgroupSize();
            out << "[numthreads(";
            for (size_t i = 0; i < 3; i++) {
                if (i > 0) {
                    out << ", ";
                }
                if (!wgsize[i].has_value()) {
                    diagnostics_.add_error(
                        diag::System::Writer,
                        "override-expressions should have been removed with the SubstituteOverride "
                        "transform");
                    return false;
                }
                out << std::to_string(wgsize[i].value());
            }
            out << ")]" << std::endl;
        }

        if (!EmitTypeAndName(out, func_sem->ReturnType(), builtin::AddressSpace::kUndefined,
                             builtin::Access::kUndefined, func->name->symbol.Name())) {
            return false;
        }
        out << "(";

        bool first = true;

        // Emit entry point parameters.
        for (auto* var : func->params) {
            auto* sem = builder_.Sem().Get(var);
            auto* type = sem->Type();
            if (TINT_UNLIKELY(!type->Is<type::Struct>())) {
                // ICE likely indicates that the CanonicalizeEntryPointIO transform was
                // not run, or a builtin parameter was added after it was run.
                TINT_ICE(Writer, diagnostics_) << "Unsupported non-struct entry point parameter";
            }

            if (!first) {
                out << ", ";
            }
            first = false;

            if (!EmitTypeAndName(out, type, sem->AddressSpace(), sem->Access(),
                                 var->name->symbol.Name())) {
                return false;
            }
        }

        out << ") {";
    }

    {
        ScopedIndent si(this);

        if (!EmitStatements(func->body->statements)) {
            return false;
        }

        if (!Is<ast::ReturnStatement>(func->body->Last())) {
            ast::ReturnStatement ret(ProgramID(), ast::NodeID{}, Source{});
            if (!EmitStatement(&ret)) {
                return false;
            }
        }
    }

    line() << "}";

    return true;
}

bool GeneratorImpl::EmitConstant(utils::StringStream& out,
                                 const constant::Value* constant,
                                 bool is_variable_initializer) {
    return Switch(
        constant->Type(),  //
        [&](const type::Bool*) {
            out << (constant->ValueAs<AInt>() ? "true" : "false");
            return true;
        },
        [&](const type::F32*) {
            PrintF32(out, constant->ValueAs<f32>());
            return true;
        },
        [&](const type::F16*) {
            // emit a f16 scalar with explicit float16_t type declaration.
            out << "float16_t(";
            PrintF16(out, constant->ValueAs<f16>());
            out << ")";
            return true;
        },
        [&](const type::I32*) {
            out << constant->ValueAs<AInt>();
            return true;
        },
        [&](const type::U32*) {
            out << constant->ValueAs<AInt>() << "u";
            return true;
        },
        [&](const type::Vector* v) {
            if (auto* splat = constant->As<constant::Splat>()) {
                {
                    ScopedParen sp(out);
                    if (!EmitConstant(out, splat->el, is_variable_initializer)) {
                        return false;
                    }
                }
                out << ".";
                for (size_t i = 0; i < v->Width(); i++) {
                    out << "x";
                }
                return true;
            }

            if (!EmitType(out, v, builtin::AddressSpace::kUndefined, builtin::Access::kUndefined,
                          "")) {
                return false;
            }

            ScopedParen sp(out);

            for (size_t i = 0; i < v->Width(); i++) {
                if (i > 0) {
                    out << ", ";
                }
                if (!EmitConstant(out, constant->Index(i), is_variable_initializer)) {
                    return false;
                }
            }
            return true;
        },
        [&](const type::Matrix* m) {
            if (!EmitType(out, m, builtin::AddressSpace::kUndefined, builtin::Access::kUndefined,
                          "")) {
                return false;
            }

            ScopedParen sp(out);

            for (size_t i = 0; i < m->columns(); i++) {
                if (i > 0) {
                    out << ", ";
                }
                if (!EmitConstant(out, constant->Index(i), is_variable_initializer)) {
                    return false;
                }
            }
            return true;
        },
        [&](const type::Array* a) {
            if (constant->AllZero()) {
                out << "(";
                if (!EmitType(out, a, builtin::AddressSpace::kUndefined,
                              builtin::Access::kUndefined, "")) {
                    return false;
                }
                out << ")0";
                return true;
            }

            out << "{";
            TINT_DEFER(out << "}");

            auto count = a->ConstantCount();
            if (!count) {
                diagnostics_.add_error(diag::System::Writer,
                                       type::Array::kErrExpectedConstantCount);
                return false;
            }

            for (size_t i = 0; i < count; i++) {
                if (i > 0) {
                    out << ", ";
                }
                if (!EmitConstant(out, constant->Index(i), is_variable_initializer)) {
                    return false;
                }
            }

            return true;
        },
        [&](const type::Struct* s) {
            if (!EmitStructType(&helpers_, s)) {
                return false;
            }

            if (constant->AllZero()) {
                out << "(" << StructName(s) << ")0";
                return true;
            }

            auto emit_member_values = [&](utils::StringStream& o) {
                o << "{";
                for (size_t i = 0; i < s->Members().Length(); i++) {
                    if (i > 0) {
                        o << ", ";
                    }
                    if (!EmitConstant(o, constant->Index(i), is_variable_initializer)) {
                        return false;
                    }
                }
                o << "}";
                return true;
            };

            if (is_variable_initializer) {
                if (!emit_member_values(out)) {
                    return false;
                }
            } else {
                // HLSL requires structure initializers to be assigned directly to a variable.
                auto name = UniqueIdentifier("c");
                {
                    auto decl = line();
                    decl << "const " << StructName(s) << " " << name << " = ";
                    if (!emit_member_values(decl)) {
                        return false;
                    }
                    decl << ";";
                }
                out << name;
            }

            return true;
        },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer,
                                   "unhandled constant type: " + constant->Type()->FriendlyName());
            return false;
        });
}

bool GeneratorImpl::EmitLiteral(utils::StringStream& out, const ast::LiteralExpression* lit) {
    return Switch(
        lit,
        [&](const ast::BoolLiteralExpression* l) {
            out << (l->value ? "true" : "false");
            return true;
        },
        [&](const ast::FloatLiteralExpression* l) {
            if (l->suffix == ast::FloatLiteralExpression::Suffix::kH) {
                // Emit f16 literal with explicit float16_t type declaration.
                out << "float16_t(";
                PrintF16(out, static_cast<float>(l->value));
                out << ")";
            }
            PrintF32(out, static_cast<float>(l->value));
            return true;
        },
        [&](const ast::IntLiteralExpression* i) {
            out << i->value;
            switch (i->suffix) {
                case ast::IntLiteralExpression::Suffix::kNone:
                case ast::IntLiteralExpression::Suffix::kI:
                    return true;
                case ast::IntLiteralExpression::Suffix::kU:
                    out << "u";
                    return true;
            }
            diagnostics_.add_error(diag::System::Writer, "unknown integer literal suffix type");
            return false;
        },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer, "unknown literal type");
            return false;
        });
}

bool GeneratorImpl::EmitValue(utils::StringStream& out, const type::Type* type, int value) {
    return Switch(
        type,
        [&](const type::Bool*) {
            out << (value == 0 ? "false" : "true");
            return true;
        },
        [&](const type::F32*) {
            out << value << ".0f";
            return true;
        },
        [&](const type::F16*) {
            out << "float16_t(" << value << ".0h)";
            return true;
        },
        [&](const type::I32*) {
            out << value;
            return true;
        },
        [&](const type::U32*) {
            out << value << "u";
            return true;
        },
        [&](const type::Vector* vec) {
            if (!EmitType(out, type, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite,
                          "")) {
                return false;
            }
            ScopedParen sp(out);
            for (uint32_t i = 0; i < vec->Width(); i++) {
                if (i != 0) {
                    out << ", ";
                }
                if (!EmitValue(out, vec->type(), value)) {
                    return false;
                }
            }
            return true;
        },
        [&](const type::Matrix* mat) {
            if (!EmitType(out, type, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite,
                          "")) {
                return false;
            }
            ScopedParen sp(out);
            for (uint32_t i = 0; i < (mat->rows() * mat->columns()); i++) {
                if (i != 0) {
                    out << ", ";
                }
                if (!EmitValue(out, mat->type(), value)) {
                    return false;
                }
            }
            return true;
        },
        [&](const type::Struct*) {
            out << "(";
            TINT_DEFER(out << ")" << value);
            return EmitType(out, type, builtin::AddressSpace::kUndefined,
                            builtin::Access::kUndefined, "");
        },
        [&](const type::Array*) {
            out << "(";
            TINT_DEFER(out << ")" << value);
            return EmitType(out, type, builtin::AddressSpace::kUndefined,
                            builtin::Access::kUndefined, "");
        },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer,
                                   "Invalid type for value emission: " + type->FriendlyName());
            return false;
        });
}

bool GeneratorImpl::EmitZeroValue(utils::StringStream& out, const type::Type* type) {
    return EmitValue(out, type, 0);
}

bool GeneratorImpl::EmitLoop(const ast::LoopStatement* stmt) {
    auto emit_continuing = [this, stmt]() {
        if (stmt->continuing && !stmt->continuing->Empty()) {
            if (!EmitBlock(stmt->continuing)) {
                return false;
            }
        }
        return true;
    };

    TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);
    line() << "while (true) {";
    {
        ScopedIndent si(this);
        if (!EmitStatements(stmt->body->statements)) {
            return false;
        }
        if (!emit_continuing_()) {
            return false;
        }
    }
    line() << "}";

    return true;
}

bool GeneratorImpl::EmitForLoop(const ast::ForLoopStatement* stmt) {
    // Nest a for loop with a new block. In HLSL the initializer scope is not
    // nested by the for-loop, so we may get variable redefinitions.
    line() << "{";
    increment_indent();
    TINT_DEFER({
        decrement_indent();
        line() << "}";
    });

    TextBuffer init_buf;
    if (auto* init = stmt->initializer) {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &init_buf);
        if (!EmitStatement(init)) {
            return false;
        }
    }

    TextBuffer cond_pre;
    utils::StringStream cond_buf;
    if (auto* cond = stmt->condition) {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &cond_pre);
        if (!EmitExpression(cond_buf, cond)) {
            return false;
        }
    }

    TextBuffer cont_buf;
    if (auto* cont = stmt->continuing) {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &cont_buf);
        if (!EmitStatement(cont)) {
            return false;
        }
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
        auto emit_continuing = [&]() {
            current_buffer_->Append(cont_buf);
            return true;
        };

        TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);
        line() << "while (true) {";
        increment_indent();
        TINT_DEFER({
            decrement_indent();
            line() << "}";
        });

        if (stmt->condition) {
            current_buffer_->Append(cond_pre);
            line() << "if (!(" << cond_buf.str() << ")) { break; }";
        }

        if (!EmitStatements(stmt->body->statements)) {
            return false;
        }

        if (!emit_continuing_()) {
            return false;
        }
    } else {
        // For-loop can be generated.
        {
            auto out = line();
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
                    out << utils::TrimSuffix(cont_buf.lines[0].content, ";");
                }
            }
            out << " {";
        }
        {
            auto emit_continuing = [] { return true; };
            TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);
            if (!EmitStatementsWithIndent(stmt->body->statements)) {
                return false;
            }
        }
        line() << "}";
    }

    return true;
}

bool GeneratorImpl::EmitWhile(const ast::WhileStatement* stmt) {
    TextBuffer cond_pre;
    utils::StringStream cond_buf;
    {
        auto* cond = stmt->condition;
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &cond_pre);
        if (!EmitExpression(cond_buf, cond)) {
            return false;
        }
    }

    auto emit_continuing = [&]() { return true; };
    TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);

    // If the while has a multi-statement conditional, then we cannot emit this
    // as a regular while in HLSL. Instead we need to generate a `while(true)` loop.
    bool emit_as_loop = cond_pre.lines.size() > 0;
    if (emit_as_loop) {
        line() << "while (true) {";
        increment_indent();
        TINT_DEFER({
            decrement_indent();
            line() << "}";
        });

        current_buffer_->Append(cond_pre);
        line() << "if (!(" << cond_buf.str() << ")) { break; }";
        if (!EmitStatements(stmt->body->statements)) {
            return false;
        }
    } else {
        // While can be generated.
        {
            auto out = line();
            out << "while";
            {
                ScopedParen sp(out);
                out << cond_buf.str();
            }
            out << " {";
        }
        if (!EmitStatementsWithIndent(stmt->body->statements)) {
            return false;
        }
        line() << "}";
    }

    return true;
}

bool GeneratorImpl::EmitMemberAccessor(utils::StringStream& out,
                                       const ast::MemberAccessorExpression* expr) {
    if (!EmitExpression(out, expr->object)) {
        return false;
    }
    out << ".";

    auto* sem = builder_.Sem().Get(expr)->UnwrapLoad();

    return Switch(
        sem,
        [&](const sem::Swizzle*) {
            // Swizzles output the name directly
            out << expr->member->symbol.Name();
            return true;
        },
        [&](const sem::StructMemberAccess* member_access) {
            out << member_access->Member()->Name().Name();
            return true;
        },
        [&](Default) {
            TINT_ICE(Writer, diagnostics_)
                << "unknown member access type: " << sem->TypeInfo().name;
            return false;
        });
}

bool GeneratorImpl::EmitReturn(const ast::ReturnStatement* stmt) {
    if (stmt->value) {
        auto out = line();
        out << "return ";
        if (!EmitExpression(out, stmt->value)) {
            return false;
        }
        out << ";";
    } else {
        line() << "return;";
    }
    return true;
}

bool GeneratorImpl::EmitStatement(const ast::Statement* stmt) {
    return Switch(
        stmt,
        [&](const ast::AssignmentStatement* a) {  //
            return EmitAssign(a);
        },
        [&](const ast::BlockStatement* b) {  //
            return EmitBlock(b);
        },
        [&](const ast::BreakStatement* b) {  //
            return EmitBreak(b);
        },
        [&](const ast::BreakIfStatement* b) {  //
            return EmitBreakIf(b);
        },
        [&](const ast::CallStatement* c) {  //
            auto out = line();
            if (!EmitCall(out, c->expr)) {
                return false;
            }
            out << ";";
            return true;
        },
        [&](const ast::ContinueStatement* c) {  //
            return EmitContinue(c);
        },
        [&](const ast::DiscardStatement* d) {  //
            return EmitDiscard(d);
        },
        [&](const ast::IfStatement* i) {  //
            return EmitIf(i);
        },
        [&](const ast::LoopStatement* l) {  //
            return EmitLoop(l);
        },
        [&](const ast::ForLoopStatement* l) {  //
            return EmitForLoop(l);
        },
        [&](const ast::WhileStatement* l) {  //
            return EmitWhile(l);
        },
        [&](const ast::ReturnStatement* r) {  //
            return EmitReturn(r);
        },
        [&](const ast::SwitchStatement* s) {  //
            return EmitSwitch(s);
        },
        [&](const ast::VariableDeclStatement* v) {  //
            return Switch(
                v->variable,  //
                [&](const ast::Var* var) { return EmitVar(var); },
                [&](const ast::Let* let) { return EmitLet(let); },
                [&](const ast::Const*) {
                    return true;  // Constants are embedded at their use
                },
                [&](Default) {  //
                    TINT_ICE(Writer, diagnostics_)
                        << "unknown variable type: " << v->variable->TypeInfo().name;
                    return false;
                });
        },
        [&](const ast::ConstAssert*) {
            return true;  // Not emitted
        },
        [&](Default) {  //
            diagnostics_.add_error(diag::System::Writer,
                                   "unknown statement type: " + std::string(stmt->TypeInfo().name));
            return false;
        });
}

bool GeneratorImpl::EmitDefaultOnlySwitch(const ast::SwitchStatement* stmt) {
    TINT_ASSERT(Writer, stmt->body.Length() == 1 && stmt->body[0]->ContainsDefault());

    // FXC fails to compile a switch with just a default case, ignoring the
    // default case body. We work around this here by emitting the default case
    // without the switch.

    // Emit the switch condition as-is if it has side-effects (e.g.
    // function call). Note that we can ignore the result of the expression (if any).
    if (auto* sem_cond = builder_.Sem().GetVal(stmt->condition); sem_cond->HasSideEffects()) {
        auto out = line();
        if (!EmitExpression(out, stmt->condition)) {
            return false;
        }
        out << ";";
    }

    // Emit "do { <default case body> } while(false);". We use a 'do' loop so
    // that break statements work as expected, and make it 'while (false)' in
    // case there isn't a break statement.
    line() << "do {";
    {
        ScopedIndent si(this);
        if (!EmitStatements(stmt->body[0]->body->statements)) {
            return false;
        }
    }
    line() << "} while (false);";
    return true;
}

bool GeneratorImpl::EmitSwitch(const ast::SwitchStatement* stmt) {
    // BUG(crbug.com/tint/1188): work around default-only switches
    if (stmt->body.Length() == 1 && stmt->body[0]->selectors.Length() == 1 &&
        stmt->body[0]->ContainsDefault()) {
        return EmitDefaultOnlySwitch(stmt);
    }

    {  // switch(expr) {
        auto out = line();
        out << "switch(";
        if (!EmitExpression(out, stmt->condition)) {
            return false;
        }
        out << ") {";
    }

    {
        ScopedIndent si(this);
        for (size_t i = 0; i < stmt->body.Length(); i++) {
            if (!EmitCase(stmt, i)) {
                return false;
            }
        }
    }

    line() << "}";

    return true;
}

bool GeneratorImpl::EmitType(utils::StringStream& out,
                             const type::Type* type,
                             builtin::AddressSpace address_space,
                             builtin::Access access,
                             const std::string& name,
                             bool* name_printed /* = nullptr */) {
    if (name_printed) {
        *name_printed = false;
    }
    switch (address_space) {
        case builtin::AddressSpace::kStorage:
            if (access != builtin::Access::kRead) {
                out << "RW";
            }
            out << "ByteAddressBuffer";
            return true;
        case builtin::AddressSpace::kUniform: {
            auto array_length = (type->Size() + 15) / 16;
            out << "uint4 " << name << "[" << array_length << "]";
            if (name_printed) {
                *name_printed = true;
            }
            return true;
        }
        default:
            break;
    }

    return Switch(
        type,
        [&](const type::Array* ary) {
            const type::Type* base_type = ary;
            std::vector<uint32_t> sizes;
            while (auto* arr = base_type->As<type::Array>()) {
                if (TINT_UNLIKELY(arr->Count()->Is<type::RuntimeArrayCount>())) {
                    TINT_ICE(Writer, diagnostics_)
                        << "runtime arrays may only exist in storage buffers, which should have "
                           "been transformed into a ByteAddressBuffer";
                    return false;
                }
                const auto count = arr->ConstantCount();
                if (!count) {
                    diagnostics_.add_error(diag::System::Writer,
                                           type::Array::kErrExpectedConstantCount);
                    return false;
                }

                sizes.push_back(count.value());
                base_type = arr->ElemType();
            }
            if (!EmitType(out, base_type, address_space, access, "")) {
                return false;
            }
            if (!name.empty()) {
                out << " " << name;
                if (name_printed) {
                    *name_printed = true;
                }
            }
            for (uint32_t size : sizes) {
                out << "[" << size << "]";
            }
            return true;
        },
        [&](const type::Bool*) {
            out << "bool";
            return true;
        },
        [&](const type::F32*) {
            out << "float";
            return true;
        },
        [&](const type::F16*) {
            out << "float16_t";
            return true;
        },
        [&](const type::I32*) {
            out << "int";
            return true;
        },
        [&](const type::Matrix* mat) {
            if (mat->type()->Is<type::F16>()) {
                // Use matrix<type, N, M> for f16 matrix
                out << "matrix<";
                if (!EmitType(out, mat->type(), address_space, access, "")) {
                    return false;
                }
                out << ", " << mat->columns() << ", " << mat->rows() << ">";
                return true;
            }
            if (!EmitType(out, mat->type(), address_space, access, "")) {
                return false;
            }
            // Note: HLSL's matrices are declared as <type>NxM, where N is the
            // number of rows and M is the number of columns. Despite HLSL's
            // matrices being column-major by default, the index operator and
            // initializers actually operate on row-vectors, where as WGSL operates
            // on column vectors. To simplify everything we use the transpose of the
            // matrices. See:
            // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-per-component-math#matrix-ordering
            out << mat->columns() << "x" << mat->rows();
            return true;
        },
        [&](const type::Pointer*) {
            TINT_ICE(Writer, diagnostics_) << "Attempting to emit pointer type. These should have "
                                              "been removed with the SimplifyPointers transform";
            return false;
        },
        [&](const type::Sampler* sampler) {
            out << "Sampler";
            if (sampler->IsComparison()) {
                out << "Comparison";
            }
            out << "State";
            return true;
        },
        [&](const type::Struct* str) {
            out << StructName(str);
            return true;
        },
        [&](const type::Texture* tex) {
            if (TINT_UNLIKELY(tex->Is<type::ExternalTexture>())) {
                TINT_ICE(Writer, diagnostics_)
                    << "Multiplanar external texture transform was not run.";
                return false;
            }

            auto* storage = tex->As<type::StorageTexture>();
            auto* ms = tex->As<type::MultisampledTexture>();
            auto* depth_ms = tex->As<type::DepthMultisampledTexture>();
            auto* sampled = tex->As<type::SampledTexture>();

            if (storage && storage->access() != builtin::Access::kRead) {
                out << "RW";
            }
            out << "Texture";

            switch (tex->dim()) {
                case type::TextureDimension::k1d:
                    out << "1D";
                    break;
                case type::TextureDimension::k2d:
                    out << ((ms || depth_ms) ? "2DMS" : "2D");
                    break;
                case type::TextureDimension::k2dArray:
                    out << ((ms || depth_ms) ? "2DMSArray" : "2DArray");
                    break;
                case type::TextureDimension::k3d:
                    out << "3D";
                    break;
                case type::TextureDimension::kCube:
                    out << "Cube";
                    break;
                case type::TextureDimension::kCubeArray:
                    out << "CubeArray";
                    break;
                default:
                    TINT_UNREACHABLE(Writer, diagnostics_)
                        << "unexpected TextureDimension " << tex->dim();
                    return false;
            }

            if (storage) {
                auto* component = image_format_to_rwtexture_type(storage->texel_format());
                if (TINT_UNLIKELY(!component)) {
                    TINT_ICE(Writer, diagnostics_) << "Unsupported StorageTexture TexelFormat: "
                                                   << static_cast<int>(storage->texel_format());
                    return false;
                }
                out << "<" << component << ">";
            } else if (depth_ms) {
                out << "<float4>";
            } else if (sampled || ms) {
                auto* subtype = sampled ? sampled->type() : ms->type();
                out << "<";
                if (subtype->Is<type::F32>()) {
                    out << "float4";
                } else if (subtype->Is<type::I32>()) {
                    out << "int4";
                } else if (TINT_LIKELY(subtype->Is<type::U32>())) {
                    out << "uint4";
                } else {
                    TINT_ICE(Writer, diagnostics_) << "Unsupported multisampled texture type";
                    return false;
                }
                out << ">";
            }
            return true;
        },
        [&](const type::U32*) {
            out << "uint";
            return true;
        },
        [&](const type::Vector* vec) {
            auto width = vec->Width();
            if (vec->type()->Is<type::F32>() && width >= 1 && width <= 4) {
                out << "float" << width;
            } else if (vec->type()->Is<type::I32>() && width >= 1 && width <= 4) {
                out << "int" << width;
            } else if (vec->type()->Is<type::U32>() && width >= 1 && width <= 4) {
                out << "uint" << width;
            } else if (vec->type()->Is<type::Bool>() && width >= 1 && width <= 4) {
                out << "bool" << width;
            } else {
                // For example, use "vector<float16_t, N>" for f16 vector.
                out << "vector<";
                if (!EmitType(out, vec->type(), address_space, access, "")) {
                    return false;
                }
                out << ", " << width << ">";
            }
            return true;
        },
        [&](const type::Atomic* atomic) {
            return EmitType(out, atomic->Type(), address_space, access, name);
        },
        [&](const type::Void*) {
            out << "void";
            return true;
        },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer, "unknown type in EmitType");
            return false;
        });
}

bool GeneratorImpl::EmitTypeAndName(utils::StringStream& out,
                                    const type::Type* type,
                                    builtin::AddressSpace address_space,
                                    builtin::Access access,
                                    const std::string& name) {
    bool name_printed = false;
    if (!EmitType(out, type, address_space, access, name, &name_printed)) {
        return false;
    }
    if (!name.empty() && !name_printed) {
        out << " " << name;
    }
    return true;
}

bool GeneratorImpl::EmitStructType(TextBuffer* b, const type::Struct* str) {
    auto it = emitted_structs_.emplace(str);
    if (!it.second) {
        return true;
    }

    line(b) << "struct " << StructName(str) << " {";
    {
        ScopedIndent si(b);
        for (auto* mem : str->Members()) {
            auto mem_name = mem->Name().Name();
            auto* ty = mem->Type();
            auto out = line(b);
            std::string pre, post;

            auto& attributes = mem->Attributes();

            if (auto location = attributes.location) {
                auto& pipeline_stage_uses = str->PipelineStageUses();
                if (TINT_UNLIKELY(pipeline_stage_uses.size() != 1)) {
                    TINT_ICE(Writer, diagnostics_) << "invalid entry point IO struct uses";
                }
                if (pipeline_stage_uses.count(type::PipelineStageUsage::kVertexInput)) {
                    post += " : TEXCOORD" + std::to_string(location.value());
                } else if (pipeline_stage_uses.count(type::PipelineStageUsage::kVertexOutput)) {
                    post += " : TEXCOORD" + std::to_string(location.value());
                } else if (pipeline_stage_uses.count(type::PipelineStageUsage::kFragmentInput)) {
                    post += " : TEXCOORD" + std::to_string(location.value());
                } else if (TINT_LIKELY(pipeline_stage_uses.count(
                               type::PipelineStageUsage::kFragmentOutput))) {
                    post += " : SV_Target" + std::to_string(location.value());
                } else {
                    TINT_ICE(Writer, diagnostics_) << "invalid use of location attribute";
                }
            }
            if (auto builtin = attributes.builtin) {
                auto name = builtin_to_attribute(builtin.value());
                if (name.empty()) {
                    diagnostics_.add_error(diag::System::Writer, "unsupported builtin");
                    return false;
                }
                post += " : " + name;
            }
            if (auto interpolation = attributes.interpolation) {
                auto mod = interpolation_to_modifiers(interpolation->type, interpolation->sampling);
                if (mod.empty()) {
                    diagnostics_.add_error(diag::System::Writer, "unsupported interpolation");
                    return false;
                }
                pre += mod;
            }
            if (attributes.invariant) {
                // Note: `precise` is not exactly the same as `invariant`, but is
                // stricter and therefore provides the necessary guarantees.
                // See discussion here: https://github.com/gpuweb/gpuweb/issues/893
                pre += "precise ";
            }

            out << pre;
            if (!EmitTypeAndName(out, ty, builtin::AddressSpace::kUndefined,
                                 builtin::Access::kReadWrite, mem_name)) {
                return false;
            }
            out << post << ";";
        }
    }

    line(b) << "};";
    return true;
}

bool GeneratorImpl::EmitUnaryOp(utils::StringStream& out, const ast::UnaryOpExpression* expr) {
    switch (expr->op) {
        case ast::UnaryOp::kIndirection:
        case ast::UnaryOp::kAddressOf:
            return EmitExpression(out, expr->expr);
        case ast::UnaryOp::kComplement:
            out << "~";
            break;
        case ast::UnaryOp::kNot:
            out << "!";
            break;
        case ast::UnaryOp::kNegation:
            out << "-";
            break;
    }
    out << "(";

    if (!EmitExpression(out, expr->expr)) {
        return false;
    }

    out << ")";

    return true;
}

bool GeneratorImpl::EmitVar(const ast::Var* var) {
    auto* sem = builder_.Sem().Get(var);
    auto* type = sem->Type()->UnwrapRef();

    auto out = line();
    if (!EmitTypeAndName(out, type, sem->AddressSpace(), sem->Access(), var->name->symbol.Name())) {
        return false;
    }

    out << " = ";

    if (var->initializer) {
        if (!EmitExpression(out, var->initializer)) {
            return false;
        }
    } else {
        if (!EmitZeroValue(out, type)) {
            return false;
        }
    }
    out << ";";

    return true;
}

bool GeneratorImpl::EmitLet(const ast::Let* let) {
    auto* sem = builder_.Sem().Get(let);
    auto* type = sem->Type()->UnwrapRef();

    auto out = line();
    out << "const ";
    if (!EmitTypeAndName(out, type, builtin::AddressSpace::kUndefined, builtin::Access::kUndefined,
                         let->name->symbol.Name())) {
        return false;
    }
    out << " = ";
    if (!EmitExpression(out, let->initializer)) {
        return false;
    }
    out << ";";

    return true;
}

template <typename F>
bool GeneratorImpl::CallBuiltinHelper(utils::StringStream& out,
                                      const ast::CallExpression* call,
                                      const sem::Builtin* builtin,
                                      F&& build) {
    // Generate the helper function if it hasn't been created already
    auto fn = utils::GetOrCreate(builtins_, builtin, [&]() -> std::string {
        TextBuffer b;
        TINT_DEFER(helpers_.Append(b));

        auto fn_name = UniqueIdentifier(std::string("tint_") + builtin::str(builtin->Type()));
        std::vector<std::string> parameter_names;
        {
            auto decl = line(&b);
            if (!EmitTypeAndName(decl, builtin->ReturnType(), builtin::AddressSpace::kUndefined,
                                 builtin::Access::kUndefined, fn_name)) {
                return "";
            }
            {
                ScopedParen sp(decl);
                for (auto* param : builtin->Parameters()) {
                    if (!parameter_names.empty()) {
                        decl << ", ";
                    }
                    auto param_name = "param_" + std::to_string(parameter_names.size());
                    const auto* ty = param->Type();
                    if (auto* ptr = ty->As<type::Pointer>()) {
                        decl << "inout ";
                        ty = ptr->StoreType();
                    }
                    if (!EmitTypeAndName(decl, ty, builtin::AddressSpace::kUndefined,
                                         builtin::Access::kUndefined, param_name)) {
                        return "";
                    }
                    parameter_names.emplace_back(std::move(param_name));
                }
            }
            decl << " {";
        }
        {
            ScopedIndent si(&b);
            if (!build(&b, parameter_names)) {
                return "";
            }
        }
        line(&b) << "}";
        line(&b);
        return fn_name;
    });

    if (fn.empty()) {
        return false;
    }

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
            if (!EmitExpression(out, arg)) {
                return false;
            }
        }
    }
    return true;
}

}  // namespace tint::writer::hlsl
