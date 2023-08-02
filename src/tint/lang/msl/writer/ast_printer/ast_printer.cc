// Copyright 2020 The Tint Authors.
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

#include "src/tint/lang/msl/writer/ast_printer/ast_printer.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <utility>
#include <vector>

#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/constant/value.h"
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
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/lang/msl/writer/common/printer_support.h"
#include "src/tint/lang/wgsl/ast/alias.h"
#include "src/tint/lang/wgsl/ast/bool_literal_expression.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/disable_validation_attribute.h"
#include "src/tint/lang/wgsl/ast/float_literal_expression.h"
#include "src/tint/lang/wgsl/ast/id_attribute.h"
#include "src/tint/lang/wgsl/ast/interpolate_attribute.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/ast/transform/array_length_from_uniform.h"
#include "src/tint/lang/wgsl/ast/transform/binding_remapper.h"
#include "src/tint/lang/wgsl/ast/transform/builtin_polyfill.h"
#include "src/tint/lang/wgsl/ast/transform/canonicalize_entry_point_io.h"
#include "src/tint/lang/wgsl/ast/transform/demote_to_helper.h"
#include "src/tint/lang/wgsl/ast/transform/disable_uniformity_analysis.h"
#include "src/tint/lang/wgsl/ast/transform/expand_compound_assignment.h"
#include "src/tint/lang/wgsl/ast/transform/manager.h"
#include "src/tint/lang/wgsl/ast/transform/module_scope_var_to_entry_point_param.h"
#include "src/tint/lang/wgsl/ast/transform/multiplanar_external_texture.h"
#include "src/tint/lang/wgsl/ast/transform/packed_vec3.h"
#include "src/tint/lang/wgsl/ast/transform/preserve_padding.h"
#include "src/tint/lang/wgsl/ast/transform/promote_initializers_to_let.h"
#include "src/tint/lang/wgsl/ast/transform/promote_side_effects_to_decl.h"
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

