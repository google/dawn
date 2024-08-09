// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/msl/writer/ast_printer/ast_printer.h"

#include <cmath>
#include <iomanip>
#include <limits>
#include <utility>
#include <vector>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/constant/value.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/lang/msl/writer/ast_raise/module_scope_var_to_entry_point_param.h"
#include "src/tint/lang/msl/writer/ast_raise/packed_vec3.h"
#include "src/tint/lang/msl/writer/ast_raise/pixel_local.h"
#include "src/tint/lang/msl/writer/ast_raise/subgroup_ballot.h"
#include "src/tint/lang/msl/writer/common/option_helpers.h"
#include "src/tint/lang/msl/writer/common/printer_support.h"
#include "src/tint/lang/wgsl/ast/alias.h"
#include "src/tint/lang/wgsl/ast/bool_literal_expression.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/float_literal_expression.h"
#include "src/tint/lang/wgsl/ast/interpolate_attribute.h"
#include "src/tint/lang/wgsl/ast/transform/array_length_from_uniform.h"
#include "src/tint/lang/wgsl/ast/transform/binding_remapper.h"
#include "src/tint/lang/wgsl/ast/transform/builtin_polyfill.h"
#include "src/tint/lang/wgsl/ast/transform/canonicalize_entry_point_io.h"
#include "src/tint/lang/wgsl/ast/transform/demote_to_helper.h"
#include "src/tint/lang/wgsl/ast/transform/disable_uniformity_analysis.h"
#include "src/tint/lang/wgsl/ast/transform/expand_compound_assignment.h"
#include "src/tint/lang/wgsl/ast/transform/fold_constants.h"
#include "src/tint/lang/wgsl/ast/transform/manager.h"
#include "src/tint/lang/wgsl/ast/transform/multiplanar_external_texture.h"
#include "src/tint/lang/wgsl/ast/transform/preserve_padding.h"
#include "src/tint/lang/wgsl/ast/transform/promote_initializers_to_let.h"
#include "src/tint/lang/wgsl/ast/transform/promote_side_effects_to_decl.h"
#include "src/tint/lang/wgsl/ast/transform/remove_continue_in_switch.h"
#include "src/tint/lang/wgsl/ast/transform/remove_phonies.h"
#include "src/tint/lang/wgsl/ast/transform/robustness.h"
#include "src/tint/lang/wgsl/ast/transform/simplify_pointers.h"
#include "src/tint/lang/wgsl/ast/transform/unshadow.h"
#include "src/tint/lang/wgsl/ast/transform/vectorize_scalar_matrix_initializers.h"
#include "src/tint/lang/wgsl/ast/transform/zero_init_workgroup_memory.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/helpers/check_supported_extensions.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/module.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/lang/wgsl/sem/switch_statement.h"
#include "src/tint/lang/wgsl/sem/value_constructor.h"
#include "src/tint/lang/wgsl/sem/value_conversion.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string_stream.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::msl::writer {
namespace {

bool last_is_break(const ast::BlockStatement* stmts) {
    return tint::IsAnyOf<ast::BreakStatement>(stmts->Last());
}

class ScopedBitCast {
  public:
    ScopedBitCast(ASTPrinter* generator,
                  StringStream& stream,
                  const core::type::Type* curr_type,
                  const core::type::Type* target_type)
        : s(stream) {
        auto* target_vec_type = target_type->As<core::type::Vector>();

        // If we need to promote from scalar to vector, bitcast the scalar to the
        // vector element type.
        if (curr_type->Is<core::type::Scalar>() && target_vec_type) {
            target_type = target_vec_type->type();
        }

        // Bit cast
        s << "as_type<";
        generator->EmitType(s, target_type);
        s << ">(";
    }

    ~ScopedBitCast() { s << ")"; }

  private:
    StringStream& s;
};

}  // namespace

SanitizedResult::SanitizedResult() = default;
SanitizedResult::~SanitizedResult() = default;
SanitizedResult::SanitizedResult(SanitizedResult&&) = default;

SanitizedResult Sanitize(const Program& in, const Options& options) {
    ast::transform::Manager manager;
    ast::transform::DataMap data;

    manager.Add<ast::transform::FoldConstants>();

    manager.Add<ast::transform::DisableUniformityAnalysis>();

    // ExpandCompoundAssignment must come before BuiltinPolyfill
    manager.Add<ast::transform::ExpandCompoundAssignment>();

    manager.Add<ast::transform::PreservePadding>();

    manager.Add<ast::transform::Unshadow>();

    manager.Add<ast::transform::PromoteSideEffectsToDecl>();

    if (!options.disable_robustness) {
        // Robustness must come after PromoteSideEffectsToDecl
        // Robustness must come before BuiltinPolyfill and CanonicalizeEntryPointIO
        // Robustness must come before ArrayLengthFromUniform
        manager.Add<ast::transform::Robustness>();
    }

    tint::transform::multiplanar::BindingsMap multiplanar_map{};
    RemapperData remapper_data{};
    ArrayLengthFromUniformOptions array_length_from_uniform_options{};
    PopulateBindingRelatedOptions(options, remapper_data, multiplanar_map,
                                  array_length_from_uniform_options);

    manager.Add<ast::transform::BindingRemapper>();
    data.Add<ast::transform::BindingRemapper::Remappings>(
        remapper_data, std::unordered_map<BindingPoint, core::Access>{},
        /* allow_collisions */ true);

    // Note: it is more efficient for MultiplanarExternalTexture to come after Robustness
    // MultiplanarExternalTexture must come after BindingRemapper
    data.Add<ast::transform::MultiplanarExternalTexture::NewBindingPoints>(
        multiplanar_map, /* allow_collisions */ true);
    manager.Add<ast::transform::MultiplanarExternalTexture>();

    {  // Builtin polyfills
        ast::transform::BuiltinPolyfill::Builtins polyfills;
        polyfills.acosh = ast::transform::BuiltinPolyfill::Level::kRangeCheck;
        polyfills.atanh = ast::transform::BuiltinPolyfill::Level::kRangeCheck;
        polyfills.bitshift_modulo = true;  // crbug.com/tint/1543
        polyfills.clamp_int = true;
        polyfills.conv_f32_to_iu32 = true;
        polyfills.extract_bits = ast::transform::BuiltinPolyfill::Level::kClampParameters;
        polyfills.first_leading_bit = true;
        polyfills.first_trailing_bit = true;
        polyfills.fwidth_fine = true;
        polyfills.insert_bits = ast::transform::BuiltinPolyfill::Level::kClampParameters;
        polyfills.int_div_mod = !options.disable_polyfill_integer_div_mod;
        polyfills.sign_int = true;
        polyfills.texture_sample_base_clamp_to_edge_2d_f32 = true;
        polyfills.workgroup_uniform_load = true;
        polyfills.pack_unpack_4x8 = true;
        polyfills.pack_4xu8_clamp = true;
        data.Add<ast::transform::BuiltinPolyfill::Config>(polyfills);
        manager.Add<ast::transform::BuiltinPolyfill>();
    }

    if (!options.disable_workgroup_init) {
        // ZeroInitWorkgroupMemory must come before CanonicalizeEntryPointIO as
        // ZeroInitWorkgroupMemory may inject new builtin parameters.
        manager.Add<ast::transform::ZeroInitWorkgroupMemory>();
    }

    {
        PixelLocal::Config cfg;
        for (auto it : options.pixel_local_attachments) {
            cfg.attachments.Add(it.first, it.second);
        }
        data.Add<PixelLocal::Config>(cfg);
        manager.Add<PixelLocal>();
    }

    // Build the configs for the internal CanonicalizeEntryPointIO transform.
    auto entry_point_io_cfg = ast::transform::CanonicalizeEntryPointIO::Config(
        ast::transform::CanonicalizeEntryPointIO::ShaderStyle::kMsl, options.fixed_sample_mask,
        options.emit_vertex_point_size);
    // CanonicalizeEntryPointIO must come after Robustness
    manager.Add<ast::transform::CanonicalizeEntryPointIO>();
    data.Add<ast::transform::CanonicalizeEntryPointIO::Config>(std::move(entry_point_io_cfg));

    manager.Add<ast::transform::PromoteInitializersToLet>();
    manager.Add<ast::transform::RemoveContinueInSwitch>();

    // DemoteToHelper must come after PromoteSideEffectsToDecl and ExpandCompoundAssignment.
    // TODO(crbug.com/tint/1752): This is only necessary for Metal versions older than 2.3.
    manager.Add<ast::transform::DemoteToHelper>();

    manager.Add<ast::transform::VectorizeScalarMatrixInitializers>();
    manager.Add<ast::transform::RemovePhonies>();
    manager.Add<ast::transform::SimplifyPointers>();

    // SubgroupBallot() must come after CanonicalizeEntryPointIO.
    manager.Add<SubgroupBallot>();

    // ArrayLengthFromUniform must come after SimplifyPointers, as
    // it assumes that the form of the array length argument is &var.array.
    manager.Add<ast::transform::ArrayLengthFromUniform>();
    // Build the config for the internal ArrayLengthFromUniform transform.
    ast::transform::ArrayLengthFromUniform::Config array_length_from_uniform_cfg(
        BindingPoint{0, array_length_from_uniform_options.ubo_binding});
    array_length_from_uniform_cfg.bindpoint_to_size_index =
        std::move(array_length_from_uniform_options.bindpoint_to_size_index);
    data.Add<ast::transform::ArrayLengthFromUniform::Config>(
        std::move(array_length_from_uniform_cfg));

    // PackedVec3 must come after ExpandCompoundAssignment.
    manager.Add<PackedVec3>();
    manager.Add<ModuleScopeVarToEntryPointParam>();

    SanitizedResult result;
    ast::transform::DataMap outputs;
    result.program = manager.Run(in, data, outputs);
    if (!result.program.IsValid()) {
        return result;
    }
    if (auto* res = outputs.Get<ast::transform::ArrayLengthFromUniform::Result>()) {
        result.used_array_length_from_uniform_indices = std::move(res->used_size_indices);
    }
    result.needs_storage_buffer_sizes = !result.used_array_length_from_uniform_indices.empty();
    return result;
}

ASTPrinter::ASTPrinter(const Program& program) : builder_(ProgramBuilder::Wrap(program)) {}

ASTPrinter::~ASTPrinter() = default;

