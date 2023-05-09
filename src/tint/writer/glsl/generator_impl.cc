/// Copyright 2021 The Tint Authors.
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

#include "src/tint/writer/glsl/generator_impl.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
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
#include "src/tint/transform/add_block_attribute.h"
#include "src/tint/transform/add_empty_entry_point.h"
#include "src/tint/transform/binding_remapper.h"
#include "src/tint/transform/builtin_polyfill.h"
#include "src/tint/transform/canonicalize_entry_point_io.h"
#include "src/tint/transform/combine_samplers.h"
#include "src/tint/transform/decompose_memory_access.h"
#include "src/tint/transform/demote_to_helper.h"
#include "src/tint/transform/direct_variable_access.h"
#include "src/tint/transform/disable_uniformity_analysis.h"
#include "src/tint/transform/expand_compound_assignment.h"
#include "src/tint/transform/manager.h"
#include "src/tint/transform/multiplanar_external_texture.h"
#include "src/tint/transform/pad_structs.h"
#include "src/tint/transform/preserve_padding.h"
#include "src/tint/transform/promote_initializers_to_let.h"
#include "src/tint/transform/promote_side_effects_to_decl.h"
#include "src/tint/transform/remove_phonies.h"
#include "src/tint/transform/renamer.h"
#include "src/tint/transform/robustness.h"
#include "src/tint/transform/simplify_pointers.h"
#include "src/tint/transform/single_entry_point.h"
#include "src/tint/transform/std140.h"
#include "src/tint/transform/texture_1d_to_2d.h"
#include "src/tint/transform/unshadow.h"
#include "src/tint/transform/zero_init_workgroup_memory.h"
#include "src/tint/type/array.h"
#include "src/tint/type/atomic.h"
#include "src/tint/type/depth_multisampled_texture.h"
#include "src/tint/type/depth_texture.h"
#include "src/tint/type/multisampled_texture.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/storage_texture.h"
#include "src/tint/type/texture_dimension.h"
#include "src/tint/utils/defer.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/scoped_assignment.h"
#include "src/tint/utils/string.h"
#include "src/tint/utils/string_stream.h"
#include "src/tint/writer/append_vector.h"
#include "src/tint/writer/float_to_string.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::glsl {
namespace {

const char kTempNamePrefix[] = "tint_tmp";

bool last_is_break(const ast::BlockStatement* stmts) {
    return utils::IsAnyOf<ast::BreakStatement>(stmts->Last());
}

bool IsRelational(tint::ast::BinaryOp op) {
    return op == tint::ast::BinaryOp::kEqual || op == tint::ast::BinaryOp::kNotEqual ||
           op == tint::ast::BinaryOp::kLessThan || op == tint::ast::BinaryOp::kGreaterThan ||
           op == tint::ast::BinaryOp::kLessThanEqual ||
           op == tint::ast::BinaryOp::kGreaterThanEqual;
}

bool RequiresOESSampleVariables(tint::builtin::BuiltinValue builtin) {
    switch (builtin) {
        case tint::builtin::BuiltinValue::kSampleIndex:
        case tint::builtin::BuiltinValue::kSampleMask:
            return true;
        default:
            return false;
    }
}

void PrintI32(utils::StringStream& out, int32_t value) {
    // GLSL parses `-2147483648` as a unary minus and `2147483648` as separate tokens, and the
    // latter doesn't fit into an (32-bit) `int`. Emit `(-2147483647 - 1)` instead, which ensures
    // the expression type is `int`.
    if (auto int_min = std::numeric_limits<int32_t>::min(); value == int_min) {
        out << "(" << int_min + 1 << " - 1)";
    } else {
        out << value;
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
        out << "0.0hf " << (value >= 0 ? "/* inf */" : "/* -inf */");
    } else if (std::isnan(value)) {
        out << "0.0hf /* nan */";
    } else {
        out << FloatToString(value) << "hf";
    }
}

}  // namespace

SanitizedResult::SanitizedResult() = default;
SanitizedResult::~SanitizedResult() = default;
SanitizedResult::SanitizedResult(SanitizedResult&&) = default;

SanitizedResult Sanitize(const Program* in,
                         const Options& options,
                         const std::string& entry_point) {
    transform::Manager manager;
    transform::DataMap data;

    manager.Add<transform::DisableUniformityAnalysis>();

    // ExpandCompoundAssignment must come before BuiltinPolyfill
    manager.Add<transform::ExpandCompoundAssignment>();

    if (!entry_point.empty()) {
        manager.Add<transform::SingleEntryPoint>();
        data.Add<transform::SingleEntryPoint::Config>(entry_point);
    }
    manager.Add<transform::Renamer>();
    data.Add<transform::Renamer::Config>(transform::Renamer::Target::kGlslKeywords,
                                         /* preserve_unicode */ false);

    manager.Add<transform::PreservePadding>();  // Must come before DirectVariableAccess

    manager.Add<transform::Unshadow>();  // Must come before DirectVariableAccess

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

    {  // Builtin polyfills
        transform::BuiltinPolyfill::Builtins polyfills;
        polyfills.acosh = transform::BuiltinPolyfill::Level::kRangeCheck;
        polyfills.atanh = transform::BuiltinPolyfill::Level::kRangeCheck;
        polyfills.bgra8unorm = true;
        polyfills.bitshift_modulo = true;
        polyfills.conv_f32_to_iu32 = true;
        polyfills.count_leading_zeros = true;
        polyfills.count_trailing_zeros = true;
        polyfills.extract_bits = transform::BuiltinPolyfill::Level::kClampParameters;
        polyfills.first_leading_bit = true;
        polyfills.first_trailing_bit = true;
        polyfills.insert_bits = transform::BuiltinPolyfill::Level::kClampParameters;
        polyfills.int_div_mod = true;
        polyfills.saturate = true;
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

    // PadStructs must come after CanonicalizeEntryPointIO
    manager.Add<transform::PadStructs>();

    // DemoteToHelper must come after PromoteSideEffectsToDecl and ExpandCompoundAssignment.
    manager.Add<transform::DemoteToHelper>();

    manager.Add<transform::RemovePhonies>();

    data.Add<transform::CombineSamplers::BindingInfo>(options.binding_map,
                                                      options.placeholder_binding_point);
    manager.Add<transform::CombineSamplers>();

    data.Add<transform::BindingRemapper::Remappings>(
        options.binding_points, options.access_controls, options.allow_collisions);
    manager.Add<transform::BindingRemapper>();

    manager.Add<transform::PromoteInitializersToLet>();
    manager.Add<transform::AddEmptyEntryPoint>();
    manager.Add<transform::AddBlockAttribute>();

    // Std140 must come after PromoteSideEffectsToDecl and before SimplifyPointers.
    manager.Add<transform::Std140>();

    manager.Add<transform::Texture1DTo2D>();

    manager.Add<transform::SimplifyPointers>();

    data.Add<transform::CanonicalizeEntryPointIO::Config>(
        transform::CanonicalizeEntryPointIO::ShaderStyle::kGlsl);

    auto out = manager.Run(in, data);

    SanitizedResult result;
    result.program = std::move(out.program);
    return result;
}

GeneratorImpl::GeneratorImpl(const Program* program, const Version& version)
    : TextGenerator(program), version_(version) {}

GeneratorImpl::~GeneratorImpl() = default;

void GeneratorImpl::Generate() {
    {
        auto out = line();
        out << "#version " << version_.major_version << version_.minor_version << "0";
        if (version_.IsES()) {
            out << " es";
        }
    }

    auto helpers_insertion_point = current_buffer_->lines.size();

    line();

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
                if (auto* arr = sem->Members().Back()->Type()->As<type::Array>()) {
                    has_rt_arr = arr->Count()->Is<type::RuntimeArrayCount>();
                }
                bool is_block = ast::HasAttribute<transform::AddBlockAttribute::BlockAttribute>(
                    str->attributes);
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
            },
            [&](Default) {
                TINT_ICE(Writer, diagnostics_)
                    << "unhandled module-scope declaration: " << decl->TypeInfo().name;
            });
    }

    TextBuffer extensions;

    if (version_.IsES() && requires_oes_sample_variables_) {
        extensions.Append("#extension GL_OES_sample_variables : require");
    }

    if (requires_f16_extension_) {
        extensions.Append("#extension GL_AMD_gpu_shader_half_float : require");
    }

    auto indent = current_buffer_->current_indent;

    if (!extensions.lines.empty()) {
        current_buffer_->Insert(extensions, helpers_insertion_point, indent);
        helpers_insertion_point += extensions.lines.size();
    }

    if (version_.IsES() && requires_default_precision_qualifier_) {
        current_buffer_->Insert("precision highp float;", helpers_insertion_point++, indent);
    }

    if (!helpers_.lines.empty()) {
        current_buffer_->Insert("", helpers_insertion_point++, indent);
        current_buffer_->Insert(helpers_, helpers_insertion_point, indent);
        helpers_insertion_point += helpers_.lines.size();
    }
}

void GeneratorImpl::RecordExtension(const ast::Enable* enable) {
    // Deal with extension node here, recording it within the generator for later emition.

    if (enable->HasExtension(builtin::Extension::kF16)) {
        requires_f16_extension_ = true;
    }
}