namespace tint::msl::writer {
namespace {

bool last_is_break(const ast::BlockStatement* stmts) {
    return tint::IsAnyOf<ast::BreakStatement>(stmts->Last());
}

class ScopedBitCast {
  public:
    ScopedBitCast(ASTPrinter* generator,
                  StringStream& stream,
                  const type::Type* curr_type,
                  const type::Type* target_type)
        : s(stream) {
        auto* target_vec_type = target_type->As<type::Vector>();

        // If we need to promote from scalar to vector, bitcast the scalar to the
        // vector element type.
        if (curr_type->Is<type::Scalar>() && target_vec_type) {
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

SanitizedResult Sanitize(const Program* in, const Options& options) {
    ast::transform::Manager manager;
    ast::transform::DataMap data;

    manager.Add<ast::transform::DisableUniformityAnalysis>();

    // ExpandCompoundAssignment must come before BuiltinPolyfill
    manager.Add<ast::transform::ExpandCompoundAssignment>();

    // Build the configs for the internal CanonicalizeEntryPointIO transform.
    auto entry_point_io_cfg = ast::transform::CanonicalizeEntryPointIO::Config(
        ast::transform::CanonicalizeEntryPointIO::ShaderStyle::kMsl, options.fixed_sample_mask,
        options.emit_vertex_point_size);

    manager.Add<ast::transform::PreservePadding>();

    manager.Add<ast::transform::Unshadow>();

    manager.Add<ast::transform::PromoteSideEffectsToDecl>();

    if (!options.disable_robustness) {
        // Robustness must come after PromoteSideEffectsToDecl
        // Robustness must come before BuiltinPolyfill and CanonicalizeEntryPointIO
        // Robustness must come before ArrayLengthFromUniform
        manager.Add<ast::transform::Robustness>();
    }

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
        polyfills.insert_bits = ast::transform::BuiltinPolyfill::Level::kClampParameters;
        polyfills.int_div_mod = true;
        polyfills.sign_int = true;
        polyfills.texture_sample_base_clamp_to_edge_2d_f32 = true;
        polyfills.workgroup_uniform_load = true;
        data.Add<ast::transform::BuiltinPolyfill::Config>(polyfills);
        manager.Add<ast::transform::BuiltinPolyfill>();
    }

    // Note: it is more efficient for MultiplanarExternalTexture to come after Robustness
    data.Add<ast::transform::MultiplanarExternalTexture::NewBindingPoints>(
        options.external_texture_options.bindings_map);
    manager.Add<ast::transform::MultiplanarExternalTexture>();

    // BindingRemapper must come after MultiplanarExternalTexture
    manager.Add<ast::transform::BindingRemapper>();
    data.Add<ast::transform::BindingRemapper::Remappings>(
        options.binding_remapper_options.binding_points,
        options.binding_remapper_options.access_controls,
        options.binding_remapper_options.allow_collisions);

    if (!options.disable_workgroup_init) {
        // ZeroInitWorkgroupMemory must come before CanonicalizeEntryPointIO as
        // ZeroInitWorkgroupMemory may inject new builtin parameters.
        manager.Add<ast::transform::ZeroInitWorkgroupMemory>();
    }

    // CanonicalizeEntryPointIO must come after Robustness
    manager.Add<ast::transform::CanonicalizeEntryPointIO>();
    data.Add<ast::transform::CanonicalizeEntryPointIO::Config>(std::move(entry_point_io_cfg));

    manager.Add<ast::transform::PromoteInitializersToLet>();

    // DemoteToHelper must come after PromoteSideEffectsToDecl and ExpandCompoundAssignment.
    // TODO(crbug.com/tint/1752): This is only necessary for Metal versions older than 2.3.
    manager.Add<ast::transform::DemoteToHelper>();

    manager.Add<ast::transform::VectorizeScalarMatrixInitializers>();
    manager.Add<ast::transform::RemovePhonies>();
    manager.Add<ast::transform::SimplifyPointers>();

    // ArrayLengthFromUniform must come after SimplifyPointers, as
    // it assumes that the form of the array length argument is &var.array.
    manager.Add<ast::transform::ArrayLengthFromUniform>();

    ast::transform::ArrayLengthFromUniform::Config array_length_cfg(
        std::move(options.array_length_from_uniform.ubo_binding));
    array_length_cfg.bindpoint_to_size_index =
        std::move(options.array_length_from_uniform.bindpoint_to_size_index);
    data.Add<ast::transform::ArrayLengthFromUniform::Config>(array_length_cfg);

    // PackedVec3 must come after ExpandCompoundAssignment.
    manager.Add<ast::transform::PackedVec3>();
    manager.Add<ast::transform::ModuleScopeVarToEntryPointParam>();

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

ASTPrinter::ASTPrinter(const Program* program) : builder_(ProgramBuilder::Wrap(program)) {}

ASTPrinter::~ASTPrinter() = default;

bool ASTPrinter::Generate() {
    if (!tint::writer::CheckSupportedExtensions(
            "MSL", builder_.AST(), diagnostics_,
            Vector{
                builtin::Extension::kChromiumDisableUniformityAnalysis,
                builtin::Extension::kChromiumExperimentalFullPtrParameters,
                builtin::Extension::kChromiumExperimentalPushConstant,
                builtin::Extension::kChromiumInternalRelaxedUniformLayout,
                builtin::Extension::kF16,
                builtin::Extension::kChromiumInternalDualSourceBlending,
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
                diagnostics_.add_error(diag::System::Writer,
                                       "override-expressions should have been removed with the "
                                       "SubstituteOverride transform.");
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
            [&](const ast::ConstAssert*) {
                return true;  // Not emitted
            },
            [&](Default) {
                // These are pushed into the entry point by sanitizer transforms.
                TINT_ICE() << "unhandled type: " << decl->TypeInfo().name;
                return false;
            });
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

bool ASTPrinter::EmitTypeDecl(const type::Type* ty) {
    if (auto* str = ty->As<type::Struct>()) {
        if (!EmitStructType(current_buffer_, str)) {
            return false;
        }
    } else {
        diagnostics_.add_error(diag::System::Writer, "unknown alias type: " + ty->FriendlyName());
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

bool ASTPrinter::EmitBitcast(StringStream& out, const ast::BitcastExpression* expr) {
    out << "as_type<";
    if (!EmitType(out, TypeOf(expr)->UnwrapRef())) {
        return false;
    }

    out << ">(";
    if (!EmitExpression(out, expr->expr)) {
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
                out << "&&";
                break;
            case ast::BinaryOp::kLogicalOr:
                out << "||";
                break;
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
                // generate extra code to implement WGSL-specified behaviour for
                // negative LHS.
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
        return true;
    };

    auto signed_type_of = [&](const type::Type* ty) -> const type::Type* {
        if (ty->is_integer_scalar()) {
            return builder_.create<type::I32>();
        } else if (auto* v = ty->As<type::Vector>()) {
            return builder_.create<type::Vector>(builder_.create<type::I32>(), v->Width());
        }
        return {};
    };

    auto unsigned_type_of = [&](const type::Type* ty) -> const type::Type* {
        if (ty->is_integer_scalar()) {
            return builder_.create<type::U32>();
        } else if (auto* v = ty->As<type::Vector>()) {
            return builder_.create<type::Vector>(builder_.create<type::U32>(), v->Width());
        }
        return {};
    };

    auto* lhs_type = TypeOf(expr->lhs)->UnwrapRef();
    auto* rhs_type = TypeOf(expr->rhs)->UnwrapRef();

    // Handle fmod
    if (expr->op == ast::BinaryOp::kModulo && lhs_type->is_float_scalar_or_vector()) {
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
        auto* target_type = lhs_type->Is<type::Vector>()
                                ? lhs_type
                                : (rhs_type->Is<type::Vector>() ? rhs_type : lhs_type);

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
    if ((expr->IsAnd() || expr->IsOr()) && lhs_type->Is<type::Bool>()) {
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
        target, [&](const sem::Function* func) { return EmitFunctionCall(out, call, func); },
        [&](const sem::Builtin* builtin) { return EmitBuiltinCall(out, call, builtin); },
        [&](const sem::ValueConversion* conv) { return EmitTypeConversion(out, call, conv); },
        [&](const sem::ValueConstructor* ctor) { return EmitTypeInitializer(out, call, ctor); },
        [&](Default) {
            TINT_ICE() << "unhandled call target: " << target->TypeInfo().name;
            return false;
        });
}

bool ASTPrinter::EmitFunctionCall(StringStream& out,
                                  const sem::Call* call,
                                  const sem::Function* fn) {
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
                                 const sem::Builtin* builtin) {
    auto* expr = call->Declaration();
    if (builtin->IsAtomic()) {
        return EmitAtomicCall(out, expr, builtin);
    }
    if (builtin->IsTexture()) {
        return EmitTextureCall(out, call, builtin);
    }

    auto name = generate_builtin_name(builtin);

    switch (builtin->Type()) {
        case builtin::Function::kDot:
            return EmitDotCall(out, expr, builtin);
        case builtin::Function::kModf:
            return EmitModfCall(out, expr, builtin);
        case builtin::Function::kFrexp:
            return EmitFrexpCall(out, expr, builtin);
        case builtin::Function::kDegrees:
            return EmitDegreesCall(out, expr, builtin);
        case builtin::Function::kRadians:
            return EmitRadiansCall(out, expr, builtin);

        case builtin::Function::kPack2X16Float:
        case builtin::Function::kUnpack2X16Float: {
            if (builtin->Type() == builtin::Function::kPack2X16Float) {
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
        case builtin::Function::kQuantizeToF16: {
            std::string width = "";
            if (auto* vec = builtin->ReturnType()->As<type::Vector>()) {
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
        case builtin::Function::kStorageBarrier: {
            out << "threadgroup_barrier(mem_flags::mem_device)";
            return true;
        }
        case builtin::Function::kWorkgroupBarrier: {
            out << "threadgroup_barrier(mem_flags::mem_threadgroup)";
            return true;
        }

        case builtin::Function::kLength: {
            auto* sem = builder_.Sem().GetVal(expr->args[0]);
            if (sem->Type()->UnwrapRef()->Is<type::Scalar>()) {
                // Emulate scalar overload using fabs(x).
                name = "fabs";
            }
            break;
        }

        case builtin::Function::kDistance: {
            auto* sem = builder_.Sem().GetVal(expr->args[0]);
            if (sem->Type()->UnwrapRef()->Is<type::Scalar>()) {
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
        [&](const type::Array*) {
            if (!EmitType(out, type)) {
                return false;
            }
            out << "{";
            terminator = "}";
            return true;
        },
        [&](const type::Struct*) {
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

        if (auto* struct_ty = type->As<type::Struct>()) {
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
                                const sem::Builtin* builtin) {
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

    switch (builtin->Type()) {
        case builtin::Function::kAtomicLoad:
            return call("atomic_load_explicit", true);

        case builtin::Function::kAtomicStore:
            return call("atomic_store_explicit", true);

        case builtin::Function::kAtomicAdd:
            return call("atomic_fetch_add_explicit", true);

        case builtin::Function::kAtomicSub:
            return call("atomic_fetch_sub_explicit", true);

        case builtin::Function::kAtomicMax:
            return call("atomic_fetch_max_explicit", true);

        case builtin::Function::kAtomicMin:
            return call("atomic_fetch_min_explicit", true);

        case builtin::Function::kAtomicAnd:
            return call("atomic_fetch_and_explicit", true);

        case builtin::Function::kAtomicOr:
            return call("atomic_fetch_or_explicit", true);

        case builtin::Function::kAtomicXor:
            return call("atomic_fetch_xor_explicit", true);

        case builtin::Function::kAtomicExchange:
            return call("atomic_exchange_explicit", true);

        case builtin::Function::kAtomicCompareExchangeWeak: {
            auto* ptr_ty = TypeOf(expr->args[0])->UnwrapRef()->As<type::Pointer>();
            auto sc = ptr_ty->AddressSpace();
            auto* str = builtin->ReturnType()->As<type::Struct>();

            auto func = tint::GetOrCreate(
                atomicCompareExchangeWeak_, ACEWKeyType{{sc, str}}, [&]() -> std::string {
                    if (!EmitStructType(&helpers_, builtin->ReturnType()->As<type::Struct>())) {
                        return "";
                    }

                    auto name = UniqueIdentifier("atomicCompareExchangeWeak");
                    auto& buf = helpers_;
                    auto* atomic_ty = builtin->Parameters()[0]->Type();
                    auto* arg_ty = builtin->Parameters()[1]->Type();

                    {
                        auto f = Line(&buf);
                        auto str_name = StructName(builtin->ReturnType()->As<type::Struct>());
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

    TINT_UNREACHABLE() << "unsupported atomic builtin: " << builtin->Type();
    return false;
}

bool ASTPrinter::EmitTextureCall(StringStream& out,
                                 const sem::Call* call,
                                 const sem::Builtin* builtin) {
    using Usage = sem::ParameterUsage;

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
        return false;
    }

    auto* texture_type = TypeOf(texture)->UnwrapRef()->As<type::Texture>();

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
    bool level_is_constant_zero = texture_type->dim() == type::TextureDimension::k1d;

    switch (builtin->Type()) {
        case builtin::Function::kTextureDimensions: {
            std::vector<const char*> dims;
            switch (texture_type->dim()) {
                case type::TextureDimension::kNone:
                    diagnostics_.add_error(diag::System::Writer, "texture dimension is kNone");
                    return false;
                case type::TextureDimension::k1d:
                    dims = {"width"};
                    break;
                case type::TextureDimension::k2d:
                case type::TextureDimension::k2dArray:
                case type::TextureDimension::kCube:
                case type::TextureDimension::kCubeArray:
                    dims = {"width", "height"};
                    break;
                case type::TextureDimension::k3d:
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
        case builtin::Function::kTextureNumLayers: {
            if (!texture_expr()) {
                return false;
            }
            out << ".get_array_size()";
            return true;
        }
        case builtin::Function::kTextureNumLevels: {
            if (!texture_expr()) {
                return false;
            }
            out << ".get_num_mip_levels()";
            return true;
        }
        case builtin::Function::kTextureNumSamples: {
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

    switch (builtin->Type()) {
        case builtin::Function::kTextureSample:
        case builtin::Function::kTextureSampleBias:
        case builtin::Function::kTextureSampleLevel:
        case builtin::Function::kTextureSampleGrad:
            out << ".sample(";
            break;
        case builtin::Function::kTextureSampleCompare:
        case builtin::Function::kTextureSampleCompareLevel:
            out << ".sample_compare(";
            break;
        case builtin::Function::kTextureGather:
            out << ".gather(";
            break;
        case builtin::Function::kTextureGatherCompare:
            out << ".gather_compare(";
            break;
        case builtin::Function::kTextureLoad:
            out << ".read(";
            lod_param_is_named = false;
            break;
        case builtin::Function::kTextureStore:
            out << ".write(";
            break;
        default:
            TINT_UNREACHABLE() << "Unhandled texture builtin '" << builtin->str() << "'";
            return false;
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
                    case type::TextureDimension::k1d:
                        out << "uint(";
                        break;
                    case type::TextureDimension::k2d:
                    case type::TextureDimension::k2dArray:
                        out << "uint2(";
                        break;
                    case type::TextureDimension::k3d:
                        out << "uint3(";
                        break;
                    default:
                        TINT_ICE() << "unhandled texture dimensionality";
                        break;
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
    if (builtin->Type() == builtin::Function::kTextureSampleCompareLevel) {
        maybe_write_comma();
        out << "level(0)";
    }
    if (auto* ddx = arg(Usage::kDdx)) {
        auto dim = texture_type->dim();
        switch (dim) {
            case type::TextureDimension::k2d:
            case type::TextureDimension::k2dArray:
                maybe_write_comma();
                out << "gradient2d(";
                break;
            case type::TextureDimension::k3d:
                maybe_write_comma();
                out << "gradient3d(";
                break;
            case type::TextureDimension::kCube:
            case type::TextureDimension::kCubeArray:
                maybe_write_comma();
                out << "gradientcube(";
                break;
            default: {
                StringStream err;
                err << "MSL does not support gradients for " << dim << " textures";
                diagnostics_.add_error(diag::System::Writer, err.str());
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
                case type::TextureDimension::k2d:
                case type::TextureDimension::k2dArray:
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
                break;
        }
    }

    out << ")";

    return true;
}

bool ASTPrinter::EmitDotCall(StringStream& out,
                             const ast::CallExpression* expr,
                             const sem::Builtin* builtin) {
    auto* vec_ty = builtin->Parameters()[0]->Type()->As<type::Vector>();
    std::string fn = "dot";
    if (vec_ty->type()->is_integer_scalar()) {
        // MSL does not have a builtin for dot() with integer vector types.
        // Generate the helper function if it hasn't been created already
        fn = tint::GetOrCreate(int_dot_funcs_, vec_ty->Width(), [&]() -> std::string {
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

bool ASTPrinter::EmitModfCall(StringStream& out,
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

            Line(b) << StructName(builtin->ReturnType()->As<type::Struct>()) << " result;";
            Line(b) << "result.fract = modf(" << in << ", result.whole);";
            Line(b) << "return result;";
            return true;
        });
}

bool ASTPrinter::EmitFrexpCall(StringStream& out,
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

            Line(b) << StructName(builtin->ReturnType()->As<type::Struct>()) << " result;";
            Line(b) << "result.fract = frexp(" << in << ", result.exp);";
            Line(b) << "return result;";
            return true;
        });
}

bool ASTPrinter::EmitDegreesCall(StringStream& out,
                                 const ast::CallExpression* expr,
                                 const sem::Builtin* builtin) {
    return CallBuiltinHelper(out, expr, builtin,
                             [&](TextBuffer* b, const std::vector<std::string>& params) {
                                 Line(b) << "return " << params[0] << " * " << std::setprecision(20)
                                         << sem::kRadToDeg << ";";
                                 return true;
                             });
}

bool ASTPrinter::EmitRadiansCall(StringStream& out,
                                 const ast::CallExpression* expr,
                                 const sem::Builtin* builtin) {
    return CallBuiltinHelper(out, expr, builtin,
                             [&](TextBuffer* b, const std::vector<std::string>& params) {
                                 Line(b) << "return " << params[0] << " * " << std::setprecision(20)
                                         << sem::kDegToRad << ";";
                                 return true;
                             });
}

std::string ASTPrinter::generate_builtin_name(const sem::Builtin* builtin) {
    std::string out = "";
    switch (builtin->Type()) {
        case builtin::Function::kAcos:
        case builtin::Function::kAcosh:
        case builtin::Function::kAll:
        case builtin::Function::kAny:
        case builtin::Function::kAsin:
        case builtin::Function::kAsinh:
        case builtin::Function::kAtanh:
        case builtin::Function::kAtan:
        case builtin::Function::kAtan2:
        case builtin::Function::kCeil:
        case builtin::Function::kCos:
        case builtin::Function::kCosh:
        case builtin::Function::kCross:
        case builtin::Function::kDeterminant:
        case builtin::Function::kDistance:
        case builtin::Function::kDot:
        case builtin::Function::kExp:
        case builtin::Function::kExp2:
        case builtin::Function::kFloor:
        case builtin::Function::kFma:
        case builtin::Function::kFract:
        case builtin::Function::kFrexp:
        case builtin::Function::kLength:
        case builtin::Function::kLdexp:
        case builtin::Function::kLog:
        case builtin::Function::kLog2:
        case builtin::Function::kMix:
        case builtin::Function::kModf:
        case builtin::Function::kNormalize:
        case builtin::Function::kPow:
        case builtin::Function::kReflect:
        case builtin::Function::kRefract:
        case builtin::Function::kSaturate:
        case builtin::Function::kSelect:
        case builtin::Function::kSin:
        case builtin::Function::kSinh:
        case builtin::Function::kSqrt:
        case builtin::Function::kStep:
        case builtin::Function::kTan:
        case builtin::Function::kTanh:
        case builtin::Function::kTranspose:
        case builtin::Function::kTrunc:
        case builtin::Function::kSign:
        case builtin::Function::kClamp:
            out += builtin->str();
            break;
        case builtin::Function::kAbs:
            if (builtin->ReturnType()->is_float_scalar_or_vector()) {
                out += "fabs";
            } else {
                out += "abs";
            }
            break;
        case builtin::Function::kCountLeadingZeros:
            out += "clz";
            break;
        case builtin::Function::kCountOneBits:
            out += "popcount";
            break;
        case builtin::Function::kCountTrailingZeros:
            out += "ctz";
            break;
        case builtin::Function::kDpdx:
        case builtin::Function::kDpdxCoarse:
        case builtin::Function::kDpdxFine:
            out += "dfdx";
            break;
        case builtin::Function::kDpdy:
        case builtin::Function::kDpdyCoarse:
        case builtin::Function::kDpdyFine:
            out += "dfdy";
            break;
        case builtin::Function::kExtractBits:
            out += "extract_bits";
            break;
        case builtin::Function::kInsertBits:
            out += "insert_bits";
            break;
        case builtin::Function::kFwidth:
        case builtin::Function::kFwidthCoarse:
        case builtin::Function::kFwidthFine:
            out += "fwidth";
            break;
        case builtin::Function::kMax:
            if (builtin->ReturnType()->is_float_scalar_or_vector()) {
                out += "fmax";
            } else {
                out += "max";
            }
            break;
        case builtin::Function::kMin:
            if (builtin->ReturnType()->is_float_scalar_or_vector()) {
                out += "fmin";
            } else {
                out += "min";
            }
            break;
        case builtin::Function::kFaceForward:
            out += "faceforward";
            break;
        case builtin::Function::kPack4X8Snorm:
            out += "pack_float_to_snorm4x8";
            break;
        case builtin::Function::kPack4X8Unorm:
            out += "pack_float_to_unorm4x8";
            break;
        case builtin::Function::kPack2X16Snorm:
            out += "pack_float_to_snorm2x16";
            break;
        case builtin::Function::kPack2X16Unorm:
            out += "pack_float_to_unorm2x16";
            break;
        case builtin::Function::kReverseBits:
            out += "reverse_bits";
            break;
        case builtin::Function::kRound:
            out += "rint";
            break;
        case builtin::Function::kSmoothstep:
            out += "smoothstep";
            break;
        case builtin::Function::kInverseSqrt:
            out += "rsqrt";
            break;
        case builtin::Function::kUnpack4X8Snorm:
            out += "unpack_snorm4x8_to_float";
            break;
        case builtin::Function::kUnpack4X8Unorm:
            out += "unpack_unorm4x8_to_float";
            break;
        case builtin::Function::kUnpack2X16Snorm:
            out += "unpack_snorm2x16_to_float";
            break;
        case builtin::Function::kUnpack2X16Unorm:
            out += "unpack_unorm2x16_to_float";
            break;
        case builtin::Function::kArrayLength:
            diagnostics_.add_error(
                diag::System::Writer,
                "Unable to translate builtin: " + std::string(builtin->str()) +
                    "\nDid you forget to pass array_length_from_uniform generator "
                    "options?");
            return "";
        default:
            diagnostics_.add_error(diag::System::Writer,
                                   "Unknown import method: " + std::string(builtin->str()));
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

bool ASTPrinter::EmitZeroValue(StringStream& out, const type::Type* type) {
    return Switch(
        type,
        [&](const type::Bool*) {
            out << "false";
            return true;
        },
        [&](const type::F16*) {
            out << "0.0h";
            return true;
        },
        [&](const type::F32*) {
            out << "0.0f";
            return true;
        },
        [&](const type::I32*) {
            out << "0";
            return true;
        },
        [&](const type::U32*) {
            out << "0u";
            return true;
        },
        [&](const type::Vector* vec) {  //
            return EmitZeroValue(out, vec->type());
        },
        [&](const type::Matrix* mat) {
            if (!EmitType(out, mat)) {
                return false;
            }
            ScopedParen sp(out);
            return EmitZeroValue(out, mat->type());
        },
        [&](const type::Array*) {
            out << "{}";
            return true;
        },
        [&](const type::Struct*) {
            out << "{}";
            return true;
        },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer,
                                   "Invalid type for zero emission: " + type->FriendlyName());
            return false;
        });
}

bool ASTPrinter::EmitConstant(StringStream& out, const constant::Value* constant) {
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
            PrintF16(out, constant->ValueAs<f16>());
            return true;
        },
        [&](const type::I32*) {
            PrintI32(out, constant->ValueAs<i32>());
            return true;
        },
        [&](const type::U32*) {
            out << constant->ValueAs<AInt>() << "u";
            return true;
        },
        [&](const type::Vector* v) {
            if (!EmitType(out, v)) {
                return false;
            }

            ScopedParen sp(out);

            if (auto* splat = constant->As<constant::Splat>()) {
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
        [&](const type::Matrix* m) {
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
        [&](const type::Array* a) {
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
                diagnostics_.add_error(diag::System::Writer,
                                       type::Array::kErrExpectedConstantCount);
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
        [&](const type::Struct* s) {
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
        },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer,
                                   "unhandled constant type: " + constant->Type()->FriendlyName());
            return false;
        });
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
            diagnostics_.add_error(diag::System::Writer, "unknown integer literal suffix type");
            return false;
        },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer, "unknown literal type");
            return false;
        });
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
        [&](const ast::BitcastExpression* b) { return EmitBitcast(out, b); },
        [&](const ast::CallExpression* c) { return EmitCall(out, c); },
        [&](const ast::IdentifierExpression* i) { return EmitIdentifier(out, i); },
        [&](const ast::LiteralExpression* l) { return EmitLiteral(out, l); },
        [&](const ast::MemberAccessorExpression* m) { return EmitMemberAccessor(out, m); },
        [&](const ast::UnaryOpExpression* u) { return EmitUnaryOp(out, u); },
        [&](Default) {  //
            diagnostics_.add_error(diag::System::Writer, "unknown expression type: " +
                                                             std::string(expr->TypeInfo().name));
            return false;
        });
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
            if (type->Is<type::Pointer>()) {
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
            return kInvalidBindingIndex;
        }
        auto* param_sem = builder_.Sem().Get<sem::Parameter>(param);
        auto bp = param_sem->BindingPoint();
        if (TINT_UNLIKELY(bp->group != 0)) {
            TINT_ICE() << "encountered non-zero resource group index (use "
                          "BindingRemapper to fix)";
            return kInvalidBindingIndex;
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
                [&](const type::Struct*) {
                    out << " [[stage_in]]";
                    return true;
                },
                [&](const type::Texture*) {
                    uint32_t binding = get_binding_index(param);
                    if (binding == kInvalidBindingIndex) {
                        return false;
                    }
                    out << " [[texture(" << binding << ")]]";
                    return true;
                },
                [&](const type::Sampler*) {
                    uint32_t binding = get_binding_index(param);
                    if (binding == kInvalidBindingIndex) {
                        return false;
                    }
                    out << " [[sampler(" << binding << ")]]";
                    return true;
                },
                [&](const type::Pointer* ptr) {
                    switch (ptr->AddressSpace()) {
                        case builtin::AddressSpace::kWorkgroup: {
                            auto& allocations = workgroup_allocations_[func_name];
                            out << " [[threadgroup(" << allocations.size() << ")]]";
                            allocations.push_back(ptr->StoreType()->Size());
                            return true;
                        }

                        case builtin::AddressSpace::kStorage:
                        case builtin::AddressSpace::kUniform: {
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
                    return false;
                },
                [&](Default) {
                    auto& attrs = param->attributes;
                    bool builtin_found = false;
                    for (auto* attr : attrs) {
                        auto* builtin_attr = attr->As<ast::BuiltinAttribute>();
                        if (!builtin_attr) {
                            continue;
                        }
                        auto builtin = builder_.Sem().Get(builtin_attr)->Value();

                        builtin_found = true;

                        auto name = BuiltinToAttribute(builtin);
                        if (name.empty()) {
                            diagnostics_.add_error(diag::System::Writer, "unknown builtin");
                            return false;
                        }
                        out << " [[" << name << "]]";
                    }
                    if (TINT_UNLIKELY(!builtin_found)) {
                        TINT_ICE() << "Unsupported entry point parameter";
                        return false;
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
    Line() << "while (true) {";
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
        Line() << "while (true) {";
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
        {
            auto out = Line();
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
        },
        [&](Default) {
            TINT_ICE() << "unknown member access type: " << sem->TypeInfo().name;
            return false;
        });
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
                },
                [&](Default) {  //
                    TINT_ICE() << "unknown statement type: " << stmt->TypeInfo().name;
                    return false;
                });
        },
        [&](const ast::ConstAssert*) {
            return true;  // Not emitted
        },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer,
                                   "unknown statement type: " + std::string(stmt->TypeInfo().name));
            return false;
        });
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

bool ASTPrinter::EmitType(StringStream& out, const type::Type* type) {
    return Switch(
        type,
        [&](const type::Atomic* atomic) {
            if (atomic->Type()->Is<type::I32>()) {
                out << "atomic_int";
                return true;
            }
            if (TINT_LIKELY(atomic->Type()->Is<type::U32>())) {
                out << "atomic_uint";
                return true;
            }
            TINT_ICE() << "unhandled atomic type " << atomic->Type()->FriendlyName();
            return false;
        },
        [&](const type::Array* arr) {
            out << ArrayType() << "<";
            if (!EmitType(out, arr->ElemType())) {
                return false;
            }
            out << ", ";
            if (arr->Count()->Is<type::RuntimeArrayCount>()) {
                out << "1";
            } else {
                auto count = arr->ConstantCount();
                if (!count) {
                    diagnostics_.add_error(diag::System::Writer,
                                           type::Array::kErrExpectedConstantCount);
                    return false;
                }

                out << count.value();
            }
            out << ">";
            return true;
        },
        [&](const type::Bool*) {
            out << "bool";
            return true;
        },
        [&](const type::F16*) {
            out << "half";
            return true;
        },
        [&](const type::F32*) {
            out << "float";
            return true;
        },
        [&](const type::I32*) {
            out << "int";
            return true;
        },
        [&](const type::Matrix* mat) {
            if (!EmitType(out, mat->type())) {
                return false;
            }
            out << mat->columns() << "x" << mat->rows();
            return true;
        },
        [&](const type::Pointer* ptr) {
            if (ptr->Access() == builtin::Access::kRead) {
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
        [&](const type::Sampler*) {
            out << "sampler";
            return true;
        },
        [&](const type::Struct* str) {
            // The struct type emits as just the name. The declaration would be
            // emitted as part of emitting the declared types.
            out << StructName(str);
            return true;
        },
        [&](const type::Texture* tex) {
            if (TINT_UNLIKELY(tex->Is<type::ExternalTexture>())) {
                TINT_ICE() << "Multiplanar external texture transform was not run.";
                return false;
            }

            if (tex->IsAnyOf<type::DepthTexture, type::DepthMultisampledTexture>()) {
                out << "depth";
            } else {
                out << "texture";
            }

            switch (tex->dim()) {
                case type::TextureDimension::k1d:
                    out << "1d";
                    break;
                case type::TextureDimension::k2d:
                    out << "2d";
                    break;
                case type::TextureDimension::k2dArray:
                    out << "2d_array";
                    break;
                case type::TextureDimension::k3d:
                    out << "3d";
                    break;
                case type::TextureDimension::kCube:
                    out << "cube";
                    break;
                case type::TextureDimension::kCubeArray:
                    out << "cube_array";
                    break;
                default:
                    diagnostics_.add_error(diag::System::Writer, "Invalid texture dimensions");
                    return false;
            }
            if (tex->IsAnyOf<type::MultisampledTexture, type::DepthMultisampledTexture>()) {
                out << "_ms";
            }
            out << "<";
            TINT_DEFER(out << ">");

            return Switch(
                tex,
                [&](const type::DepthTexture*) {
                    out << "float, access::sample";
                    return true;
                },
                [&](const type::DepthMultisampledTexture*) {
                    out << "float, access::read";
                    return true;
                },
                [&](const type::StorageTexture* storage) {
                    if (!EmitType(out, storage->type())) {
                        return false;
                    }

                    std::string access_str;
                    if (storage->access() == builtin::Access::kRead) {
                        out << ", access::read";
                    } else if (storage->access() == builtin::Access::kWrite) {
                        out << ", access::write";
                    } else {
                        diagnostics_.add_error(diag::System::Writer,
                                               "Invalid access control for storage texture");
                        return false;
                    }
                    return true;
                },
                [&](const type::MultisampledTexture* ms) {
                    if (!EmitType(out, ms->type())) {
                        return false;
                    }
                    out << ", access::read";
                    return true;
                },
                [&](const type::SampledTexture* sampled) {
                    if (!EmitType(out, sampled->type())) {
                        return false;
                    }
                    out << ", access::sample";
                    return true;
                },
                [&](Default) {
                    diagnostics_.add_error(diag::System::Writer, "invalid texture type");
                    return false;
                });
        },
        [&](const type::U32*) {
            out << "uint";
            return true;
        },
        [&](const type::Vector* vec) {
            if (vec->Packed()) {
                out << "packed_";
            }
            if (!EmitType(out, vec->type())) {
                return false;
            }
            out << vec->Width();
            return true;
        },
        [&](const type::Void*) {
            out << "void";
            return true;
        },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer,
                                   "unknown type in EmitType: " + type->FriendlyName());
            return false;
        });
}

bool ASTPrinter::EmitTypeAndName(StringStream& out,
                                 const type::Type* type,
                                 const std::string& name) {
    if (!EmitType(out, type)) {
        return false;
    }
    out << " " << name;
    return true;
}

bool ASTPrinter::EmitAddressSpace(StringStream& out, builtin::AddressSpace sc) {
    switch (sc) {
        case builtin::AddressSpace::kFunction:
        case builtin::AddressSpace::kPrivate:
        case builtin::AddressSpace::kHandle:
            out << "thread";
            return true;
        case builtin::AddressSpace::kWorkgroup:
            out << "threadgroup";
            return true;
        case builtin::AddressSpace::kStorage:
            out << "device";
            return true;
        case builtin::AddressSpace::kUniform:
            out << "constant";
            return true;
        default:
            break;
    }
    TINT_ICE() << "unhandled address space: " << sc;
    return false;
}

bool ASTPrinter::EmitStructType(TextBuffer* b, const type::Struct* str) {
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
                return false;
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
                diagnostics_.add_error(diag::System::Writer, "unknown builtin");
                return false;
            }
            out << " [[" << name << "]]";
        }

        if (auto location = attributes.location) {
            auto& pipeline_stage_uses = str->PipelineStageUses();
            if (TINT_UNLIKELY(pipeline_stage_uses.size() != 1)) {
                TINT_ICE() << "invalid entry point IO struct uses";
                return false;
            }

            if (pipeline_stage_uses.count(type::PipelineStageUsage::kVertexInput)) {
                out << " [[attribute(" + std::to_string(location.value()) + ")]]";
            } else if (pipeline_stage_uses.count(type::PipelineStageUsage::kVertexOutput)) {
                out << " [[user(locn" + std::to_string(location.value()) + ")]]";
            } else if (pipeline_stage_uses.count(type::PipelineStageUsage::kFragmentInput)) {
                out << " [[user(locn" + std::to_string(location.value()) + ")]]";
            } else if (TINT_LIKELY(
                           pipeline_stage_uses.count(type::PipelineStageUsage::kFragmentOutput))) {
                if (auto index = attributes.index) {
                    out << " [[color(" + std::to_string(location.value()) + ") index(" +
                               std::to_string(index.value()) + ")]]";
                } else {
                    out << " [[color(" + std::to_string(location.value()) + ")]]";
                }
            } else {
                TINT_ICE() << "invalid use of location decoration";
                return false;
            }
        }

        if (auto interpolation = attributes.interpolation) {
            auto name = InterpolationToAttribute(interpolation->type, interpolation->sampling);
            if (name.empty()) {
                diagnostics_.add_error(diag::System::Writer, "unknown interpolation attribute");
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
            auto size_align = MslPackedTypeSizeAndAlign(diagnostics_, ty);
            if (TINT_UNLIKELY(msl_offset % size_align.align)) {
                TINT_ICE() << "Misaligned MSL structure member " << ty->FriendlyName() << " "
                           << mem_name;
                return false;
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
    if (expr->op == ast::UnaryOp::kNegation && expr_type->is_signed_integer_scalar_or_vector()) {
        auto fn = tint::GetOrCreate(unary_minus_funcs_, expr_type, [&]() -> std::string {
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
        case ast::UnaryOp::kAddressOf:
            out << "&";
            break;
        case ast::UnaryOp::kComplement:
            out << "~";
            break;
        case ast::UnaryOp::kIndirection:
            out << "*";
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

bool ASTPrinter::EmitVar(const ast::Var* var) {
    auto* sem = builder_.Sem().Get(var);
    auto* type = sem->Type()->UnwrapRef();

    auto out = Line();

    switch (sem->AddressSpace()) {
        case builtin::AddressSpace::kFunction:
        case builtin::AddressSpace::kHandle:
            break;
        case builtin::AddressSpace::kPrivate:
            out << "thread ";
            break;
        case builtin::AddressSpace::kWorkgroup:
            out << "threadgroup ";
            break;
        default:
            TINT_ICE() << "unhandled variable address space";
            return false;
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
    } else if (sem->AddressSpace() == builtin::AddressSpace::kPrivate ||
               sem->AddressSpace() == builtin::AddressSpace::kFunction ||
               sem->AddressSpace() == builtin::AddressSpace::kUndefined) {
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
        case builtin::AddressSpace::kFunction:
        case builtin::AddressSpace::kHandle:
        case builtin::AddressSpace::kUndefined:
            break;
        case builtin::AddressSpace::kPrivate:
            out << "thread ";
            break;
        case builtin::AddressSpace::kWorkgroup:
            out << "threadgroup ";
            break;
        default:
            TINT_ICE() << "unhandled variable address space";
            return false;
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

template <typename F>
bool ASTPrinter::CallBuiltinHelper(StringStream& out,
                                   const ast::CallExpression* call,
                                   const sem::Builtin* builtin,
                                   F&& build) {
    // Generate the helper function if it hasn't been created already
    auto fn = tint::GetOrCreate(builtins_, builtin, [&]() -> std::string {
        TextBuffer b;
        TINT_DEFER(helpers_.Append(b));

        auto fn_name = UniqueIdentifier(std::string("tint_") + builtin::str(builtin->Type()));
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

std::string ASTPrinter::StructName(const type::Struct* s) {
    auto name = s->Name().Name();
    if (HasPrefix(name, "__")) {
        name = tint::GetOrCreate(builtin_struct_names_, s,
                                 [&] { return UniqueIdentifier(name.substr(2)); });
    }
    return name;
}

std::string ASTPrinter::UniqueIdentifier(const std::string& prefix /* = "" */) {
    return builder_.Symbols().New(prefix).Name();
}

}  // namespace tint::msl::writer