bool ASTPrinter::Generate() {
    if (!tint::wgsl::CheckSupportedExtensions(
            "MSL", builder_.AST(), diagnostics_,
            Vector{
                wgsl::Extension::kChromiumDisableUniformityAnalysis,
                wgsl::Extension::kChromiumExperimentalFramebufferFetch,
                wgsl::Extension::kChromiumExperimentalPixelLocal,
                wgsl::Extension::kChromiumExperimentalSubgroups,
                wgsl::Extension::kChromiumInternalGraphite,
                wgsl::Extension::kChromiumInternalRelaxedUniformLayout,
                wgsl::Extension::kF16,
                wgsl::Extension::kDualSourceBlending,
                wgsl::Extension::kSubgroups,
                wgsl::Extension::kSubgroupsF16,
            })) {
        return false;
    }

    Line() << "#include <metal_stdlib>";
    Line();
    Line() << "using namespace metal;";

    auto helpers_insertion_point = current_buffer_->lines.size();

    auto* mod = builder_.Sem().Module();
    for (auto* decl : mod->DependencyOrderedDeclarations()) {
        bool ok = Switch(
            decl,  //
            [&](const ast::Struct* str) {
                TINT_DEFER(Line());
                return EmitTypeDecl(TypeOf(str));
            },
            [&](const ast::Alias*) {
                return true;  // folded away by the writer
            },
            [&](const ast::Const*) {
                return true;  // Constants are embedded at their use
            },
            [&](const ast::Override*) {
                // Override is removed with SubstituteOverride
                diagnostics_.AddError(Source{})
                    << "override-expressions should have been removed with the "
                       "SubstituteOverride transform.";
                return false;
            },
            [&](const ast::Function* func) {
                TINT_DEFER(Line());
                if (func->IsEntryPoint()) {
                    return EmitEntryPointFunction(func);
                }
                return EmitFunction(func);
            },
            [&](const ast::DiagnosticDirective*) {
                // Do nothing for diagnostic directives in MSL
                return true;
            },
            [&](const ast::Enable*) {
                // Do nothing for enabling extension in MSL
                return true;
            },
            [&](const ast::Requires*) {
                // Do nothing for requiring language features in MSL.
                return true;
            },
            [&](const ast::ConstAssert*) {
                return true;  // Not emitted
            },                //
            TINT_ICE_ON_NO_MATCH);
        if (!ok) {
            return false;
        }
    }

    if (!invariant_define_name_.empty()) {
        // 'invariant' attribute requires MSL 2.1 or higher.
        // WGSL can ignore the invariant attribute on pre MSL 2.1 devices.
        // See: https://github.com/gpuweb/gpuweb/issues/893#issuecomment-745537465
        Line(&helpers_) << "#if __METAL_VERSION__ >= 210";
        Line(&helpers_) << "#define " << invariant_define_name_ << " [[invariant]]";
        Line(&helpers_) << "#else";
        Line(&helpers_) << "#define " << invariant_define_name_;
        Line(&helpers_) << "#endif";
        Line(&helpers_);
    }

    if (!helpers_.lines.empty()) {
        current_buffer_->Insert("", helpers_insertion_point++, 0);
        current_buffer_->Insert(helpers_, helpers_insertion_point++, 0);
    }

    return true;
}

bool ASTPrinter::EmitTypeDecl(const core::type::Type* ty) {
    if (auto* str = ty->As<core::type::Struct>()) {
        if (!EmitStructType(current_buffer_, str)) {
            return false;
        }
    } else {
        diagnostics_.AddError(Source{}) << "unknown alias type: " << ty->FriendlyName();
        return false;
    }

    return true;
}

bool ASTPrinter::EmitIndexAccessor(StringStream& out, const ast::IndexAccessorExpression* expr) {
    bool paren_lhs =
        !expr->object
             ->IsAnyOf<ast::AccessorExpression, ast::CallExpression, ast::IdentifierExpression>();

    if (paren_lhs) {
        out << "(";
    }
    if (!EmitExpression(out, expr->object)) {
        return false;
    }
    if (paren_lhs) {
        out << ")";
    }

    out << "[";

    if (!EmitExpression(out, expr->index)) {
        return false;
    }
    out << "]";

    return true;
}

bool ASTPrinter::EmitBitcastCall(StringStream& out, const ast::CallExpression* call) {
    auto* arg = call->args[0];
    auto* dst_type = TypeOf(call);

    out << "as_type<";
    if (!EmitType(out, dst_type)) {
        return false;
    }

    out << ">(";
    if (!EmitExpression(out, arg)) {
        return false;
    }

    out << ")";
    return true;
}