void GeneratorImpl::EmitIndexAccessor(utils::StringStream& out,
                                      const ast::IndexAccessorExpression* expr) {
    EmitExpression(out, expr->object);
    out << "[";
    EmitExpression(out, expr->index);
    out << "]";
}

void GeneratorImpl::EmitBitcast(utils::StringStream& out, const ast::BitcastExpression* expr) {
    auto* src_type = TypeOf(expr->expr)->UnwrapRef();
    auto* dst_type = TypeOf(expr)->UnwrapRef();

    if (!dst_type->is_integer_scalar_or_vector() && !dst_type->is_float_scalar_or_vector()) {
        diagnostics_.add_error(diag::System::Writer,
                               "Unable to do bitcast to type " + dst_type->FriendlyName());
        return;
    }

    if (src_type == dst_type) {
        return EmitExpression(out, expr->expr);
    }

    if (src_type->is_float_scalar_or_vector() && dst_type->is_signed_integer_scalar_or_vector()) {
        out << "floatBitsToInt";
    } else if (src_type->is_float_scalar_or_vector() &&
               dst_type->is_unsigned_integer_scalar_or_vector()) {
        out << "floatBitsToUint";
    } else if (src_type->is_signed_integer_scalar_or_vector() &&
               dst_type->is_float_scalar_or_vector()) {
        out << "intBitsToFloat";
    } else if (src_type->is_unsigned_integer_scalar_or_vector() &&
               dst_type->is_float_scalar_or_vector()) {
        out << "uintBitsToFloat";
    } else {
        EmitType(out, dst_type, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
    }
    ScopedParen sp(out);
    EmitExpression(out, expr->expr);
}

void GeneratorImpl::EmitAssign(const ast::AssignmentStatement* stmt) {
    auto out = line();
    EmitExpression(out, stmt->lhs);
    out << " = ";
    EmitExpression(out, stmt->rhs);
    out << ";";
}

void GeneratorImpl::EmitVectorRelational(utils::StringStream& out,
                                         const ast::BinaryExpression* expr) {
    switch (expr->op) {
        case ast::BinaryOp::kEqual:
            out << "equal";
            break;
        case ast::BinaryOp::kNotEqual:
            out << "notEqual";
            break;
        case ast::BinaryOp::kLessThan:
            out << "lessThan";
            break;
        case ast::BinaryOp::kGreaterThan:
            out << "greaterThan";
            break;
        case ast::BinaryOp::kLessThanEqual:
            out << "lessThanEqual";
            break;
        case ast::BinaryOp::kGreaterThanEqual:
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

void GeneratorImpl::EmitBitwiseBoolOp(utils::StringStream& out, const ast::BinaryExpression* expr) {
    auto* bool_type = TypeOf(expr->lhs)->UnwrapRef();
    auto* uint_type = BoolTypeToUint(bool_type);

    // Cast result to bool scalar or vector type.
    EmitType(out, bool_type, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
    ScopedParen outerCastParen(out);
    // Cast LHS to uint scalar or vector type.
    EmitType(out, uint_type, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
    {
        ScopedParen innerCastParen(out);
        // Emit LHS.
        EmitExpression(out, expr->lhs);
    }
    // Emit operator.
    if (expr->op == ast::BinaryOp::kAnd) {
        out << " & ";
    } else if (TINT_LIKELY(expr->op == ast::BinaryOp::kOr)) {
        out << " | ";
    } else {
        TINT_ICE(Writer, diagnostics_) << "unexpected binary op: " << FriendlyName(expr->op);
        return;
    }

    // Cast RHS to uint scalar or vector type.
    EmitType(out, uint_type, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
    {
        ScopedParen innerCastParen(out);
        // Emit RHS.
        EmitExpression(out, expr->rhs);
    }
}

void GeneratorImpl::EmitFloatModulo(utils::StringStream& out, const ast::BinaryExpression* expr) {
    std::string fn;
    auto* ret_ty = TypeOf(expr)->UnwrapRef();
    auto* lhs_ty = TypeOf(expr->lhs)->UnwrapRef();
    auto* rhs_ty = TypeOf(expr->rhs)->UnwrapRef();
    fn = utils::GetOrCreate(float_modulo_funcs_, BinaryOperandType{{lhs_ty, rhs_ty}},
                            [&]() -> std::string {
                                TextBuffer b;
                                TINT_DEFER(helpers_.Append(b));

                                auto fn_name = UniqueIdentifier("tint_float_modulo");
                                std::vector<std::string> parameter_names;
                                {
                                    auto decl = line(&b);
                                    EmitTypeAndName(decl, ret_ty, builtin::AddressSpace::kUndefined,
                                                    builtin::Access::kUndefined, fn_name);
                                    {
                                        ScopedParen sp(decl);
                                        const auto* ty = TypeOf(expr->lhs)->UnwrapRef();
                                        EmitTypeAndName(decl, ty, builtin::AddressSpace::kUndefined,
                                                        builtin::Access::kUndefined, "lhs");
                                        decl << ", ";
                                        ty = TypeOf(expr->rhs)->UnwrapRef();
                                        EmitTypeAndName(decl, ty, builtin::AddressSpace::kUndefined,
                                                        builtin::Access::kUndefined, "rhs");
                                    }
                                    decl << " {";
                                }
                                {
                                    ScopedIndent si(&b);
                                    line(&b) << "return (lhs - rhs * trunc(lhs / rhs));";
                                }
                                line(&b) << "}";
                                line(&b);
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

void GeneratorImpl::EmitBinary(utils::StringStream& out, const ast::BinaryExpression* expr) {
    if (IsRelational(expr->op) && !TypeOf(expr->lhs)->UnwrapRef()->is_scalar()) {
        EmitVectorRelational(out, expr);
        return;
    }

    if (expr->op == ast::BinaryOp::kLogicalAnd || expr->op == ast::BinaryOp::kLogicalOr) {
        auto name = UniqueIdentifier(kTempNamePrefix);

        {
            auto pre = line();
            pre << "bool " << name << " = ";
            EmitExpression(pre, expr->lhs);
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
            EmitExpression(pre, expr->rhs);
            pre << ";";
        }

        line() << "}";

        out << "(" << name << ")";
        return;
    }

    if ((expr->op == ast::BinaryOp::kAnd || expr->op == ast::BinaryOp::kOr) &&
        TypeOf(expr->lhs)->UnwrapRef()->is_bool_scalar_or_vector()) {
        EmitBitwiseBoolOp(out, expr);
        return;
    }

    if (expr->op == ast::BinaryOp::kModulo &&
        (TypeOf(expr->lhs)->UnwrapRef()->is_float_scalar_or_vector() ||
         TypeOf(expr->rhs)->UnwrapRef()->is_float_scalar_or_vector())) {
        EmitFloatModulo(out, expr);
        return;
    }

    ScopedParen sp(out);
    EmitExpression(out, expr->lhs);
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
            return;
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
            return;
    }
    out << " ";
    EmitExpression(out, expr->rhs);
}

void GeneratorImpl::EmitStatements(utils::VectorRef<const ast::Statement*> stmts) {
    for (auto* s : stmts) {
        EmitStatement(s);
    }
}

void GeneratorImpl::EmitStatementsWithIndent(utils::VectorRef<const ast::Statement*> stmts) {
    ScopedIndent si(this);
    EmitStatements(stmts);
}

void GeneratorImpl::EmitBlock(const ast::BlockStatement* stmt) {
    line() << "{";
    EmitStatementsWithIndent(stmt->statements);
    line() << "}";
}

void GeneratorImpl::EmitBreak(const ast::BreakStatement*) {
    line() << "break;";
}

void GeneratorImpl::EmitBreakIf(const ast::BreakIfStatement* b) {
    auto out = line();
    out << "if (";
    EmitExpression(out, b->condition);
    out << ") { break; }";
}

void GeneratorImpl::EmitCall(utils::StringStream& out, const ast::CallExpression* expr) {
    auto* call = builder_.Sem().Get<sem::Call>(expr);
    Switch(
        call->Target(),  //
        [&](const sem::Function* fn) { EmitFunctionCall(out, call, fn); },
        [&](const sem::Builtin* builtin) { EmitBuiltinCall(out, call, builtin); },
        [&](const sem::ValueConversion* conv) { EmitValueConversion(out, call, conv); },
        [&](const sem::ValueConstructor* ctor) { EmitValueConstructor(out, call, ctor); },
        [&](Default) {
            TINT_ICE(Writer, diagnostics_)
                << "unhandled call target: " << call->Target()->TypeInfo().name;
        });
}

void GeneratorImpl::EmitFunctionCall(utils::StringStream& out,
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

void GeneratorImpl::EmitBuiltinCall(utils::StringStream& out,
                                    const sem::Call* call,
                                    const sem::Builtin* builtin) {
    auto* expr = call->Declaration();
    if (builtin->IsTexture()) {
        EmitTextureCall(out, call, builtin);
    } else if (builtin->Type() == builtin::Function::kCountOneBits) {
        EmitCountOneBitsCall(out, expr);
    } else if (builtin->Type() == builtin::Function::kSelect) {
        EmitSelectCall(out, expr, builtin);
    } else if (builtin->Type() == builtin::Function::kDot) {
        EmitDotCall(out, expr, builtin);
    } else if (builtin->Type() == builtin::Function::kModf) {
        EmitModfCall(out, expr, builtin);
    } else if (builtin->Type() == builtin::Function::kFrexp) {
        EmitFrexpCall(out, expr, builtin);
    } else if (builtin->Type() == builtin::Function::kDegrees) {
        EmitDegreesCall(out, expr, builtin);
    } else if (builtin->Type() == builtin::Function::kRadians) {
        EmitRadiansCall(out, expr, builtin);
    } else if (builtin->Type() == builtin::Function::kQuantizeToF16) {
        EmitQuantizeToF16Call(out, expr, builtin);
    } else if (builtin->Type() == builtin::Function::kArrayLength) {
        EmitArrayLength(out, expr);
    } else if (builtin->Type() == builtin::Function::kExtractBits) {
        EmitExtractBits(out, expr);
    } else if (builtin->Type() == builtin::Function::kInsertBits) {
        EmitInsertBits(out, expr);
    } else if (builtin->Type() == builtin::Function::kFma && version_.IsES()) {
        EmitEmulatedFMA(out, expr);
    } else if (builtin->Type() == builtin::Function::kAbs &&
               TypeOf(expr->args[0])->UnwrapRef()->is_unsigned_integer_scalar_or_vector()) {
        // GLSL does not support abs() on unsigned arguments. However, it's a no-op.
        EmitExpression(out, expr->args[0]);
    } else if ((builtin->Type() == builtin::Function::kAny ||
                builtin->Type() == builtin::Function::kAll) &&
               TypeOf(expr->args[0])->UnwrapRef()->is_scalar()) {
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

void GeneratorImpl::EmitValueConversion(utils::StringStream& out,
                                        const sem::Call* call,
                                        const sem::ValueConversion* conv) {
    EmitType(out, conv->Target(), builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite,
             "");
    ScopedParen sp(out);
    EmitExpression(out, call->Arguments()[0]->Declaration());
}

void GeneratorImpl::EmitValueConstructor(utils::StringStream& out,
                                         const sem::Call* call,
                                         const sem::ValueConstructor* ctor) {
    auto* type = ctor->ReturnType();

    // If the value constructor is empty then we need to construct with the zero value for all
    // components.
    if (call->Arguments().IsEmpty()) {
        EmitZeroValue(out, type);
        return;
    }

    EmitType(out, type, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
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

void GeneratorImpl::EmitWorkgroupAtomicCall(utils::StringStream& out,
                                            const ast::CallExpression* expr,
                                            const sem::Builtin* builtin) {
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

    switch (builtin->Type()) {
        case builtin::Function::kAtomicLoad: {
            // GLSL does not have an atomicLoad, so we emulate it with
            // atomicOr using 0 as the OR value
            out << "atomicOr";
            {
                ScopedParen sp(out);
                EmitExpression(out, expr->args[0]);
                out << ", 0";
                if (builtin->ReturnType()->Is<type::U32>()) {
                    out << "u";
                }
            }
            return;
        }
        case builtin::Function::kAtomicCompareExchangeWeak: {
            EmitStructType(&helpers_, builtin->ReturnType()->As<type::Struct>());

            auto* dest = expr->args[0];
            auto* compare_value = expr->args[1];
            auto* value = expr->args[2];

            std::string result = UniqueIdentifier("atomic_compare_result");

            {
                auto pre = line();
                EmitTypeAndName(pre, builtin->ReturnType(), builtin::AddressSpace::kUndefined,
                                builtin::Access::kUndefined, result);
                pre << ";";
            }
            {
                auto pre = line();
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
                auto pre = line();
                pre << result << ".exchanged = " << result << ".old_value == ";
                EmitExpression(pre, compare_value);
                pre << ";";
            }

            out << result;
            return;
        }

        case builtin::Function::kAtomicAdd:
        case builtin::Function::kAtomicSub:
            call("atomicAdd");
            return;

        case builtin::Function::kAtomicMax:
            call("atomicMax");
            return;

        case builtin::Function::kAtomicMin:
            call("atomicMin");
            return;

        case builtin::Function::kAtomicAnd:
            call("atomicAnd");
            return;

        case builtin::Function::kAtomicOr:
            call("atomicOr");
            return;

        case builtin::Function::kAtomicXor:
            call("atomicXor");
            return;

        case builtin::Function::kAtomicExchange:
        case builtin::Function::kAtomicStore:
            // GLSL does not have an atomicStore, so we emulate it with
            // atomicExchange.
            call("atomicExchange");
            return;

        default:
            break;
    }

    TINT_UNREACHABLE(Writer, diagnostics_) << "unsupported atomic builtin: " << builtin->Type();
}

void GeneratorImpl::EmitArrayLength(utils::StringStream& out, const ast::CallExpression* expr) {
    out << "uint(";
    EmitExpression(out, expr->args[0]);
    out << ".length())";
}

void GeneratorImpl::EmitExtractBits(utils::StringStream& out, const ast::CallExpression* expr) {
    out << "bitfieldExtract(";
    EmitExpression(out, expr->args[0]);
    out << ", int(";
    EmitExpression(out, expr->args[1]);
    out << "), int(";
    EmitExpression(out, expr->args[2]);
    out << "))";
}

void GeneratorImpl::EmitInsertBits(utils::StringStream& out, const ast::CallExpression* expr) {
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

void GeneratorImpl::EmitEmulatedFMA(utils::StringStream& out, const ast::CallExpression* expr) {
    out << "((";
    EmitExpression(out, expr->args[0]);
    out << ") * (";
    EmitExpression(out, expr->args[1]);
    out << ") + (";
    EmitExpression(out, expr->args[2]);
    out << "))";
}

void GeneratorImpl::EmitCountOneBitsCall(utils::StringStream& out,
                                         const ast::CallExpression* expr) {
    // GLSL's bitCount returns an integer type, so cast it to the appropriate
    // unsigned type.
    EmitType(out, TypeOf(expr)->UnwrapRef(), builtin::AddressSpace::kUndefined,
             builtin::Access::kReadWrite, "");
    out << "(bitCount(";
    EmitExpression(out, expr->args[0]);
    out << "))";
}

void GeneratorImpl::EmitSelectCall(utils::StringStream& out,
                                   const ast::CallExpression* expr,
                                   const sem::Builtin* builtin) {
    // GLSL does not support ternary expressions with a bool vector conditional,
    // so polyfill with a helper.
    if (auto* vec = builtin->Parameters()[2]->Type()->As<type::Vector>()) {
        CallBuiltinHelper(out, expr, builtin,
                          [&](TextBuffer* b, const std::vector<std::string>& params) {
                              auto l = line(b);
                              l << "  return ";
                              EmitType(l, builtin->ReturnType(), builtin::AddressSpace::kUndefined,
                                       builtin::Access::kUndefined, "");
                              {
                                  ScopedParen sp(l);
                                  for (uint32_t i = 0; i < vec->Width(); i++) {
                                      if (i > 0) {
                                          l << ", ";
                                      }
                                      l << params[2] << "[" << i << "] ? " << params[1] << "[" << i
                                        << "] : " << params[0] << "[" << i << "]";
                                  }
                              }
                              l << ";";
                          });
        return;
    }

    auto* expr_false = expr->args[0];
    auto* expr_true = expr->args[1];
    auto* expr_cond = expr->args[2];

    ScopedParen paren(out);
    EmitExpression(out, expr_cond);

    out << " ? ";
    EmitExpression(out, expr_true);
    out << " : ";
    EmitExpression(out, expr_false);
}

void GeneratorImpl::EmitDotCall(utils::StringStream& out,
                                const ast::CallExpression* expr,
                                const sem::Builtin* builtin) {
    auto* vec_ty = builtin->Parameters()[0]->Type()->As<type::Vector>();
    std::string fn = "dot";
    if (vec_ty->type()->is_integer_scalar()) {
        // GLSL does not have a builtin for dot() with integer vector types.
        // Generate the helper function if it hasn't been created already
        fn = utils::GetOrCreate(int_dot_funcs_, vec_ty, [&]() -> std::string {
            TextBuffer b;
            TINT_DEFER(helpers_.Append(b));

            auto fn_name = UniqueIdentifier("tint_int_dot");

            std::string v;
            {
                utils::StringStream s;
                EmitType(s, vec_ty->type(), builtin::AddressSpace::kUndefined,
                         builtin::Access::kRead, "");
                v = s.str();
            }
            {  // (u)int tint_int_dot([i|u]vecN a, [i|u]vecN b) {
                auto l = line(&b);
                EmitType(l, vec_ty->type(), builtin::AddressSpace::kUndefined,
                         builtin::Access::kRead, "");
                l << " " << fn_name << "(";
                EmitType(l, vec_ty, builtin::AddressSpace::kUndefined, builtin::Access::kRead, "");
                l << " a, ";
                EmitType(l, vec_ty, builtin::AddressSpace::kUndefined, builtin::Access::kRead, "");
                l << " b) {";
            }
            {
                auto l = line(&b);
                l << "  return ";
                for (uint32_t i = 0; i < vec_ty->Width(); i++) {
                    if (i > 0) {
                        l << " + ";
                    }
                    l << "a[" << i << "]*b[" << i << "]";
                }
                l << ";";
            }
            line(&b) << "}";
            return fn_name;
        });
    }

    out << fn;
    ScopedParen sp(out);

    EmitExpression(out, expr->args[0]);
    out << ", ";
    EmitExpression(out, expr->args[1]);
}

void GeneratorImpl::EmitModfCall(utils::StringStream& out,
                                 const ast::CallExpression* expr,
                                 const sem::Builtin* builtin) {
    TINT_ASSERT(Writer, expr->args.Length() == 1);
    CallBuiltinHelper(out, expr, builtin,
                      [&](TextBuffer* b, const std::vector<std::string>& params) {
                          // Emit the builtin return type unique to this overload. This does not
                          // exist in the AST, so it will not be generated in Generate().
                          EmitStructType(&helpers_, builtin->ReturnType()->As<type::Struct>());

                          {
                              auto l = line(b);
                              EmitType(l, builtin->ReturnType(), builtin::AddressSpace::kUndefined,
                                       builtin::Access::kUndefined, "");
                              l << " result;";
                          }
                          line(b) << "result.fract = modf(" << params[0] << ", result.whole);";
                          line(b) << "return result;";
                      });
}

void GeneratorImpl::EmitFrexpCall(utils::StringStream& out,
                                  const ast::CallExpression* expr,
                                  const sem::Builtin* builtin) {
    TINT_ASSERT(Writer, expr->args.Length() == 1);
    CallBuiltinHelper(out, expr, builtin,
                      [&](TextBuffer* b, const std::vector<std::string>& params) {
                          // Emit the builtin return type unique to this overload. This does not
                          // exist in the AST, so it will not be generated in Generate().
                          EmitStructType(&helpers_, builtin->ReturnType()->As<type::Struct>());

                          {
                              auto l = line(b);
                              EmitType(l, builtin->ReturnType(), builtin::AddressSpace::kUndefined,
                                       builtin::Access::kUndefined, "");
                              l << " result;";
                          }
                          line(b) << "result.fract = frexp(" << params[0] << ", result.exp);";
                          line(b) << "return result;";
                      });
}

void GeneratorImpl::EmitDegreesCall(utils::StringStream& out,
                                    const ast::CallExpression* expr,
                                    const sem::Builtin* builtin) {
    auto* return_elem_type = type::Type::DeepestElementOf(builtin->ReturnType());
    const std::string suffix = Is<type::F16>(return_elem_type) ? "hf" : "f";
    CallBuiltinHelper(out, expr, builtin,
                      [&](TextBuffer* b, const std::vector<std::string>& params) {
                          line(b) << "return " << params[0] << " * " << std::setprecision(20)
                                  << sem::kRadToDeg << suffix << ";";
                      });
}

void GeneratorImpl::EmitRadiansCall(utils::StringStream& out,
                                    const ast::CallExpression* expr,
                                    const sem::Builtin* builtin) {
    auto* return_elem_type = type::Type::DeepestElementOf(builtin->ReturnType());
    const std::string suffix = Is<type::F16>(return_elem_type) ? "hf" : "f";
    CallBuiltinHelper(out, expr, builtin,
                      [&](TextBuffer* b, const std::vector<std::string>& params) {
                          line(b) << "return " << params[0] << " * " << std::setprecision(20)
                                  << sem::kDegToRad << suffix << ";";
                      });
}

void GeneratorImpl::EmitQuantizeToF16Call(utils::StringStream& out,
                                          const ast::CallExpression* expr,
                                          const sem::Builtin* builtin) {
    // Emulate by casting to f16 and back again.
    CallBuiltinHelper(
        out, expr, builtin, [&](TextBuffer* b, const std::vector<std::string>& params) {
            const auto v = params[0];
            if (auto* vec = builtin->ReturnType()->As<type::Vector>()) {
                switch (vec->Width()) {
                    case 2: {
                        line(b) << "return unpackHalf2x16(packHalf2x16(" << v << "));";
                        return;
                    }
                    case 3: {
                        line(b) << "return vec3(";
                        line(b) << "  unpackHalf2x16(packHalf2x16(" << v << ".xy)),";
                        line(b) << "  unpackHalf2x16(packHalf2x16(" << v << ".zz)).x);";
                        return;
                    }
                    default: {
                        line(b) << "return vec4(";
                        line(b) << "  unpackHalf2x16(packHalf2x16(" << v << ".xy)),";
                        line(b) << "  unpackHalf2x16(packHalf2x16(" << v << ".zw)));";
                        return;
                    }
                }
            }
            line(b) << "return unpackHalf2x16(packHalf2x16(vec2(" << v << "))).x;";
        });
}

void GeneratorImpl::EmitBarrierCall(utils::StringStream& out, const sem::Builtin* builtin) {
    // TODO(crbug.com/tint/661): Combine sequential barriers to a single
    // instruction.
    if (builtin->Type() == builtin::Function::kWorkgroupBarrier) {
        out << "barrier()";
    } else if (builtin->Type() == builtin::Function::kStorageBarrier) {
        out << "{ barrier(); memoryBarrierBuffer(); }";
    } else {
        TINT_UNREACHABLE(Writer, diagnostics_)
            << "unexpected barrier builtin type " << builtin::str(builtin->Type());
    }
}

const ast::Expression* GeneratorImpl::CreateF32Zero(const sem::Statement* stmt) {
    auto* zero = builder_.Expr(0_f);
    auto* f32 = builder_.create<type::F32>();
    auto* sem_zero = builder_.create<sem::ValueExpression>(
        zero, f32, sem::EvaluationStage::kRuntime, stmt, /* constant_value */ nullptr,
        /* has_side_effects */ false);
    builder_.Sem().Add(zero, sem_zero);
    return zero;
}

void GeneratorImpl::EmitTextureCall(utils::StringStream& out,
                                    const sem::Call* call,
                                    const sem::Builtin* builtin) {
    using Usage = sem::ParameterUsage;

    auto& signature = builtin->Signature();
    auto* expr = call->Declaration();
    auto arguments = expr->args;

    // Returns the argument with the given usage
    auto arg = [&](Usage usage) {
        auto idx = signature.IndexOf(usage);
        return (idx >= 0) ? arguments[static_cast<size_t>(idx)] : nullptr;
    };

    auto* texture = arg(Usage::kTexture);
    if (TINT_UNLIKELY(!texture)) {
        TINT_ICE(Writer, diagnostics_) << "missing texture argument";
        return;
    }

    auto* texture_type = TypeOf(texture)->UnwrapRef()->As<type::Texture>();

    auto emit_signed_int_type = [&](const type::Type* ty) {
        uint32_t width = 0;
        type::Type::ElementOf(ty, &width);
        if (width > 1) {
            out << "ivec" << width;
        } else {
            out << "int";
        }
    };

    auto emit_unsigned_int_type = [&](const type::Type* ty) {
        uint32_t width = 0;
        type::Type::ElementOf(ty, &width);
        if (width > 1) {
            out << "uvec" << width;
        } else {
            out << "uint";
        }
    };

    auto emit_expr_as_signed = [&](const ast::Expression* e) {
        auto* ty = TypeOf(e)->UnwrapRef();
        if (!ty->is_unsigned_integer_scalar_or_vector()) {
            EmitExpression(out, e);
            return;
        }
        emit_signed_int_type(ty);
        ScopedParen sp(out);
        EmitExpression(out, e);
        return;
    };

    switch (builtin->Type()) {
        case builtin::Function::kTextureDimensions: {
            // textureDimensions() returns an unsigned scalar / vector in WGSL.
            // textureSize() / imageSize() returns a signed scalar / vector in GLSL.
            // Cast.
            emit_unsigned_int_type(call->Type());
            ScopedParen sp(out);

            if (texture_type->Is<type::StorageTexture>()) {
                out << "imageSize(";
            } else {
                out << "textureSize(";
            }
            EmitExpression(out, texture);

            // The LOD parameter is mandatory on textureSize() for non-multisampled
            // textures.
            if (!texture_type->Is<type::StorageTexture>() &&
                !texture_type->Is<type::MultisampledTexture>() &&
                !texture_type->Is<type::DepthMultisampledTexture>()) {
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
            if (texture_type->dim() == type::TextureDimension::k2dArray ||
                texture_type->dim() == type::TextureDimension::kCubeArray) {
                out << ".xy";
            }
            return;
        }
        case builtin::Function::kTextureNumLayers: {
            // textureNumLayers() returns an unsigned scalar in WGSL.
            // textureSize() / imageSize() returns a signed scalar / vector in GLSL.
            // Cast.
            out << "uint";
            ScopedParen sp(out);

            if (texture_type->Is<type::StorageTexture>()) {
                out << "imageSize(";
            } else {
                out << "textureSize(";
            }
            // textureSize() on sampler2dArray returns the array size in the
            // final component, so return it
            EmitExpression(out, texture);

            // The LOD parameter is mandatory on textureSize() for non-multisampled
            // textures.
            if (!texture_type->Is<type::StorageTexture>() &&
                !texture_type->Is<type::MultisampledTexture>() &&
                !texture_type->Is<type::DepthMultisampledTexture>()) {
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
        case builtin::Function::kTextureNumLevels: {
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
        case builtin::Function::kTextureNumSamples: {
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
    bool is_depth = texture_type->Is<type::DepthTexture>();

    switch (builtin->Type()) {
        case builtin::Function::kTextureSample:
        case builtin::Function::kTextureSampleBias:
            out << "texture";
            if (is_depth) {
                glsl_ret_width = 1u;
            }
            break;
        case builtin::Function::kTextureSampleLevel:
            out << "textureLod";
            if (is_depth) {
                glsl_ret_width = 1u;
            }
            break;
        case builtin::Function::kTextureGather:
        case builtin::Function::kTextureGatherCompare:
            out << "textureGather";
            append_depth_ref_to_coords = false;
            break;
        case builtin::Function::kTextureSampleGrad:
            out << "textureGrad";
            break;
        case builtin::Function::kTextureSampleCompare:
        case builtin::Function::kTextureSampleCompareLevel:
            out << "texture";
            glsl_ret_width = 1;
            break;
        case builtin::Function::kTextureLoad:
            out << "texelFetch";
            break;
        case builtin::Function::kTextureStore:
            out << "imageStore";
            break;
        default:
            diagnostics_.add_error(diag::System::Writer,
                                   "Internal compiler error: Unhandled texture builtin '" +
                                       std::string(builtin->str()) + "'");
            return;
    }

    if (builtin->Signature().IndexOf(sem::ParameterUsage::kOffset) >= 0) {
        out << "Offset";
    }

    out << "(";
    EmitExpression(out, texture);
    out << ", ";

    auto* param_coords = arg(Usage::kCoords);
    if (TINT_UNLIKELY(!param_coords)) {
        TINT_ICE(Writer, diagnostics_) << "missing coords argument";
        return;
    }

    if (auto* array_index = arg(Usage::kArrayIndex)) {
        // Array index needs to be appended to the coordinates.
        param_coords = AppendVector(&builder_, param_coords, array_index)->Declaration();
    }

    // GLSL requires Dref to be appended to the coordinates, *unless* it's
    // samplerCubeArrayShadow, in which case it will be handled as a separate
    // parameter.
    if (texture_type->dim() == type::TextureDimension::kCubeArray) {
        append_depth_ref_to_coords = false;
    }

    if (is_depth && append_depth_ref_to_coords) {
        auto* depth_ref = arg(Usage::kDepthRef);
        if (!depth_ref) {
            // Sampling a depth texture in GLSL always requires a depth reference, so
            // append zero here.
            depth_ref = CreateF32Zero(builder_.Sem().Get(param_coords)->Stmt());
        }
        param_coords = AppendVector(&builder_, param_coords, depth_ref)->Declaration();
    }

    emit_expr_as_signed(param_coords);

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
    if (is_depth && builtin->Type() == builtin::Function::kTextureGather) {
        out << ", 0.0";
    }

    // [1] samplerCubeArrayShadow requires a separate depthRef parameter
    if (is_depth && !append_depth_ref_to_coords) {
        if (auto* e = arg(Usage::kDepthRef)) {
            out << ", ";
            EmitExpression(out, e);
        } else if (builtin->Type() == builtin::Function::kTextureSample) {
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

    if (builtin->ReturnType()->Is<type::Void>()) {
        return;
    }
    // If the builtin return type does not match the number of elements of the
    // GLSL builtin, we need to swizzle the expression to generate the correct
    // number of components.
    uint32_t wgsl_ret_width = 1;
    if (auto* vec = builtin->ReturnType()->As<type::Vector>()) {
        wgsl_ret_width = vec->Width();
    }
    if (wgsl_ret_width < glsl_ret_width) {
        out << ".";
        for (uint32_t i = 0; i < wgsl_ret_width; i++) {
            out << "xyz"[i];
        }
    }
    if (TINT_UNLIKELY(wgsl_ret_width > glsl_ret_width)) {
        TINT_ICE(Writer, diagnostics_)
            << "WGSL return width (" << wgsl_ret_width << ") is wider than GLSL return width ("
            << glsl_ret_width << ") for " << builtin->Type();
        return;
    }
}

std::string GeneratorImpl::generate_builtin_name(const sem::Builtin* builtin) {
    switch (builtin->Type()) {
        case builtin::Function::kAbs:
        case builtin::Function::kAcos:
        case builtin::Function::kAcosh:
        case builtin::Function::kAll:
        case builtin::Function::kAny:
        case builtin::Function::kAsin:
        case builtin::Function::kAsinh:
        case builtin::Function::kAtan:
        case builtin::Function::kAtanh:
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
        case builtin::Function::kSign:
        case builtin::Function::kSin:
        case builtin::Function::kSinh:
        case builtin::Function::kSqrt:
        case builtin::Function::kStep:
        case builtin::Function::kTan:
        case builtin::Function::kTanh:
        case builtin::Function::kTranspose:
        case builtin::Function::kTrunc:
            return builtin->str();
        case builtin::Function::kAtan2:
            return "atan";
        case builtin::Function::kCountOneBits:
            return "bitCount";
        case builtin::Function::kDpdx:
            return "dFdx";
        case builtin::Function::kDpdxCoarse:
            if (version_.IsES()) {
                return "dFdx";
            }
            return "dFdxCoarse";
        case builtin::Function::kDpdxFine:
            if (version_.IsES()) {
                return "dFdx";
            }
            return "dFdxFine";
        case builtin::Function::kDpdy:
            return "dFdy";
        case builtin::Function::kDpdyCoarse:
            if (version_.IsES()) {
                return "dFdy";
            }
            return "dFdyCoarse";
        case builtin::Function::kDpdyFine:
            if (version_.IsES()) {
                return "dFdy";
            }
            return "dFdyFine";
        case builtin::Function::kFaceForward:
            return "faceforward";
        case builtin::Function::kFract:
            return "fract";
        case builtin::Function::kFma:
            return "fma";
        case builtin::Function::kFwidth:
        case builtin::Function::kFwidthCoarse:
        case builtin::Function::kFwidthFine:
            return "fwidth";
        case builtin::Function::kInverseSqrt:
            return "inversesqrt";
        case builtin::Function::kMix:
            return "mix";
        case builtin::Function::kPack2X16Float:
            return "packHalf2x16";
        case builtin::Function::kPack2X16Snorm:
            return "packSnorm2x16";
        case builtin::Function::kPack2X16Unorm:
            return "packUnorm2x16";
        case builtin::Function::kPack4X8Snorm:
            return "packSnorm4x8";
        case builtin::Function::kPack4X8Unorm:
            return "packUnorm4x8";
        case builtin::Function::kReverseBits:
            return "bitfieldReverse";
        case builtin::Function::kSmoothstep:
            return "smoothstep";
        case builtin::Function::kUnpack2X16Float:
            return "unpackHalf2x16";
        case builtin::Function::kUnpack2X16Snorm:
            return "unpackSnorm2x16";
        case builtin::Function::kUnpack2X16Unorm:
            return "unpackUnorm2x16";
        case builtin::Function::kUnpack4X8Snorm:
            return "unpackSnorm4x8";
        case builtin::Function::kUnpack4X8Unorm:
            return "unpackUnorm4x8";
        default:
            diagnostics_.add_error(diag::System::Writer,
                                   "Unknown builtin method: " + std::string(builtin->str()));
    }

    return "";
}

void GeneratorImpl::EmitCase(const ast::CaseStatement* stmt) {
    auto* sem = builder_.Sem().Get<sem::CaseStatement>(stmt);
    for (auto* selector : sem->Selectors()) {
        auto out = line();

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
            line() << "break;";
        }
    }

    line() << "}";
}

void GeneratorImpl::EmitContinue(const ast::ContinueStatement*) {
    if (emit_continuing_) {
        emit_continuing_();
    }
    line() << "continue;";
}

void GeneratorImpl::EmitDiscard(const ast::DiscardStatement*) {
    // TODO(dsinclair): Verify this is correct when the discard semantics are
    // defined for WGSL (https://github.com/gpuweb/gpuweb/issues/361)
    line() << "discard;";
}

void GeneratorImpl::EmitExpression(utils::StringStream& out, const ast::Expression* expr) {
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
        [&](const ast::BitcastExpression* b) { EmitBitcast(out, b); },
        [&](const ast::CallExpression* c) { EmitCall(out, c); },
        [&](const ast::IdentifierExpression* i) { EmitIdentifier(out, i); },
        [&](const ast::LiteralExpression* l) { EmitLiteral(out, l); },
        [&](const ast::MemberAccessorExpression* m) { EmitMemberAccessor(out, m); },
        [&](const ast::UnaryOpExpression* u) { EmitUnaryOp(out, u); },
        [&](Default) {  //
            diagnostics_.add_error(diag::System::Writer, "unknown expression type: " +
                                                             std::string(expr->TypeInfo().name));
        });
}

void GeneratorImpl::EmitIdentifier(utils::StringStream& out,
                                   const ast::IdentifierExpression* expr) {
    out << expr->identifier->symbol.Name();
}

void GeneratorImpl::EmitIf(const ast::IfStatement* stmt) {
    {
        auto out = line();
        out << "if (";
        EmitExpression(out, stmt->condition);
        out << ") {";
    }
    EmitStatementsWithIndent(stmt->body->statements);

    if (stmt->else_statement) {
        line() << "} else {";
        if (auto* block = stmt->else_statement->As<ast::BlockStatement>()) {
            EmitStatementsWithIndent(block->statements);
        } else {
            EmitStatementsWithIndent(utils::Vector{stmt->else_statement});
        }
    }
    line() << "}";
}

void GeneratorImpl::EmitFunction(const ast::Function* func) {
    auto* sem = builder_.Sem().Get(func);

    if (ast::HasAttribute<ast::InternalAttribute>(func->attributes)) {
        // An internal function. Do not emit.
        return;
    }

    {
        auto out = line();
        auto name = func->name->symbol.Name();
        EmitType(out, sem->ReturnType(), builtin::AddressSpace::kUndefined,
                 builtin::Access::kReadWrite, "");
        out << " " << name << "(";

        bool first = true;
        for (auto* v : sem->Parameters()) {
            if (!first) {
                out << ", ";
            }
            first = false;

            auto const* type = v->Type();

            if (auto* ptr = type->As<type::Pointer>()) {
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

    line() << "}";
    line();
}

void GeneratorImpl::EmitGlobalVariable(const ast::Variable* global) {
    Switch(
        global,  //
        [&](const ast::Var* var) {
            auto* sem = builder_.Sem().Get<sem::GlobalVariable>(global);
            switch (sem->AddressSpace()) {
                case builtin::AddressSpace::kUniform:
                    EmitUniformVariable(var, sem);
                    return;
                case builtin::AddressSpace::kStorage:
                    EmitStorageVariable(var, sem);
                    return;
                case builtin::AddressSpace::kHandle:
                    EmitHandleVariable(var, sem);
                    return;
                case builtin::AddressSpace::kPrivate:
                    EmitPrivateVariable(sem);
                    return;
                case builtin::AddressSpace::kWorkgroup:
                    EmitWorkgroupVariable(sem);
                    return;
                case builtin::AddressSpace::kIn:
                case builtin::AddressSpace::kOut:
                    EmitIOVariable(sem);
                    return;
                case builtin::AddressSpace::kPushConstant:
                    diagnostics_.add_error(
                        diag::System::Writer,
                        "unhandled address space " + utils::ToString(sem->AddressSpace()));
                    return;
                default: {
                    TINT_ICE(Writer, diagnostics_)
                        << "unhandled address space " << sem->AddressSpace();
                    break;
                }
            }
        },
        [&](const ast::Let* let) { EmitProgramConstVariable(let); },
        [&](const ast::Override*) {
            // Override is removed with SubstituteOverride
            diagnostics_.add_error(diag::System::Writer,
                                   "override-expressions should have been removed with the "
                                   "SubstituteOverride transform");
        },
        [&](const ast::Const*) {
            // Constants are embedded at their use
        },
        [&](Default) {
            TINT_ICE(Writer, diagnostics_)
                << "unhandled global variable type " << global->TypeInfo().name;
        });
}

void GeneratorImpl::EmitUniformVariable(const ast::Var* var, const sem::Variable* sem) {
    auto* type = sem->Type()->UnwrapRef();
    auto* str = type->As<type::Struct>();
    if (TINT_UNLIKELY(!str)) {
        TINT_ICE(Writer, builder_.Diagnostics()) << "storage variable must be of struct type";
        return;
    }
    auto bp = *sem->As<sem::GlobalVariable>()->BindingPoint();
    {
        auto out = line();
        out << "layout(binding = " << bp.binding << ", std140";
        out << ") uniform " << UniqueIdentifier(StructName(str) + "_ubo") << " {";
    }
    EmitStructMembers(current_buffer_, str);
    auto name = var->name->symbol.Name();
    line() << "} " << name << ";";
    line();
}

void GeneratorImpl::EmitStorageVariable(const ast::Var* var, const sem::Variable* sem) {
    auto* type = sem->Type()->UnwrapRef();
    auto* str = type->As<type::Struct>();
    if (TINT_UNLIKELY(!str)) {
        TINT_ICE(Writer, builder_.Diagnostics()) << "storage variable must be of struct type";
        return;
    }
    auto bp = *sem->As<sem::GlobalVariable>()->BindingPoint();
    line() << "layout(binding = " << bp.binding << ", std430) buffer "
           << UniqueIdentifier(StructName(str) + "_ssbo") << " {";
    EmitStructMembers(current_buffer_, str);
    auto name = var->name->symbol.Name();
    line() << "} " << name << ";";
    line();
}

void GeneratorImpl::EmitHandleVariable(const ast::Var* var, const sem::Variable* sem) {
    auto out = line();

    auto name = var->name->symbol.Name();
    auto* type = sem->Type()->UnwrapRef();
    if (type->Is<type::Sampler>()) {
        // GLSL ignores Sampler variables.
        return;
    }

    if (auto* storage = type->As<type::StorageTexture>()) {
        out << "layout(";
        switch (storage->texel_format()) {
            case builtin::TexelFormat::kBgra8Unorm:
                TINT_ICE(Writer, diagnostics_)
                    << "bgra8unorm should have been polyfilled to rgba8unorm";
                break;
            case builtin::TexelFormat::kR32Uint:
                out << "r32ui";
                break;
            case builtin::TexelFormat::kR32Sint:
                out << "r32i";
                break;
            case builtin::TexelFormat::kR32Float:
                out << "r32f";
                break;
            case builtin::TexelFormat::kRgba8Unorm:
                out << "rgba8";
                break;
            case builtin::TexelFormat::kRgba8Snorm:
                out << "rgba8_snorm";
                break;
            case builtin::TexelFormat::kRgba8Uint:
                out << "rgba8ui";
                break;
            case builtin::TexelFormat::kRgba8Sint:
                out << "rgba8i";
                break;
            case builtin::TexelFormat::kRg32Uint:
                out << "rg32ui";
                break;
            case builtin::TexelFormat::kRg32Sint:
                out << "rg32i";
                break;
            case builtin::TexelFormat::kRg32Float:
                out << "rg32f";
                break;
            case builtin::TexelFormat::kRgba16Uint:
                out << "rgba16ui";
                break;
            case builtin::TexelFormat::kRgba16Sint:
                out << "rgba16i";
                break;
            case builtin::TexelFormat::kRgba16Float:
                out << "rgba16f";
                break;
            case builtin::TexelFormat::kRgba32Uint:
                out << "rgba32ui";
                break;
            case builtin::TexelFormat::kRgba32Sint:
                out << "rgba32i";
                break;
            case builtin::TexelFormat::kRgba32Float:
                out << "rgba32f";
                break;
            case builtin::TexelFormat::kUndefined:
                TINT_ICE(Writer, diagnostics_) << "invalid texel format";
                return;
        }
        out << ") ";
    }
    EmitTypeAndName(out, type, sem->AddressSpace(), sem->Access(), name);
    out << ";";
}

void GeneratorImpl::EmitPrivateVariable(const sem::Variable* var) {
    auto* decl = var->Declaration();
    auto out = line();

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

void GeneratorImpl::EmitWorkgroupVariable(const sem::Variable* var) {
    auto* decl = var->Declaration();
    auto out = line();

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

void GeneratorImpl::EmitIOVariable(const sem::GlobalVariable* var) {
    auto* decl = var->Declaration();

    if (auto* attr = ast::GetAttribute<ast::BuiltinAttribute>(decl->attributes)) {
        auto builtin = program_->Sem().Get(attr)->Value();
        // Use of gl_SampleID requires the GL_OES_sample_variables extension
        if (RequiresOESSampleVariables(builtin)) {
            requires_oes_sample_variables_ = true;
        }
        // Do not emit builtin (gl_) variables.
        return;
    }

    auto out = line();
    EmitAttributes(out, var, decl->attributes);
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

void GeneratorImpl::EmitInterpolationQualifiers(
    utils::StringStream& out,
    utils::VectorRef<const ast::Attribute*> attributes) {
    for (auto* attr : attributes) {
        if (auto* interpolate = attr->As<ast::InterpolateAttribute>()) {
            auto& sem = program_->Sem();
            auto i_type =
                sem.Get<sem::BuiltinEnumExpression<builtin::InterpolationType>>(interpolate->type)
                    ->Value();
            switch (i_type) {
                case builtin::InterpolationType::kPerspective:
                case builtin::InterpolationType::kLinear:
                case builtin::InterpolationType::kUndefined:
                    break;
                case builtin::InterpolationType::kFlat:
                    out << "flat ";
                    break;
            }

            if (interpolate->sampling) {
                auto i_smpl = sem.Get<sem::BuiltinEnumExpression<builtin::InterpolationSampling>>(
                                     interpolate->sampling)
                                  ->Value();
                switch (i_smpl) {
                    case builtin::InterpolationSampling::kCentroid:
                        out << "centroid ";
                        break;
                    case builtin::InterpolationSampling::kSample:
                    case builtin::InterpolationSampling::kCenter:
                    case builtin::InterpolationSampling::kUndefined:
                        break;
                }
            }
        }
    }
}

void GeneratorImpl::EmitAttributes(utils::StringStream& out,
                                   const sem::GlobalVariable* var,
                                   utils::VectorRef<const ast::Attribute*> attributes) {
    if (attributes.IsEmpty()) {
        return;
    }

    bool first = true;
    for (auto* attr : attributes) {
        if (attr->As<ast::LocationAttribute>()) {
            out << (first ? "layout(" : ", ");
            out << "location = " << std::to_string(var->Location().value());
            first = false;
        }
    }
    if (!first) {
        out << ") ";
    }
}

void GeneratorImpl::EmitEntryPointFunction(const ast::Function* func) {
    auto* func_sem = builder_.Sem().Get(func);

    if (func->PipelineStage() == ast::PipelineStage::kFragment) {
        requires_default_precision_qualifier_ = true;
    }

    if (func->PipelineStage() == ast::PipelineStage::kCompute) {
        auto out = line();
        // Emit the layout(local_size) attributes.
        auto wgsize = func_sem->WorkgroupSize();
        out << "layout(";
        for (size_t i = 0; i < 3; i++) {
            if (i > 0) {
                out << ", ";
            }
            out << "local_size_" << (i == 0 ? "x" : i == 1 ? "y" : "z") << " = ";

            if (!wgsize[i].has_value()) {
                diagnostics_.add_error(
                    diag::System::Writer,
                    "override-expressions should have been removed with the SubstituteOverride "
                    "transform");
                return;
            }
            out << std::to_string(wgsize[i].value());
        }
        out << ") in;";
    }

    // Emit original entry point signature
    {
        auto out = line();
        EmitTypeAndName(out, func_sem->ReturnType(), builtin::AddressSpace::kUndefined,
                        builtin::Access::kUndefined, func->name->symbol.Name());
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

            EmitTypeAndName(out, type, sem->AddressSpace(), sem->Access(),
                            var->name->symbol.Name());
        }

        out << ") {";
    }

    // Emit original entry point function body
    {
        ScopedIndent si(this);
        if (func->PipelineStage() == ast::PipelineStage::kVertex) {
            line() << "gl_PointSize = 1.0;";
        }

        EmitStatements(func->body->statements);

        if (!Is<ast::ReturnStatement>(func->body->Last())) {
            ast::ReturnStatement ret(ProgramID{}, ast::NodeID{}, Source{});
            EmitStatement(&ret);
        }
    }

    line() << "}";
}

void GeneratorImpl::EmitConstant(utils::StringStream& out, const constant::Value* constant) {
    Switch(
        constant->Type(),  //
        [&](const type::Bool*) { out << (constant->ValueAs<AInt>() ? "true" : "false"); },
        [&](const type::F32*) { PrintF32(out, constant->ValueAs<f32>()); },
        [&](const type::F16*) { PrintF16(out, constant->ValueAs<f16>()); },
        [&](const type::I32*) { PrintI32(out, constant->ValueAs<i32>()); },
        [&](const type::U32*) { out << constant->ValueAs<AInt>() << "u"; },
        [&](const type::Vector* v) {
            EmitType(out, v, builtin::AddressSpace::kUndefined, builtin::Access::kUndefined, "");

            ScopedParen sp(out);

            if (auto* splat = constant->As<constant::Splat>()) {
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
        [&](const type::Matrix* m) {
            EmitType(out, m, builtin::AddressSpace::kUndefined, builtin::Access::kUndefined, "");

            ScopedParen sp(out);

            for (size_t column_idx = 0; column_idx < m->columns(); column_idx++) {
                if (column_idx > 0) {
                    out << ", ";
                }
                EmitConstant(out, constant->Index(column_idx));
            }
        },
        [&](const type::Array* a) {
            EmitType(out, a, builtin::AddressSpace::kUndefined, builtin::Access::kUndefined, "");

            ScopedParen sp(out);

            auto count = a->ConstantCount();
            if (!count) {
                diagnostics_.add_error(diag::System::Writer,
                                       type::Array::kErrExpectedConstantCount);
                return;
            }

            for (size_t i = 0; i < count; i++) {
                if (i > 0) {
                    out << ", ";
                }
                EmitConstant(out, constant->Index(i));
            }
        },
        [&](const type::Struct* s) {
            EmitStructType(&helpers_, s);

            out << StructName(s);

            ScopedParen sp(out);

            for (size_t i = 0; i < s->Members().Length(); i++) {
                if (i > 0) {
                    out << ", ";
                }
                EmitConstant(out, constant->Index(i));
            }
        },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer,
                                   "unhandled constant type: " + constant->Type()->FriendlyName());
        });
}

void GeneratorImpl::EmitLiteral(utils::StringStream& out, const ast::LiteralExpression* lit) {
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
            diagnostics_.add_error(diag::System::Writer, "unknown integer literal suffix type");
        },
        [&](Default) { diagnostics_.add_error(diag::System::Writer, "unknown literal type"); });
}

void GeneratorImpl::EmitZeroValue(utils::StringStream& out, const type::Type* type) {
    if (type->Is<type::Bool>()) {
        out << "false";
    } else if (type->Is<type::F32>()) {
        out << "0.0f";
    } else if (type->Is<type::F16>()) {
        out << "0.0hf";
    } else if (type->Is<type::I32>()) {
        out << "0";
    } else if (type->Is<type::U32>()) {
        out << "0u";
    } else if (auto* vec = type->As<type::Vector>()) {
        EmitType(out, type, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
        ScopedParen sp(out);
        for (uint32_t i = 0; i < vec->Width(); i++) {
            if (i != 0) {
                out << ", ";
            }
            EmitZeroValue(out, vec->type());
        }
    } else if (auto* mat = type->As<type::Matrix>()) {
        EmitType(out, type, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
        ScopedParen sp(out);
        for (uint32_t i = 0; i < (mat->rows() * mat->columns()); i++) {
            if (i != 0) {
                out << ", ";
            }
            EmitZeroValue(out, mat->type());
        }
    } else if (auto* str = type->As<type::Struct>()) {
        EmitType(out, type, builtin::AddressSpace::kUndefined, builtin::Access::kUndefined, "");
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
    } else if (auto* arr = type->As<type::Array>()) {
        EmitType(out, type, builtin::AddressSpace::kUndefined, builtin::Access::kUndefined, "");
        ScopedParen sp(out);

        auto count = arr->ConstantCount();
        if (!count) {
            diagnostics_.add_error(diag::System::Writer, type::Array::kErrExpectedConstantCount);
            return;
        }

        for (uint32_t i = 0; i < count; i++) {
            if (i != 0) {
                out << ", ";
            }
            EmitZeroValue(out, arr->ElemType());
        }
    } else {
        diagnostics_.add_error(diag::System::Writer,
                               "Invalid type for zero emission: " + type->FriendlyName());
    }
}

void GeneratorImpl::EmitLoop(const ast::LoopStatement* stmt) {
    auto emit_continuing = [this, stmt]() {
        if (stmt->continuing && !stmt->continuing->Empty()) {
            EmitBlock(stmt->continuing);
        }
    };

    TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);
    line() << "while (true) {";
    {
        ScopedIndent si(this);
        EmitStatements(stmt->body->statements);
        emit_continuing_();
    }
    line() << "}";
}

void GeneratorImpl::EmitForLoop(const ast::ForLoopStatement* stmt) {
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
        EmitStatement(init);
    }

    TextBuffer cond_pre;
    utils::StringStream cond_buf;
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
        auto emit_continuing = [&]() { current_buffer_->Append(cont_buf); };

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

        EmitStatements(stmt->body->statements);
        emit_continuing_();
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
            EmitStatementsWithIndent(stmt->body->statements);
        }
        line() << "}";
    }
}

void GeneratorImpl::EmitWhile(const ast::WhileStatement* stmt) {
    TextBuffer cond_pre;
    utils::StringStream cond_buf;
    {
        auto* cond = stmt->condition;
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &cond_pre);
        EmitExpression(cond_buf, cond);
    }

    auto emit_continuing = [&]() {};
    TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);

    // If the whilehas a multi-statement conditional, then we cannot emit this
    // as a regular while in GLSL. Instead we need to generate a `while(true)` loop.
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

        EmitStatements(stmt->body->statements);
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
        EmitStatementsWithIndent(stmt->body->statements);
        line() << "}";
    }
}

void GeneratorImpl::EmitMemberAccessor(utils::StringStream& out,
                                       const ast::MemberAccessorExpression* expr) {
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
        },
        [&](Default) {
            TINT_ICE(Writer, diagnostics_)
                << "unknown member access type: " << sem->TypeInfo().name;
        });
}

void GeneratorImpl::EmitReturn(const ast::ReturnStatement* stmt) {
    if (stmt->value) {
        auto out = line();
        out << "return ";
        EmitExpression(out, stmt->value);
        out << ";";
    } else {
        line() << "return;";
    }
}

void GeneratorImpl::EmitStatement(const ast::Statement* stmt) {
    Switch(
        stmt,  //
        [&](const ast::AssignmentStatement* a) { EmitAssign(a); },
        [&](const ast::BlockStatement* b) { EmitBlock(b); },
        [&](const ast::BreakStatement* b) { EmitBreak(b); },
        [&](const ast::BreakIfStatement* b) { EmitBreakIf(b); },
        [&](const ast::CallStatement* c) {
            auto out = line();
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
                },
                [&](Default) {  //
                    TINT_ICE(Writer, diagnostics_)
                        << "unknown variable type: " << v->variable->TypeInfo().name;
                });
        },
        [&](const ast::ConstAssert*) {
            // Not emitted
        },
        [&](Default) {
            diagnostics_.add_error(diag::System::Writer,
                                   "unknown statement type: " + std::string(stmt->TypeInfo().name));
        });
}

void GeneratorImpl::EmitSwitch(const ast::SwitchStatement* stmt) {
    {  // switch(expr) {
        auto out = line();
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

    line() << "}";
}

void GeneratorImpl::EmitType(utils::StringStream& out,
                             const type::Type* type,
                             builtin::AddressSpace address_space,
                             builtin::Access access,
                             const std::string& name,
                             bool* name_printed /* = nullptr */) {
    if (name_printed) {
        *name_printed = false;
    }
    switch (address_space) {
        case builtin::AddressSpace::kIn: {
            out << "in ";
            break;
        }
        case builtin::AddressSpace::kOut: {
            out << "out ";
            break;
        }
        case builtin::AddressSpace::kUniform:
        case builtin::AddressSpace::kHandle: {
            out << "uniform ";
            break;
        }
        default:
            break;
    }

    if (auto* ary = type->As<type::Array>()) {
        const type::Type* base_type = ary;
        std::vector<uint32_t> sizes;
        while (auto* arr = base_type->As<type::Array>()) {
            if (arr->Count()->Is<type::RuntimeArrayCount>()) {
                sizes.push_back(0);
            } else {
                auto count = arr->ConstantCount();
                if (!count) {
                    diagnostics_.add_error(diag::System::Writer,
                                           type::Array::kErrExpectedConstantCount);
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
    } else if (type->Is<type::Bool>()) {
        out << "bool";
    } else if (type->Is<type::F32>()) {
        out << "float";
    } else if (type->Is<type::F16>()) {
        out << "float16_t";
    } else if (type->Is<type::I32>()) {
        out << "int";
    } else if (auto* mat = type->As<type::Matrix>()) {
        TINT_ASSERT(Writer, (mat->type()->IsAnyOf<type::F32, type::F16>()));
        if (mat->type()->Is<type::F16>()) {
            out << "f16";
        }
        out << "mat" << mat->columns();
        if (mat->rows() != mat->columns()) {
            out << "x" << mat->rows();
        }
    } else if (TINT_UNLIKELY(type->Is<type::Pointer>())) {
        TINT_ICE(Writer, diagnostics_) << "Attempting to emit pointer type. These should have been "
                                          "removed with the SimplifyPointers transform";
    } else if (type->Is<type::Sampler>()) {
    } else if (auto* str = type->As<type::Struct>()) {
        out << StructName(str);
    } else if (auto* tex = type->As<type::Texture>()) {
        if (TINT_UNLIKELY(tex->Is<type::ExternalTexture>())) {
            TINT_ICE(Writer, diagnostics_) << "Multiplanar external texture transform was not run.";
            return;
        }

        auto* storage = tex->As<type::StorageTexture>();
        auto* ms = tex->As<type::MultisampledTexture>();
        auto* depth_ms = tex->As<type::DepthMultisampledTexture>();
        auto* sampled = tex->As<type::SampledTexture>();

        out << "highp ";

        if (storage && storage->access() != builtin::Access::kRead) {
            out << "writeonly ";
        }
        auto* subtype = sampled   ? sampled->type()
                        : storage ? storage->type()
                        : ms      ? ms->type()
                                  : nullptr;
        if (!subtype || subtype->Is<type::F32>()) {
        } else if (subtype->Is<type::I32>()) {
            out << "i";
        } else if (TINT_LIKELY(subtype->Is<type::U32>())) {
            out << "u";
        } else {
            TINT_ICE(Writer, diagnostics_) << "Unsupported texture type";
            return;
        }

        out << (storage ? "image" : "sampler");

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
                return;
        }
        if (tex->Is<type::DepthTexture>()) {
            out << "Shadow";
        }
    } else if (type->Is<type::U32>()) {
        out << "uint";
    } else if (auto* vec = type->As<type::Vector>()) {
        auto width = vec->Width();
        if (vec->type()->Is<type::F32>() && width >= 1 && width <= 4) {
            out << "vec" << width;
        } else if (vec->type()->Is<type::F16>() && width >= 1 && width <= 4) {
            out << "f16vec" << width;
        } else if (vec->type()->Is<type::I32>() && width >= 1 && width <= 4) {
            out << "ivec" << width;
        } else if (vec->type()->Is<type::U32>() && width >= 1 && width <= 4) {
            out << "uvec" << width;
        } else if (vec->type()->Is<type::Bool>() && width >= 1 && width <= 4) {
            out << "bvec" << width;
        } else {
            out << "vector<";
            EmitType(out, vec->type(), address_space, access, "");
            out << ", " << width << ">";
        }
    } else if (auto* atomic = type->As<type::Atomic>()) {
        EmitType(out, atomic->Type(), address_space, access, name);
    } else if (type->Is<type::Void>()) {
        out << "void";
    } else {
        diagnostics_.add_error(diag::System::Writer, "unknown type in EmitType");
    }
}

void GeneratorImpl::EmitTypeAndName(utils::StringStream& out,
                                    const type::Type* type,
                                    builtin::AddressSpace address_space,
                                    builtin::Access access,
                                    const std::string& name) {
    bool printed_name = false;
    EmitType(out, type, address_space, access, name, &printed_name);
    if (!name.empty() && !printed_name) {
        out << " " << name;
    }
}

void GeneratorImpl::EmitStructType(TextBuffer* b, const type::Struct* str) {
    auto it = emitted_structs_.emplace(str);
    if (!it.second) {
        return;
    }

    auto address_space_uses = str->AddressSpaceUsage();
    line(b) << "struct " << StructName(str) << " {";
    EmitStructMembers(b, str);
    line(b) << "};";
    line(b);
}

void GeneratorImpl::EmitStructMembers(TextBuffer* b, const type::Struct* str) {
    ScopedIndent si(b);
    for (auto* mem : str->Members()) {
        auto name = mem->Name().Name();
        auto* ty = mem->Type();

        auto out = line(b);
        EmitTypeAndName(out, ty, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite,
                        name);
        out << ";";
    }
}

void GeneratorImpl::EmitUnaryOp(utils::StringStream& out, const ast::UnaryOpExpression* expr) {
    switch (expr->op) {
        case ast::UnaryOp::kIndirection:
        case ast::UnaryOp::kAddressOf:
            EmitExpression(out, expr->expr);
            return;
        case ast::UnaryOp::kComplement:
            out << "~";
            break;
        case ast::UnaryOp::kNot:
            if (TypeOf(expr)->UnwrapRef()->is_scalar()) {
                out << "!";
            } else {
                out << "not";
            }
            break;
        case ast::UnaryOp::kNegation:
            out << "-";
            break;
    }

    ScopedParen sp(out);
    EmitExpression(out, expr->expr);
}

void GeneratorImpl::EmitVar(const ast::Var* var) {
    auto* sem = builder_.Sem().Get(var);
    auto* type = sem->Type()->UnwrapRef();

    auto out = line();
    EmitTypeAndName(out, type, sem->AddressSpace(), sem->Access(), var->name->symbol.Name());

    out << " = ";

    if (var->initializer) {
        EmitExpression(out, var->initializer);
    } else {
        EmitZeroValue(out, type);
    }
    out << ";";
}

void GeneratorImpl::EmitLet(const ast::Let* let) {
    auto* sem = builder_.Sem().Get(let);
    auto* type = sem->Type()->UnwrapRef();

    auto out = line();
    // TODO(senorblanco): handle const
    EmitTypeAndName(out, type, builtin::AddressSpace::kUndefined, builtin::Access::kUndefined,
                    let->name->symbol.Name());

    out << " = ";
    EmitExpression(out, let->initializer);
    out << ";";
}

void GeneratorImpl::EmitProgramConstVariable(const ast::Variable* var) {
    auto* sem = builder_.Sem().Get(var);
    auto* type = sem->Type();

    auto out = line();
    out << "const ";
    EmitTypeAndName(out, type, builtin::AddressSpace::kUndefined, builtin::Access::kUndefined,
                    var->name->symbol.Name());
    out << " = ";
    EmitExpression(out, var->initializer);
    out << ";";
}

template <typename F>
void GeneratorImpl::CallBuiltinHelper(utils::StringStream& out,
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
            EmitTypeAndName(decl, builtin->ReturnType(), builtin::AddressSpace::kUndefined,
                            builtin::Access::kUndefined, fn_name);
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
                    EmitTypeAndName(decl, ty, builtin::AddressSpace::kUndefined,
                                    builtin::Access::kUndefined, param_name);
                    parameter_names.emplace_back(std::move(param_name));
                }
            }
            decl << " {";
        }
        {
            ScopedIndent si(&b);
            build(&b, parameter_names);
        }
        line(&b) << "}";
        line(&b);
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

type::Type* GeneratorImpl::BoolTypeToUint(const type::Type* type) {
    auto* u32 = builder_.create<type::U32>();
    if (type->Is<type::Bool>()) {
        return u32;
    } else if (auto* vec = type->As<type::Vector>()) {
        return builder_.create<type::Vector>(u32, vec->Width());
    } else {
        return nullptr;
    }
}

}  // namespace tint::writer::glsl
