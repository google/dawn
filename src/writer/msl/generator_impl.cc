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

#include "src/writer/msl/generator_impl.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <utility>
#include <vector>

#include "src/ast/alias.h"
#include "src/ast/bool_literal_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/disable_validation_decoration.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/float_literal_expression.h"
#include "src/ast/interpolate_decoration.h"
#include "src/ast/module.h"
#include "src/ast/override_decoration.h"
#include "src/ast/sint_literal_expression.h"
#include "src/ast/uint_literal_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/void.h"
#include "src/sem/array.h"
#include "src/sem/atomic_type.h"
#include "src/sem/bool_type.h"
#include "src/sem/call.h"
#include "src/sem/depth_multisampled_texture_type.h"
#include "src/sem/depth_texture_type.h"
#include "src/sem/f32_type.h"
#include "src/sem/function.h"
#include "src/sem/i32_type.h"
#include "src/sem/matrix_type.h"
#include "src/sem/member_accessor_expression.h"
#include "src/sem/multisampled_texture_type.h"
#include "src/sem/pointer_type.h"
#include "src/sem/reference_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/storage_texture_type.h"
#include "src/sem/struct.h"
#include "src/sem/type_constructor.h"
#include "src/sem/type_conversion.h"
#include "src/sem/u32_type.h"
#include "src/sem/variable.h"
#include "src/sem/vector_type.h"
#include "src/sem/void_type.h"
#include "src/transform/array_length_from_uniform.h"
#include "src/transform/canonicalize_entry_point_io.h"
#include "src/transform/external_texture_transform.h"
#include "src/transform/manager.h"
#include "src/transform/module_scope_var_to_entry_point_param.h"
#include "src/transform/pad_array_elements.h"
#include "src/transform/promote_initializers_to_const_var.h"
#include "src/transform/remove_phonies.h"
#include "src/transform/simplify_pointers.h"
#include "src/transform/unshadow.h"
#include "src/transform/vectorize_scalar_matrix_constructors.h"
#include "src/transform/wrap_arrays_in_structs.h"
#include "src/transform/zero_init_workgroup_memory.h"
#include "src/utils/defer.h"
#include "src/utils/map.h"
#include "src/utils/scoped_assignment.h"
#include "src/writer/float_to_string.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

bool last_is_break_or_fallthrough(const ast::BlockStatement* stmts) {
  return IsAnyOf<ast::BreakStatement, ast::FallthroughStatement>(stmts->Last());
}

class ScopedBitCast {
 public:
  ScopedBitCast(GeneratorImpl* generator,
                std::ostream& stream,
                const sem::Type* curr_type,
                const sem::Type* target_type)
      : s(stream) {
    auto* target_vec_type = target_type->As<sem::Vector>();

    // If we need to promote from scalar to vector, bitcast the scalar to the
    // vector element type.
    if (curr_type->is_scalar() && target_vec_type) {
      target_type = target_vec_type->type();
    }

    // Bit cast
    s << "as_type<";
    generator->EmitType(s, target_type, "");
    s << ">(";
  }

  ~ScopedBitCast() { s << ")"; }

 private:
  std::ostream& s;
};
}  // namespace

SanitizedResult::SanitizedResult() = default;
SanitizedResult::~SanitizedResult() = default;
SanitizedResult::SanitizedResult(SanitizedResult&&) = default;

SanitizedResult Sanitize(
    const Program* in,
    uint32_t buffer_size_ubo_index,
    uint32_t fixed_sample_mask,
    bool emit_vertex_point_size,
    bool disable_workgroup_init,
    const ArrayLengthFromUniformOptions& array_length_from_uniform) {
  transform::Manager manager;
  transform::DataMap internal_inputs;

  // Build the config for the internal ArrayLengthFromUniform transform.
  transform::ArrayLengthFromUniform::Config array_length_from_uniform_cfg(
      array_length_from_uniform.ubo_binding);
  if (!array_length_from_uniform.bindpoint_to_size_index.empty()) {
    // If |array_length_from_uniform| bindings are provided, use that config.
    array_length_from_uniform_cfg.bindpoint_to_size_index =
        array_length_from_uniform.bindpoint_to_size_index;
  } else {
    // If the binding map is empty, use the deprecated |buffer_size_ubo_index|
    // and automatically choose indices using the binding numbers.
    array_length_from_uniform_cfg = transform::ArrayLengthFromUniform::Config(
        sem::BindingPoint{0, buffer_size_ubo_index});
    // Use the SSBO binding numbers as the indices for the buffer size lookups.
    for (auto* var : in->AST().GlobalVariables()) {
      auto* global = in->Sem().Get<sem::GlobalVariable>(var);
      if (global && global->StorageClass() == ast::StorageClass::kStorage) {
        array_length_from_uniform_cfg.bindpoint_to_size_index.emplace(
            global->BindingPoint(), global->BindingPoint().binding);
      }
    }
  }

  // Build the configs for the internal CanonicalizeEntryPointIO transform.
  auto entry_point_io_cfg = transform::CanonicalizeEntryPointIO::Config(
      transform::CanonicalizeEntryPointIO::ShaderStyle::kMsl, fixed_sample_mask,
      emit_vertex_point_size);

  manager.Add<transform::Unshadow>();

  if (!disable_workgroup_init) {
    // ZeroInitWorkgroupMemory must come before CanonicalizeEntryPointIO as
    // ZeroInitWorkgroupMemory may inject new builtin parameters.
    manager.Add<transform::ZeroInitWorkgroupMemory>();
  }
  manager.Add<transform::CanonicalizeEntryPointIO>();
  manager.Add<transform::ExternalTextureTransform>();
  manager.Add<transform::PromoteInitializersToConstVar>();
  manager.Add<transform::VectorizeScalarMatrixConstructors>();
  manager.Add<transform::WrapArraysInStructs>();
  manager.Add<transform::PadArrayElements>();
  manager.Add<transform::RemovePhonies>();
  manager.Add<transform::SimplifyPointers>();
  // ArrayLengthFromUniform must come after SimplifyPointers, as
  // it assumes that the form of the array length argument is &var.array.
  manager.Add<transform::ArrayLengthFromUniform>();
  manager.Add<transform::ModuleScopeVarToEntryPointParam>();
  internal_inputs.Add<transform::ArrayLengthFromUniform::Config>(
      std::move(array_length_from_uniform_cfg));
  internal_inputs.Add<transform::CanonicalizeEntryPointIO::Config>(
      std::move(entry_point_io_cfg));
  auto out = manager.Run(in, internal_inputs);

  SanitizedResult result;
  result.program = std::move(out.program);
  if (!result.program.IsValid()) {
    return result;
  }
  result.used_array_length_from_uniform_indices =
      std::move(out.data.Get<transform::ArrayLengthFromUniform::Result>()
                    ->used_size_indices);
  result.needs_storage_buffer_sizes =
      !result.used_array_length_from_uniform_indices.empty();
  return result;
}

GeneratorImpl::GeneratorImpl(const Program* program) : TextGenerator(program) {}

GeneratorImpl::~GeneratorImpl() = default;

bool GeneratorImpl::Generate() {
  line() << "#include <metal_stdlib>";
  line();
  line() << "using namespace metal;";

  auto helpers_insertion_point = current_buffer_->lines.size();

  for (auto* const type_decl : program_->AST().TypeDecls()) {
    if (!type_decl->Is<ast::Alias>()) {
      if (!EmitTypeDecl(TypeOf(type_decl))) {
        return false;
      }
    }
  }

  if (!program_->AST().TypeDecls().empty()) {
    line();
  }

  for (auto* var : program_->AST().GlobalVariables()) {
    if (var->is_const) {
      if (!EmitProgramConstVariable(var)) {
        return false;
      }
    } else {
      // These are pushed into the entry point by sanitizer transforms.
      TINT_ICE(Writer, diagnostics_) << "module-scope variables should have "
                                        "been handled by the MSL sanitizer";
      break;
    }
  }

  for (auto* func : program_->AST().Functions()) {
    if (!func->IsEntryPoint()) {
      if (!EmitFunction(func)) {
        return false;
      }
    } else {
      if (!EmitEntryPointFunction(func)) {
        return false;
      }
    }
    line();
  }

  if (!invariant_define_name_.empty()) {
    // 'invariant' attribute requires MSL 2.1 or higher.
    // WGSL can ignore the invariant attribute on pre MSL 2.1 devices.
    // See: https://github.com/gpuweb/gpuweb/issues/893#issuecomment-745537465
    line(&helpers_) << "#if __METAL_VERSION__ >= 210";
    line(&helpers_) << "#define " << invariant_define_name_ << " [[invariant]]";
    line(&helpers_) << "#else";
    line(&helpers_) << "#define " << invariant_define_name_;
    line(&helpers_) << "#endif";
    line(&helpers_);
  }

  if (!helpers_.lines.empty()) {
    current_buffer_->Insert("", helpers_insertion_point++, 0);
    current_buffer_->Insert(helpers_, helpers_insertion_point++, 0);
  }

  return true;
}