bool ASTPrinter::EmitAssign(const ast::AssignmentStatement* stmt) {
    auto out = Line();

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

bool ASTPrinter::EmitBinary(StringStream& out, const ast::BinaryExpression* expr) {
    auto emit_op = [&] {
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
                out << "&&";
                break;
            case core::BinaryOp::kLogicalOr:
                out << "||";
                break;
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
                // TODO(dsinclair): MSL is based on C++14, and >> in C++14 has
                // implementation-defined behaviour for negative LHS.  We may have to
                // generate extra code to implement WGSL-specified behaviour for
                // negative LHS.
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
        return true;
    };

    auto signed_type_of = [&](const core::type::Type* ty) -> const core::type::Type* {
        if (ty->is_integer_scalar()) {
            return builder_.create<core::type::I32>();
        } else if (auto* v = ty->As<core::type::Vector>()) {
            return builder_.create<core::type::Vector>(builder_.create<core::type::I32>(),
                                                       v->Width());
        }
        return {};
    };

    auto unsigned_type_of = [&](const core::type::Type* ty) -> const core::type::Type* {
        if (ty->is_integer_scalar()) {
            return builder_.create<core::type::U32>();
        } else if (auto* v = ty->As<core::type::Vector>()) {
            return builder_.create<core::type::Vector>(builder_.create<core::type::U32>(),
                                                       v->Width());
        }
        return {};
    };

    auto* lhs_type = TypeOf(expr->lhs)->UnwrapRef();
    auto* rhs_type = TypeOf(expr->rhs)->UnwrapRef();

    // Handle fmod
    if (expr->op == core::BinaryOp::kModulo && lhs_type->is_float_scalar_or_vector()) {
        out << "fmod";
        ScopedParen sp(out);
        if (!EmitExpression(out, expr->lhs)) {
            return false;
        }
        out << ", ";
        if (!EmitExpression(out, expr->rhs)) {
            return false;
        }
        return true;
    }

    // Handle +/-/* of signed values
    if ((expr->IsAdd() || expr->IsSubtract() || expr->IsMultiply()) &&
        lhs_type->is_signed_integer_scalar_or_vector() &&
        rhs_type->is_signed_integer_scalar_or_vector()) {
        // If lhs or rhs is a vector, use that type (support implicit scalar to
        // vector promotion)
        auto* target_type = lhs_type->Is<core::type::Vector>()
                                ? lhs_type
                                : (rhs_type->Is<core::type::Vector>() ? rhs_type : lhs_type);

        // WGSL defines behaviour for signed overflow, MSL does not. For these
        // cases, bitcast operands to unsigned, then cast result to signed.
        ScopedBitCast outer_int_cast(this, out, target_type, signed_type_of(target_type));
        ScopedParen sp(out);
        {
            ScopedBitCast lhs_uint_cast(this, out, lhs_type, unsigned_type_of(target_type));
            if (!EmitExpression(out, expr->lhs)) {
                return false;
            }
        }
        if (!emit_op()) {
            return false;
        }
        {
            ScopedBitCast rhs_uint_cast(this, out, rhs_type, unsigned_type_of(target_type));
            if (!EmitExpression(out, expr->rhs)) {
                return false;
            }
        }
        return true;
    }

    // Handle left bit shifting a signed value
    // TODO(crbug.com/tint/1077): This may not be necessary. The MSL spec
    // seems to imply that left shifting a signed value is treated the same as
    // left shifting an unsigned value, but we need to make sure.
    if (expr->IsShiftLeft() && lhs_type->is_signed_integer_scalar_or_vector()) {
        // Shift left: discards top bits, so convert first operand to unsigned
        // first, then convert result back to signed
        ScopedBitCast outer_int_cast(this, out, lhs_type, signed_type_of(lhs_type));
        ScopedParen sp(out);
        {
            ScopedBitCast lhs_uint_cast(this, out, lhs_type, unsigned_type_of(lhs_type));
            if (!EmitExpression(out, expr->lhs)) {
                return false;
            }
        }
        if (!emit_op()) {
            return false;
        }
        if (!EmitExpression(out, expr->rhs)) {
            return false;
        }
        return true;
    }

    // Handle '&' and '|' of booleans.
    if ((expr->IsAnd() || expr->IsOr()) && lhs_type->Is<core::type::Bool>()) {
        out << "bool";
        ScopedParen sp(out);
        if (!EmitExpression(out, expr->lhs)) {
            return false;
        }
        if (!emit_op()) {
            return false;
        }
        if (!EmitExpression(out, expr->rhs)) {
            return false;
        }
        return true;
    }

    // Emit as usual
    ScopedParen sp(out);
    if (!EmitExpression(out, expr->lhs)) {
        return false;
    }
    if (!emit_op()) {
        return false;
    }
    if (!EmitExpression(out, expr->rhs)) {
        return false;
    }

    return true;
}

bool ASTPrinter::EmitBreak(const ast::BreakStatement*) {
    Line() << "break;";
    return true;
}

bool ASTPrinter::EmitBreakIf(const ast::BreakIfStatement* b) {
    auto out = Line();
    out << "if (";
    if (!EmitExpression(out, b->condition)) {
        return false;
    }
    out << ") { break; }";
    return true;
}

bool ASTPrinter::EmitCall(StringStream& out, const ast::CallExpression* expr) {
    auto* call = builder_.Sem().Get<sem::Call>(expr);
    auto* target = call->Target();
    return Switch(
        target,  //
        [&](const sem::Function* func) { return EmitFunctionCall(out, call, func); },
        [&](const sem::BuiltinFn* builtin) { return EmitBuiltinCall(out, call, builtin); },
        [&](const sem::ValueConversion* conv) { return EmitTypeConversion(out, call, conv); },
        [&](const sem::ValueConstructor* ctor) { return EmitTypeInitializer(out, call, ctor); },  //
        TINT_ICE_ON_NO_MATCH);
}

bool ASTPrinter::EmitFunctionCall(StringStream& out,
                                  const sem::Call* call,
                                  const sem::Function* fn) {
    if (ast::GetAttribute<SubgroupBallot::SimdBallot>(fn->Declaration()->attributes) != nullptr) {
        out << "as_type<uint2>((ulong)simd_ballot(";
        if (!EmitExpression(out, call->Arguments()[0]->Declaration())) {
            return false;
        }
        out << "))";
        return true;
    }

    out << fn->Declaration()->name->symbol.Name() << "(";

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

bool ASTPrinter::EmitBuiltinCall(StringStream& out,
                                 const sem::Call* call,
                                 const sem::BuiltinFn* builtin) {
    auto* expr = call->Declaration();
    if (builtin->IsAtomic()) {
        return EmitAtomicCall(out, expr, builtin);
    }
    if (builtin->IsTexture()) {
        return EmitTextureCall(out, call, builtin);
    }

    auto name = generate_builtin_name(builtin);

    switch (builtin->Fn()) {
        case wgsl::BuiltinFn::kBitcast:
            return EmitBitcastCall(out, expr);
        case wgsl::BuiltinFn::kDot:
            return EmitDotCall(out, expr, builtin);
        case wgsl::BuiltinFn::kModf:
            return EmitModfCall(out, expr, builtin);
        case wgsl::BuiltinFn::kFrexp:
            return EmitFrexpCall(out, expr, builtin);
        case wgsl::BuiltinFn::kDegrees:
            return EmitDegreesCall(out, expr, builtin);
        case wgsl::BuiltinFn::kRadians:
            return EmitRadiansCall(out, expr, builtin);
        case wgsl::BuiltinFn::kDot4I8Packed:
            return EmitDot4I8PackedCall(out, expr, builtin);
        case wgsl::BuiltinFn::kDot4U8Packed:
            return EmitDot4U8PackedCall(out, expr, builtin);

        case wgsl::BuiltinFn::kPack2X16Float:
        case wgsl::BuiltinFn::kUnpack2X16Float: {
            if (builtin->Fn() == wgsl::BuiltinFn::kPack2X16Float) {
                out << "as_type<uint>(half2(";
            } else {
                out << "float2(as_type<half2>(";
            }
            if (!EmitExpression(out, expr->args[0])) {
                return false;
            }
            out << "))";
            return true;
        }
        case wgsl::BuiltinFn::kQuantizeToF16: {
            std::string width = "";
            if (auto* vec = builtin->ReturnType()->As<core::type::Vector>()) {
                width = std::to_string(vec->Width());
            }
            out << "float" << width << "(half" << width << "(";
            if (!EmitExpression(out, expr->args[0])) {
                return false;
            }
            out << "))";
            return true;
        }
        // TODO(crbug.com/tint/661): Combine sequential barriers to a single
        // instruction.
        case wgsl::BuiltinFn::kStorageBarrier: {
            out << "threadgroup_barrier(mem_flags::mem_device)";
            return true;
        }
        case wgsl::BuiltinFn::kWorkgroupBarrier: {
            out << "threadgroup_barrier(mem_flags::mem_threadgroup)";
            return true;
        }
        case wgsl::BuiltinFn::kTextureBarrier: {
            out << "threadgroup_barrier(mem_flags::mem_texture)";
            return true;
        }

        case wgsl::BuiltinFn::kLength: {
            auto* sem = builder_.Sem().GetVal(expr->args[0]);
            if (sem->Type()->UnwrapRef()->Is<core::type::Scalar>()) {
                // Emulate scalar overload using fabs(x).
                name = "fabs";
            }
            break;
        }

        case wgsl::BuiltinFn::kDistance: {
            auto* sem = builder_.Sem().GetVal(expr->args[0]);
            if (sem->Type()->UnwrapRef()->Is<core::type::Scalar>()) {
                // Emulate scalar overload using fabs(x - y);
                out << "fabs";
                ScopedParen sp(out);
                if (!EmitExpression(out, expr->args[0])) {
                    return false;
                }
                out << " - ";
                if (!EmitExpression(out, expr->args[1])) {
                    return false;
                }
                return true;
            }
            break;
        }

        case wgsl::BuiltinFn::kSubgroupBroadcast: {
            // The lane argument is ushort.
            out << "simd_broadcast(";
            if (!EmitExpression(out, expr->args[0])) {
                return false;
            }
            out << ",ushort(";
            if (!EmitExpression(out, expr->args[1])) {
                return false;
            }
            out << "))";
            return true;
        }

        case wgsl::BuiltinFn::kSubgroupAdd: {
            out << "simd_sum(";
            if (!EmitExpression(out, expr->args[0])) {
                return false;
            }
            out << ")";
            return true;
        }

        case wgsl::BuiltinFn::kSubgroupExclusiveAdd: {
            out << "simd_prefix_exclusive_sum(";
            if (!EmitExpression(out, expr->args[0])) {
                return false;
            }
            out << ")";
            return true;
        }

        case wgsl::BuiltinFn::kSubgroupMul: {
            out << "simd_product(";
            if (!EmitExpression(out, expr->args[0])) {
                return false;
            }
            out << ")";
            return true;
        }

        case wgsl::BuiltinFn::kSubgroupExclusiveMul: {
            out << "simd_prefix_exclusive_product(";
            if (!EmitExpression(out, expr->args[0])) {
                return false;
            }
            out << ")";
            return true;
        }

        case wgsl::BuiltinFn::kSubgroupAnd: {
            out << "simd_and(";
            if (!EmitExpression(out, expr->args[0])) {
                return false;
            }
            out << ")";
            return true;
        }

        case wgsl::BuiltinFn::kSubgroupOr: {
            out << "simd_or(";
            if (!EmitExpression(out, expr->args[0])) {
                return false;
            }
            out << ")";
            return true;
        }

        case wgsl::BuiltinFn::kSubgroupXor: {
            out << "simd_xor(";
            if (!EmitExpression(out, expr->args[0])) {
                return false;
            }
            out << ")";
            return true;
        }

        case wgsl::BuiltinFn::kSubgroupMin: {
            out << "simd_min(";
            if (!EmitExpression(out, expr->args[0])) {
                return false;
            }
            out << ")";
            return true;
        }

        case wgsl::BuiltinFn::kSubgroupMax: {
            out << "simd_max(";
            if (!EmitExpression(out, expr->args[0])) {
                return false;
            }
            out << ")";
            return true;
        }

        case wgsl::BuiltinFn::kSubgroupAll: {
            out << "simd_all(";
            if (!EmitExpression(out, expr->args[0])) {
                return false;
            }
            out << ")";
            return true;
        }

        case wgsl::BuiltinFn::kSubgroupAny: {
            out << "simd_any(";
            if (!EmitExpression(out, expr->args[0])) {
                return false;
            }
            out << ")";
            return true;
        }

        default:
            break;
    }

    if (name.empty()) {
        return false;
    }

    out << name << "(";

    bool first = true;
    for (auto* arg : expr->args) {
        if (!first) {
            out << ", ";
        }
        first = false;

        if (!EmitExpression(out, arg)) {
            return false;
        }
    }

    out << ")";
    return true;
}

bool ASTPrinter::EmitTypeConversion(StringStream& out,
                                    const sem::Call* call,
                                    const sem::ValueConversion* conv) {
    if (!EmitType(out, conv->Target())) {
        return false;
    }
    out << "(";

    if (!EmitExpression(out, call->Arguments()[0]->Declaration())) {
        return false;
    }

    out << ")";
    return true;
}

bool ASTPrinter::EmitTypeInitializer(StringStream& out,
                                     const sem::Call* call,
                                     const sem::ValueConstructor* ctor) {
    auto* type = ctor->ReturnType();

    const char* terminator = ")";
    TINT_DEFER(out << terminator);

    bool ok = Switch(
        type,
        [&](const core::type::Array*) {
            if (!EmitType(out, type)) {
                return false;
            }
            out << "{";
            terminator = "}";
            return true;
        },
        [&](const core::type::Struct*) {
            out << "{";
            terminator = "}";
            return true;
        },
        [&](Default) {
            if (!EmitType(out, type)) {
                return false;
            }
            out << "(";
            return true;
        });
    if (!ok) {
        return false;
    }

    size_t i = 0;
    for (auto* arg : call->Arguments()) {
        if (i > 0) {
            out << ", ";
        }

        if (auto* struct_ty = type->As<core::type::Struct>()) {
            // Emit field designators for structures to account for padding members.
            auto name = struct_ty->Members()[i]->Name().Name();
            out << "." << name << "=";
        }

        if (!EmitExpression(out, arg->Declaration())) {
            return false;
        }

        i++;
    }

    return true;
}

bool ASTPrinter::EmitAtomicCall(StringStream& out,
                                const ast::CallExpression* expr,
                                const sem::BuiltinFn* builtin) {
    auto call = [&](const std::string& name, bool append_memory_order_relaxed) {
        out << name;
        {
            ScopedParen sp(out);
            for (size_t i = 0; i < expr->args.Length(); i++) {
                auto* arg = expr->args[i];
                if (i > 0) {
                    out << ", ";
                }
                if (!EmitExpression(out, arg)) {
                    return false;
                }
            }
            if (append_memory_order_relaxed) {
                out << ", memory_order_relaxed";
            }
        }
        return true;
    };

    switch (builtin->Fn()) {
        case wgsl::BuiltinFn::kAtomicLoad:
            return call("atomic_load_explicit", true);

        case wgsl::BuiltinFn::kAtomicStore:
            return call("atomic_store_explicit", true);

        case wgsl::BuiltinFn::kAtomicAdd:
            return call("atomic_fetch_add_explicit", true);

        case wgsl::BuiltinFn::kAtomicSub:
            return call("atomic_fetch_sub_explicit", true);

        case wgsl::BuiltinFn::kAtomicMax:
            return call("atomic_fetch_max_explicit", true);

        case wgsl::BuiltinFn::kAtomicMin:
            return call("atomic_fetch_min_explicit", true);

        case wgsl::BuiltinFn::kAtomicAnd:
            return call("atomic_fetch_and_explicit", true);

        case wgsl::BuiltinFn::kAtomicOr:
            return call("atomic_fetch_or_explicit", true);

        case wgsl::BuiltinFn::kAtomicXor:
            return call("atomic_fetch_xor_explicit", true);

        case wgsl::BuiltinFn::kAtomicExchange:
            return call("atomic_exchange_explicit", true);

        case wgsl::BuiltinFn::kAtomicCompareExchangeWeak: {
            auto* ptr_ty = TypeOf(expr->args[0])->UnwrapRef()->As<core::type::Pointer>();
            auto sc = ptr_ty->AddressSpace();
            auto* str = builtin->ReturnType()->As<core::type::Struct>();

            auto func = tint::GetOrAdd(
                atomicCompareExchangeWeak_, ACEWKeyType{{sc, str}}, [&]() -> std::string {
                    if (!EmitStructType(&helpers_,
                                        builtin->ReturnType()->As<core::type::Struct>())) {
                        return "";
                    }

                    auto name = UniqueIdentifier("atomicCompareExchangeWeak");
                    auto& buf = helpers_;
                    auto* atomic_ty = builtin->Parameters()[0]->Type();
                    auto* arg_ty = builtin->Parameters()[1]->Type();

                    {
                        auto f = Line(&buf);
                        auto str_name = StructName(builtin->ReturnType()->As<core::type::Struct>());
                        f << str_name << " " << name << "(";
                        if (!EmitTypeAndName(f, atomic_ty, "atomic")) {
                            return "";
                        }
                        f << ", ";
                        if (!EmitTypeAndName(f, arg_ty, "compare")) {
                            return "";
                        }
                        f << ", ";
                        if (!EmitTypeAndName(f, arg_ty, "value")) {
                            return "";
                        }
                        f << ") {";
                    }

                    buf.IncrementIndent();
                    TINT_DEFER({
                        buf.DecrementIndent();
                        Line(&buf) << "}";
                        Line(&buf);
                    });

                    {
                        auto f = Line(&buf);
                        if (!EmitTypeAndName(f, arg_ty, "old_value")) {
                            return "";
                        }
                        f << " = compare;";
                    }
                    Line(&buf) << "bool exchanged = "
                                  "atomic_compare_exchange_weak_explicit(atomic, "
                                  "&old_value, value, memory_order_relaxed, "
                                  "memory_order_relaxed);";
                    Line(&buf) << "return {old_value, exchanged};";
                    return name;
                });

            if (func.empty()) {
                return false;
            }
            return call(func, false);
        }

        default:
            break;
    }

    TINT_UNREACHABLE() << "unsupported atomic builtin: " << builtin->Fn();
}

bool ASTPrinter::EmitTextureCall(StringStream& out,
                                 const sem::Call* call,
                                 const sem::BuiltinFn* builtin) {
    using Usage = core::ParameterUsage;

    auto& signature = builtin->Signature();
    auto* expr = call->Declaration();
    auto& arguments = call->Arguments();

    // Returns the argument with the given usage
    auto arg = [&](Usage usage) {
        int idx = signature.IndexOf(usage);
        return (idx >= 0) ? arguments[static_cast<size_t>(idx)] : nullptr;
    };

    auto* texture = arg(Usage::kTexture)->Declaration();
    if (TINT_UNLIKELY(!texture)) {
        TINT_ICE() << "missing texture arg";
    }

    auto* texture_type = TypeOf(texture)->UnwrapRef()->As<core::type::Texture>();

    // Helper to emit the texture expression, wrapped in parentheses if the
    // expression includes an operator with lower precedence than the member
    // accessor used for the function calls.
    auto texture_expr = [&] {
        bool paren_lhs = !texture->IsAnyOf<ast::AccessorExpression, ast::CallExpression,
                                           ast::IdentifierExpression>();
        if (paren_lhs) {
            out << "(";
        }
        if (!EmitExpression(out, texture)) {
            return false;
        }
        if (paren_lhs) {
            out << ")";
        }
        return true;
    };

    // MSL requires that `lod` is a constant 0 for 1D textures.
    bool level_is_constant_zero = texture_type->dim() == core::type::TextureDimension::k1d;

    switch (builtin->Fn()) {
        case wgsl::BuiltinFn::kTextureDimensions: {
            std::vector<const char*> dims;
            switch (texture_type->dim()) {
                case core::type::TextureDimension::kNone:
                    diagnostics_.AddError(Source{}) << "texture dimension is kNone";
                    return false;
                case core::type::TextureDimension::k1d:
                    dims = {"width"};
                    break;
                case core::type::TextureDimension::k2d:
                case core::type::TextureDimension::k2dArray:
                case core::type::TextureDimension::kCube:
                case core::type::TextureDimension::kCubeArray:
                    dims = {"width", "height"};
                    break;
                case core::type::TextureDimension::k3d:
                    dims = {"width", "height", "depth"};
                    break;
            }

            auto get_dim = [&](const char* name) {
                if (!texture_expr()) {
                    return false;
                }
                out << ".get_" << name << "(";
                if (level_is_constant_zero) {
                    out << "0";
                } else {
                    if (auto* level = arg(Usage::kLevel)) {
                        if (!EmitExpression(out, level->Declaration())) {
                            return false;
                        }
                    }
                }
                out << ")";
                return true;
            };

            if (dims.size() == 1) {
                get_dim(dims[0]);
            } else {
                EmitType(out, TypeOf(expr)->UnwrapRef());
                out << "(";
                for (size_t i = 0; i < dims.size(); i++) {
                    if (i > 0) {
                        out << ", ";
                    }
                    get_dim(dims[i]);
                }
                out << ")";
            }
            return true;
        }
        case wgsl::BuiltinFn::kTextureNumLayers: {
            if (!texture_expr()) {
                return false;
            }
            out << ".get_array_size()";
            return true;
        }
        case wgsl::BuiltinFn::kTextureNumLevels: {
            if (!texture_expr()) {
                return false;
            }
            out << ".get_num_mip_levels()";
            return true;
        }
        case wgsl::BuiltinFn::kTextureNumSamples: {
            if (!texture_expr()) {
                return false;
            }
            out << ".get_num_samples()";
            return true;
        }
        default:
            break;
    }

    if (!texture_expr()) {
        return false;
    }

    bool lod_param_is_named = true;

    switch (builtin->Fn()) {
        case wgsl::BuiltinFn::kTextureSample:
        case wgsl::BuiltinFn::kTextureSampleBias:
        case wgsl::BuiltinFn::kTextureSampleLevel:
        case wgsl::BuiltinFn::kTextureSampleGrad:
            out << ".sample(";
            break;
        case wgsl::BuiltinFn::kTextureSampleCompare:
        case wgsl::BuiltinFn::kTextureSampleCompareLevel:
            out << ".sample_compare(";
            break;
        case wgsl::BuiltinFn::kTextureGather:
            out << ".gather(";
            break;
        case wgsl::BuiltinFn::kTextureGatherCompare:
            out << ".gather_compare(";
            break;
        case wgsl::BuiltinFn::kTextureLoad:
            out << ".read(";
            lod_param_is_named = false;
            break;
        case wgsl::BuiltinFn::kTextureStore:
            out << ".write(";
            break;
        default:
            TINT_UNREACHABLE() << "Unhandled texture builtin '" << builtin->str() << "'";
    }

    bool first_arg = true;
    auto maybe_write_comma = [&] {
        if (!first_arg) {
            out << ", ";
        }
        first_arg = false;
    };

    for (auto usage : {Usage::kValue, Usage::kSampler, Usage::kCoords, Usage::kArrayIndex,
                       Usage::kDepthRef, Usage::kSampleIndex}) {
        if (auto* e = arg(usage)) {
            maybe_write_comma();

            // Cast the coordinates to unsigned integers if necessary.
            bool casted = false;
            if (usage == Usage::kCoords && e->Type()->UnwrapRef()->is_integer_scalar_or_vector()) {
                casted = true;
                switch (texture_type->dim()) {
                    case core::type::TextureDimension::k1d:
                        out << "uint(";
                        break;
                    case core::type::TextureDimension::k2d:
                    case core::type::TextureDimension::k2dArray:
                        out << "uint2(";
                        break;
                    case core::type::TextureDimension::k3d:
                        out << "uint3(";
                        break;
                    default:
                        TINT_ICE() << "unhandled texture dimensionality";
                }
            }

            if (!EmitExpression(out, e->Declaration())) {
                return false;
            }

            if (casted) {
                out << ")";
            }
        }
    }

    if (auto* bias = arg(Usage::kBias)) {
        maybe_write_comma();
        out << "bias(";
        if (!EmitExpression(out, bias->Declaration())) {
            return false;
        }
        out << ")";
    }
    if (auto* level = arg(Usage::kLevel)) {
        maybe_write_comma();
        if (lod_param_is_named) {
            out << "level(";
        }
        if (level_is_constant_zero) {
            out << "0";
        } else {
            if (!EmitExpression(out, level->Declaration())) {
                return false;
            }
        }
        if (lod_param_is_named) {
            out << ")";
        }
    }
    if (builtin->Fn() == wgsl::BuiltinFn::kTextureSampleCompareLevel) {
        maybe_write_comma();
        out << "level(0)";
    }
    if (auto* ddx = arg(Usage::kDdx)) {
        auto dim = texture_type->dim();
        switch (dim) {
            case core::type::TextureDimension::k2d:
            case core::type::TextureDimension::k2dArray:
                maybe_write_comma();
                out << "gradient2d(";
                break;
            case core::type::TextureDimension::k3d:
                maybe_write_comma();
                out << "gradient3d(";
                break;
            case core::type::TextureDimension::kCube:
            case core::type::TextureDimension::kCubeArray:
                maybe_write_comma();
                out << "gradientcube(";
                break;
            default: {
                diagnostics_.AddError(Source{})
                    << "MSL does not support gradients for " << dim << " textures";
                return false;
            }
        }
        if (!EmitExpression(out, ddx->Declaration())) {
            return false;
        }
        out << ", ";
        if (!EmitExpression(out, arg(Usage::kDdy)->Declaration())) {
            return false;
        }
        out << ")";
    }

    bool has_offset = false;
    if (auto* offset = arg(Usage::kOffset)) {
        has_offset = true;
        maybe_write_comma();
        if (!EmitExpression(out, offset->Declaration())) {
            return false;
        }
    }

    if (auto* component = arg(Usage::kComponent)) {
        maybe_write_comma();
        if (!has_offset) {
            // offset argument may need to be provided if we have a component.
            switch (texture_type->dim()) {
                case core::type::TextureDimension::k2d:
                case core::type::TextureDimension::k2dArray:
                    out << "int2(0), ";
                    break;
                default:
                    break;  // Other texture dimensions don't have an offset
            }
        }
        auto c = component->ConstantValue()->ValueAs<AInt>();
        switch (c.value) {
            case 0:
                out << "component::x";
                break;
            case 1:
                out << "component::y";
                break;
            case 2:
                out << "component::z";
                break;
            case 3:
                out << "component::w";
                break;
            default:
                TINT_ICE() << "invalid textureGather component: " << c;
        }
    }

    out << ")";

    // If this is a `textureStore()` for a read-write texture, add a fence to ensure that the
    // written values are visible to subsequent reads from the same thread.
    if (auto* storage = texture_type->As<core::type::StorageTexture>();
        builtin->Fn() == wgsl::BuiltinFn::kTextureStore &&
        storage->access() == core::Access::kReadWrite) {
        out << "; ";
        texture_expr();
        out << ".fence()";
    }

    return true;
}

bool ASTPrinter::EmitDotCall(StringStream& out,
                             const ast::CallExpression* expr,
                             const sem::BuiltinFn* builtin) {
    auto* vec_ty = builtin->Parameters()[0]->Type()->As<core::type::Vector>();
    std::string fn = "dot";
    if (vec_ty->type()->is_integer_scalar()) {
        // MSL does not have a builtin for dot() with integer vector types.
        // Generate the helper function if it hasn't been created already
        fn = tint::GetOrAdd(int_dot_funcs_, vec_ty->Width(), [&]() -> std::string {
            TextBuffer b;
            TINT_DEFER(helpers_.Append(b));

            auto fn_name = UniqueIdentifier("tint_dot" + std::to_string(vec_ty->Width()));
            auto v = "vec<T," + std::to_string(vec_ty->Width()) + ">";

            Line(&b) << "template<typename T>";
            Line(&b) << "T " << fn_name << "(" << v << " a, " << v << " b) {";
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

    out << fn << "(";
    if (!EmitExpression(out, expr->args[0])) {
        return false;
    }
    out << ", ";
    if (!EmitExpression(out, expr->args[1])) {
        return false;
    }
    out << ")";
    return true;
}

bool ASTPrinter::EmitDot4I8PackedCall(StringStream& out,
                                      const ast::CallExpression* expr,
                                      const sem::BuiltinFn* builtin) {
    return CallBuiltinHelper(
        out, expr, builtin, [&](TextBuffer* b, const std::vector<std::string>& params) {
            Line(b) << "char4 vec1 = as_type<char4>(" << params[0] << ");";
            Line(b) << "char4 vec2 = as_type<char4>(" << params[1] << ");";
            Line(b) << "return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2] + vec1[3] "
                       "* vec2[3];";
            return true;
        });
}

bool ASTPrinter::EmitDot4U8PackedCall(StringStream& out,
                                      const ast::CallExpression* expr,
                                      const sem::BuiltinFn* builtin) {
    return CallBuiltinHelper(
        out, expr, builtin, [&](TextBuffer* b, const std::vector<std::string>& params) {
            Line(b) << "uchar4 vec1 = as_type<uchar4>(" << params[0] << ");";
            Line(b) << "uchar4 vec2 = as_type<uchar4>(" << params[1] << ");";
            Line(b) << "return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2] + vec1[3] "
                       "* vec2[3];";
            return true;
        });
}

bool ASTPrinter::EmitModfCall(StringStream& out,
                              const ast::CallExpression* expr,
                              const sem::BuiltinFn* builtin) {
    return CallBuiltinHelper(
        out, expr, builtin, [&](TextBuffer* b, const std::vector<std::string>& params) {
            auto* ty = builtin->Parameters()[0]->Type();
            auto in = params[0];

            std::string width;
            if (auto* vec = ty->As<core::type::Vector>()) {
                width = std::to_string(vec->Width());
            }

            // Emit the builtin return type unique to this overload. This does not
            // exist in the AST, so it will not be generated in Generate().
            if (!EmitStructType(&helpers_, builtin->ReturnType()->As<core::type::Struct>())) {
                return false;
            }

            Line(b) << StructName(builtin->ReturnType()->As<core::type::Struct>()) << " result;";
            Line(b) << "result.fract = modf(" << in << ", result.whole);";
            Line(b) << "return result;";
            return true;
        });
}

bool ASTPrinter::EmitFrexpCall(StringStream& out,
                               const ast::CallExpression* expr,
                               const sem::BuiltinFn* builtin) {
    return CallBuiltinHelper(
        out, expr, builtin, [&](TextBuffer* b, const std::vector<std::string>& params) {
            auto* ty = builtin->Parameters()[0]->Type();
            auto in = params[0];

            std::string width;
            if (auto* vec = ty->As<core::type::Vector>()) {
                width = std::to_string(vec->Width());
            }

            // Emit the builtin return type unique to this overload. This does not
            // exist in the AST, so it will not be generated in Generate().
            if (!EmitStructType(&helpers_, builtin->ReturnType()->As<core::type::Struct>())) {
                return false;
            }

            Line(b) << StructName(builtin->ReturnType()->As<core::type::Struct>()) << " result;";
            Line(b) << "result.fract = frexp(" << in << ", result.exp);";
            Line(b) << "return result;";
            return true;
        });
}

bool ASTPrinter::EmitDegreesCall(StringStream& out,
                                 const ast::CallExpression* expr,
                                 const sem::BuiltinFn* builtin) {
    return CallBuiltinHelper(out, expr, builtin,
                             [&](TextBuffer* b, const std::vector<std::string>& params) {
                                 Line(b) << "return " << params[0] << " * " << std::setprecision(20)
                                         << sem::kRadToDeg << ";";
                                 return true;
                             });
}

bool ASTPrinter::EmitRadiansCall(StringStream& out,
                                 const ast::CallExpression* expr,
                                 const sem::BuiltinFn* builtin) {
    return CallBuiltinHelper(out, expr, builtin,
                             [&](TextBuffer* b, const std::vector<std::string>& params) {
                                 Line(b) << "return " << params[0] << " * " << std::setprecision(20)
                                         << sem::kDegToRad << ";";
                                 return true;
                             });
}

std::string ASTPrinter::generate_builtin_name(const sem::BuiltinFn* builtin) {
    std::string out = "";
    switch (builtin->Fn()) {
        case wgsl::BuiltinFn::kAcos:
        case wgsl::BuiltinFn::kAcosh:
        case wgsl::BuiltinFn::kAll:
        case wgsl::BuiltinFn::kAny:
        case wgsl::BuiltinFn::kAsin:
        case wgsl::BuiltinFn::kAsinh:
        case wgsl::BuiltinFn::kAtanh:
        case wgsl::BuiltinFn::kAtan:
        case wgsl::BuiltinFn::kAtan2:
        case wgsl::BuiltinFn::kCeil:
        case wgsl::BuiltinFn::kCos:
        case wgsl::BuiltinFn::kCosh:
        case wgsl::BuiltinFn::kCross:
        case wgsl::BuiltinFn::kDeterminant:
        case wgsl::BuiltinFn::kDistance:
        case wgsl::BuiltinFn::kDot:
        case wgsl::BuiltinFn::kExp:
        case wgsl::BuiltinFn::kExp2:
        case wgsl::BuiltinFn::kFloor:
        case wgsl::BuiltinFn::kFma:
        case wgsl::BuiltinFn::kFract:
        case wgsl::BuiltinFn::kFrexp:
        case wgsl::BuiltinFn::kLength:
        case wgsl::BuiltinFn::kLdexp:
        case wgsl::BuiltinFn::kLog:
        case wgsl::BuiltinFn::kLog2:
        case wgsl::BuiltinFn::kMix:
        case wgsl::BuiltinFn::kModf:
        case wgsl::BuiltinFn::kNormalize:
        case wgsl::BuiltinFn::kReflect:
        case wgsl::BuiltinFn::kRefract:
        case wgsl::BuiltinFn::kSaturate:
        case wgsl::BuiltinFn::kSelect:
        case wgsl::BuiltinFn::kSin:
        case wgsl::BuiltinFn::kSinh:
        case wgsl::BuiltinFn::kSqrt:
        case wgsl::BuiltinFn::kStep:
        case wgsl::BuiltinFn::kTan:
        case wgsl::BuiltinFn::kTanh:
        case wgsl::BuiltinFn::kTranspose:
        case wgsl::BuiltinFn::kTrunc:
        case wgsl::BuiltinFn::kSign:
        case wgsl::BuiltinFn::kClamp:
            out += builtin->str();
            break;
        case wgsl::BuiltinFn::kPow:
            out += "powr";
            break;
        case wgsl::BuiltinFn::kAbs:
            if (builtin->ReturnType()->is_float_scalar_or_vector()) {
                out += "fabs";
            } else {
                out += "abs";
            }
            break;
        case wgsl::BuiltinFn::kCountLeadingZeros:
            out += "clz";
            break;
        case wgsl::BuiltinFn::kCountOneBits:
            out += "popcount";
            break;
        case wgsl::BuiltinFn::kCountTrailingZeros:
            out += "ctz";
            break;
        case wgsl::BuiltinFn::kDpdx:
        case wgsl::BuiltinFn::kDpdxCoarse:
        case wgsl::BuiltinFn::kDpdxFine:
            out += "dfdx";
            break;
        case wgsl::BuiltinFn::kDpdy:
        case wgsl::BuiltinFn::kDpdyCoarse:
        case wgsl::BuiltinFn::kDpdyFine:
            out += "dfdy";
            break;
        case wgsl::BuiltinFn::kExtractBits:
            out += "extract_bits";
            break;
        case wgsl::BuiltinFn::kInsertBits:
            out += "insert_bits";
            break;
        case wgsl::BuiltinFn::kFwidth:
        case wgsl::BuiltinFn::kFwidthCoarse:
        case wgsl::BuiltinFn::kFwidthFine:
            out += "fwidth";
            break;
        case wgsl::BuiltinFn::kMax:
            if (builtin->ReturnType()->is_float_scalar_or_vector()) {
                out += "fmax";
            } else {
                out += "max";
            }
            break;
        case wgsl::BuiltinFn::kMin:
            if (builtin->ReturnType()->is_float_scalar_or_vector()) {
                out += "fmin";
            } else {
                out += "min";
            }
            break;
        case wgsl::BuiltinFn::kFaceForward:
            out += "faceforward";
            break;
        case wgsl::BuiltinFn::kPack4X8Snorm:
            out += "pack_float_to_snorm4x8";
            break;
        case wgsl::BuiltinFn::kPack4X8Unorm:
            out += "pack_float_to_unorm4x8";
            break;
        case wgsl::BuiltinFn::kPack2X16Snorm:
            out += "pack_float_to_snorm2x16";
            break;
        case wgsl::BuiltinFn::kPack2X16Unorm:
            out += "pack_float_to_unorm2x16";
            break;
        case wgsl::BuiltinFn::kReverseBits:
            out += "reverse_bits";
            break;
        case wgsl::BuiltinFn::kRound:
            out += "rint";
            break;
        case wgsl::BuiltinFn::kSmoothstep:
            out += "smoothstep";
            break;
        case wgsl::BuiltinFn::kInverseSqrt:
            out += "rsqrt";
            break;
        case wgsl::BuiltinFn::kUnpack4X8Snorm:
            out += "unpack_snorm4x8_to_float";
            break;
        case wgsl::BuiltinFn::kUnpack4X8Unorm:
            out += "unpack_unorm4x8_to_float";
            break;
        case wgsl::BuiltinFn::kUnpack2X16Snorm:
            out += "unpack_snorm2x16_to_float";
            break;
        case wgsl::BuiltinFn::kUnpack2X16Unorm:
            out += "unpack_unorm2x16_to_float";
            break;
        case wgsl::BuiltinFn::kArrayLength:
            diagnostics_.AddError(Source{})
                << "Unable to translate builtin: " << builtin->Fn()
                << "\nDid you forget to pass array_length_from_uniform generator options?";
            return "";
        default:
            diagnostics_.AddError(Source{}) << "Unknown import method: " << builtin->Fn();
            return "";
    }
    return out;
}

bool ASTPrinter::EmitCase(const ast::CaseStatement* stmt) {
    auto* sem = builder_.Sem().Get<sem::CaseStatement>(stmt);
    for (auto* selector : sem->Selectors()) {
        auto out = Line();

        if (selector->IsDefault()) {
            out << "default";
        } else {
            out << "case ";
            if (!EmitConstant(out, selector->Value())) {
                return false;
            }
        }
        out << ":";
        if (selector == sem->Selectors().back()) {
            out << " {";
        }
    }

    {
        ScopedIndent si(this);

        for (auto* s : stmt->body->statements) {
            if (!EmitStatement(s)) {
                return false;
            }
        }

        if (!last_is_break(stmt->body)) {
            Line() << "break;";
        }
    }

    Line() << "}";

    return true;
}

bool ASTPrinter::EmitContinue(const ast::ContinueStatement*) {
    if (!emit_continuing_ || !emit_continuing_()) {
        return false;
    }

    Line() << "continue;";
    return true;
}

bool ASTPrinter::EmitZeroValue(StringStream& out, const core::type::Type* type) {
    return Switch(
        type,
        [&](const core::type::Bool*) {
            out << "false";
            return true;
        },
        [&](const core::type::F16*) {
            out << "0.0h";
            return true;
        },
        [&](const core::type::F32*) {
            out << "0.0f";
            return true;
        },
        [&](const core::type::I32*) {
            out << "0";
            return true;
        },
        [&](const core::type::U32*) {
            out << "0u";
            return true;
        },
        [&](const core::type::Vector* vec) {  //
            return EmitZeroValue(out, vec->type());
        },
        [&](const core::type::Matrix* mat) {
            if (!EmitType(out, mat)) {
                return false;
            }
            ScopedParen sp(out);
            return EmitZeroValue(out, mat->type());
        },
        [&](const core::type::Array*) {
            out << "{}";
            return true;
        },
        [&](const core::type::Struct*) {
            out << "{}";
            return true;
        },  //
        TINT_ICE_ON_NO_MATCH);
}

bool ASTPrinter::EmitConstant(StringStream& out, const core::constant::Value* constant) {
    return Switch(
        constant->Type(),  //
        [&](const core::type::Bool*) {
            out << (constant->ValueAs<AInt>() ? "true" : "false");
            return true;
        },
        [&](const core::type::F32*) {
            PrintF32(out, constant->ValueAs<f32>());
            return true;
        },
        [&](const core::type::F16*) {
            PrintF16(out, constant->ValueAs<f16>());
            return true;
        },
        [&](const core::type::I32*) {
            PrintI32(out, constant->ValueAs<i32>());
            return true;
        },
        [&](const core::type::U32*) {
            out << constant->ValueAs<AInt>() << "u";
            return true;
        },
        [&](const core::type::Vector* v) {
            if (!EmitType(out, v)) {
                return false;
            }

            ScopedParen sp(out);

            if (auto* splat = constant->As<core::constant::Splat>()) {
                if (!EmitConstant(out, splat->el)) {
                    return false;
                }
                return true;
            }

            for (size_t i = 0; i < v->Width(); i++) {
                if (i > 0) {
                    out << ", ";
                }
                if (!EmitConstant(out, constant->Index(i))) {
                    return false;
                }
            }
            return true;
        },
        [&](const core::type::Matrix* m) {
            if (!EmitType(out, m)) {
                return false;
            }

            ScopedParen sp(out);

            for (size_t i = 0; i < m->columns(); i++) {
                if (i > 0) {
                    out << ", ";
                }
                if (!EmitConstant(out, constant->Index(i))) {
                    return false;
                }
            }
            return true;
        },
        [&](const core::type::Array* a) {
            if (!EmitType(out, a)) {
                return false;
            }

            out << "{";
            TINT_DEFER(out << "}");

            if (constant->AllZero()) {
                return true;
            }

            auto count = a->ConstantCount();
            if (!count) {
                diagnostics_.AddError(Source{}) << core::type::Array::kErrExpectedConstantCount;
                return false;
            }

            for (size_t i = 0; i < count; i++) {
                if (i > 0) {
                    out << ", ";
                }
                if (!EmitConstant(out, constant->Index(i))) {
                    return false;
                }
            }

            return true;
        },
        [&](const core::type::Struct* s) {
            if (!EmitStructType(&helpers_, s)) {
                return false;
            }

            out << StructName(s) << "{";
            TINT_DEFER(out << "}");

            if (constant->AllZero()) {
                return true;
            }

            auto members = s->Members();
            for (size_t i = 0; i < members.Length(); i++) {
                if (i > 0) {
                    out << ", ";
                }
                out << "." << members[i]->Name().Name() << "=";
                if (!EmitConstant(out, constant->Index(i))) {
                    return false;
                }
            }

            return true;
        },  //
        TINT_ICE_ON_NO_MATCH);
}

bool ASTPrinter::EmitLiteral(StringStream& out, const ast::LiteralExpression* lit) {
    return Switch(
        lit,
        [&](const ast::BoolLiteralExpression* l) {
            out << (l->value ? "true" : "false");
            return true;
        },
        [&](const ast::FloatLiteralExpression* l) {
            if (l->suffix == ast::FloatLiteralExpression::Suffix::kH) {
                PrintF16(out, static_cast<float>(l->value));
            } else {
                PrintF32(out, static_cast<float>(l->value));
            }
            return true;
        },
        [&](const ast::IntLiteralExpression* i) {
            switch (i->suffix) {
                case ast::IntLiteralExpression::Suffix::kNone:
                case ast::IntLiteralExpression::Suffix::kI: {
                    PrintI32(out, static_cast<int32_t>(i->value));
                    return true;
                }
                case ast::IntLiteralExpression::Suffix::kU: {
                    out << i->value << "u";
                    return true;
                }
            }
            diagnostics_.AddError(Source{}) << "unknown integer literal suffix type";
            return false;
        },  //
        TINT_ICE_ON_NO_MATCH);
}

bool ASTPrinter::EmitExpression(StringStream& out, const ast::Expression* expr) {
    if (auto* sem = builder_.Sem().GetVal(expr)) {
        if (auto* constant = sem->ConstantValue()) {
            return EmitConstant(out, constant);
        }
    }
    return Switch(
        expr,  //
        [&](const ast::IndexAccessorExpression* a) { return EmitIndexAccessor(out, a); },
        [&](const ast::BinaryExpression* b) { return EmitBinary(out, b); },
        [&](const ast::CallExpression* c) { return EmitCall(out, c); },
        [&](const ast::IdentifierExpression* i) { return EmitIdentifier(out, i); },
        [&](const ast::LiteralExpression* l) { return EmitLiteral(out, l); },
        [&](const ast::MemberAccessorExpression* m) { return EmitMemberAccessor(out, m); },
        [&](const ast::UnaryOpExpression* u) { return EmitUnaryOp(out, u); },  //
        TINT_ICE_ON_NO_MATCH);
}

void ASTPrinter::EmitStage(StringStream& out, ast::PipelineStage stage) {
    switch (stage) {
        case ast::PipelineStage::kFragment:
            out << "fragment";
            break;
        case ast::PipelineStage::kVertex:
            out << "vertex";
            break;
        case ast::PipelineStage::kCompute:
            out << "kernel";
            break;
        case ast::PipelineStage::kNone:
            break;
    }
    return;
}

bool ASTPrinter::EmitFunction(const ast::Function* func) {
    if (func->body == nullptr) {
        // An internal function. Do not emit.
        return true;
    }

    auto* func_sem = builder_.Sem().Get(func);

    {
        auto out = Line();
        if (!EmitType(out, func_sem->ReturnType())) {
            return false;
        }
        out << " " << func->name->symbol.Name() << "(";

        bool first = true;
        for (auto* v : func->params) {
            if (!first) {
                out << ", ";
            }
            first = false;

            auto* type = builder_.Sem().Get(v)->Type();

            if (!EmitType(out, type)) {
                return false;
            }
            if (type->Is<core::type::Pointer>()) {
                out << " const";
            }
            out << " " << v->name->symbol.Name();
        }

        out << ") {";
    }

    if (!EmitStatementsWithIndent(func->body->statements)) {
        return false;
    }

    Line() << "}";

    return true;
}

bool ASTPrinter::EmitEntryPointFunction(const ast::Function* func) {
    auto* func_sem = builder_.Sem().Get(func);

    auto func_name = func->name->symbol.Name();
    workgroup_allocations_.insert({func_name, {}});

    // Returns the binding index of a variable, requiring that the group
    // attribute have a value of zero.
    const uint32_t kInvalidBindingIndex = std::numeric_limits<uint32_t>::max();
    auto get_binding_index = [&](const ast::Parameter* param) -> uint32_t {
        if (TINT_UNLIKELY(!param->HasBindingPoint())) {
            TINT_ICE() << "missing binding attributes for entry point parameter";
        }
        auto* param_sem = builder_.Sem().Get(param);
        auto bp = param_sem->Attributes().binding_point;
        if (TINT_UNLIKELY(bp->group != 0)) {
            TINT_ICE() << "encountered non-zero resource group index (use BindingRemapper to fix)";
        }
        return bp->binding;
    };

    {
        auto out = Line();

        EmitStage(out, func->PipelineStage());
        out << " ";
        if (!EmitTypeAndName(out, func_sem->ReturnType(), func_name)) {
            return false;
        }
        out << "(";

        // Emit entry point parameters.
        bool first = true;
        for (auto* param : func->params) {
            if (!first) {
                out << ", ";
            }
            first = false;

            auto* type = builder_.Sem().Get(param)->Type()->UnwrapRef();

            if (!EmitType(out, type)) {
                return false;
            }
            out << " " << param->name->symbol.Name();

            bool ok = Switch(
                type,  //
                [&](const core::type::Struct*) {
                    out << " [[stage_in]]";
                    return true;
                },
                [&](const core::type::Texture*) {
                    uint32_t binding = get_binding_index(param);
                    if (binding == kInvalidBindingIndex) {
                        return false;
                    }
                    out << " [[texture(" << binding << ")]]";
                    return true;
                },
                [&](const core::type::Sampler*) {
                    uint32_t binding = get_binding_index(param);
                    if (binding == kInvalidBindingIndex) {
                        return false;
                    }
                    out << " [[sampler(" << binding << ")]]";
                    return true;
                },
                [&](const core::type::Pointer* ptr) {
                    switch (ptr->AddressSpace()) {
                        case core::AddressSpace::kWorkgroup: {
                            auto& allocations = workgroup_allocations_[func_name];
                            out << " [[threadgroup(" << allocations.size() << ")]]";
                            allocations.push_back(ptr->StoreType()->Size());
                            return true;
                        }

                        case core::AddressSpace::kStorage:
                        case core::AddressSpace::kUniform: {
                            uint32_t binding = get_binding_index(param);
                            if (binding == kInvalidBindingIndex) {
                                return false;
                            }
                            out << " [[buffer(" << binding << ")]]";
                            return true;
                        }

                        default:
                            break;
                    }
                    TINT_ICE() << "invalid pointer address space for entry point parameter";
                },
                [&](Default) {
                    auto& attrs = param->attributes;
                    bool builtin_found = false;
                    for (auto* attr : attrs) {
                        auto* builtin_attr = attr->As<ast::BuiltinAttribute>();
                        if (!builtin_attr) {
                            continue;
                        }

                        builtin_found = true;

                        auto name = BuiltinToAttribute(builtin_attr->builtin);
                        if (name.empty()) {
                            diagnostics_.AddError(Source{}) << "unknown builtin";
                            return false;
                        }
                        out << " [[" << name << "]]";
                    }
                    if (TINT_UNLIKELY(!builtin_found)) {
                        TINT_ICE() << "Unsupported entry point parameter";
                    }
                    return true;
                });
            if (!ok) {
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
            ast::ReturnStatement ret(GenerationID{}, ast::NodeID{}, Source{});
            if (!EmitStatement(&ret)) {
                return false;
            }
        }
    }

    Line() << "}";
    return true;
}

bool ASTPrinter::EmitIdentifier(StringStream& out, const ast::IdentifierExpression* expr) {
    out << expr->identifier->symbol.Name();
    return true;
}

bool ASTPrinter::EmitLoop(const ast::LoopStatement* stmt) {
    auto emit_continuing = [this, stmt] {
        if (stmt->continuing && !stmt->continuing->Empty()) {
            if (!EmitBlock(stmt->continuing)) {
                return false;
            }
        }
        return true;
    };

    TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);
    Line() << IsolateUB() << " while(true) {";
    {
        ScopedIndent si(this);
        if (!EmitStatements(stmt->body->statements)) {
            return false;
        }
        if (!emit_continuing_()) {
            return false;
        }
    }
    Line() << "}";

    return true;
}

bool ASTPrinter::EmitForLoop(const ast::ForLoopStatement* stmt) {
    TextBuffer init_buf;
    if (auto* init = stmt->initializer) {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &init_buf);
        if (!EmitStatement(init)) {
            return false;
        }
    }

    TextBuffer cond_pre;
    StringStream cond_buf;
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

    // If the for-loop has a multi-statement conditional and / or continuing,
    // then we cannot emit this as a regular for-loop in MSL. Instead we need to
    // generate a `while(true)` loop.
    bool emit_as_loop = cond_pre.lines.size() > 0 || cont_buf.lines.size() > 1;

    // If the for-loop has multi-statement initializer, or is going to be
    // emitted as a `while(true)` loop, then declare the initializer
    // statement(s) before the loop in a new block.
    bool nest_in_block = init_buf.lines.size() > 1 || (stmt->initializer && emit_as_loop);
    if (nest_in_block) {
        Line() << "{";
        IncrementIndent();
        current_buffer_->Append(init_buf);
        init_buf.lines.clear();  // Don't emit the initializer again in the 'for'
    }
    TINT_DEFER({
        if (nest_in_block) {
            DecrementIndent();
            Line() << "}";
        }
    });

    if (emit_as_loop) {
        auto emit_continuing = [&] {
            current_buffer_->Append(cont_buf);
            return true;
        };

        TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);
        Line() << IsolateUB() << " while(true) {";
        IncrementIndent();
        TINT_DEFER({
            DecrementIndent();
            Line() << "}";
        });

        if (stmt->condition) {
            current_buffer_->Append(cond_pre);
            Line() << "if (!(" << cond_buf.str() << ")) { break; }";
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
            auto out = Line();
            out << IsolateUB() << " for";
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
            if (!EmitStatementsWithIndent(stmt->body->statements)) {
                return false;
            }
        }
        Line() << "}";
    }

    return true;
}

bool ASTPrinter::EmitWhile(const ast::WhileStatement* stmt) {
    TextBuffer cond_pre;
    StringStream cond_buf;

    {
        auto* cond = stmt->condition;
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &cond_pre);
        if (!EmitExpression(cond_buf, cond)) {
            return false;
        }
    }

    auto emit_continuing = [&] { return true; };
    TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);

    // If the while has a multi-statement conditional, then we cannot emit this
    // as a regular while in MSL. Instead we need to generate a `while(true)` loop.
    bool emit_as_loop = cond_pre.lines.size() > 0;
    if (emit_as_loop) {
        Line() << IsolateUB() << " while(true) {";
        IncrementIndent();
        TINT_DEFER({
            DecrementIndent();
            Line() << "}";
        });

        current_buffer_->Append(cond_pre);
        Line() << "if (!(" << cond_buf.str() << ")) { break; }";
        if (!EmitStatements(stmt->body->statements)) {
            return false;
        }
    } else {
        // While can be generated.
        Line() << IsolateUB() << " while(" << cond_buf.str() << ") {";
        if (!EmitStatementsWithIndent(stmt->body->statements)) {
            return false;
        }
        Line() << "}";
    }
    return true;
}

bool ASTPrinter::EmitDiscard(const ast::DiscardStatement*) {
    // TODO(dsinclair): Verify this is correct when the discard semantics are
    // defined for WGSL (https://github.com/gpuweb/gpuweb/issues/361)
    Line() << "discard_fragment();";
    return true;
}

bool ASTPrinter::EmitIf(const ast::IfStatement* stmt) {
    {
        auto out = Line();
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
        Line() << "} else {";
        if (auto* block = stmt->else_statement->As<ast::BlockStatement>()) {
            if (!EmitStatementsWithIndent(block->statements)) {
                return false;
            }
        } else {
            if (!EmitStatementsWithIndent(Vector{stmt->else_statement})) {
                return false;
            }
        }
    }
    Line() << "}";

    return true;
}

bool ASTPrinter::EmitMemberAccessor(StringStream& out, const ast::MemberAccessorExpression* expr) {
    auto write_lhs = [&] {
        bool paren_lhs = !expr->object->IsAnyOf<ast::AccessorExpression, ast::CallExpression,
                                                ast::IdentifierExpression>();
        if (paren_lhs) {
            out << "(";
        }
        if (!EmitExpression(out, expr->object)) {
            return false;
        }
        if (paren_lhs) {
            out << ")";
        }
        return true;
    };

    auto* sem = builder_.Sem().Get(expr)->UnwrapLoad();

    return Switch(
        sem,
        [&](const sem::Swizzle* swizzle) {
            // Metal did not add support for swizzle syntax with packed vector types until
            // Metal 2.1, so we need to use the index operator for single-element selection instead.
            // For multi-component swizzles, the PackedVec3 transform will have inserted casts to
            // the non-packed types, so we can safely use swizzle syntax here.
            if (swizzle->Indices().Length() == 1) {
                if (!write_lhs()) {
                    return false;
                }
                out << "[" << swizzle->Indices()[0] << "]";
            } else {
                if (!write_lhs()) {
                    return false;
                }
                out << "." << expr->member->symbol.Name();
            }
            return true;
        },
        [&](const sem::StructMemberAccess* member_access) {
            if (!write_lhs()) {
                return false;
            }
            out << "." << member_access->Member()->Name().Name();
            return true;
        },  //
        TINT_ICE_ON_NO_MATCH);
}

bool ASTPrinter::EmitReturn(const ast::ReturnStatement* stmt) {
    auto out = Line();
    out << "return";
    if (stmt->value) {
        out << " ";
        if (!EmitExpression(out, stmt->value)) {
            return false;
        }
    }
    out << ";";
    return true;
}

bool ASTPrinter::EmitBlock(const ast::BlockStatement* stmt) {
    Line() << "{";

    if (!EmitStatementsWithIndent(stmt->statements)) {
        return false;
    }

    Line() << "}";

    return true;
}

bool ASTPrinter::EmitStatement(const ast::Statement* stmt) {
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
            auto out = Line();
            if (!EmitCall(out, c->expr)) {  //
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
                },                //
                TINT_ICE_ON_NO_MATCH);
        },
        [&](const ast::ConstAssert*) {
            return true;  // Not emitted
        },                //
        TINT_ICE_ON_NO_MATCH);
}

