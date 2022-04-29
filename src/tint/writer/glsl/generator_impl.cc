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
#include <set>
#include <utility>
#include <vector>

#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/fallthrough_statement.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/internal_attribute.h"
#include "src/tint/ast/interpolate_attribute.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/debug.h"
#include "src/tint/sem/array.h"
#include "src/tint/sem/atomic.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/depth_multisampled_texture.h"
#include "src/tint/sem/depth_texture.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/module.h"
#include "src/tint/sem/multisampled_texture.h"
#include "src/tint/sem/sampled_texture.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/storage_texture.h"
#include "src/tint/sem/struct.h"
#include "src/tint/sem/type_constructor.h"
#include "src/tint/sem/type_conversion.h"
#include "src/tint/sem/variable.h"
#include "src/tint/transform/add_empty_entry_point.h"
#include "src/tint/transform/add_spirv_block_attribute.h"
#include "src/tint/transform/binding_remapper.h"
#include "src/tint/transform/builtin_polyfill.h"
#include "src/tint/transform/canonicalize_entry_point_io.h"
#include "src/tint/transform/combine_samplers.h"
#include "src/tint/transform/decompose_memory_access.h"
#include "src/tint/transform/expand_compound_assignment.h"
#include "src/tint/transform/fold_trivial_single_use_lets.h"
#include "src/tint/transform/loop_to_for_loop.h"
#include "src/tint/transform/manager.h"
#include "src/tint/transform/promote_initializers_to_const_var.h"
#include "src/tint/transform/promote_side_effects_to_decl.h"
#include "src/tint/transform/remove_phonies.h"
#include "src/tint/transform/renamer.h"
#include "src/tint/transform/simplify_pointers.h"
#include "src/tint/transform/single_entry_point.h"
#include "src/tint/transform/unshadow.h"
#include "src/tint/transform/unwind_discard_functions.h"
#include "src/tint/transform/zero_init_workgroup_memory.h"
#include "src/tint/utils/defer.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/scoped_assignment.h"
#include "src/tint/writer/append_vector.h"
#include "src/tint/writer/float_to_string.h"
#include "src/tint/writer/generate_external_texture_bindings.h"

namespace {

bool IsRelational(tint::ast::BinaryOp op) {
  return op == tint::ast::BinaryOp::kEqual ||
         op == tint::ast::BinaryOp::kNotEqual ||
         op == tint::ast::BinaryOp::kLessThan ||
         op == tint::ast::BinaryOp::kGreaterThan ||
         op == tint::ast::BinaryOp::kLessThanEqual ||
         op == tint::ast::BinaryOp::kGreaterThanEqual;
}

bool RequiresOESSampleVariables(tint::ast::Builtin builtin) {
  switch (builtin) {
    case tint::ast::Builtin::kSampleIndex:
    case tint::ast::Builtin::kSampleMask:
      return true;
    default:
      return false;
  }
}

}  // namespace