bool GeneratorImpl::EmitTypeDecl(const sem::Type* ty) {
  if (auto* str = ty->As<sem::Struct>()) {
    if (!EmitStructType(current_buffer_, str)) {
      return false;
    }
  } else {
    diagnostics_.add_error(diag::System::Writer,
                           "unknown alias type: " + ty->type_name());
    return false;
  }

  return true;
}

bool GeneratorImpl::EmitIndexAccessor(
    std::ostream& out,
    const ast::IndexAccessorExpression* expr) {
  bool paren_lhs =
      !expr->object->IsAnyOf<ast::IndexAccessorExpression, ast::CallExpression,
                             ast::IdentifierExpression,
                             ast::MemberAccessorExpression>();

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

bool GeneratorImpl::EmitBitcast(std::ostream& out,
                                const ast::BitcastExpression* expr) {
  out << "as_type<";
  if (!EmitType(out, TypeOf(expr)->UnwrapRef(), "")) {
    return false;
  }

  out << ">(";
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

bool GeneratorImpl::EmitBinary(std::ostream& out,
                               const ast::BinaryExpression* expr) {
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
        diagnostics_.add_error(diag::System::Writer,
                               "missing binary operation type");
        return false;
    }
    out << " ";
    return true;
  };

  auto signed_type_of = [&](const sem::Type* ty) -> const sem::Type* {
    if (ty->is_integer_scalar()) {
      return builder_.create<sem::I32>();
    } else if (auto* v = ty->As<sem::Vector>()) {
      return builder_.create<sem::Vector>(builder_.create<sem::I32>(),
                                          v->Width());
    }
    return {};
  };

  auto unsigned_type_of = [&](const sem::Type* ty) -> const sem::Type* {
    if (ty->is_integer_scalar()) {
      return builder_.create<sem::U32>();
    } else if (auto* v = ty->As<sem::Vector>()) {
      return builder_.create<sem::Vector>(builder_.create<sem::U32>(),
                                          v->Width());
    }
    return {};
  };

  auto* lhs_type = TypeOf(expr->lhs)->UnwrapRef();
  auto* rhs_type = TypeOf(expr->rhs)->UnwrapRef();

  // Handle fmod
  if (expr->op == ast::BinaryOp::kModulo &&
      lhs_type->is_float_scalar_or_vector()) {
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
      lhs_type->is_signed_scalar_or_vector() &&
      rhs_type->is_signed_scalar_or_vector()) {
    // If lhs or rhs is a vector, use that type (support implicit scalar to
    // vector promotion)
    auto* target_type =
        lhs_type->Is<sem::Vector>()
            ? lhs_type
            : (rhs_type->Is<sem::Vector>() ? rhs_type : lhs_type);

    // WGSL defines behaviour for signed overflow, MSL does not. For these
    // cases, bitcast operands to unsigned, then cast result to signed.
    ScopedBitCast outer_int_cast(this, out, target_type,
                                 signed_type_of(target_type));
    ScopedParen sp(out);
    {
      ScopedBitCast lhs_uint_cast(this, out, lhs_type,
                                  unsigned_type_of(target_type));
      if (!EmitExpression(out, expr->lhs)) {
        return false;
      }
    }
    if (!emit_op()) {
      return false;
    }
    {
      ScopedBitCast rhs_uint_cast(this, out, rhs_type,
                                  unsigned_type_of(target_type));
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
  if (expr->IsShiftLeft() && lhs_type->is_signed_scalar_or_vector()) {
    // Shift left: discards top bits, so convert first operand to unsigned
    // first, then convert result back to signed
    ScopedBitCast outer_int_cast(this, out, lhs_type, signed_type_of(lhs_type));
    ScopedParen sp(out);
    {
      ScopedBitCast lhs_uint_cast(this, out, lhs_type,
                                  unsigned_type_of(lhs_type));
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

bool GeneratorImpl::EmitBreak(const ast::BreakStatement*) {
  line() << "break;";
  return true;
}

bool GeneratorImpl::EmitCall(std::ostream& out,
                             const ast::CallExpression* expr) {
  auto* call = program_->Sem().Get(expr);
  auto* target = call->Target();

  if (auto* func = target->As<sem::Function>()) {
    return EmitFunctionCall(out, call, func);
  }
  if (auto* intrinsic = target->As<sem::Intrinsic>()) {
    return EmitIntrinsicCall(out, call, intrinsic);
  }
  if (auto* conv = target->As<sem::TypeConversion>()) {
    return EmitTypeConversion(out, call, conv);
  }
  if (auto* ctor = target->As<sem::TypeConstructor>()) {
    return EmitTypeConstructor(out, call, ctor);
  }

  TINT_ICE(Writer, diagnostics_)
      << "unhandled call target: " << target->TypeInfo().name;
  return false;
}

bool GeneratorImpl::EmitFunctionCall(std::ostream& out,
                                     const sem::Call* call,
                                     const sem::Function*) {
  auto* ident = call->Declaration()->target.name;
  out << program_->Symbols().NameFor(ident->symbol) << "(";

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

bool GeneratorImpl::EmitIntrinsicCall(std::ostream& out,
                                      const sem::Call* call,
                                      const sem::Intrinsic* intrinsic) {
  auto* expr = call->Declaration();
  if (intrinsic->IsAtomic()) {
    return EmitAtomicCall(out, expr, intrinsic);
  }
  if (intrinsic->IsTexture()) {
    return EmitTextureCall(out, call, intrinsic);
  }

  auto name = generate_builtin_name(intrinsic);

  switch (intrinsic->Type()) {
    case sem::IntrinsicType::kDot:
      return EmitDotCall(out, expr, intrinsic);
    case sem::IntrinsicType::kModf:
      return EmitModfCall(out, expr, intrinsic);
    case sem::IntrinsicType::kFrexp:
      return EmitFrexpCall(out, expr, intrinsic);

    case sem::IntrinsicType::kPack2x16float:
    case sem::IntrinsicType::kUnpack2x16float: {
      if (intrinsic->Type() == sem::IntrinsicType::kPack2x16float) {
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
    // TODO(crbug.com/tint/661): Combine sequential barriers to a single
    // instruction.
    case sem::IntrinsicType::kStorageBarrier: {
      out << "threadgroup_barrier(mem_flags::mem_device)";
      return true;
    }
    case sem::IntrinsicType::kWorkgroupBarrier: {
      out << "threadgroup_barrier(mem_flags::mem_threadgroup)";
      return true;
    }
    case sem::IntrinsicType::kIgnore: {  // [DEPRECATED]
      out << "(void) ";
      if (!EmitExpression(out, expr->args[0])) {
        return false;
      }
      return true;
    }

    case sem::IntrinsicType::kLength: {
      auto* sem = builder_.Sem().Get(expr->args[0]);
      if (sem->Type()->UnwrapRef()->is_scalar()) {
        // Emulate scalar overload using fabs(x).
        name = "fabs";
      }
      break;
    }

    case sem::IntrinsicType::kDistance: {
      auto* sem = builder_.Sem().Get(expr->args[0]);
      if (sem->Type()->UnwrapRef()->is_scalar()) {
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

bool GeneratorImpl::EmitTypeConversion(std::ostream& out,
                                       const sem::Call* call,
                                       const sem::TypeConversion* conv) {
  if (!EmitType(out, conv->Target(), "")) {
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

  if (type->IsAnyOf<sem::Array, sem::Struct>()) {
    out << "{";
  } else {
    if (!EmitType(out, type, "")) {
      return false;
    }
    out << "(";
  }

  int i = 0;
  for (auto* arg : call->Arguments()) {
    if (i > 0) {
      out << ", ";
    }

    if (auto* struct_ty = type->As<sem::Struct>()) {
      // Emit field designators for structures to account for padding members.
      auto* member = struct_ty->Members()[i]->Declaration();
      auto name = program_->Symbols().NameFor(member->symbol);
      out << "." << name << "=";
    }

    if (!EmitExpression(out, arg->Declaration())) {
      return false;
    }

    i++;
  }

  if (type->IsAnyOf<sem::Array, sem::Struct>()) {
    out << "}";
  } else {
    out << ")";
  }
  return true;
}

bool GeneratorImpl::EmitAtomicCall(std::ostream& out,
                                   const ast::CallExpression* expr,
                                   const sem::Intrinsic* intrinsic) {
  auto call = [&](const std::string& name, bool append_memory_order_relaxed) {
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
      if (append_memory_order_relaxed) {
        out << ", memory_order_relaxed";
      }
    }
    return true;
  };

  switch (intrinsic->Type()) {
    case sem::IntrinsicType::kAtomicLoad:
      return call("atomic_load_explicit", true);

    case sem::IntrinsicType::kAtomicStore:
      return call("atomic_store_explicit", true);

    case sem::IntrinsicType::kAtomicAdd:
      return call("atomic_fetch_add_explicit", true);

    case sem::IntrinsicType::kAtomicSub:
      return call("atomic_fetch_sub_explicit", true);

    case sem::IntrinsicType::kAtomicMax:
      return call("atomic_fetch_max_explicit", true);

    case sem::IntrinsicType::kAtomicMin:
      return call("atomic_fetch_min_explicit", true);

    case sem::IntrinsicType::kAtomicAnd:
      return call("atomic_fetch_and_explicit", true);

    case sem::IntrinsicType::kAtomicOr:
      return call("atomic_fetch_or_explicit", true);

    case sem::IntrinsicType::kAtomicXor:
      return call("atomic_fetch_xor_explicit", true);

    case sem::IntrinsicType::kAtomicExchange:
      return call("atomic_exchange_explicit", true);

    case sem::IntrinsicType::kAtomicCompareExchangeWeak: {
      auto* ptr_ty = TypeOf(expr->args[0])->UnwrapRef()->As<sem::Pointer>();
      auto sc = ptr_ty->StorageClass();

      auto func = utils::GetOrCreate(
          atomicCompareExchangeWeak_, sc, [&]() -> std::string {
            auto name = UniqueIdentifier("atomicCompareExchangeWeak");
            auto& buf = helpers_;

            line(&buf) << "template <typename A, typename T>";
            {
              auto f = line(&buf);
              f << "vec<T, 2> " << name << "(";
              if (!EmitStorageClass(f, sc)) {
                return "";
              }
              f << " A* atomic, T compare, T value) {";
            }

            buf.IncrementIndent();
            TINT_DEFER({
              buf.DecrementIndent();
              line(&buf) << "}";
              line(&buf);
            });

            line(&buf) << "T prev_value = compare;";
            line(&buf) << "bool matched = "
                          "atomic_compare_exchange_weak_explicit(atomic, "
                          "&prev_value, value, memory_order_relaxed, "
                          "memory_order_relaxed);";
            line(&buf) << "return {prev_value, matched};";
            return name;
          });

      return call(func, false);
    }

    default:
      break;
  }

  TINT_UNREACHABLE(Writer, diagnostics_)
      << "unsupported atomic intrinsic: " << intrinsic->Type();
  return false;
}

bool GeneratorImpl::EmitTextureCall(std::ostream& out,
                                    const sem::Call* call,
                                    const sem::Intrinsic* intrinsic) {
  using Usage = sem::ParameterUsage;

  auto& signature = intrinsic->Signature();
  auto* expr = call->Declaration();
  auto& arguments = call->Arguments();

  // Returns the argument with the given usage
  auto arg = [&](Usage usage) {
    int idx = signature.IndexOf(usage);
    return (idx >= 0) ? arguments[idx] : nullptr;
  };

  auto* texture = arg(Usage::kTexture)->Declaration();
  if (!texture) {
    TINT_ICE(Writer, diagnostics_) << "missing texture arg";
    return false;
  }

  auto* texture_type = TypeOf(texture)->UnwrapRef()->As<sem::Texture>();

  // Helper to emit the texture expression, wrapped in parentheses if the
  // expression includes an operator with lower precedence than the member
  // accessor used for the function calls.
  auto texture_expr = [&]() {
    bool paren_lhs =
        !texture->IsAnyOf<ast::IndexAccessorExpression, ast::CallExpression,
                          ast::IdentifierExpression,
                          ast::MemberAccessorExpression>();
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

  switch (intrinsic->Type()) {
    case sem::IntrinsicType::kTextureDimensions: {
      std::vector<const char*> dims;
      switch (texture_type->dim()) {
        case ast::TextureDimension::kNone:
          diagnostics_.add_error(diag::System::Writer,
                                 "texture dimension is kNone");
          return false;
        case ast::TextureDimension::k1d:
          dims = {"width"};
          break;
        case ast::TextureDimension::k2d:
        case ast::TextureDimension::k2dArray:
        case ast::TextureDimension::kCube:
        case ast::TextureDimension::kCubeArray:
          dims = {"width", "height"};
          break;
        case ast::TextureDimension::k3d:
          dims = {"width", "height", "depth"};
          break;
      }

      auto get_dim = [&](const char* name) {
        if (!texture_expr()) {
          return false;
        }
        out << ".get_" << name << "(";
        if (auto* level = arg(Usage::kLevel)) {
          if (!EmitExpression(out, level->Declaration())) {
            return false;
          }
        }
        out << ")";
        return true;
      };

      if (dims.size() == 1) {
        out << "int(";
        get_dim(dims[0]);
        out << ")";
      } else {
        EmitType(out, TypeOf(expr)->UnwrapRef(), "");
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
    case sem::IntrinsicType::kTextureNumLayers: {
      out << "int(";
      if (!texture_expr()) {
        return false;
      }
      out << ".get_array_size())";
      return true;
    }
    case sem::IntrinsicType::kTextureNumLevels: {
      out << "int(";
      if (!texture_expr()) {
        return false;
      }
      out << ".get_num_mip_levels())";
      return true;
    }
    case sem::IntrinsicType::kTextureNumSamples: {
      out << "int(";
      if (!texture_expr()) {
        return false;
      }
      out << ".get_num_samples())";
      return true;
    }
    default:
      break;
  }

  if (!texture_expr()) {
    return false;
  }

  bool lod_param_is_named = true;

  switch (intrinsic->Type()) {
    case sem::IntrinsicType::kTextureSample:
    case sem::IntrinsicType::kTextureSampleBias:
    case sem::IntrinsicType::kTextureSampleLevel:
    case sem::IntrinsicType::kTextureSampleGrad:
      out << ".sample(";
      break;
    case sem::IntrinsicType::kTextureSampleCompare:
    case sem::IntrinsicType::kTextureSampleCompareLevel:
      out << ".sample_compare(";
      break;
    case sem::IntrinsicType::kTextureGather:
      out << ".gather(";
      break;
    case sem::IntrinsicType::kTextureGatherCompare:
      out << ".gather_compare(";
      break;
    case sem::IntrinsicType::kTextureLoad:
      out << ".read(";
      lod_param_is_named = false;
      break;
    case sem::IntrinsicType::kTextureStore:
      out << ".write(";
      break;
    default:
      TINT_UNREACHABLE(Writer, diagnostics_)
          << "Unhandled texture intrinsic '" << intrinsic->str() << "'";
      return false;
  }

  bool first_arg = true;
  auto maybe_write_comma = [&] {
    if (!first_arg) {
      out << ", ";
    }
    first_arg = false;
  };

  for (auto usage :
       {Usage::kValue, Usage::kSampler, Usage::kCoords, Usage::kArrayIndex,
        Usage::kDepthRef, Usage::kSampleIndex}) {
    if (auto* e = arg(usage)) {
      maybe_write_comma();

      // Cast the coordinates to unsigned integers if necessary.
      bool casted = false;
      if (usage == Usage::kCoords &&
          e->Type()->UnwrapRef()->is_integer_scalar_or_vector()) {
        casted = true;
        switch (texture_type->dim()) {
          case ast::TextureDimension::k1d:
            out << "uint(";
            break;
          case ast::TextureDimension::k2d:
          case ast::TextureDimension::k2dArray:
            out << "uint2(";
            break;
          case ast::TextureDimension::k3d:
            out << "uint3(";
            break;
          default:
            TINT_ICE(Writer, diagnostics_)
                << "unhandled texture dimensionality";
            break;
        }
      }

      if (!EmitExpression(out, e->Declaration()))
        return false;

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
    if (!EmitExpression(out, level->Declaration())) {
      return false;
    }
    if (lod_param_is_named) {
      out << ")";
    }
  }
  if (intrinsic->Type() == sem::IntrinsicType::kTextureSampleCompareLevel) {
    maybe_write_comma();
    out << "level(0)";
  }
  if (auto* ddx = arg(Usage::kDdx)) {
    auto dim = texture_type->dim();
    switch (dim) {
      case ast::TextureDimension::k2d:
      case ast::TextureDimension::k2dArray:
        maybe_write_comma();
        out << "gradient2d(";
        break;
      case ast::TextureDimension::k3d:
        maybe_write_comma();
        out << "gradient3d(";
        break;
      case ast::TextureDimension::kCube:
      case ast::TextureDimension::kCubeArray:
        maybe_write_comma();
        out << "gradientcube(";
        break;
      default: {
        std::stringstream err;
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
        case ast::TextureDimension::k2d:
        case ast::TextureDimension::k2dArray:
          out << "int2(0), ";
          break;
        default:
          break;  // Other texture dimensions don't have an offset
      }
    }
    auto c = component->ConstantValue().Elements()[0].i32;
    switch (c) {
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
        TINT_ICE(Writer, diagnostics_)
            << "invalid textureGather component: " << c;
        break;
    }
  }

  out << ")";

  return true;
}

bool GeneratorImpl::EmitDotCall(std::ostream& out,
                                const ast::CallExpression* expr,
                                const sem::Intrinsic* intrinsic) {
  auto* vec_ty = intrinsic->Parameters()[0]->Type()->As<sem::Vector>();
  std::string fn = "dot";
  if (vec_ty->type()->is_integer_scalar()) {
    // MSL does not have a builtin for dot() with integer vector types.
    // Generate the helper function if it hasn't been created already
    fn = utils::GetOrCreate(
        int_dot_funcs_, vec_ty->Width(), [&]() -> std::string {
          TextBuffer b;
          TINT_DEFER(helpers_.Append(b));

          auto fn_name =
              UniqueIdentifier("tint_dot" + std::to_string(vec_ty->Width()));
          auto v = "vec<T," + std::to_string(vec_ty->Width()) + ">";

          line(&b) << "template<typename T>";
          line(&b) << "T " << fn_name << "(" << v << " a, " << v << " b) {";
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
                                 const sem::Intrinsic* intrinsic) {
  return CallIntrinsicHelper(
      out, expr, intrinsic,
      [&](TextBuffer* b, const std::vector<std::string>& params) {
        auto* ty = intrinsic->Parameters()[0]->Type();
        auto in = params[0];

        std::string width;
        if (auto* vec = ty->As<sem::Vector>()) {
          width = std::to_string(vec->Width());
        }

        // Emit the builtin return type unique to this overload. This does not
        // exist in the AST, so it will not be generated in Generate().
        if (!EmitStructType(&helpers_,
                            intrinsic->ReturnType()->As<sem::Struct>())) {
          return false;
        }

        line(b) << "float" << width << " whole;";
        line(b) << "float" << width << " fract = modf(" << in << ", whole);";
        line(b) << "return {fract, whole};";
        return true;
      });
}

bool GeneratorImpl::EmitFrexpCall(std::ostream& out,
                                  const ast::CallExpression* expr,
                                  const sem::Intrinsic* intrinsic) {
  return CallIntrinsicHelper(
      out, expr, intrinsic,
      [&](TextBuffer* b, const std::vector<std::string>& params) {
        auto* ty = intrinsic->Parameters()[0]->Type();
        auto in = params[0];

        std::string width;
        if (auto* vec = ty->As<sem::Vector>()) {
          width = std::to_string(vec->Width());
        }

        // Emit the builtin return type unique to this overload. This does not
        // exist in the AST, so it will not be generated in Generate().
        if (!EmitStructType(&helpers_,
                            intrinsic->ReturnType()->As<sem::Struct>())) {
          return false;
        }

        line(b) << "int" << width << " exp;";
        line(b) << "float" << width << " sig = frexp(" << in << ", exp);";
        line(b) << "return {sig, exp};";
        return true;
      });
}

std::string GeneratorImpl::generate_builtin_name(
    const sem::Intrinsic* intrinsic) {
  std::string out = "";
  switch (intrinsic->Type()) {
    case sem::IntrinsicType::kAcos:
    case sem::IntrinsicType::kAll:
    case sem::IntrinsicType::kAny:
    case sem::IntrinsicType::kAsin:
    case sem::IntrinsicType::kAtan:
    case sem::IntrinsicType::kAtan2:
    case sem::IntrinsicType::kCeil:
    case sem::IntrinsicType::kCos:
    case sem::IntrinsicType::kCosh:
    case sem::IntrinsicType::kCross:
    case sem::IntrinsicType::kDeterminant:
    case sem::IntrinsicType::kDistance:
    case sem::IntrinsicType::kDot:
    case sem::IntrinsicType::kExp:
    case sem::IntrinsicType::kExp2:
    case sem::IntrinsicType::kFloor:
    case sem::IntrinsicType::kFma:
    case sem::IntrinsicType::kFract:
    case sem::IntrinsicType::kFrexp:
    case sem::IntrinsicType::kLength:
    case sem::IntrinsicType::kLdexp:
    case sem::IntrinsicType::kLog:
    case sem::IntrinsicType::kLog2:
    case sem::IntrinsicType::kMix:
    case sem::IntrinsicType::kModf:
    case sem::IntrinsicType::kNormalize:
    case sem::IntrinsicType::kPow:
    case sem::IntrinsicType::kReflect:
    case sem::IntrinsicType::kRefract:
    case sem::IntrinsicType::kSelect:
    case sem::IntrinsicType::kSin:
    case sem::IntrinsicType::kSinh:
    case sem::IntrinsicType::kSqrt:
    case sem::IntrinsicType::kStep:
    case sem::IntrinsicType::kTan:
    case sem::IntrinsicType::kTanh:
    case sem::IntrinsicType::kTranspose:
    case sem::IntrinsicType::kTrunc:
    case sem::IntrinsicType::kSign:
    case sem::IntrinsicType::kClamp:
      out += intrinsic->str();
      break;
    case sem::IntrinsicType::kAbs:
      if (intrinsic->ReturnType()->is_float_scalar_or_vector()) {
        out += "fabs";
      } else {
        out += "abs";
      }
      break;
    case sem::IntrinsicType::kCountOneBits:
      out += "popcount";
      break;
    case sem::IntrinsicType::kDpdx:
    case sem::IntrinsicType::kDpdxCoarse:
    case sem::IntrinsicType::kDpdxFine:
      out += "dfdx";
      break;
    case sem::IntrinsicType::kDpdy:
    case sem::IntrinsicType::kDpdyCoarse:
    case sem::IntrinsicType::kDpdyFine:
      out += "dfdy";
      break;
    case sem::IntrinsicType::kFwidth:
    case sem::IntrinsicType::kFwidthCoarse:
    case sem::IntrinsicType::kFwidthFine:
      out += "fwidth";
      break;
    case sem::IntrinsicType::kIsFinite:
      out += "isfinite";
      break;
    case sem::IntrinsicType::kIsInf:
      out += "isinf";
      break;
    case sem::IntrinsicType::kIsNan:
      out += "isnan";
      break;
    case sem::IntrinsicType::kIsNormal:
      out += "isnormal";
      break;
    case sem::IntrinsicType::kMax:
      if (intrinsic->ReturnType()->is_float_scalar_or_vector()) {
        out += "fmax";
      } else {
        out += "max";
      }
      break;
    case sem::IntrinsicType::kMin:
      if (intrinsic->ReturnType()->is_float_scalar_or_vector()) {
        out += "fmin";
      } else {
        out += "min";
      }
      break;
    case sem::IntrinsicType::kFaceForward:
      out += "faceforward";
      break;
    case sem::IntrinsicType::kPack4x8snorm:
      out += "pack_float_to_snorm4x8";
      break;
    case sem::IntrinsicType::kPack4x8unorm:
      out += "pack_float_to_unorm4x8";
      break;
    case sem::IntrinsicType::kPack2x16snorm:
      out += "pack_float_to_snorm2x16";
      break;
    case sem::IntrinsicType::kPack2x16unorm:
      out += "pack_float_to_unorm2x16";
      break;
    case sem::IntrinsicType::kReverseBits:
      out += "reverse_bits";
      break;
    case sem::IntrinsicType::kRound:
      out += "rint";
      break;
    case sem::IntrinsicType::kSmoothStep:
      out += "smoothstep";
      break;
    case sem::IntrinsicType::kInverseSqrt:
      out += "rsqrt";
      break;
    case sem::IntrinsicType::kUnpack4x8snorm:
      out += "unpack_snorm4x8_to_float";
      break;
    case sem::IntrinsicType::kUnpack4x8unorm:
      out += "unpack_unorm4x8_to_float";
      break;
    case sem::IntrinsicType::kUnpack2x16snorm:
      out += "unpack_snorm2x16_to_float";
      break;
    case sem::IntrinsicType::kUnpack2x16unorm:
      out += "unpack_unorm2x16_to_float";
      break;
    case sem::IntrinsicType::kArrayLength:
      diagnostics_.add_error(
          diag::System::Writer,
          "Unable to translate builtin: " + std::string(intrinsic->str()) +
              "\nDid you forget to pass array_length_from_uniform generator "
              "options?");
      return "";
    default:
      diagnostics_.add_error(
          diag::System::Writer,
          "Unknown import method: " + std::string(intrinsic->str()));
      return "";
  }
  return out;
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

    for (auto* s : stmt->body->statements) {
      if (!EmitStatement(s)) {
        return false;
      }
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
    return EmitZeroValue(out, vec->type());
  } else if (auto* mat = type->As<sem::Matrix>()) {
    if (!EmitType(out, mat, "")) {
      return false;
    }
    out << "(";
    if (!EmitZeroValue(out, mat->type())) {
      return false;
    }
    out << ")";
  } else if (auto* arr = type->As<sem::Array>()) {
    out << "{";
    if (!EmitZeroValue(out, arr->ElemType())) {
      return false;
    }
    out << "}";
  } else if (type->As<sem::Struct>()) {
    out << "{}";
  } else {
    diagnostics_.add_error(
        diag::System::Writer,
        "Invalid type for zero emission: " + type->type_name());
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitLiteral(std::ostream& out,
                                const ast::LiteralExpression* lit) {
  if (auto* l = lit->As<ast::BoolLiteralExpression>()) {
    out << (l->value ? "true" : "false");
  } else if (auto* fl = lit->As<ast::FloatLiteralExpression>()) {
    if (std::isinf(fl->value)) {
      out << (fl->value >= 0 ? "INFINITY" : "-INFINITY");
    } else if (std::isnan(fl->value)) {
      out << "NAN";
    } else {
      out << FloatToString(fl->value) << "f";
    }
  } else if (auto* sl = lit->As<ast::SintLiteralExpression>()) {
    // MSL (and C++) parse `-2147483648` as a `long` because it parses unary
    // minus and `2147483648` as separate tokens, and the latter doesn't
    // fit into an (32-bit) `int`. WGSL, OTOH, parses this as an `i32`. To avoid
    // issues with `long` to `int` casts, emit `(2147483647 - 1)` instead, which
    // ensures the expression type is `int`.
    const auto int_min = std::numeric_limits<int32_t>::min();
    if (sl->ValueAsI32() == int_min) {
      out << "(" << int_min + 1 << " - 1)";
    } else {
      out << sl->value;
    }
  } else if (auto* ul = lit->As<ast::UintLiteralExpression>()) {
    out << ul->value << "u";
  } else {
    diagnostics_.add_error(diag::System::Writer, "unknown literal type");
    return false;
  }
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

void GeneratorImpl::EmitStage(std::ostream& out, ast::PipelineStage stage) {
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

bool GeneratorImpl::EmitFunction(const ast::Function* func) {
  auto* func_sem = program_->Sem().Get(func);

  {
    auto out = line();
    if (!EmitType(out, func_sem->ReturnType(), "")) {
      return false;
    }
    out << " " << program_->Symbols().NameFor(func->symbol) << "(";

    bool first = true;
    for (auto* v : func->params) {
      if (!first) {
        out << ", ";
      }
      first = false;

      auto* type = program_->Sem().Get(v)->Type();

      std::string param_name =
          "const " + program_->Symbols().NameFor(v->symbol);
      if (!EmitType(out, type, param_name)) {
        return false;
      }
      // Parameter name is output as part of the type for arrays and pointers.
      if (!type->Is<sem::Array>() && !type->Is<sem::Pointer>()) {
        out << " " << program_->Symbols().NameFor(v->symbol);
      }
    }

    out << ") {";
  }

  if (!EmitStatementsWithIndent(func->body->statements)) {
    return false;
  }

  line() << "}";

  return true;
}

std::string GeneratorImpl::builtin_to_attribute(ast::Builtin builtin) const {
  switch (builtin) {
    case ast::Builtin::kPosition:
      return "position";
    case ast::Builtin::kVertexIndex:
      return "vertex_id";
    case ast::Builtin::kInstanceIndex:
      return "instance_id";
    case ast::Builtin::kFrontFacing:
      return "front_facing";
    case ast::Builtin::kFragDepth:
      return "depth(any)";
    case ast::Builtin::kLocalInvocationId:
      return "thread_position_in_threadgroup";
    case ast::Builtin::kLocalInvocationIndex:
      return "thread_index_in_threadgroup";
    case ast::Builtin::kGlobalInvocationId:
      return "thread_position_in_grid";
    case ast::Builtin::kWorkgroupId:
      return "threadgroup_position_in_grid";
    case ast::Builtin::kNumWorkgroups:
      return "threadgroups_per_grid";
    case ast::Builtin::kSampleIndex:
      return "sample_id";
    case ast::Builtin::kSampleMask:
      return "sample_mask";
    case ast::Builtin::kPointSize:
      return "point_size";
    default:
      break;
  }
  return "";
}

std::string GeneratorImpl::interpolation_to_attribute(
    ast::InterpolationType type,
    ast::InterpolationSampling sampling) const {
  std::string attr;
  switch (sampling) {
    case ast::InterpolationSampling::kCenter:
      attr = "center_";
      break;
    case ast::InterpolationSampling::kCentroid:
      attr = "centroid_";
      break;
    case ast::InterpolationSampling::kSample:
      attr = "sample_";
      break;
    case ast::InterpolationSampling::kNone:
      break;
  }
  switch (type) {
    case ast::InterpolationType::kPerspective:
      attr += "perspective";
      break;
    case ast::InterpolationType::kLinear:
      attr += "no_perspective";
      break;
    case ast::InterpolationType::kFlat:
      attr += "flat";
      break;
  }
  return attr;
}

bool GeneratorImpl::EmitEntryPointFunction(const ast::Function* func) {
  auto func_name = program_->Symbols().NameFor(func->symbol);

  // Returns the binding index of a variable, requiring that the group attribute
  // have a value of zero.
  const uint32_t kInvalidBindingIndex = std::numeric_limits<uint32_t>::max();
  auto get_binding_index = [&](const ast::Variable* var) -> uint32_t {
    auto bp = var->BindingPoint();
    if (bp.group == nullptr || bp.binding == nullptr) {
      TINT_ICE(Writer, diagnostics_)
          << "missing binding attributes for entry point parameter";
      return kInvalidBindingIndex;
    }
    if (bp.group->value != 0) {
      TINT_ICE(Writer, diagnostics_)
          << "encountered non-zero resource group index (use "
             "BindingRemapper to fix)";
      return kInvalidBindingIndex;
    }
    return bp.binding->value;
  };

  {
    auto out = line();

    EmitStage(out, func->PipelineStage());
    out << " " << func->return_type->FriendlyName(program_->Symbols());
    out << " " << func_name << "(";

    // Emit entry point parameters.
    bool first = true;
    for (auto* var : func->params) {
      if (!first) {
        out << ", ";
      }
      first = false;

      auto* type = program_->Sem().Get(var)->Type()->UnwrapRef();

      auto param_name = program_->Symbols().NameFor(var->symbol);
      if (!EmitType(out, type, param_name)) {
        return false;
      }
      // Parameter name is output as part of the type for arrays and pointers.
      if (!type->Is<sem::Array>() && !type->Is<sem::Pointer>()) {
        out << " " << param_name;
      }

      if (type->Is<sem::Struct>()) {
        out << " [[stage_in]]";
      } else if (type->is_handle()) {
        uint32_t binding = get_binding_index(var);
        if (binding == kInvalidBindingIndex) {
          return false;
        }
        if (var->type->Is<ast::Sampler>()) {
          out << " [[sampler(" << binding << ")]]";
        } else if (var->type->Is<ast::Texture>()) {
          out << " [[texture(" << binding << ")]]";
        } else {
          TINT_ICE(Writer, diagnostics_)
              << "invalid handle type entry point parameter";
          return false;
        }
      } else if (auto* ptr = var->type->As<ast::Pointer>()) {
        auto sc = ptr->storage_class;
        if (sc == ast::StorageClass::kWorkgroup) {
          auto& allocations = workgroup_allocations_[func_name];
          out << " [[threadgroup(" << allocations.size() << ")]]";
          allocations.push_back(program_->Sem().Get(ptr->type)->Size());
        } else if (sc == ast::StorageClass::kStorage ||
                   sc == ast::StorageClass::kUniform) {
          uint32_t binding = get_binding_index(var);
          if (binding == kInvalidBindingIndex) {
            return false;
          }
          out << " [[buffer(" << binding << ")]]";
        } else {
          TINT_ICE(Writer, diagnostics_)
              << "invalid pointer storage class for entry point parameter";
          return false;
        }
      } else {
        auto& decos = var->decorations;
        bool builtin_found = false;
        for (auto* deco : decos) {
          auto* builtin = deco->As<ast::BuiltinDecoration>();
          if (!builtin) {
            continue;
          }

          builtin_found = true;

          auto attr = builtin_to_attribute(builtin->builtin);
          if (attr.empty()) {
            diagnostics_.add_error(diag::System::Writer, "unknown builtin");
            return false;
          }
          out << " [[" << attr << "]]";
        }
        if (!builtin_found) {
          TINT_ICE(Writer, diagnostics_) << "Unsupported entry point parameter";
        }
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
      ast::ReturnStatement ret(ProgramID{}, Source{});
      if (!EmitStatement(&ret)) {
        return false;
      }
    }
  }

  line() << "}";
  return true;
}

bool GeneratorImpl::EmitIdentifier(std::ostream& out,
                                   const ast::IdentifierExpression* expr) {
  out << program_->Symbols().NameFor(expr->symbol);
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
    if (!emit_continuing()) {
      return false;
    }
  }
  line() << "}";

  return true;
}

bool GeneratorImpl::EmitForLoop(const ast::ForLoopStatement* stmt) {
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
  // we cannot emit this as a regular for-loop in MSL. Instead we need to
  // generate a `while(true)` loop.
  bool emit_as_loop = cond_pre.lines.size() > 0 || cont_buf.lines.size() > 1;

  // If the for-loop has multi-statement initializer, or is going to be emitted
  // as a `while(true)` loop, then declare the initializer statement(s) before
  // the loop in a new block.
  bool nest_in_block =
      init_buf.lines.size() > 1 || (stmt->initializer && emit_as_loop);
  if (nest_in_block) {
    line() << "{";
    increment_indent();
    current_buffer_->Append(init_buf);
    init_buf.lines.clear();  // Don't emit the initializer again in the 'for'
  }
  TINT_DEFER({
    if (nest_in_block) {
      decrement_indent();
      line() << "}";
    }
  });

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

    if (!emit_continuing()) {
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

bool GeneratorImpl::EmitDiscard(const ast::DiscardStatement*) {
  // TODO(dsinclair): Verify this is correct when the discard semantics are
  // defined for WGSL (https://github.com/gpuweb/gpuweb/issues/361)
  line() << "discard_fragment();";
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

  for (auto* e : stmt->else_statements) {
    if (e->condition) {
      line() << "} else {";
      increment_indent();

      {
        auto out = line();
        out << "if (";
        if (!EmitExpression(out, e->condition)) {
          return false;
        }
        out << ") {";
      }
    } else {
      line() << "} else {";
    }

    if (!EmitStatementsWithIndent(e->body->statements)) {
      return false;
    }
  }

  line() << "}";

  for (auto* e : stmt->else_statements) {
    if (e->condition) {
      decrement_indent();
      line() << "}";
    }
  }
  return true;
}

bool GeneratorImpl::EmitMemberAccessor(
    std::ostream& out,
    const ast::MemberAccessorExpression* expr) {
  auto write_lhs = [&] {
    bool paren_lhs = !expr->structure->IsAnyOf<
        ast::IndexAccessorExpression, ast::CallExpression,
        ast::IdentifierExpression, ast::MemberAccessorExpression>();
    if (paren_lhs) {
      out << "(";
    }
    if (!EmitExpression(out, expr->structure)) {
      return false;
    }
    if (paren_lhs) {
      out << ")";
    }
    return true;
  };

  auto& sem = program_->Sem();

  if (auto* swizzle = sem.Get(expr)->As<sem::Swizzle>()) {
    // Metal 1.x does not support swizzling of packed vector types.
    // For single element swizzles, we can use the index operator.
    // For multi-element swizzles, we need to cast to a regular vector type
    // first. Note that we do not currently allow assignments to swizzles, so
    // the casting which will convert the l-value to r-value is fine.
    if (swizzle->Indices().size() == 1) {
      if (!write_lhs()) {
        return false;
      }
      out << "[" << swizzle->Indices()[0] << "]";
    } else {
      if (!EmitType(out, sem.Get(expr->structure)->Type()->UnwrapRef(), "")) {
        return false;
      }
      out << "(";
      if (!write_lhs()) {
        return false;
      }
      out << ")." << program_->Symbols().NameFor(expr->member->symbol);
    }
  } else {
    if (!write_lhs()) {
      return false;
    }
    out << ".";
    if (!EmitExpression(out, expr->member)) {
      return false;
    }
  }

  return true;
}

bool GeneratorImpl::EmitReturn(const ast::ReturnStatement* stmt) {
  auto out = line();
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

bool GeneratorImpl::EmitBlock(const ast::BlockStatement* stmt) {
  line() << "{";

  if (!EmitStatementsWithIndent(stmt->statements)) {
    return false;
  }

  line() << "}";

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
    auto* var = program_->Sem().Get(v->variable);
    return EmitVariable(var);
  }

  diagnostics_.add_error(
      diag::System::Writer,
      "unknown statement type: " + std::string(stmt->TypeInfo().name));
  return false;
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

bool GeneratorImpl::EmitSwitch(const ast::SwitchStatement* stmt) {
  {
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
                             const std::string& name,
                             bool* name_printed /* = nullptr */) {
  if (name_printed) {
    *name_printed = false;
  }
  if (auto* atomic = type->As<sem::Atomic>()) {
    if (atomic->Type()->Is<sem::I32>()) {
      out << "atomic_int";
      return true;
    }
    if (atomic->Type()->Is<sem::U32>()) {
      out << "atomic_uint";
      return true;
    }
    TINT_ICE(Writer, diagnostics_)
        << "unhandled atomic type " << atomic->Type()->type_name();
    return false;
  }

  if (auto* ary = type->As<sem::Array>()) {
    const sem::Type* base_type = ary;
    std::vector<uint32_t> sizes;
    while (auto* arr = base_type->As<sem::Array>()) {
      if (arr->IsRuntimeSized()) {
        sizes.push_back(1);
      } else {
        sizes.push_back(arr->Count());
      }
      base_type = arr->ElemType();
    }
    if (!EmitType(out, base_type, "")) {
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
  }

  if (type->Is<sem::Bool>()) {
    out << "bool";
    return true;
  }

  if (type->Is<sem::F32>()) {
    out << "float";
    return true;
  }

  if (type->Is<sem::I32>()) {
    out << "int";
    return true;
  }

  if (auto* mat = type->As<sem::Matrix>()) {
    if (!EmitType(out, mat->type(), "")) {
      return false;
    }
    out << mat->columns() << "x" << mat->rows();
    return true;
  }

  if (auto* ptr = type->As<sem::Pointer>()) {
    if (ptr->Access() == ast::Access::kRead) {
      out << "const ";
    }
    if (!EmitStorageClass(out, ptr->StorageClass())) {
      return false;
    }
    out << " ";
    if (ptr->StoreType()->Is<sem::Array>()) {
      std::string inner = "(*" + name + ")";
      if (!EmitType(out, ptr->StoreType(), inner)) {
        return false;
      }
      if (name_printed) {
        *name_printed = true;
      }
    } else {
      if (!EmitType(out, ptr->StoreType(), "")) {
        return false;
      }
      out << "* " << name;
      if (name_printed) {
        *name_printed = true;
      }
    }
    return true;
  }

  if (type->Is<sem::Sampler>()) {
    out << "sampler";
    return true;
  }

  if (auto* str = type->As<sem::Struct>()) {
    // The struct type emits as just the name. The declaration would be emitted
    // as part of emitting the declared types.
    out << StructName(str);
    return true;
  }

  if (auto* tex = type->As<sem::Texture>()) {
    if (tex->IsAnyOf<sem::DepthTexture, sem::DepthMultisampledTexture>()) {
      out << "depth";
    } else {
      out << "texture";
    }

    switch (tex->dim()) {
      case ast::TextureDimension::k1d:
        out << "1d";
        break;
      case ast::TextureDimension::k2d:
        out << "2d";
        break;
      case ast::TextureDimension::k2dArray:
        out << "2d_array";
        break;
      case ast::TextureDimension::k3d:
        out << "3d";
        break;
      case ast::TextureDimension::kCube:
        out << "cube";
        break;
      case ast::TextureDimension::kCubeArray:
        out << "cube_array";
        break;
      default:
        diagnostics_.add_error(diag::System::Writer,
                               "Invalid texture dimensions");
        return false;
    }
    if (tex->IsAnyOf<sem::MultisampledTexture,
                     sem::DepthMultisampledTexture>()) {
      out << "_ms";
    }
    out << "<";
    if (tex->Is<sem::DepthTexture>()) {
      out << "float, access::sample";
    } else if (tex->Is<sem::DepthMultisampledTexture>()) {
      out << "float, access::read";
    } else if (auto* storage = tex->As<sem::StorageTexture>()) {
      if (!EmitType(out, storage->type(), "")) {
        return false;
      }

      std::string access_str;
      if (storage->access() == ast::Access::kRead) {
        out << ", access::read";
      } else if (storage->access() == ast::Access::kWrite) {
        out << ", access::write";
      } else {
        diagnostics_.add_error(diag::System::Writer,
                               "Invalid access control for storage texture");
        return false;
      }
    } else if (auto* ms = tex->As<sem::MultisampledTexture>()) {
      if (!EmitType(out, ms->type(), "")) {
        return false;
      }
      out << ", access::read";
    } else if (auto* sampled = tex->As<sem::SampledTexture>()) {
      if (!EmitType(out, sampled->type(), "")) {
        return false;
      }
      out << ", access::sample";
    } else {
      diagnostics_.add_error(diag::System::Writer, "invalid texture type");
      return false;
    }
    out << ">";
    return true;
  }

  if (type->Is<sem::U32>()) {
    out << "uint";
    return true;
  }

  if (auto* vec = type->As<sem::Vector>()) {
    if (!EmitType(out, vec->type(), "")) {
      return false;
    }
    out << vec->Width();
    return true;
  }

  if (type->Is<sem::Void>()) {
    out << "void";
    return true;
  }

  diagnostics_.add_error(diag::System::Writer,
                         "unknown type in EmitType: " + type->type_name());
  return false;
}

bool GeneratorImpl::EmitTypeAndName(std::ostream& out,
                                    const sem::Type* type,
                                    const std::string& name) {
  bool name_printed = false;
  if (!EmitType(out, type, name, &name_printed)) {
    return false;
  }
  if (!name_printed) {
    out << " " << name;
  }
  return true;
}

bool GeneratorImpl::EmitStorageClass(std::ostream& out, ast::StorageClass sc) {
  switch (sc) {
    case ast::StorageClass::kFunction:
    case ast::StorageClass::kPrivate:
    case ast::StorageClass::kUniformConstant:
      out << "thread";
      return true;
    case ast::StorageClass::kWorkgroup:
      out << "threadgroup";
      return true;
    case ast::StorageClass::kStorage:
      out << "device";
      return true;
    case ast::StorageClass::kUniform:
      out << "constant";
      return true;
    default:
      break;
  }
  TINT_ICE(Writer, diagnostics_) << "unhandled storage class: " << sc;
  return false;
}

bool GeneratorImpl::EmitPackedType(std::ostream& out,
                                   const sem::Type* type,
                                   const std::string& name) {
  auto* vec = type->As<sem::Vector>();
  if (vec && vec->Width() == 3) {
    out << "packed_";
    if (!EmitType(out, vec, "")) {
      return false;
    }

    if (vec->is_float_vector() && !matrix_packed_vector_overloads_) {
      // Overload operators for matrix-vector arithmetic where the vector
      // operand is packed, as these overloads to not exist in the metal
      // namespace.
      TextBuffer b;
      TINT_DEFER(helpers_.Append(b));
      line(&b) << R"(template<typename T, int N, int M>
inline vec<T, M> operator*(matrix<T, N, M> lhs, packed_vec<T, N> rhs) {
  return lhs * vec<T, N>(rhs);
}

template<typename T, int N, int M>
inline vec<T, N> operator*(packed_vec<T, M> lhs, matrix<T, N, M> rhs) {
  return vec<T, M>(lhs) * rhs;
}
)";
      matrix_packed_vector_overloads_ = true;
    }

    return true;
  }

  return EmitType(out, type, name);
}

bool GeneratorImpl::EmitStructType(TextBuffer* b, const sem::Struct* str) {
  line(b) << "struct " << StructName(str) << " {";

  bool is_host_shareable = str->IsHostShareable();

  // Emits a `/* 0xnnnn */` byte offset comment for a struct member.
  auto add_byte_offset_comment = [&](std::ostream& out, uint32_t offset) {
    std::ios_base::fmtflags saved_flag_state(out.flags());
    out << "/* 0x" << std::hex << std::setfill('0') << std::setw(4) << offset
        << " */ ";
    out.flags(saved_flag_state);
  };

  auto add_padding = [&](uint32_t size, uint32_t msl_offset) {
    std::string name;
    do {
      name = UniqueIdentifier("tint_pad");
    } while (str->FindMember(program_->Symbols().Get(name)));

    auto out = line(b);
    add_byte_offset_comment(out, msl_offset);
    out << "int8_t " << name << "[" << size << "];";
  };

  b->IncrementIndent();

  uint32_t msl_offset = 0;
  for (auto* mem : str->Members()) {
    auto out = line(b);
    auto name = program_->Symbols().NameFor(mem->Name());
    auto wgsl_offset = mem->Offset();

    if (is_host_shareable) {
      if (wgsl_offset < msl_offset) {
        // Unimplementable layout
        TINT_ICE(Writer, diagnostics_)
            << "Structure member WGSL offset (" << wgsl_offset
            << ") is behind MSL offset (" << msl_offset << ")";
        return false;
      }

      // Generate padding if required
      if (auto padding = wgsl_offset - msl_offset) {
        add_padding(padding, msl_offset);
        msl_offset += padding;
      }

      add_byte_offset_comment(out, msl_offset);

      if (!EmitPackedType(out, mem->Type(), name)) {
        return false;
      }
    } else {
      if (!EmitType(out, mem->Type(), name)) {
        return false;
      }
    }

    auto* ty = mem->Type();

    // Array member name will be output with the type
    if (!ty->Is<sem::Array>()) {
      out << " " << name;
    }

    // Emit decorations
    if (auto* decl = mem->Declaration()) {
      for (auto* deco : decl->decorations) {
        if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
          auto attr = builtin_to_attribute(builtin->builtin);
          if (attr.empty()) {
            diagnostics_.add_error(diag::System::Writer, "unknown builtin");
            return false;
          }
          out << " [[" << attr << "]]";
        } else if (auto* loc = deco->As<ast::LocationDecoration>()) {
          auto& pipeline_stage_uses = str->PipelineStageUses();
          if (pipeline_stage_uses.size() != 1) {
            TINT_ICE(Writer, diagnostics_)
                << "invalid entry point IO struct uses";
          }

          if (pipeline_stage_uses.count(
                  sem::PipelineStageUsage::kVertexInput)) {
            out << " [[attribute(" + std::to_string(loc->value) + ")]]";
          } else if (pipeline_stage_uses.count(
                         sem::PipelineStageUsage::kVertexOutput)) {
            out << " [[user(locn" + std::to_string(loc->value) + ")]]";
          } else if (pipeline_stage_uses.count(
                         sem::PipelineStageUsage::kFragmentInput)) {
            out << " [[user(locn" + std::to_string(loc->value) + ")]]";
          } else if (pipeline_stage_uses.count(
                         sem::PipelineStageUsage::kFragmentOutput)) {
            out << " [[color(" + std::to_string(loc->value) + ")]]";
          } else {
            TINT_ICE(Writer, diagnostics_)
                << "invalid use of location decoration";
          }
        } else if (auto* interpolate = deco->As<ast::InterpolateDecoration>()) {
          auto attr = interpolation_to_attribute(interpolate->type,
                                                 interpolate->sampling);
          if (attr.empty()) {
            diagnostics_.add_error(diag::System::Writer,
                                   "unknown interpolation attribute");
            return false;
          }
          out << " [[" << attr << "]]";
        } else if (deco->Is<ast::InvariantDecoration>()) {
          if (invariant_define_name_.empty()) {
            invariant_define_name_ = UniqueIdentifier("TINT_INVARIANT");
          }
          out << " " << invariant_define_name_;
        } else if (!deco->IsAnyOf<ast::StructMemberOffsetDecoration,
                                  ast::StructMemberAlignDecoration,
                                  ast::StructMemberSizeDecoration>()) {
          TINT_ICE(Writer, diagnostics_)
              << "unhandled struct member attribute: " << deco->Name();
        }
      }
    }

    out << ";";

    if (is_host_shareable) {
      // Calculate new MSL offset
      auto size_align = MslPackedTypeSizeAndAlign(ty);
      if (msl_offset % size_align.align) {
        TINT_ICE(Writer, diagnostics_)
            << "Misaligned MSL structure member "
            << ty->FriendlyName(program_->Symbols()) << " " << name;
        return false;
      }
      msl_offset += size_align.size;
    }
  }

  if (is_host_shareable && str->Size() != msl_offset) {
    add_padding(str->Size() - msl_offset, msl_offset);
  }

  b->DecrementIndent();

  line(b) << "};";
  return true;
}

bool GeneratorImpl::EmitUnaryOp(std::ostream& out,
                                const ast::UnaryOpExpression* expr) {
  // Handle `-e` when `e` is signed, so that we ensure that if `e` is the
  // largest negative value, it returns `e`.
  auto* expr_type = TypeOf(expr->expr)->UnwrapRef();
  if (expr->op == ast::UnaryOp::kNegation &&
      expr_type->is_signed_scalar_or_vector()) {
    auto fn =
        utils::GetOrCreate(unary_minus_funcs_, expr_type, [&]() -> std::string {
          // e.g.:
          // int tint_unary_minus(const int v) {
          //     return (v == -2147483648) ? v : -v;
          // }
          TextBuffer b;
          TINT_DEFER(helpers_.Append(b));

          auto fn_name = UniqueIdentifier("tint_unary_minus");
          {
            auto decl = line(&b);
            if (!EmitTypeAndName(decl, expr_type, fn_name)) {
              return "";
            }
            decl << "(const ";
            if (!EmitType(decl, expr_type, "")) {
              return "";
            }
            decl << " v) {";
          }

          {
            ScopedIndent si(&b);
            const auto largest_negative_value =
                std::to_string(std::numeric_limits<int32_t>::min());
            line(&b) << "return select(-v, v, v == " << largest_negative_value
                     << ");";
          }
          line(&b) << "}";
          line(&b);
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

bool GeneratorImpl::EmitVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();

  for (auto* deco : decl->decorations) {
    if (!deco->Is<ast::InternalDecoration>()) {
      TINT_ICE(Writer, diagnostics_) << "unexpected variable decoration";
      return false;
    }
  }

  auto out = line();

  switch (var->StorageClass()) {
    case ast::StorageClass::kFunction:
    case ast::StorageClass::kUniformConstant:
    case ast::StorageClass::kNone:
      break;
    case ast::StorageClass::kPrivate:
      out << "thread ";
      break;
    case ast::StorageClass::kWorkgroup:
      out << "threadgroup ";
      break;
    default:
      TINT_ICE(Writer, diagnostics_) << "unhandled variable storage class";
      return false;
  }

  auto* type = var->Type()->UnwrapRef();

  std::string name = program_->Symbols().NameFor(decl->symbol);
  if (decl->is_const) {
    name = "const " + name;
  }
  if (!EmitType(out, type, name)) {
    return false;
  }
  // Variable name is output as part of the type for arrays and pointers.
  if (!type->Is<sem::Array>() && !type->Is<sem::Pointer>()) {
    out << " " << name;
  }

  if (decl->constructor != nullptr) {
    out << " = ";
    if (!EmitExpression(out, decl->constructor)) {
      return false;
    }
  } else if (var->StorageClass() == ast::StorageClass::kPrivate ||
             var->StorageClass() == ast::StorageClass::kFunction ||
             var->StorageClass() == ast::StorageClass::kNone) {
    out << " = ";
    if (!EmitZeroValue(out, type)) {
      return false;
    }
  }
  out << ";";

  return true;
}

bool GeneratorImpl::EmitProgramConstVariable(const ast::Variable* var) {
  for (auto* d : var->decorations) {
    if (!d->Is<ast::OverrideDecoration>()) {
      diagnostics_.add_error(diag::System::Writer,
                             "Decorated const values not valid");
      return false;
    }
  }
  if (!var->is_const) {
    diagnostics_.add_error(diag::System::Writer, "Expected a const value");
    return false;
  }

  auto out = line();
  out << "constant ";
  auto* type = program_->Sem().Get(var)->Type()->UnwrapRef();
  if (!EmitType(out, type, program_->Symbols().NameFor(var->symbol))) {
    return false;
  }
  if (!type->Is<sem::Array>()) {
    out << " " << program_->Symbols().NameFor(var->symbol);
  }

  auto* global = program_->Sem().Get<sem::GlobalVariable>(var);
  if (global && global->IsOverridable()) {
    out << " [[function_constant(" << global->ConstantId() << ")]]";
  } else if (var->constructor != nullptr) {
    out << " = ";
    if (!EmitExpression(out, var->constructor)) {
      return false;
    }
  }
  out << ";";

  return true;
}

GeneratorImpl::SizeAndAlign GeneratorImpl::MslPackedTypeSizeAndAlign(
    const sem::Type* ty) {
  if (ty->IsAnyOf<sem::U32, sem::I32, sem::F32>()) {
    // https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf
    // 2.1 Scalar Data Types
    return {4, 4};
  }

  if (auto* vec = ty->As<sem::Vector>()) {
    auto num_els = vec->Width();
    auto* el_ty = vec->type();
    if (el_ty->IsAnyOf<sem::U32, sem::I32, sem::F32>()) {
      // Use a packed_vec type for 3-element vectors only.
      if (num_els == 3) {
        // https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf
        // 2.2.3 Packed Vector Types
        return SizeAndAlign{num_els * 4, 4};
      } else {
        // https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf
        // 2.2 Vector Data Types
        return SizeAndAlign{num_els * 4, num_els * 4};
      }
    }
  }

  if (auto* mat = ty->As<sem::Matrix>()) {
    // https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf
    // 2.3 Matrix Data Types
    auto cols = mat->columns();
    auto rows = mat->rows();
    auto* el_ty = mat->type();
    if (el_ty->IsAnyOf<sem::U32, sem::I32, sem::F32>()) {
      static constexpr SizeAndAlign table[] = {
          /* float2x2 */ {16, 8},
          /* float2x3 */ {32, 16},
          /* float2x4 */ {32, 16},
          /* float3x2 */ {24, 8},
          /* float3x3 */ {48, 16},
          /* float3x4 */ {48, 16},
          /* float4x2 */ {32, 8},
          /* float4x3 */ {64, 16},
          /* float4x4 */ {64, 16},
      };
      if (cols >= 2 && cols <= 4 && rows >= 2 && rows <= 4) {
        return table[(3 * (cols - 2)) + (rows - 2)];
      }
    }
  }

  if (auto* arr = ty->As<sem::Array>()) {
    if (!arr->IsStrideImplicit()) {
      TINT_ICE(Writer, diagnostics_)
          << "arrays with explicit strides should have "
             "removed with the PadArrayElements transform";
      return {};
    }
    auto num_els = std::max<uint32_t>(arr->Count(), 1);
    return SizeAndAlign{arr->Stride() * num_els, arr->Align()};
  }

  if (auto* str = ty->As<sem::Struct>()) {
    // TODO(crbug.com/tint/650): There's an assumption here that MSL's default
    // structure size and alignment matches WGSL's. We need to confirm this.
    return SizeAndAlign{str->Size(), str->Align()};
  }

  if (auto* atomic = ty->As<sem::Atomic>()) {
    return MslPackedTypeSizeAndAlign(atomic->Type());
  }

  TINT_UNREACHABLE(Writer, diagnostics_)
      << "Unhandled type " << ty->TypeInfo().name;
  return {};
}

template <typename F>
bool GeneratorImpl::CallIntrinsicHelper(std::ostream& out,
                                        const ast::CallExpression* call,
                                        const sem::Intrinsic* intrinsic,
                                        F&& build) {
  // Generate the helper function if it hasn't been created already
  auto fn = utils::GetOrCreate(intrinsics_, intrinsic, [&]() -> std::string {
    TextBuffer b;
    TINT_DEFER(helpers_.Append(b));

    auto fn_name =
        UniqueIdentifier(std::string("tint_") + sem::str(intrinsic->Type()));
    std::vector<std::string> parameter_names;
    {
      auto decl = line(&b);
      if (!EmitTypeAndName(decl, intrinsic->ReturnType(), fn_name)) {
        return "";
      }
      {
        ScopedParen sp(decl);
        for (auto* param : intrinsic->Parameters()) {
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

}  // namespace msl
}  // namespace writer
}  // namespace tint