bool ASTPrinter::EmitStatements(VectorRef<const ast::Statement*> stmts) {
    for (auto* s : stmts) {
        if (!EmitStatement(s)) {
            return false;
        }
    }
    return true;
}

bool ASTPrinter::EmitStatementsWithIndent(VectorRef<const ast::Statement*> stmts) {
    ScopedIndent si(this);
    return EmitStatements(stmts);
}

bool ASTPrinter::EmitSwitch(const ast::SwitchStatement* stmt) {
    {
        auto out = Line();
        out << "switch(";
        if (!EmitExpression(out, stmt->condition)) {
            return false;
        }
        out << ") {";
    }

    {
        ScopedIndent si(this);
        for (auto* s : stmt->body) {
            if (!EmitCase(s)) {
                return false;
            }
        }
    }

    Line() << "}";

    return true;
}

bool ASTPrinter::EmitType(StringStream& out, const core::type::Type* type) {
    return Switch(
        type,
        [&](const core::type::Atomic* atomic) {
            if (atomic->Type()->Is<core::type::I32>()) {
                out << "atomic_int";
                return true;
            }
            if (TINT_LIKELY(atomic->Type()->Is<core::type::U32>())) {
                out << "atomic_uint";
                return true;
            }
            TINT_ICE() << "unhandled atomic type " << atomic->Type()->FriendlyName();
        },
        [&](const core::type::Array* arr) {
            out << ArrayType() << "<";
            if (!EmitType(out, arr->ElemType())) {
                return false;
            }
            out << ", ";
            if (arr->Count()->Is<core::type::RuntimeArrayCount>()) {
                out << "1";
            } else {
                auto count = arr->ConstantCount();
                if (!count) {
                    diagnostics_.AddError(Source{}) << core::type::Array::kErrExpectedConstantCount;
                    return false;
                }

                out << count.value();
            }
            out << ">";
            return true;
        },
        [&](const core::type::Bool*) {
            out << "bool";
            return true;
        },
        [&](const core::type::F16*) {
            out << "half";
            return true;
        },
        [&](const core::type::F32*) {
            out << "float";
            return true;
        },
        [&](const core::type::I32*) {
            out << "int";
            return true;
        },
        [&](const core::type::Matrix* mat) {
            if (!EmitType(out, mat->type())) {
                return false;
            }
            out << mat->columns() << "x" << mat->rows();
            return true;
        },
        [&](const core::type::Pointer* ptr) {
            if (ptr->Access() == core::Access::kRead) {
                out << "const ";
            }
            if (!EmitAddressSpace(out, ptr->AddressSpace())) {
                return false;
            }
            out << " ";
            if (!EmitType(out, ptr->StoreType())) {
                return false;
            }
            out << "*";
            return true;
        },
        [&](const core::type::Sampler*) {
            out << "sampler";
            return true;
        },
        [&](const core::type::Struct* str) {
            // Make sure the struct type gets emitted. There are some cases where the types are
            // defined internal (like modf) which can end up in structures. The usage may be
            // removed by phonies, but the declaration still needs to exist.
            if (!EmitStructType(&helpers_, str)) {
                return false;
            }

            // The struct type emits as just the name. The declaration would be
            // emitted as part of emitting the declared types.
            out << StructName(str);
            return true;
        },
        [&](const core::type::Texture* tex) {
            if (TINT_UNLIKELY(tex->Is<core::type::ExternalTexture>())) {
                TINT_ICE() << "Multiplanar external texture transform was not run.";
            }

            if (tex->IsAnyOf<core::type::DepthTexture, core::type::DepthMultisampledTexture>()) {
                out << "depth";
            } else {
                out << "texture";
            }

            switch (tex->dim()) {
                case core::type::TextureDimension::k1d:
                    out << "1d";
                    break;
                case core::type::TextureDimension::k2d:
                    out << "2d";
                    break;
                case core::type::TextureDimension::k2dArray:
                    out << "2d_array";
                    break;
                case core::type::TextureDimension::k3d:
                    out << "3d";
                    break;
                case core::type::TextureDimension::kCube:
                    out << "cube";
                    break;
                case core::type::TextureDimension::kCubeArray:
                    out << "cube_array";
                    break;
                default:
                    diagnostics_.AddError(Source{}) << "Invalid texture dimensions";
                    return false;
            }
            if (tex->IsAnyOf<core::type::MultisampledTexture,
                             core::type::DepthMultisampledTexture>()) {
                out << "_ms";
            }
            out << "<";
            TINT_DEFER(out << ">");

            return Switch(
                tex,
                [&](const core::type::DepthTexture*) {
                    out << "float, access::sample";
                    return true;
                },
                [&](const core::type::DepthMultisampledTexture*) {
                    out << "float, access::read";
                    return true;
                },
                [&](const core::type::StorageTexture* storage) {
                    if (!EmitType(out, storage->type())) {
                        return false;
                    }

                    std::string access_str;
                    if (storage->access() == core::Access::kRead) {
                        out << ", access::read";
                    } else if (storage->access() == core::Access::kReadWrite) {
                        out << ", access::read_write";
                    } else if (storage->access() == core::Access::kWrite) {
                        out << ", access::write";
                    } else {
                        diagnostics_.AddError(Source{})
                            << "Invalid access control for storage texture";
                        return false;
                    }
                    return true;
                },
                [&](const core::type::MultisampledTexture* ms) {
                    if (!EmitType(out, ms->type())) {
                        return false;
                    }
                    out << ", access::read";
                    return true;
                },
                [&](const core::type::SampledTexture* sampled) {
                    if (!EmitType(out, sampled->type())) {
                        return false;
                    }
                    out << ", access::sample";
                    return true;
                },  //
                TINT_ICE_ON_NO_MATCH);
        },
        [&](const core::type::U32*) {
            out << "uint";
            return true;
        },
        [&](const core::type::Vector* vec) {
            if (vec->Packed()) {
                out << "packed_";
            }
            if (!EmitType(out, vec->type())) {
                return false;
            }
            out << vec->Width();
            return true;
        },
        [&](const core::type::Void*) {
            out << "void";
            return true;
        },  //
        TINT_ICE_ON_NO_MATCH);
}