namespace tint::writer::glsl {
namespace {

const char kTempNamePrefix[] = "tint_tmp";
const char kSpecConstantPrefix[] = "WGSL_SPEC_CONSTANT_";

bool last_is_break_or_fallthrough(const ast::BlockStatement* stmts) {
  return IsAnyOf<ast::BreakStatement, ast::FallthroughStatement>(stmts->Last());
}

const char* convert_texel_format_to_glsl(const ast::TexelFormat format) {
  switch (format) {
    case ast::TexelFormat::kR32Uint:
      return "r32ui";
    case ast::TexelFormat::kR32Sint:
      return "r32i";
    case ast::TexelFormat::kR32Float:
      return "r32f";
    case ast::TexelFormat::kRgba8Unorm:
      return "rgba8";
    case ast::TexelFormat::kRgba8Snorm:
      return "rgba8_snorm";
    case ast::TexelFormat::kRgba8Uint:
      return "rgba8ui";
    case ast::TexelFormat::kRgba8Sint:
      return "rgba8i";
    case ast::TexelFormat::kRg32Uint:
      return "rg32ui";
    case ast::TexelFormat::kRg32Sint:
      return "rg32i";
    case ast::TexelFormat::kRg32Float:
      return "rg32f";
    case ast::TexelFormat::kRgba16Uint:
      return "rgba16ui";
    case ast::TexelFormat::kRgba16Sint:
      return "rgba16i";
    case ast::TexelFormat::kRgba16Float:
      return "rgba16f";
    case ast::TexelFormat::kRgba32Uint:
      return "rgba32ui";
    case ast::TexelFormat::kRgba32Sint:
      return "rgba32i";
    case ast::TexelFormat::kRgba32Float:
      return "rgba32f";
    case ast::TexelFormat::kNone:
      return "unknown";
  }
  return "unknown";
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

  {  // Builtin polyfills
    transform::BuiltinPolyfill::Builtins polyfills;
    polyfills.count_leading_zeros = true;
    polyfills.count_trailing_zeros = true;
    polyfills.extract_bits =
        transform::BuiltinPolyfill::Level::kClampParameters;
    polyfills.first_leading_bit = true;
    polyfills.first_trailing_bit = true;
    polyfills.insert_bits = transform::BuiltinPolyfill::Level::kClampParameters;
    data.Add<transform::BuiltinPolyfill::Config>(polyfills);
    manager.Add<transform::BuiltinPolyfill>();
  }

  if (!entry_point.empty()) {
    manager.Add<transform::SingleEntryPoint>();
    data.Add<transform::SingleEntryPoint::Config>(entry_point);
  }
  manager.Add<transform::Renamer>();
  data.Add<transform::Renamer::Config>(
      transform::Renamer::Target::kGlslKeywords,
      /* preserve_unicode */ false);
  manager.Add<transform::Unshadow>();

  // Attempt to convert `loop`s into for-loops. This is to try and massage the
  // output into something that will not cause FXC to choke or misbehave.
  manager.Add<transform::FoldTrivialSingleUseLets>();
  manager.Add<transform::LoopToForLoop>();

  if (!options.disable_workgroup_init) {
    // ZeroInitWorkgroupMemory must come before CanonicalizeEntryPointIO as
    // ZeroInitWorkgroupMemory may inject new builtin parameters.
    manager.Add<transform::ZeroInitWorkgroupMemory>();
  }
  manager.Add<transform::CanonicalizeEntryPointIO>();
  manager.Add<transform::ExpandCompoundAssignment>();
  manager.Add<transform::PromoteSideEffectsToDecl>();
  manager.Add<transform::UnwindDiscardFunctions>();
  manager.Add<transform::SimplifyPointers>();

  manager.Add<transform::RemovePhonies>();

  if (options.generate_external_texture_bindings) {
    auto new_bindings_map = writer::GenerateExternalTextureBindings(in);
    data.Add<transform::MultiplanarExternalTexture::NewBindingPoints>(
        new_bindings_map);
  }
  manager.Add<transform::MultiplanarExternalTexture>();

  data.Add<transform::CombineSamplers::BindingInfo>(
      options.binding_map, options.placeholder_binding_point);
  manager.Add<transform::CombineSamplers>();

  data.Add<transform::BindingRemapper::Remappings>(options.binding_points,
                                                   options.access_controls,
                                                   options.allow_collisions);
  manager.Add<transform::BindingRemapper>();

  manager.Add<transform::PromoteInitializersToConstVar>();
  manager.Add<transform::AddEmptyEntryPoint>();
  manager.Add<transform::AddSpirvBlockAttribute>();
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

bool GeneratorImpl::Generate() {
  {
    auto out = line();
    out << "#version " << version_.major_version << version_.minor_version
        << "0";
    if (version_.IsES()) {
      out << " es";
    }
  }

  auto helpers_insertion_point = current_buffer_->lines.size();

  line();

  auto* mod = builder_.Sem().Module();
  for (auto* decl : mod->DependencyOrderedDeclarations()) {
    if (decl->Is<ast::Alias>()) {
      continue;  // Ignore aliases.
    }

    if (auto* global = decl->As<ast::Variable>()) {
      if (!EmitGlobalVariable(global)) {
        return false;
      }
    } else if (auto* str = decl->As<ast::Struct>()) {
      // Skip emission if the struct contains a runtime-sized array, since its
      // only use will be as the store-type of a buffer and we emit those
      // elsewhere.
      // TODO(crbug.com/tint/1339): We could also avoid emitting any other
      // struct that is only used as a buffer store type.
      const sem::Struct* sem_str = builder_.Sem().Get(str);
      const auto& members = sem_str->Members();
      TINT_ASSERT(Writer, members.size() > 0);
      auto* last_member = members[members.size() - 1];
      auto* arr = last_member->Type()->As<sem::Array>();
      if (!arr || !arr->IsRuntimeSized()) {
        if (!EmitStructType(current_buffer_, sem_str)) {
          return false;
        }
      }
    } else if (auto* func = decl->As<ast::Function>()) {
      if (func->IsEntryPoint()) {
        if (!EmitEntryPointFunction(func)) {
          return false;
        }
      } else {
        if (!EmitFunction(func)) {
          return false;
        }
      }
    } else if (auto* ext = decl->As<ast::Enable>()) {
      // Record the required extension for generating extension directive later
      if (!RecordExtension(ext)) {
        return false;
      }
    } else {
      TINT_ICE(Writer, diagnostics_)
          << "unhandled module-scope declaration: " << decl->TypeInfo().name;
      return false;
    }
  }

  TextBuffer extensions;

  if (version_.IsES() && requires_oes_sample_variables_) {
    extensions.Append("#extension GL_OES_sample_variables : require");
  }

  auto indent = current_buffer_->current_indent;

  if (!extensions.lines.empty()) {
    current_buffer_->Insert(extensions, helpers_insertion_point, indent);
    helpers_insertion_point += extensions.lines.size();
  }

  if (version_.IsES() && requires_default_precision_qualifier_) {
    current_buffer_->Insert("precision mediump float;",
                            helpers_insertion_point++, indent);
  }

  if (!helpers_.lines.empty()) {
    current_buffer_->Insert("", helpers_insertion_point++, indent);
    current_buffer_->Insert(helpers_, helpers_insertion_point, indent);
    helpers_insertion_point += helpers_.lines.size();
  }

  return true;
}

bool GeneratorImpl::RecordExtension(const ast::Enable*) {
  /*
  Deal with extension node here, recording it within the generator for
  later emition.
  For example:
  ```
    if (ext->kind == ast::Enable::ExtensionKind::kF16) {
    require_fp16_ = true;
    }
  ```
  */

  return true;
}

bool GeneratorImpl::EmitIndexAccessor(
    std::ostream& out,
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

bool GeneratorImpl::EmitBitcast(std::ostream& out,
                                const ast::BitcastExpression* expr) {
  auto* src_type = TypeOf(expr->expr)->UnwrapRef();
  auto* dst_type = TypeOf(expr)->UnwrapRef();

  if (!dst_type->is_integer_scalar_or_vector() &&
      !dst_type->is_float_scalar_or_vector()) {
    diagnostics_.add_error(diag::System::Writer,
                           "Unable to do bitcast to type " +
                               dst_type->FriendlyName(builder_.Symbols()));
    return false;
  }

  if (src_type == dst_type) {
    return EmitExpression(out, expr->expr);
  }

  if (src_type->is_float_scalar_or_vector() &&
      dst_type->is_signed_scalar_or_vector()) {
    out << "floatBitsToInt";
  } else if (src_type->is_float_scalar_or_vector() &&
             dst_type->is_unsigned_scalar_or_vector()) {
    out << "floatBitsToUint";
  } else if (src_type->is_signed_scalar_or_vector() &&
             dst_type->is_float_scalar_or_vector()) {
    out << "intBitsToFloat";
  } else if (src_type->is_unsigned_scalar_or_vector() &&
             dst_type->is_float_scalar_or_vector()) {
    out << "uintBitsToFloat";
  } else {
    if (!EmitType(out, dst_type, ast::StorageClass::kNone,
                  ast::Access::kReadWrite, "")) {
      return false;
    }
  }
  out << "(";
  if (!EmitExpression(out, expr->expr)) {
    return false;
  }
  out << ")";
  return true;
}

bool GeneratorImpl::EmitAssign(const ast::AssignmentStatement* stmt) {
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

bool GeneratorImpl::EmitVectorRelational(std::ostream& out,
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
  out << "(";
  if (!EmitExpression(out, expr->lhs)) {
    return false;
  }
  out << ", ";
  if (!EmitExpression(out, expr->rhs)) {
    return false;
  }
  out << ")";
  return true;
}

bool GeneratorImpl::EmitBitwiseBoolOp(std::ostream& out,
                                      const ast::BinaryExpression* expr) {
  auto* bool_type = TypeOf(expr->lhs)->UnwrapRef();
  auto* uint_type = BoolTypeToUint(bool_type);

  // Cast result to bool scalar or vector type.
  if (!EmitType(out, bool_type, ast::StorageClass::kNone,
                ast::Access::kReadWrite, "")) {
    return false;
  }
  ScopedParen outerCastParen(out);
  // Cast LHS to uint scalar or vector type.
  if (!EmitType(out, uint_type, ast::StorageClass::kNone,
                ast::Access::kReadWrite, "")) {
    return false;
  }
  {
    ScopedParen innerCastParen(out);
    // Emit LHS.
    if (!EmitExpression(out, expr->lhs)) {
      return false;
    }
  }
  // Emit operator.
  if (expr->op == ast::BinaryOp::kAnd) {
    out << " & ";
  } else if (expr->op == ast::BinaryOp::kOr) {
    out << " | ";
  } else {
    TINT_ICE(Writer, diagnostics_)
        << "unexpected binary op: " << FriendlyName(expr->op);
    return false;
  }
  // Cast RHS to uint scalar or vector type.
  if (!EmitType(out, uint_type, ast::StorageClass::kNone,
                ast::Access::kReadWrite, "")) {
    return false;
  }
  {
    ScopedParen innerCastParen(out);
    // Emit RHS.
    if (!EmitExpression(out, expr->rhs)) {
      return false;
    }
  }
  return true;
}

bool GeneratorImpl::EmitFloatModulo(std::ostream& out,
                                    const ast::BinaryExpression* expr) {
  std::string fn;
  auto* ret_ty = TypeOf(expr)->UnwrapRef();
  fn = utils::GetOrCreate(float_modulo_funcs_, ret_ty, [&]() -> std::string {
    TextBuffer b;
    TINT_DEFER(helpers_.Append(b));

    auto fn_name = UniqueIdentifier("tint_float_modulo");
    std::vector<std::string> parameter_names;
    {
      auto decl = line(&b);
      if (!EmitTypeAndName(decl, ret_ty, ast::StorageClass::kNone,
                           ast::Access::kUndefined, fn_name)) {
        return "";
      }
      {
        ScopedParen sp(decl);
        const auto* ty = TypeOf(expr->lhs)->UnwrapRef();
        if (!EmitTypeAndName(decl, ty, ast::StorageClass::kNone,
                             ast::Access::kUndefined, "lhs")) {
          return "";
        }
        decl << ", ";
        ty = TypeOf(expr->rhs)->UnwrapRef();
        if (!EmitTypeAndName(decl, ty, ast::StorageClass::kNone,
                             ast::Access::kUndefined, "rhs")) {
          return "";
        }
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

  if (fn.empty()) {
    return false;
  }

  // Call the helper
  out << fn;
  {
    ScopedParen sp(out);
    if (!EmitExpression(out, expr->lhs)) {
      return false;
    }
    out << ", ";
    if (!EmitExpression(out, expr->rhs)) {
      return false;
    }
  }
  return true;
}

bool GeneratorImpl::EmitBinary(std::ostream& out,
                               const ast::BinaryExpression* expr) {
  if (IsRelational(expr->op) && !TypeOf(expr->lhs)->UnwrapRef()->is_scalar()) {
    return EmitVectorRelational(out, expr);
  }
  if (expr->op == ast::BinaryOp::kLogicalAnd ||
      expr->op == ast::BinaryOp::kLogicalOr) {
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
  if ((expr->op == ast::BinaryOp::kAnd || expr->op == ast::BinaryOp::kOr) &&
      TypeOf(expr->lhs)->UnwrapRef()->is_bool_scalar_or_vector()) {
    return EmitBitwiseBoolOp(out, expr);
  }

  if (expr->op == ast::BinaryOp::kModulo &&
      (TypeOf(expr->lhs)->UnwrapRef()->is_float_scalar_or_vector() ||
       TypeOf(expr->rhs)->UnwrapRef()->is_float_scalar_or_vector())) {
    return EmitFloatModulo(out, expr);
  }

  out << "(";
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
      diagnostics_.add_error(diag::System::Writer,
                             "missing binary operation type");
      return false;
  }
  out << " ";

  if (!EmitExpression(out, expr->rhs)) {
    return false;
  }

  out << ")";
  return true;
}

bool GeneratorImpl::EmitStatements(const ast::StatementList& stmts) {
  for (auto* s : stmts) {
    if (!EmitStatement(s)) {
      return false;
    }
  }
  return true;
}

bool GeneratorImpl::EmitStatementsWithIndent(const ast::StatementList& stmts) {
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

bool GeneratorImpl::EmitCall(std::ostream& out,
                             const ast::CallExpression* expr) {
  auto* call = builder_.Sem().Get(expr);
  auto* target = call->Target();

  if (target->Is<sem::Function>()) {
    return EmitFunctionCall(out, call);
  }
  if (auto* builtin = target->As<sem::Builtin>()) {
    return EmitBuiltinCall(out, call, builtin);
  }
  if (auto* cast = target->As<sem::TypeConversion>()) {
    return EmitTypeConversion(out, call, cast);
  }
  if (auto* ctor = target->As<sem::TypeConstructor>()) {
    return EmitTypeConstructor(out, call, ctor);
  }
  TINT_ICE(Writer, diagnostics_)
      << "unhandled call target: " << target->TypeInfo().name;
  return false;
}

bool GeneratorImpl::EmitFunctionCall(std::ostream& out, const sem::Call* call) {
  const auto& args = call->Arguments();
  auto* decl = call->Declaration();
  auto* ident = decl->target.name;

  auto name = builder_.Symbols().NameFor(ident->symbol);
  auto caller_sym = ident->symbol;

  out << name << "(";

  bool first = true;
  for (auto* arg : args) {
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

bool GeneratorImpl::EmitBuiltinCall(std::ostream& out,
                                    const sem::Call* call,
                                    const sem::Builtin* builtin) {
  auto* expr = call->Declaration();
  if (builtin->IsTexture()) {
    return EmitTextureCall(out, call, builtin);
  }
  if (builtin->Type() == sem::BuiltinType::kCountOneBits) {
    return EmitCountOneBitsCall(out, expr);
  }
  if (builtin->Type() == sem::BuiltinType::kSelect) {
    return EmitSelectCall(out, expr);
  }
  if (builtin->Type() == sem::BuiltinType::kDot) {
    return EmitDotCall(out, expr, builtin);
  }
  if (builtin->Type() == sem::BuiltinType::kModf) {
    return EmitModfCall(out, expr, builtin);
  }
  if (builtin->Type() == sem::BuiltinType::kFrexp) {
    return EmitFrexpCall(out, expr, builtin);
  }
  if (builtin->Type() == sem::BuiltinType::kDegrees) {
    return EmitDegreesCall(out, expr, builtin);
  }
  if (builtin->Type() == sem::BuiltinType::kRadians) {
    return EmitRadiansCall(out, expr, builtin);
  }
  if (builtin->Type() == sem::BuiltinType::kArrayLength) {
    return EmitArrayLength(out, expr);
  }
  if (builtin->Type() == sem::BuiltinType::kExtractBits) {
    return EmitExtractBits(out, expr);
  }
  if (builtin->Type() == sem::BuiltinType::kInsertBits) {
    return EmitInsertBits(out, expr);
  }
  if (builtin->Type() == sem::BuiltinType::kFma && version_.IsES()) {
    return EmitEmulatedFMA(out, expr);
  }
  if (builtin->Type() == sem::BuiltinType::kAbs &&
      TypeOf(expr->args[0])->UnwrapRef()->is_unsigned_scalar_or_vector()) {
    // GLSL does not support abs() on unsigned arguments. However, it's a no-op.
    return EmitExpression(out, expr->args[0]);
  }
  if ((builtin->Type() == sem::BuiltinType::kAny ||
       builtin->Type() == sem::BuiltinType::kAll) &&
      TypeOf(expr->args[0])->UnwrapRef()->is_scalar()) {
    // GLSL does not support any() or all() on scalar arguments. It's a no-op.
    return EmitExpression(out, expr->args[0]);
  }
  if (builtin->IsBarrier()) {
    return EmitBarrierCall(out, builtin);
  }
  if (builtin->IsAtomic()) {
    return EmitWorkgroupAtomicCall(out, expr, builtin);
  }
  auto name = generate_builtin_name(builtin);
  if (name.empty()) {
    return false;
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

bool GeneratorImpl::EmitTypeConversion(std::ostream& out,
                                       const sem::Call* call,
                                       const sem::TypeConversion* conv) {
  if (!EmitType(out, conv->Target(), ast::StorageClass::kNone,
                ast::Access::kReadWrite, "")) {
    return false;
  }
  out << "(";

  if (!EmitExpression(out, call->Arguments()[0]->Declaration())) {
    return false;
  }

  out << ")";
  return true;
}

bool GeneratorImpl::EmitTypeConstructor(std::ostream& out,
                                        const sem::Call* call,
                                        const sem::TypeConstructor* ctor) {
  auto* type = ctor->ReturnType();

  // If the type constructor is empty then we need to construct with the zero
  // value for all components.
  if (call->Arguments().empty()) {
    return EmitZeroValue(out, type);
  }

  auto it = structure_builders_.find(As<sem::Struct>(type));
  if (it != structure_builders_.end()) {
    out << it->second << "(";
  } else {
    if (!EmitType(out, type, ast::StorageClass::kNone, ast::Access::kReadWrite,
                  "")) {
      return false;
    }
    out << "(";
  }

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

bool GeneratorImpl::EmitWorkgroupAtomicCall(std::ostream& out,
                                            const ast::CallExpression* expr,
                                            const sem::Builtin* builtin) {
  auto call = [&](const char* name) {
    out << name;
    {
      ScopedParen sp(out);
      for (size_t i = 0; i < expr->args.size(); i++) {
        auto* arg = expr->args[i];
        if (i > 0) {
          out << ", ";
        }
        if (!EmitExpression(out, arg)) {
          return false;
        }
      }
    }
    return true;
  };

  switch (builtin->Type()) {
    case sem::BuiltinType::kAtomicLoad: {
      // GLSL does not have an atomicLoad, so we emulate it with
      // atomicOr using 0 as the OR value
      out << "atomicOr";
      {
        ScopedParen sp(out);
        if (!EmitExpression(out, expr->args[0])) {
          return false;
        }
        out << ", 0";
        if (builtin->ReturnType()->Is<sem::U32>()) {
          out << "u";
        }
      }
      return true;
    }
    case sem::BuiltinType::kAtomicCompareExchangeWeak: {
      return CallBuiltinHelper(
          out, expr, builtin,
          [&](TextBuffer* b, const std::vector<std::string>& params) {
            {
              auto pre = line(b);
              if (!EmitTypeAndName(pre, builtin->ReturnType(),
                                   ast::StorageClass::kNone,
                                   ast::Access::kUndefined, "result")) {
                return false;
              }
              pre << ";";
            }
            {
              auto pre = line(b);
              pre << "result.x = atomicCompSwap";
              {
                ScopedParen sp(pre);
                pre << params[0];
                pre << ", " << params[1];
                pre << ", " << params[2];
              }
              pre << ";";
            }
            {
              auto pre = line(b);
              pre << "result.y = result.x == " << params[2] << " ? ";
              if (TypeOf(expr->args[2])->Is<sem::U32>()) {
                pre << "1u : 0u;";
              } else {
                pre << "1 : 0;";
              }
            }
            line(b) << "return result;";
            return true;
          });
    }

    case sem::BuiltinType::kAtomicAdd:
    case sem::BuiltinType::kAtomicSub:
      return call("atomicAdd");

    case sem::BuiltinType::kAtomicMax:
      return call("atomicMax");

    case sem::BuiltinType::kAtomicMin:
      return call("atomicMin");

    case sem::BuiltinType::kAtomicAnd:
      return call("atomicAnd");

    case sem::BuiltinType::kAtomicOr:
      return call("atomicOr");

    case sem::BuiltinType::kAtomicXor:
      return call("atomicXor");

    case sem::BuiltinType::kAtomicExchange:
    case sem::BuiltinType::kAtomicStore:
      // GLSL does not have an atomicStore, so we emulate it with
      // atomicExchange.
      return call("atomicExchange");

    default:
      break;
  }

  TINT_UNREACHABLE(Writer, diagnostics_)
      << "unsupported atomic builtin: " << builtin->Type();
  return false;
}

bool GeneratorImpl::EmitArrayLength(std::ostream& out,
                                    const ast::CallExpression* expr) {
  out << "uint(";
  if (!EmitExpression(out, expr->args[0])) {
    return false;
  }
  out << ".length())";
  return true;
}

bool GeneratorImpl::EmitExtractBits(std::ostream& out,
                                    const ast::CallExpression* expr) {
  out << "bitfieldExtract(";
  if (!EmitExpression(out, expr->args[0])) {
    return false;
  }
  out << ", int(";
  if (!EmitExpression(out, expr->args[1])) {
    return false;
  }
  out << "), int(";
  if (!EmitExpression(out, expr->args[2])) {
    return false;
  }
  out << "))";
  return true;
}

bool GeneratorImpl::EmitInsertBits(std::ostream& out,
                                   const ast::CallExpression* expr) {
  out << "bitfieldInsert(";
  if (!EmitExpression(out, expr->args[0])) {
    return false;
  }
  out << ", ";
  if (!EmitExpression(out, expr->args[1])) {
    return false;
  }
  out << ", int(";
  if (!EmitExpression(out, expr->args[2])) {
    return false;
  }
  out << "), int(";
  if (!EmitExpression(out, expr->args[3])) {
    return false;
  }
  out << "))";
  return true;
}

bool GeneratorImpl::EmitEmulatedFMA(std::ostream& out,
                                    const ast::CallExpression* expr) {
  out << "((";
  if (!EmitExpression(out, expr->args[0])) {
    return false;
  }
  out << ") * (";
  if (!EmitExpression(out, expr->args[1])) {
    return false;
  }
  out << ") + (";
  if (!EmitExpression(out, expr->args[2])) {
    return false;
  }
  out << "))";
  return true;
}

bool GeneratorImpl::EmitCountOneBitsCall(std::ostream& out,
                                         const ast::CallExpression* expr) {
  // GLSL's bitCount returns an integer type, so cast it to the appropriate
  // unsigned type.
  if (!EmitType(out, TypeOf(expr)->UnwrapRef(), ast::StorageClass::kNone,
                ast::Access::kReadWrite, "")) {
    return false;
  }
  out << "(bitCount(";

  if (!EmitExpression(out, expr->args[0])) {
    return false;
  }
  out << "))";
  return true;
}

bool GeneratorImpl::EmitSelectCall(std::ostream& out,
                                   const ast::CallExpression* expr) {
  auto* expr_false = expr->args[0];
  auto* expr_true = expr->args[1];
  auto* expr_cond = expr->args[2];
  // GLSL does not support ternary expressions with a bool vector conditional,
  // but it does support mix() with same.
  if (TypeOf(expr_cond)->UnwrapRef()->is_bool_vector()) {
    out << "mix(";
    if (!EmitExpression(out, expr_false)) {
      return false;
    }
    out << ", ";
    if (!EmitExpression(out, expr_true)) {
      return false;
    }
    out << ", ";
    if (!EmitExpression(out, expr_cond)) {
      return false;
    }
    out << ")";
    return true;
  }
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

bool GeneratorImpl::EmitDotCall(std::ostream& out,
                                const ast::CallExpression* expr,
                                const sem::Builtin* builtin) {
  auto* vec_ty = builtin->Parameters()[0]->Type()->As<sem::Vector>();
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
        std::stringstream s;
        if (!EmitType(s, vec_ty->type(), ast::StorageClass::kNone,
                      ast::Access::kRead, "")) {
          return "";
        }
        v = s.str();
      }
      {  // (u)int tint_int_dot([i|u]vecN a, [i|u]vecN b) {
        auto l = line(&b);
        if (!EmitType(l, vec_ty->type(), ast::StorageClass::kNone,
                      ast::Access::kRead, "")) {
          return "";
        }
        l << " " << fn_name << "(";
        if (!EmitType(l, vec_ty, ast::StorageClass::kNone, ast::Access::kRead,
                      "")) {
          return "";
        }
        l << " a, ";
        if (!EmitType(l, vec_ty, ast::StorageClass::kNone, ast::Access::kRead,
                      "")) {
          return "";
        }
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
    if (fn.empty()) {
      return false;
    }
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

bool GeneratorImpl::EmitModfCall(std::ostream& out,
                                 const ast::CallExpression* expr,
                                 const sem::Builtin* builtin) {
  if (expr->args.size() == 1) {
    return CallBuiltinHelper(
        out, expr, builtin,
        [&](TextBuffer* b, const std::vector<std::string>& params) {
          // Emit the builtin return type unique to this overload. This does not
          // exist in the AST, so it will not be generated in Generate().
          if (!EmitStructType(&helpers_,
                              builtin->ReturnType()->As<sem::Struct>())) {
            return false;
          }

          {
            auto l = line(b);
            if (!EmitType(l, builtin->ReturnType(), ast::StorageClass::kNone,
                          ast::Access::kUndefined, "")) {
              return false;
            }
            l << " result;";
          }
          line(b) << "result.fract = modf(" << params[0] << ", result.whole);";
          line(b) << "return result;";
          return true;
        });
  }

  // DEPRECATED
  out << "modf";
  ScopedParen sp(out);
  if (!EmitExpression(out, expr->args[0])) {
    return false;
  }
  out << ", ";
  if (!EmitExpression(out, expr->args[1])) {
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitFrexpCall(std::ostream& out,
                                  const ast::CallExpression* expr,
                                  const sem::Builtin* builtin) {
  if (expr->args.size() == 1) {
    return CallBuiltinHelper(
        out, expr, builtin,
        [&](TextBuffer* b, const std::vector<std::string>& params) {
          // Emit the builtin return type unique to this overload. This does not
          // exist in the AST, so it will not be generated in Generate().
          if (!EmitStructType(&helpers_,
                              builtin->ReturnType()->As<sem::Struct>())) {
            return false;
          }

          {
            auto l = line(b);
            if (!EmitType(l, builtin->ReturnType(), ast::StorageClass::kNone,
                          ast::Access::kUndefined, "")) {
              return false;
            }
            l << " result;";
          }
          line(b) << "result.sig = frexp(" << params[0] << ", result.exp);";
          line(b) << "return result;";
          return true;
        });
  }
  // DEPRECATED
  // Exponent is an integer in WGSL, but HLSL wants a float.
  // We need to make the call with a temporary float, and then cast.
  return CallBuiltinHelper(
      out, expr, builtin,
      [&](TextBuffer* b, const std::vector<std::string>& params) {
        auto* significand_ty = builtin->Parameters()[0]->Type();
        auto significand = params[0];
        auto* exponent_ty = builtin->Parameters()[1]->Type();
        auto exponent = params[1];

        std::string width;
        if (auto* vec = significand_ty->As<sem::Vector>()) {
          width = std::to_string(vec->Width());
        }

        // Exponent is an integer, which HLSL does not have an overload for.
        // We need to cast from a float.
        line(b) << "float" << width << " float_exp;";
        line(b) << "float" << width << " significand = frexp(" << significand
                << ", float_exp);";
        {
          auto l = line(b);
          l << exponent << " = ";
          if (!EmitType(l, exponent_ty->UnwrapPtr(), ast::StorageClass::kNone,
                        ast::Access::kUndefined, "")) {
            return false;
          }
          l << "(float_exp);";
        }
        line(b) << "return significand;";
        return true;
      });
}

bool GeneratorImpl::EmitDegreesCall(std::ostream& out,
                                    const ast::CallExpression* expr,
                                    const sem::Builtin* builtin) {
  return CallBuiltinHelper(
      out, expr, builtin,
      [&](TextBuffer* b, const std::vector<std::string>& params) {
        line(b) << "return " << params[0] << " * " << std::setprecision(20)
                << sem::kRadToDeg << ";";
        return true;
      });
}

bool GeneratorImpl::EmitRadiansCall(std::ostream& out,
                                    const ast::CallExpression* expr,
                                    const sem::Builtin* builtin) {
  return CallBuiltinHelper(
      out, expr, builtin,
      [&](TextBuffer* b, const std::vector<std::string>& params) {
        line(b) << "return " << params[0] << " * " << std::setprecision(20)
                << sem::kDegToRad << ";";
        return true;
      });
}

bool GeneratorImpl::EmitBarrierCall(std::ostream& out,
                                    const sem::Builtin* builtin) {
  // TODO(crbug.com/tint/661): Combine sequential barriers to a single
  // instruction.
  if (builtin->Type() == sem::BuiltinType::kWorkgroupBarrier) {
    out << "barrier()";
  } else if (builtin->Type() == sem::BuiltinType::kStorageBarrier) {
    out << "{ barrier(); memoryBarrierBuffer(); }";
  } else {
    TINT_UNREACHABLE(Writer, diagnostics_)
        << "unexpected barrier builtin type " << sem::str(builtin->Type());
    return false;
  }
  return true;
}

const ast::Expression* GeneratorImpl::CreateF32Zero(
    const sem::Statement* stmt) {
  auto* zero = builder_.Expr(0.0f);
  auto* f32 = builder_.create<sem::F32>();
  auto* sem_zero = builder_.create<sem::Expression>(
      zero, f32, stmt, sem::Constant{}, /* has_side_effects */ false);
  builder_.Sem().Add(zero, sem_zero);
  return zero;
}

bool GeneratorImpl::EmitTextureCall(std::ostream& out,
                                    const sem::Call* call,
                                    const sem::Builtin* builtin) {
  using Usage = sem::ParameterUsage;

  auto& signature = builtin->Signature();
  auto* expr = call->Declaration();
  auto arguments = expr->args;

  // Returns the argument with the given usage
  auto arg = [&](Usage usage) {
    int idx = signature.IndexOf(usage);
    return (idx >= 0) ? arguments[idx] : nullptr;
  };

  auto* texture = arg(Usage::kTexture);
  if (!texture) {
    TINT_ICE(Writer, diagnostics_) << "missing texture argument";
    return false;
  }

  auto* texture_type = TypeOf(texture)->UnwrapRef()->As<sem::Texture>();

  switch (builtin->Type()) {
    case sem::BuiltinType::kTextureDimensions: {
      if (texture_type->Is<sem::StorageTexture>()) {
        out << "imageSize(";
      } else {
        out << "textureSize(";
      }
      if (!EmitExpression(out, texture)) {
        return false;
      }

      // The LOD parameter is mandatory on textureSize() for non-multisampled
      // textures.
      if (!texture_type->Is<sem::StorageTexture>() &&
          !texture_type->Is<sem::MultisampledTexture>() &&
          !texture_type->Is<sem::DepthMultisampledTexture>()) {
        out << ", ";
        if (auto* level_arg = arg(Usage::kLevel)) {
          if (!EmitExpression(out, level_arg)) {
            return false;
          }
        } else {
          out << "0";
        }
      }
      out << ")";
      // textureSize() on array samplers returns the array size in the
      // final component, so strip it out.
      if (texture_type->dim() == ast::TextureDimension::k2dArray ||
          texture_type->dim() == ast::TextureDimension::kCubeArray) {
        out << ".xy";
      }
      return true;
    }
    case sem::BuiltinType::kTextureNumLayers: {
      if (texture_type->Is<sem::StorageTexture>()) {
        out << "imageSize(";
      } else {
        out << "textureSize(";
      }
      // textureSize() on sampler2dArray returns the array size in the
      // final component, so return it
      if (!EmitExpression(out, texture)) {
        return false;
      }
      // The LOD parameter is mandatory on textureSize() for non-multisampled
      // textures.
      if (!texture_type->Is<sem::StorageTexture>() &&
          !texture_type->Is<sem::MultisampledTexture>() &&
          !texture_type->Is<sem::DepthMultisampledTexture>()) {
        out << ", ";
        if (auto* level_arg = arg(Usage::kLevel)) {
          if (!EmitExpression(out, level_arg)) {
            return false;
          }
        } else {
          out << "0";
        }
      }
      out << ").z";
      return true;
    }
    case sem::BuiltinType::kTextureNumLevels: {
      out << "textureQueryLevels(";
      if (!EmitExpression(out, texture)) {
        return false;
      }
      out << ")";
      return true;
    }
    case sem::BuiltinType::kTextureNumSamples: {
      out << "textureSamples(";
      if (!EmitExpression(out, texture)) {
        return false;
      }
      out << ")";
      return true;
    }
    default:
      break;
  }

  uint32_t glsl_ret_width = 4u;
  bool append_depth_ref_to_coords = true;
  bool is_depth = texture_type->Is<sem::DepthTexture>();

  switch (builtin->Type()) {
    case sem::BuiltinType::kTextureSample:
    case sem::BuiltinType::kTextureSampleBias:
      out << "texture";
      if (is_depth) {
        glsl_ret_width = 1u;
      }
      break;
    case sem::BuiltinType::kTextureSampleLevel:
      out << "textureLod";
      if (is_depth) {
        glsl_ret_width = 1u;
      }
      break;
    case sem::BuiltinType::kTextureGather:
    case sem::BuiltinType::kTextureGatherCompare:
      out << "textureGather";
      append_depth_ref_to_coords = false;
      break;
    case sem::BuiltinType::kTextureSampleGrad:
      out << "textureGrad";
      break;
    case sem::BuiltinType::kTextureSampleCompare:
    case sem::BuiltinType::kTextureSampleCompareLevel:
      out << "texture";
      glsl_ret_width = 1;
      break;
    case sem::BuiltinType::kTextureLoad:
      out << "texelFetch";
      break;
    case sem::BuiltinType::kTextureStore:
      out << "imageStore";
      break;
    default:
      diagnostics_.add_error(
          diag::System::Writer,
          "Internal compiler error: Unhandled texture builtin '" +
              std::string(builtin->str()) + "'");
      return false;
  }

  if (builtin->Signature().IndexOf(sem::ParameterUsage::kOffset) >= 0) {
    out << "Offset";
  }

  out << "(";

  if (!EmitExpression(out, texture))
    return false;

  out << ", ";

  auto* param_coords = arg(Usage::kCoords);
  if (!param_coords) {
    TINT_ICE(Writer, diagnostics_) << "missing coords argument";
    return false;
  }

  if (auto* array_index = arg(Usage::kArrayIndex)) {
    // Array index needs to be appended to the coordinates.
    param_coords =
        AppendVector(&builder_, param_coords, array_index)->Declaration();
  }

  // GLSL requires Dref to be appended to the coordinates, *unless* it's
  // samplerCubeArrayShadow, in which case it will be handled as a separate
  // parameter.
  if (texture_type->dim() == ast::TextureDimension::kCubeArray) {
    append_depth_ref_to_coords = false;
  }

  if (is_depth && append_depth_ref_to_coords) {
    auto* depth_ref = arg(Usage::kDepthRef);
    if (!depth_ref) {
      // Sampling a depth texture in GLSL always requires a depth reference, so
      // append zero here.
      depth_ref = CreateF32Zero(builder_.Sem().Get(param_coords)->Stmt());
    }
    param_coords =
        AppendVector(&builder_, param_coords, depth_ref)->Declaration();
  }

  if (!EmitExpression(out, param_coords)) {
    return false;
  }

  for (auto usage : {Usage::kLevel, Usage::kDdx, Usage::kDdy,
                     Usage::kSampleIndex, Usage::kValue}) {
    if (auto* e = arg(usage)) {
      out << ", ";
      if (usage == Usage::kLevel && is_depth) {
        // WGSL's textureSampleLevel() "level" param is i32 for depth textures,
        // whereas GLSL's textureLod() "lod" param is always float, so cast it.
        out << "float(";
        if (!EmitExpression(out, e)) {
          return false;
        }
        out << ")";
      } else if (!EmitExpression(out, e)) {
        return false;
      }
    }
  }

  // GLSL's textureGather always requires a refZ parameter.
  if (is_depth && builtin->Type() == sem::BuiltinType::kTextureGather) {
    out << ", 0.0";
  }

  // [1] samplerCubeArrayShadow requires a separate depthRef parameter
  if (is_depth && !append_depth_ref_to_coords) {
    if (auto* e = arg(Usage::kDepthRef)) {
      out << ", ";
      if (!EmitExpression(out, e)) {
        return false;
      }
    } else if (builtin->Type() == sem::BuiltinType::kTextureSample) {
      out << ", 0.0f";
    }
  }

  for (auto usage : {Usage::kOffset, Usage::kComponent, Usage::kBias}) {
    if (auto* e = arg(usage)) {
      out << ", ";
      if (!EmitExpression(out, e)) {
        return false;
      }
    }
  }

  out << ")";

  if (builtin->ReturnType()->Is<sem::Void>()) {
    return true;
  }
  // If the builtin return type does not match the number of elements of the
  // GLSL builtin, we need to swizzle the expression to generate the correct
  // number of components.
  uint32_t wgsl_ret_width = 1;
  if (auto* vec = builtin->ReturnType()->As<sem::Vector>()) {
    wgsl_ret_width = vec->Width();
  }
  if (wgsl_ret_width < glsl_ret_width) {
    out << ".";
    for (uint32_t i = 0; i < wgsl_ret_width; i++) {
      out << "xyz"[i];
    }
  }
  if (wgsl_ret_width > glsl_ret_width) {
    TINT_ICE(Writer, diagnostics_)
        << "WGSL return width (" << wgsl_ret_width
        << ") is wider than GLSL return width (" << glsl_ret_width << ") for "
        << builtin->Type();
    return false;
  }

  return true;
}

std::string GeneratorImpl::generate_builtin_name(const sem::Builtin* builtin) {
  switch (builtin->Type()) {
    case sem::BuiltinType::kAbs:
    case sem::BuiltinType::kAcos:
    case sem::BuiltinType::kAll:
    case sem::BuiltinType::kAny:
    case sem::BuiltinType::kAsin:
    case sem::BuiltinType::kAtan:
    case sem::BuiltinType::kCeil:
    case sem::BuiltinType::kClamp:
    case sem::BuiltinType::kCos:
    case sem::BuiltinType::kCosh:
    case sem::BuiltinType::kCross:
    case sem::BuiltinType::kDeterminant:
    case sem::BuiltinType::kDistance:
    case sem::BuiltinType::kDot:
    case sem::BuiltinType::kExp:
    case sem::BuiltinType::kExp2:
    case sem::BuiltinType::kFloor:
    case sem::BuiltinType::kFrexp:
    case sem::BuiltinType::kLdexp:
    case sem::BuiltinType::kLength:
    case sem::BuiltinType::kLog:
    case sem::BuiltinType::kLog2:
    case sem::BuiltinType::kMax:
    case sem::BuiltinType::kMin:
    case sem::BuiltinType::kModf:
    case sem::BuiltinType::kNormalize:
    case sem::BuiltinType::kPow:
    case sem::BuiltinType::kReflect:
    case sem::BuiltinType::kRefract:
    case sem::BuiltinType::kRound:
    case sem::BuiltinType::kSign:
    case sem::BuiltinType::kSin:
    case sem::BuiltinType::kSinh:
    case sem::BuiltinType::kSqrt:
    case sem::BuiltinType::kStep:
    case sem::BuiltinType::kTan:
    case sem::BuiltinType::kTanh:
    case sem::BuiltinType::kTranspose:
    case sem::BuiltinType::kTrunc:
      return builtin->str();
    case sem::BuiltinType::kAtan2:
      return "atan";
    case sem::BuiltinType::kCountOneBits:
      return "bitCount";
    case sem::BuiltinType::kDpdx:
      return "dFdx";
    case sem::BuiltinType::kDpdxCoarse:
      if (version_.IsES()) {
        return "dFdx";
      }
      return "dFdxCoarse";
    case sem::BuiltinType::kDpdxFine:
      if (version_.IsES()) {
        return "dFdx";
      }
      return "dFdxFine";
    case sem::BuiltinType::kDpdy:
      return "dFdy";
    case sem::BuiltinType::kDpdyCoarse:
      if (version_.IsES()) {
        return "dFdy";
      }
      return "dFdyCoarse";
    case sem::BuiltinType::kDpdyFine:
      if (version_.IsES()) {
        return "dFdy";
      }
      return "dFdyFine";
    case sem::BuiltinType::kFaceForward:
      return "faceforward";
    case sem::BuiltinType::kFract:
      return "fract";
    case sem::BuiltinType::kFma:
      return "fma";
    case sem::BuiltinType::kFwidth:
    case sem::BuiltinType::kFwidthCoarse:
    case sem::BuiltinType::kFwidthFine:
      return "fwidth";
    case sem::BuiltinType::kInverseSqrt:
      return "inversesqrt";
    case sem::BuiltinType::kMix:
      return "mix";
    case sem::BuiltinType::kPack2x16float:
      return "packHalf2x16";
    case sem::BuiltinType::kPack2x16snorm:
      return "packSnorm2x16";
    case sem::BuiltinType::kPack2x16unorm:
      return "packUnorm2x16";
    case sem::BuiltinType::kPack4x8snorm:
      return "packSnorm4x8";
    case sem::BuiltinType::kPack4x8unorm:
      return "packUnorm4x8";
    case sem::BuiltinType::kReverseBits:
      return "bitfieldReverse";
    case sem::BuiltinType::kSmoothstep:
    case sem::BuiltinType::kSmoothStep:
      return "smoothstep";
    case sem::BuiltinType::kUnpack2x16float:
      return "unpackHalf2x16";
    case sem::BuiltinType::kUnpack2x16snorm:
      return "unpackSnorm2x16";
    case sem::BuiltinType::kUnpack2x16unorm:
      return "unpackUnorm2x16";
    case sem::BuiltinType::kUnpack4x8snorm:
      return "unpackSnorm4x8";
    case sem::BuiltinType::kUnpack4x8unorm:
      return "unpackUnorm4x8";
    default:
      diagnostics_.add_error(
          diag::System::Writer,
          "Unknown builtin method: " + std::string(builtin->str()));
  }

  return "";
}

bool GeneratorImpl::EmitCase(const ast::CaseStatement* stmt) {
  if (stmt->IsDefault()) {
    line() << "default: {";
  } else {
    for (auto* selector : stmt->selectors) {
      auto out = line();
      out << "case ";
      if (!EmitLiteral(out, selector)) {
        return false;
      }
      out << ":";
      if (selector == stmt->selectors.back()) {
        out << " {";
      }
    }
  }

  {
    ScopedIndent si(this);
    if (!EmitStatements(stmt->body->statements)) {
      return false;
    }
    if (!last_is_break_or_fallthrough(stmt->body)) {
      line() << "break;";
    }
  }

  line() << "}";

  return true;
}

bool GeneratorImpl::EmitContinue(const ast::ContinueStatement*) {
  if (!emit_continuing_()) {
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

bool GeneratorImpl::EmitExpression(std::ostream& out,
                                   const ast::Expression* expr) {
  if (auto* a = expr->As<ast::IndexAccessorExpression>()) {
    return EmitIndexAccessor(out, a);
  }
  if (auto* b = expr->As<ast::BinaryExpression>()) {
    return EmitBinary(out, b);
  }
  if (auto* b = expr->As<ast::BitcastExpression>()) {
    return EmitBitcast(out, b);
  }
  if (auto* c = expr->As<ast::CallExpression>()) {
    return EmitCall(out, c);
  }
  if (auto* i = expr->As<ast::IdentifierExpression>()) {
    return EmitIdentifier(out, i);
  }
  if (auto* l = expr->As<ast::LiteralExpression>()) {
    return EmitLiteral(out, l);
  }
  if (auto* m = expr->As<ast::MemberAccessorExpression>()) {
    return EmitMemberAccessor(out, m);
  }
  if (auto* u = expr->As<ast::UnaryOpExpression>()) {
    return EmitUnaryOp(out, u);
  }

  diagnostics_.add_error(
      diag::System::Writer,
      "unknown expression type: " + std::string(expr->TypeInfo().name));
  return false;
}

bool GeneratorImpl::EmitIdentifier(std::ostream& out,
                                   const ast::IdentifierExpression* expr) {
  out << builder_.Symbols().NameFor(expr->symbol);
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
      if (!EmitStatementsWithIndent({stmt->else_statement})) {
        return false;
      }
    }
  }
  line() << "}";

  return true;
}

bool GeneratorImpl::EmitFunction(const ast::Function* func) {
  auto* sem = builder_.Sem().Get(func);

  if (ast::HasAttribute<ast::InternalAttribute>(func->attributes)) {
    // An internal function. Do not emit.
    return true;
  }

  {
    auto out = line();
    auto name = builder_.Symbols().NameFor(func->symbol);
    if (!EmitType(out, sem->ReturnType(), ast::StorageClass::kNone,
                  ast::Access::kReadWrite, "")) {
      return false;
    }

    out << " " << name << "(";

    bool first = true;

    for (auto* v : sem->Parameters()) {
      if (!first) {
        out << ", ";
      }
      first = false;

      auto const* type = v->Type();

      if (auto* ptr = type->As<sem::Pointer>()) {
        // Transform pointer parameters in to `inout` parameters.
        // The WGSL spec is highly restrictive in what can be passed in pointer
        // parameters, which allows for this transformation. See:
        // https://gpuweb.github.io/gpuweb/wgsl/#function-restriction
        out << "inout ";
        type = ptr->StoreType();
      }

      // Note: WGSL only allows for StorageClass::kNone on parameters, however
      // the sanitizer transforms generates load / store functions for storage
      // or uniform buffers. These functions have a buffer parameter with
      // StorageClass::kStorage or StorageClass::kUniform. This is required to
      // correctly translate the parameter to a [RW]ByteAddressBuffer for
      // storage buffers and a uint4[N] for uniform buffers.
      if (!EmitTypeAndName(
              out, type, v->StorageClass(), v->Access(),
              builder_.Symbols().NameFor(v->Declaration()->symbol))) {
        return false;
      }
    }
    out << ") {";
  }

  if (!EmitStatementsWithIndent(func->body->statements)) {
    return false;
  }

  line() << "}";
  line();

  return true;
}

bool GeneratorImpl::EmitGlobalVariable(const ast::Variable* global) {
  if (global->is_const) {
    return EmitProgramConstVariable(global);
  }

  auto* sem = builder_.Sem().Get(global);
  switch (sem->StorageClass()) {
    case ast::StorageClass::kUniform:
      return EmitUniformVariable(sem);
    case ast::StorageClass::kStorage:
      return EmitStorageVariable(sem);
    case ast::StorageClass::kHandle:
      return EmitHandleVariable(sem);
    case ast::StorageClass::kPrivate:
      return EmitPrivateVariable(sem);
    case ast::StorageClass::kWorkgroup:
      return EmitWorkgroupVariable(sem);
    case ast::StorageClass::kInput:
    case ast::StorageClass::kOutput:
      return EmitIOVariable(sem);
    default:
      break;
  }

  TINT_ICE(Writer, diagnostics_)
      << "unhandled storage class " << sem->StorageClass();
  return false;
}

bool GeneratorImpl::EmitUniformVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();
  auto* type = var->Type()->UnwrapRef();
  auto* str = type->As<sem::Struct>();
  if (!str) {
    TINT_ICE(Writer, builder_.Diagnostics())
        << "storage variable must be of struct type";
    return false;
  }
  ast::VariableBindingPoint bp = decl->BindingPoint();
  {
    auto out = line();
    out << "layout(binding = " << bp.binding->value;
    if (version_.IsDesktop()) {
      out << ", std140";
    }
    out << ") uniform " << UniqueIdentifier(StructName(str)) << " {";
  }
  EmitStructMembers(current_buffer_, str, /* emit_offsets */ true);
  auto name = builder_.Symbols().NameFor(decl->symbol);
  line() << "} " << name << ";";
  line();

  return true;
}

bool GeneratorImpl::EmitStorageVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();
  auto* type = var->Type()->UnwrapRef();
  auto* str = type->As<sem::Struct>();
  if (!str) {
    TINT_ICE(Writer, builder_.Diagnostics())
        << "storage variable must be of struct type";
    return false;
  }
  ast::VariableBindingPoint bp = decl->BindingPoint();
  line() << "layout(binding = " << bp.binding->value << ", std430) buffer "
         << UniqueIdentifier(StructName(str)) << " {";
  EmitStructMembers(current_buffer_, str, /* emit_offsets */ true);
  auto name = builder_.Symbols().NameFor(decl->symbol);
  line() << "} " << name << ";";
  return true;
}

bool GeneratorImpl::EmitHandleVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();
  auto out = line();

  auto name = builder_.Symbols().NameFor(decl->symbol);
  auto* type = var->Type()->UnwrapRef();
  if (type->Is<sem::Sampler>()) {
    // GLSL ignores Sampler variables.
    return true;
  }
  if (auto* storage = type->As<sem::StorageTexture>()) {
    out << "layout(" << convert_texel_format_to_glsl(storage->texel_format())
        << ") ";
  }
  if (!EmitTypeAndName(out, type, var->StorageClass(), var->Access(), name)) {
    return false;
  }

  out << ";";
  return true;
}

bool GeneratorImpl::EmitPrivateVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();
  auto out = line();

  auto name = builder_.Symbols().NameFor(decl->symbol);
  auto* type = var->Type()->UnwrapRef();
  if (!EmitTypeAndName(out, type, var->StorageClass(), var->Access(), name)) {
    return false;
  }

  out << " = ";
  if (auto* constructor = decl->constructor) {
    if (!EmitExpression(out, constructor)) {
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

  out << "shared ";

  auto name = builder_.Symbols().NameFor(decl->symbol);
  auto* type = var->Type()->UnwrapRef();
  if (!EmitTypeAndName(out, type, var->StorageClass(), var->Access(), name)) {
    return false;
  }

  if (auto* constructor = decl->constructor) {
    out << " = ";
    if (!EmitExpression(out, constructor)) {
      return false;
    }
  }

  out << ";";
  return true;
}

bool GeneratorImpl::EmitIOVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();

  if (auto* b = ast::GetAttribute<ast::BuiltinAttribute>(decl->attributes)) {
    // Use of gl_SampleID requires the GL_OES_sample_variables extension
    if (RequiresOESSampleVariables(b->builtin)) {
      requires_oes_sample_variables_ = true;
    }
    // Do not emit builtin (gl_) variables.
    return true;
  }

  auto out = line();
  EmitAttributes(out, decl->attributes);
  EmitInterpolationQualifiers(out, decl->attributes);

  auto name = builder_.Symbols().NameFor(decl->symbol);
  auto* type = var->Type()->UnwrapRef();
  if (!EmitTypeAndName(out, type, var->StorageClass(), var->Access(), name)) {
    return false;
  }

  if (auto* constructor = decl->constructor) {
    out << " = ";
    if (!EmitExpression(out, constructor)) {
      return false;
    }
  }

  out << ";";
  return true;
}

void GeneratorImpl::EmitInterpolationQualifiers(
    std::ostream& out,
    const ast::AttributeList& attributes) {
  for (auto* attr : attributes) {
    if (auto* interpolate = attr->As<ast::InterpolateAttribute>()) {
      switch (interpolate->type) {
        case ast::InterpolationType::kPerspective:
        case ast::InterpolationType::kLinear:
          break;
        case ast::InterpolationType::kFlat:
          out << "flat ";
          break;
      }
      switch (interpolate->sampling) {
        case ast::InterpolationSampling::kCentroid:
          out << "centroid ";
          break;
        case ast::InterpolationSampling::kSample:
        case ast::InterpolationSampling::kCenter:
        case ast::InterpolationSampling::kNone:
          break;
      }
    }
  }
}

bool GeneratorImpl::EmitAttributes(std::ostream& out,
                                   const ast::AttributeList& attributes) {
  if (attributes.empty()) {
    return true;
  }
  bool first = true;
  for (auto* attr : attributes) {
    if (auto* location = attr->As<ast::LocationAttribute>()) {
      out << (first ? "layout(" : ", ");
      out << "location = " << std::to_string(location->value);
      first = false;
    }
  }
  if (!first) {
    out << ") ";
  }
  return true;
}

bool GeneratorImpl::EmitEntryPointFunction(const ast::Function* func) {
  auto* func_sem = builder_.Sem().Get(func);

  if (func->PipelineStage() == ast::PipelineStage::kFragment) {
    requires_default_precision_qualifier_ = true;
  }

  if (func->PipelineStage() == ast::PipelineStage::kCompute) {
    auto out = line();
    // Emit the layout(local_size) attributes.
    auto wgsize = func_sem->WorkgroupSize();
    out << "layout(";
    for (int i = 0; i < 3; i++) {
      if (i > 0) {
        out << ", ";
      }
      out << "local_size_" << (i == 0 ? "x" : i == 1 ? "y" : "z") << " = ";

      if (wgsize[i].overridable_const) {
        auto* global = builder_.Sem().Get<sem::GlobalVariable>(
            wgsize[i].overridable_const);
        if (!global->IsOverridable()) {
          TINT_ICE(Writer, builder_.Diagnostics())
              << "expected a pipeline-overridable constant";
        }
        out << kSpecConstantPrefix << global->ConstantId();
      } else {
        out << std::to_string(wgsize[i].value);
      }
    }
    out << ") in;";
  }

  // Emit original entry point signature
  {
    auto out = line();
    out << func->return_type->FriendlyName(builder_.Symbols()) << " "
        << builder_.Symbols().NameFor(func->symbol) << "(";

    bool first = true;

    // Emit entry point parameters.
    for (auto* var : func->params) {
      auto* sem = builder_.Sem().Get(var);
      auto* type = sem->Type();
      if (!type->Is<sem::Struct>()) {
        // ICE likely indicates that the CanonicalizeEntryPointIO transform was
        // not run, or a builtin parameter was added after it was run.
        TINT_ICE(Writer, diagnostics_)
            << "Unsupported non-struct entry point parameter";
      }

      if (!first) {
        out << ", ";
      }
      first = false;

      if (!EmitTypeAndName(out, type, sem->StorageClass(), sem->Access(),
                           builder_.Symbols().NameFor(var->symbol))) {
        return false;
      }
    }

    out << ") {";
  }

  // Emit original entry point function body
  {
    ScopedIndent si(this);
    if (func->PipelineStage() == ast::PipelineStage::kVertex) {
      line() << "gl_PointSize = 1.0;";
    }

    if (!EmitStatements(func->body->statements)) {
      return false;
    }

    if (!Is<ast::ReturnStatement>(func->body->Last())) {
      ast::ReturnStatement ret(ProgramID(), Source{});
      if (!EmitStatement(&ret)) {
        return false;
      }
    }
  }

  line() << "}";

  return true;
}

bool GeneratorImpl::EmitLiteral(std::ostream& out,
                                const ast::LiteralExpression* lit) {
  if (auto* l = lit->As<ast::BoolLiteralExpression>()) {
    out << (l->value ? "true" : "false");
  } else if (auto* fl = lit->As<ast::FloatLiteralExpression>()) {
    if (std::isinf(fl->value)) {
      out << (fl->value >= 0 ? "uintBitsToFloat(0x7f800000u)"
                             : "uintBitsToFloat(0xff800000u)");
    } else if (std::isnan(fl->value)) {
      out << "uintBitsToFloat(0x7fc00000u)";
    } else {
      out << FloatToString(fl->value) << "f";
    }
  } else if (auto* sl = lit->As<ast::SintLiteralExpression>()) {
    out << sl->value;
  } else if (auto* ul = lit->As<ast::UintLiteralExpression>()) {
    out << ul->value << "u";
  } else {
    diagnostics_.add_error(diag::System::Writer, "unknown literal type");
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitZeroValue(std::ostream& out, const sem::Type* type) {
  if (type->Is<sem::Bool>()) {
    out << "false";
  } else if (type->Is<sem::F32>()) {
    out << "0.0f";
  } else if (type->Is<sem::I32>()) {
    out << "0";
  } else if (type->Is<sem::U32>()) {
    out << "0u";
  } else if (auto* vec = type->As<sem::Vector>()) {
    if (!EmitType(out, type, ast::StorageClass::kNone, ast::Access::kReadWrite,
                  "")) {
      return false;
    }
    ScopedParen sp(out);
    for (uint32_t i = 0; i < vec->Width(); i++) {
      if (i != 0) {
        out << ", ";
      }
      if (!EmitZeroValue(out, vec->type())) {
        return false;
      }
    }
  } else if (auto* mat = type->As<sem::Matrix>()) {
    if (!EmitType(out, type, ast::StorageClass::kNone, ast::Access::kReadWrite,
                  "")) {
      return false;
    }
    ScopedParen sp(out);
    for (uint32_t i = 0; i < (mat->rows() * mat->columns()); i++) {
      if (i != 0) {
        out << ", ";
      }
      if (!EmitZeroValue(out, mat->type())) {
        return false;
      }
    }
  } else if (auto* str = type->As<sem::Struct>()) {
    if (!EmitType(out, type, ast::StorageClass::kNone, ast::Access::kUndefined,
                  "")) {
      return false;
    }
    bool first = true;
    out << "(";
    for (auto* member : str->Members()) {
      if (!first) {
        out << ", ";
      } else {
        first = false;
      }
      EmitZeroValue(out, member->Type());
    }
    out << ")";
  } else if (auto* array = type->As<sem::Array>()) {
    if (!EmitType(out, type, ast::StorageClass::kNone, ast::Access::kUndefined,
                  "")) {
      return false;
    }
    out << "(";
    for (uint32_t i = 0; i < array->Count(); i++) {
      if (i != 0) {
        out << ", ";
      }
      EmitZeroValue(out, array->ElemType());
    }
    out << ")";
  } else {
    diagnostics_.add_error(diag::System::Writer,
                           "Invalid type for zero emission: " +
                               type->FriendlyName(builder_.Symbols()));
    return false;
  }
  return true;
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
  std::stringstream cond_buf;
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
          out << TrimSuffix(cont_buf.lines[0].content, ";");
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

bool GeneratorImpl::EmitMemberAccessor(
    std::ostream& out,
    const ast::MemberAccessorExpression* expr) {
  if (!EmitExpression(out, expr->structure)) {
    return false;
  }
  out << ".";

  // Swizzles output the name directly
  if (builder_.Sem().Get(expr)->Is<sem::Swizzle>()) {
    out << builder_.Symbols().NameFor(expr->member->symbol);
  } else if (!EmitExpression(out, expr->member)) {
    return false;
  }

  return true;
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
  if (auto* a = stmt->As<ast::AssignmentStatement>()) {
    return EmitAssign(a);
  }
  if (auto* b = stmt->As<ast::BlockStatement>()) {
    return EmitBlock(b);
  }
  if (auto* b = stmt->As<ast::BreakStatement>()) {
    return EmitBreak(b);
  }
  if (auto* c = stmt->As<ast::CallStatement>()) {
    auto out = line();
    if (!EmitCall(out, c->expr)) {
      return false;
    }
    out << ";";
    return true;
  }
  if (auto* c = stmt->As<ast::ContinueStatement>()) {
    return EmitContinue(c);
  }
  if (auto* d = stmt->As<ast::DiscardStatement>()) {
    return EmitDiscard(d);
  }
  if (stmt->As<ast::FallthroughStatement>()) {
    line() << "/* fallthrough */";
    return true;
  }
  if (auto* i = stmt->As<ast::IfStatement>()) {
    return EmitIf(i);
  }
  if (auto* l = stmt->As<ast::LoopStatement>()) {
    return EmitLoop(l);
  }
  if (auto* l = stmt->As<ast::ForLoopStatement>()) {
    return EmitForLoop(l);
  }
  if (auto* r = stmt->As<ast::ReturnStatement>()) {
    return EmitReturn(r);
  }
  if (auto* s = stmt->As<ast::SwitchStatement>()) {
    return EmitSwitch(s);
  }
  if (auto* v = stmt->As<ast::VariableDeclStatement>()) {
    return EmitVariable(v->variable);
  }

  diagnostics_.add_error(
      diag::System::Writer,
      "unknown statement type: " + std::string(stmt->TypeInfo().name));
  return false;
}

bool GeneratorImpl::EmitSwitch(const ast::SwitchStatement* stmt) {
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
    for (auto* s : stmt->body) {
      if (!EmitCase(s)) {
        return false;
      }
    }
  }

  line() << "}";

  return true;
}

bool GeneratorImpl::EmitType(std::ostream& out,
                             const sem::Type* type,
                             ast::StorageClass storage_class,
                             ast::Access access,
                             const std::string& name,
                             bool* name_printed /* = nullptr */) {
  if (name_printed) {
    *name_printed = false;
  }
  switch (storage_class) {
    case ast::StorageClass::kInput: {
      out << "in ";
      break;
    }
    case ast::StorageClass::kOutput: {
      out << "out ";
      break;
    }
    case ast::StorageClass::kUniform:
    case ast::StorageClass::kHandle: {
      out << "uniform ";
      break;
    }
    default:
      break;
  }

  if (auto* ary = type->As<sem::Array>()) {
    const sem::Type* base_type = ary;
    std::vector<uint32_t> sizes;
    while (auto* arr = base_type->As<sem::Array>()) {
      sizes.push_back(arr->Count());
      base_type = arr->ElemType();
    }
    if (!EmitType(out, base_type, storage_class, access, "")) {
      return false;
    }
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
  } else if (type->Is<sem::Bool>()) {
    out << "bool";
  } else if (type->Is<sem::F32>()) {
    out << "float";
  } else if (type->Is<sem::I32>()) {
    out << "int";
  } else if (auto* mat = type->As<sem::Matrix>()) {
    TINT_ASSERT(Writer, mat->type()->Is<sem::F32>());
    out << "mat" << mat->columns();
    if (mat->rows() != mat->columns()) {
      out << "x" << mat->rows();
    }
  } else if (type->Is<sem::Pointer>()) {
    TINT_ICE(Writer, diagnostics_)
        << "Attempting to emit pointer type. These should have been removed "
           "with the InlinePointerLets transform";
    return false;
  } else if (type->Is<sem::Sampler>()) {
    return false;
  } else if (auto* str = type->As<sem::Struct>()) {
    out << StructName(str);
  } else if (auto* tex = type->As<sem::Texture>()) {
    if (tex->Is<sem::ExternalTexture>()) {
      TINT_ICE(Writer, diagnostics_)
          << "Multiplanar external texture transform was not run.";
      return false;
    }

    auto* storage = tex->As<sem::StorageTexture>();
    auto* ms = tex->As<sem::MultisampledTexture>();
    auto* depth_ms = tex->As<sem::DepthMultisampledTexture>();
    auto* sampled = tex->As<sem::SampledTexture>();

    out << "highp ";

    if (storage && storage->access() != ast::Access::kRead) {
      out << "writeonly ";
    }
    auto* subtype = sampled   ? sampled->type()
                    : storage ? storage->type()
                    : ms      ? ms->type()
                              : nullptr;
    if (!subtype || subtype->Is<sem::F32>()) {
    } else if (subtype->Is<sem::I32>()) {
      out << "i";
    } else if (subtype->Is<sem::U32>()) {
      out << "u";
    } else {
      TINT_ICE(Writer, diagnostics_) << "Unsupported texture type";
      return false;
    }

    out << (storage ? "image" : "sampler");

    switch (tex->dim()) {
      case ast::TextureDimension::k1d:
        out << "1D";
        break;
      case ast::TextureDimension::k2d:
        out << ((ms || depth_ms) ? "2DMS" : "2D");
        break;
      case ast::TextureDimension::k2dArray:
        out << ((ms || depth_ms) ? "2DMSArray" : "2DArray");
        break;
      case ast::TextureDimension::k3d:
        out << "3D";
        break;
      case ast::TextureDimension::kCube:
        out << "Cube";
        break;
      case ast::TextureDimension::kCubeArray:
        out << "CubeArray";
        break;
      default:
        TINT_UNREACHABLE(Writer, diagnostics_)
            << "unexpected TextureDimension " << tex->dim();
        return false;
    }
    if (tex->Is<sem::DepthTexture>()) {
      out << "Shadow";
    }
  } else if (type->Is<sem::U32>()) {
    out << "uint";
  } else if (auto* vec = type->As<sem::Vector>()) {
    auto width = vec->Width();
    if (vec->type()->Is<sem::F32>() && width >= 1 && width <= 4) {
      out << "vec" << width;
    } else if (vec->type()->Is<sem::I32>() && width >= 1 && width <= 4) {
      out << "ivec" << width;
    } else if (vec->type()->Is<sem::U32>() && width >= 1 && width <= 4) {
      out << "uvec" << width;
    } else if (vec->type()->Is<sem::Bool>() && width >= 1 && width <= 4) {
      out << "bvec" << width;
    } else {
      out << "vector<";
      if (!EmitType(out, vec->type(), storage_class, access, "")) {
        return false;
      }
      out << ", " << width << ">";
    }
  } else if (auto* atomic = type->As<sem::Atomic>()) {
    if (!EmitType(out, atomic->Type(), storage_class, access, name)) {
      return false;
    }
  } else if (type->Is<sem::Void>()) {
    out << "void";
  } else {
    diagnostics_.add_error(diag::System::Writer, "unknown type in EmitType");
    return false;
  }

  return true;
}

bool GeneratorImpl::EmitTypeAndName(std::ostream& out,
                                    const sem::Type* type,
                                    ast::StorageClass storage_class,
                                    ast::Access access,
                                    const std::string& name) {
  bool printed_name = false;
  if (!EmitType(out, type, storage_class, access, name, &printed_name)) {
    return false;
  }
  if (!name.empty() && !printed_name) {
    out << " " << name;
  }
  return true;
}

bool GeneratorImpl::EmitStructType(TextBuffer* b, const sem::Struct* str) {
  auto storage_class_uses = str->StorageClassUsage();
  line(b) << "struct " << StructName(str) << " {";
  EmitStructMembers(b, str, false);
  line(b) << "};";
  line(b);

  return true;
}

bool GeneratorImpl::EmitStructMembers(TextBuffer* b,
                                      const sem::Struct* str,
                                      bool emit_offsets) {
  ScopedIndent si(b);
  for (auto* mem : str->Members()) {
    auto name = builder_.Symbols().NameFor(mem->Name());

    auto* ty = mem->Type();

    auto out = line(b);

    // Note: offsets are unsupported on GLSL ES.
    if (emit_offsets && version_.IsDesktop() && mem->Offset() != 0) {
      out << "layout(offset=" << mem->Offset() << ") ";
    }
    if (!EmitTypeAndName(out, ty, ast::StorageClass::kNone,
                         ast::Access::kReadWrite, name)) {
      return false;
    }
    out << ";";
  }
  return true;
}

bool GeneratorImpl::EmitUnaryOp(std::ostream& out,
                                const ast::UnaryOpExpression* expr) {
  switch (expr->op) {
    case ast::UnaryOp::kIndirection:
    case ast::UnaryOp::kAddressOf:
      return EmitExpression(out, expr->expr);
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
  out << "(";

  if (!EmitExpression(out, expr->expr)) {
    return false;
  }

  out << ")";

  return true;
}

bool GeneratorImpl::EmitVariable(const ast::Variable* var) {
  auto* sem = builder_.Sem().Get(var);
  auto* type = sem->Type()->UnwrapRef();

  // TODO(dsinclair): Handle variable attributes
  if (!var->attributes.empty()) {
    diagnostics_.add_error(diag::System::Writer,
                           "Variable attributes are not handled yet");
    return false;
  }

  auto out = line();
  // TODO(senorblanco): handle const
  if (!EmitTypeAndName(out, type, sem->StorageClass(), sem->Access(),
                       builder_.Symbols().NameFor(var->symbol))) {
    return false;
  }

  out << " = ";

  if (var->constructor) {
    if (!EmitExpression(out, var->constructor)) {
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

bool GeneratorImpl::EmitProgramConstVariable(const ast::Variable* var) {
  for (auto* d : var->attributes) {
    if (!d->Is<ast::IdAttribute>()) {
      diagnostics_.add_error(diag::System::Writer,
                             "Decorated const values not valid");
      return false;
    }
  }
  if (!var->is_const) {
    diagnostics_.add_error(diag::System::Writer, "Expected a const value");
    return false;
  }

  auto* sem = builder_.Sem().Get(var);
  auto* type = sem->Type();

  auto* global = sem->As<sem::GlobalVariable>();
  if (global && global->IsOverridable()) {
    auto const_id = global->ConstantId();

    line() << "#ifndef " << kSpecConstantPrefix << const_id;

    if (var->constructor != nullptr) {
      auto out = line();
      out << "#define " << kSpecConstantPrefix << const_id << " ";
      if (!EmitExpression(out, var->constructor)) {
        return false;
      }
    } else {
      line() << "#error spec constant required for constant id " << const_id;
    }
    line() << "#endif";
    {
      auto out = line();
      out << "const ";
      if (!EmitTypeAndName(out, type, sem->StorageClass(), sem->Access(),
                           builder_.Symbols().NameFor(var->symbol))) {
        return false;
      }
      out << " = " << kSpecConstantPrefix << const_id << ";";
    }
  } else {
    auto out = line();
    out << "const ";
    if (!EmitTypeAndName(out, type, sem->StorageClass(), sem->Access(),
                         builder_.Symbols().NameFor(var->symbol))) {
      return false;
    }
    out << " = ";
    if (!EmitExpression(out, var->constructor)) {
      return false;
    }
    out << ";";
  }

  return true;
}

template <typename F>
bool GeneratorImpl::CallBuiltinHelper(std::ostream& out,
                                      const ast::CallExpression* call,
                                      const sem::Builtin* builtin,
                                      F&& build) {
  // Generate the helper function if it hasn't been created already
  auto fn = utils::GetOrCreate(builtins_, builtin, [&]() -> std::string {
    TextBuffer b;
    TINT_DEFER(helpers_.Append(b));

    auto fn_name =
        UniqueIdentifier(std::string("tint_") + sem::str(builtin->Type()));
    std::vector<std::string> parameter_names;
    {
      auto decl = line(&b);
      if (!EmitTypeAndName(decl, builtin->ReturnType(),
                           ast::StorageClass::kNone, ast::Access::kUndefined,
                           fn_name)) {
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
          if (auto* ptr = ty->As<sem::Pointer>()) {
            decl << "inout ";
            ty = ptr->StoreType();
          }
          if (!EmitTypeAndName(decl, ty, ast::StorageClass::kNone,
                               ast::Access::kUndefined, param_name)) {
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

sem::Type* GeneratorImpl::BoolTypeToUint(const sem::Type* type) {
  auto* u32 = builder_.create<sem::U32>();
  if (type->Is<sem::Bool>()) {
    return u32;
  } else if (auto* vec = type->As<sem::Vector>()) {
    return builder_.create<sem::Vector>(u32, vec->Width());
  } else {
    return nullptr;
  }
}

}  // namespace tint::writer::glsl