bool ASTPrinter::EmitTypeAndName(StringStream& out,
                                 const core::type::Type* type,
                                 const std::string& name) {
    if (!EmitType(out, type)) {
        return false;
    }
    out << " " << name;
    return true;
}

bool ASTPrinter::EmitAddressSpace(StringStream& out, core::AddressSpace sc) {
    switch (sc) {
        case core::AddressSpace::kFunction:
        case core::AddressSpace::kPrivate:
        case core::AddressSpace::kHandle:
            out << "thread";
            return true;
        case core::AddressSpace::kWorkgroup:
            out << "threadgroup";
            return true;
        case core::AddressSpace::kStorage:
            out << "device";
            return true;
        case core::AddressSpace::kUniform:
            out << "constant";
            return true;
        default:
            break;
    }
    TINT_ICE() << "unhandled address space: " << sc;
}

bool ASTPrinter::EmitStructType(TextBuffer* b, const core::type::Struct* str) {
    auto it = emitted_structs_.emplace(str);
    if (!it.second) {
        return true;
    }

    Line(b) << "struct " << StructName(str) << " {";

    bool is_host_shareable = str->IsHostShareable();

    // Emits a `/* 0xnnnn */` byte offset comment for a struct member.
    auto add_byte_offset_comment = [&](StringStream& out, uint32_t offset) {
        std::ios_base::fmtflags saved_flag_state(out.flags());
        out << "/* 0x" << std::hex << std::setfill('0') << std::setw(4) << offset << " */ ";
        out.flags(saved_flag_state);
    };

    auto add_padding = [&](uint32_t size, uint32_t msl_offset) {
        std::string name;
        do {
            name = UniqueIdentifier("tint_pad");
        } while (str->FindMember(builder_.Symbols().Get(name)));

        auto out = Line(b);
        add_byte_offset_comment(out, msl_offset);
        out << ArrayType() << "<int8_t, " << size << "> " << name << ";";
    };

    b->IncrementIndent();

    uint32_t msl_offset = 0;
    for (auto* mem : str->Members()) {
        auto out = Line(b);
        auto mem_name = mem->Name().Name();
        auto wgsl_offset = mem->Offset();

        if (is_host_shareable) {
            if (TINT_UNLIKELY(wgsl_offset < msl_offset)) {
                // Unimplementable layout
                TINT_ICE() << "Structure member WGSL offset (" << wgsl_offset
                           << ") is behind MSL offset (" << msl_offset << ")";
            }

            // Generate padding if required
            if (auto padding = wgsl_offset - msl_offset) {
                add_padding(padding, msl_offset);
                msl_offset += padding;
            }

            add_byte_offset_comment(out, msl_offset);
        }

        if (!EmitType(out, mem->Type())) {
            return false;
        }

        auto* ty = mem->Type();

        out << " " << mem_name;
        // Emit attributes
        auto& attributes = mem->Attributes();

        if (auto builtin = attributes.builtin) {
            auto name = BuiltinToAttribute(builtin.value());
            if (name.empty()) {
                diagnostics_.AddError(Source{}) << "unknown builtin";
                return false;
            }
            out << " [[" << name << "]]";
        }

        if (auto location = attributes.location) {
            auto& pipeline_stage_uses = str->PipelineStageUses();
            if (TINT_UNLIKELY(pipeline_stage_uses.Count() != 1)) {
                TINT_ICE() << "invalid entry point IO struct uses for " << str->Name().NameView();
            }

            if (pipeline_stage_uses.Contains(core::type::PipelineStageUsage::kVertexInput)) {
                out << " [[attribute(" + std::to_string(location.value()) + ")]]";
            } else if (pipeline_stage_uses.Contains(
                           core::type::PipelineStageUsage::kVertexOutput)) {
                out << " [[user(locn" + std::to_string(location.value()) + ")]]";
            } else if (pipeline_stage_uses.Contains(
                           core::type::PipelineStageUsage::kFragmentInput)) {
                out << " [[user(locn" + std::to_string(location.value()) + ")]]";
            } else if (TINT_LIKELY(pipeline_stage_uses.Contains(
                           core::type::PipelineStageUsage::kFragmentOutput))) {
                if (auto blend_src = attributes.blend_src) {
                    out << " [[color(" + std::to_string(location.value()) + ") index(" +
                               std::to_string(blend_src.value()) + ")]]";
                } else {
                    out << " [[color(" + std::to_string(location.value()) + ")]]";
                }
            } else {
                TINT_ICE() << "invalid use of location decoration";
            }
        }

        if (auto color = attributes.color) {
            out << " [[color(" + std::to_string(color.value()) + ")]]";
        }

        if (auto interpolation = attributes.interpolation) {
            auto name = InterpolationToAttribute(interpolation->type, interpolation->sampling);
            if (name.empty()) {
                diagnostics_.AddError(Source{}) << "unknown interpolation attribute";
                return false;
            }
            out << " [[" << name << "]]";
        }

        if (attributes.invariant) {
            invariant_define_name_ = UniqueIdentifier("TINT_INVARIANT");
            out << " " << invariant_define_name_;
        }

        out << ";";

        if (is_host_shareable) {
            // Calculate new MSL offset
            auto size_align = MslPackedTypeSizeAndAlign(ty);
            if (TINT_UNLIKELY(msl_offset % size_align.align)) {
                TINT_ICE() << "Misaligned MSL structure member " << ty->FriendlyName() << " "
                           << mem_name;
            }
            msl_offset += size_align.size;
        }
    }

    if (is_host_shareable && str->Size() != msl_offset) {
        add_padding(str->Size() - msl_offset, msl_offset);
    }

    b->DecrementIndent();

    Line(b) << "};";
    return true;
}

bool ASTPrinter::EmitUnaryOp(StringStream& out, const ast::UnaryOpExpression* expr) {
    // Handle `-e` when `e` is signed, so that we ensure that if `e` is the
    // largest negative value, it returns `e`.
    auto* expr_type = TypeOf(expr->expr)->UnwrapRef();
    if (expr->op == core::UnaryOp::kNegation && expr_type->is_signed_integer_scalar_or_vector()) {
        auto fn = tint::GetOrAdd(unary_minus_funcs_, expr_type, [&]() -> std::string {
            // e.g.:
            // int tint_unary_minus(const int v) {
            //     return (v == -2147483648) ? v : -v;
            // }
            TextBuffer b;
            TINT_DEFER(helpers_.Append(b));

            auto fn_name = UniqueIdentifier("tint_unary_minus");
            {
                auto decl = Line(&b);
                if (!EmitTypeAndName(decl, expr_type, fn_name)) {
                    return "";
                }
                decl << "(const ";
                if (!EmitType(decl, expr_type)) {
                    return "";
                }
                decl << " v) {";
            }

            {
                ScopedIndent si(&b);
                const auto largest_negative_value =
                    std::to_string(std::numeric_limits<int32_t>::min());
                Line(&b) << "return select(-v, v, v == " << largest_negative_value << ");";
            }
            Line(&b) << "}";
            Line(&b);
            return fn_name;
        });

        out << fn << "(";
        if (!EmitExpression(out, expr->expr)) {
            return false;
        }
        out << ")";
        return true;
    }

    switch (expr->op) {
        case core::UnaryOp::kAddressOf:
            out << "&";
            break;
        case core::UnaryOp::kComplement:
            out << "~";
            break;
        case core::UnaryOp::kIndirection:
            out << "*";
            break;
        case core::UnaryOp::kNot:
            out << "!";
            break;
        case core::UnaryOp::kNegation:
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

bool ASTPrinter::EmitVar(const ast::Var* var) {
    auto* sem = builder_.Sem().Get(var);
    auto* type = sem->Type()->UnwrapRef();

    auto out = Line();

    switch (sem->AddressSpace()) {
        case core::AddressSpace::kFunction:
        case core::AddressSpace::kHandle:
            break;
        case core::AddressSpace::kPrivate:
            out << "thread ";
            break;
        case core::AddressSpace::kWorkgroup:
            out << "threadgroup ";
            break;
        default:
            TINT_ICE() << "unhandled variable address space";
    }

    if (!EmitType(out, type)) {
        return false;
    }
    out << " " << var->name->symbol.Name();

    if (var->initializer != nullptr) {
        out << " = ";
        if (!EmitExpression(out, var->initializer)) {
            return false;
        }
    } else if (sem->AddressSpace() == core::AddressSpace::kPrivate ||
               sem->AddressSpace() == core::AddressSpace::kFunction ||
               sem->AddressSpace() == core::AddressSpace::kUndefined) {
        out << " = ";
        if (!EmitZeroValue(out, type)) {
            return false;
        }
    }
    out << ";";

    return true;
}

bool ASTPrinter::EmitLet(const ast::Let* let) {
    auto* sem = builder_.Sem().Get(let);
    auto* type = sem->Type();

    auto out = Line();

    switch (sem->AddressSpace()) {
        case core::AddressSpace::kFunction:
        case core::AddressSpace::kHandle:
        case core::AddressSpace::kUndefined:
            break;
        case core::AddressSpace::kPrivate:
            out << "thread ";
            break;
        case core::AddressSpace::kWorkgroup:
            out << "threadgroup ";
            break;
        default:
            TINT_ICE() << "unhandled variable address space";
    }

    if (!EmitType(out, type)) {
        return false;
    }
    out << " const " << let->name->symbol.Name();

    out << " = ";
    if (!EmitExpression(out, let->initializer)) {
        return false;
    }
    out << ";";

    return true;
}

std::string ASTPrinter::IsolateUB() {
    if (isolate_ub_macro_name_.empty()) {
        isolate_ub_macro_name_ = UniqueIdentifier("TINT_ISOLATE_UB");
        Line(&helpers_) << "#define " << isolate_ub_macro_name_ << "(VOLATILE_NAME) \\";
        Line(&helpers_) << "  volatile bool VOLATILE_NAME = true; \\";
        Line(&helpers_) << "  if (VOLATILE_NAME)";
        Line(&helpers_);
    }
    StringStream ss;
    ss << isolate_ub_macro_name_ << "(" << UniqueIdentifier("tint_volatile_true") << ")";
    return ss.str();
}

template <typename F>
bool ASTPrinter::CallBuiltinHelper(StringStream& out,
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
            if (!EmitTypeAndName(decl, builtin->ReturnType(), fn_name)) {
                return "";
            }
            {
                ScopedParen sp(decl);
                for (auto* param : builtin->Parameters()) {
                    if (!parameter_names.empty()) {
                        decl << ", ";
                    }
                    auto param_name = "param_" + std::to_string(parameter_names.size());
                    if (!EmitTypeAndName(decl, param->Type(), param_name)) {
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
        Line(&b) << "}";
        Line(&b);
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

const std::string& ASTPrinter::ArrayType() {
    if (array_template_name_.empty()) {
        array_template_name_ = UniqueIdentifier("tint_array");
        auto* buf = &helpers_;
        Line(buf) << "template<typename T, size_t N>";
        Line(buf) << "struct " << array_template_name_ << " {";
        Line(buf) << "    const constant T& operator[](size_t i) const constant"
                  << " { return elements[i]; }";
        for (auto* space : {"device", "thread", "threadgroup"}) {
            Line(buf) << "    " << space << " T& operator[](size_t i) " << space
                      << " { return elements[i]; }";
            Line(buf) << "    const " << space << " T& operator[](size_t i) const " << space
                      << " { return elements[i]; }";
        }
        Line(buf) << "    T elements[N];";
        Line(buf) << "};";
        Line(buf);
    }
    return array_template_name_;
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

}  // namespace tint::msl::writer
