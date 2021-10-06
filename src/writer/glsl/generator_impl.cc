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

#include "src/writer/glsl/generator_impl.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <set>
#include <utility>
#include <vector>

#include "src/ast/call_statement.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/internal_decoration.h"
#include "src/ast/interpolate_decoration.h"
#include "src/ast/override_decoration.h"
#include "src/ast/variable_decl_statement.h"
#include "src/debug.h"
#include "src/sem/array.h"
#include "src/sem/atomic_type.h"
#include "src/sem/block_statement.h"
#include "src/sem/call.h"
#include "src/sem/depth_multisampled_texture_type.h"
#include "src/sem/depth_texture_type.h"
#include "src/sem/function.h"
#include "src/sem/member_accessor_expression.h"
#include "src/sem/multisampled_texture_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/statement.h"
#include "src/sem/storage_texture_type.h"
#include "src/sem/struct.h"
#include "src/sem/variable.h"
#include "src/transform/calculate_array_length.h"
#include "src/transform/glsl.h"
#include "src/utils/defer.h"
#include "src/utils/get_or_create.h"
#include "src/utils/scoped_assignment.h"
#include "src/writer/append_vector.h"
#include "src/writer/float_to_string.h"

namespace tint {
namespace writer {
namespace glsl {
namespace {

const char kTempNamePrefix[] = "tint_tmp";
const char kSpecConstantPrefix[] = "WGSL_SPEC_CONSTANT_";

bool last_is_break_or_fallthrough(const ast::BlockStatement* stmts) {
  if (stmts->empty()) {
    return false;
  }

  return stmts->last()->Is<ast::BreakStatement>() ||
         stmts->last()->Is<ast::FallthroughStatement>();
}

const char* image_format_to_rwtexture_type(ast::ImageFormat image_format) {
  switch (image_format) {
    case ast::ImageFormat::kRgba8Unorm:
    case ast::ImageFormat::kRgba8Snorm:
    case ast::ImageFormat::kRgba16Float:
    case ast::ImageFormat::kR32Float:
    case ast::ImageFormat::kRg32Float:
    case ast::ImageFormat::kRgba32Float:
      return "float4";
    case ast::ImageFormat::kRgba8Uint:
    case ast::ImageFormat::kRgba16Uint:
    case ast::ImageFormat::kR32Uint:
    case ast::ImageFormat::kRg32Uint:
    case ast::ImageFormat::kRgba32Uint:
      return "uint4";
    case ast::ImageFormat::kRgba8Sint:
    case ast::ImageFormat::kRgba16Sint:
    case ast::ImageFormat::kR32Sint:
    case ast::ImageFormat::kRg32Sint:
    case ast::ImageFormat::kRgba32Sint:
      return "int4";
    default:
      return nullptr;
  }
}

// Helper for writing " : register(RX, spaceY)", where R is the register, X is
// the binding point binding value, and Y is the binding point group value.
struct RegisterAndSpace {
  RegisterAndSpace(char r, ast::Variable::BindingPoint bp)
      : reg(r), binding_point(bp) {}

  char const reg;
  ast::Variable::BindingPoint const binding_point;
};

std::ostream& operator<<(std::ostream& s, const RegisterAndSpace& rs) {
  s << " : register(" << rs.reg << rs.binding_point.binding->value()
    << ", space" << rs.binding_point.group->value() << ")";
  return s;
}

}  // namespace

GeneratorImpl::GeneratorImpl(const Program* program) : TextGenerator(program) {}

GeneratorImpl::~GeneratorImpl() = default;

bool GeneratorImpl::Generate() {
  if (!builder_.HasTransformApplied<transform::Glsl>()) {
    diagnostics_.add_error(
        diag::System::Writer,
        "GLSL writer requires the transform::Glsl sanitizer to have been "
        "applied to the input program");
    return false;
  }

  const TypeInfo* last_kind = nullptr;
  size_t last_padding_line = 0;

  line() << "#version 310 es";
  line() << "precision mediump float;" << std::endl;

  for (auto* decl : builder_.AST().GlobalDeclarations()) {
    if (decl->Is<ast::Alias>()) {
      continue;  // Ignore aliases.
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

    if (auto* global = decl->As<ast::Variable>()) {
      if (!EmitGlobalVariable(global)) {
        return false;
      }
    } else if (auto* str = decl->As<ast::Struct>()) {
      if (!EmitStructType(current_buffer_, builder_.Sem().Get(str))) {
        return false;
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
    } else {
      TINT_ICE(Writer, diagnostics_)
          << "unhandled module-scope declaration: " << decl->TypeInfo().name;
      return false;
    }
  }

  if (!helpers_.lines.empty()) {
    current_buffer_->Insert(helpers_, 0, 0);
  }

  return true;
}

bool GeneratorImpl::EmitArrayAccessor(std::ostream& out,
                                      ast::ArrayAccessorExpression* expr) {
  if (!EmitExpression(out, expr->array())) {
    return false;
  }
  out << "[";

  if (!EmitExpression(out, expr->idx_expr())) {
    return false;
  }
  out << "]";

  return true;
}

bool GeneratorImpl::EmitBitcast(std::ostream& out,
                                ast::BitcastExpression* expr) {
  auto* type = TypeOf(expr);
  if (auto* vec = type->UnwrapRef()->As<sem::Vector>()) {
    type = vec->type();
  }

  if (!type->is_integer_scalar() && !type->is_float_scalar()) {
    diagnostics_.add_error(diag::System::Writer,
                           "Unable to do bitcast to type " + type->type_name());
    return false;
  }

  out << "as";
  if (!EmitType(out, type, ast::StorageClass::kNone, ast::Access::kReadWrite,
                "")) {
    return false;
  }
  out << "(";
  if (!EmitExpression(out, expr->expr())) {
    return false;
  }
  out << ")";
  return true;
}

bool GeneratorImpl::EmitAssign(ast::AssignmentStatement* stmt) {
  auto out = line();
  if (!EmitExpression(out, stmt->lhs())) {
    return false;
  }
  out << " = ";
  if (!EmitExpression(out, stmt->rhs())) {
    return false;
  }
  out << ";";
  return true;
}

bool GeneratorImpl::EmitBinary(std::ostream& out, ast::BinaryExpression* expr) {
  if (expr->op() == ast::BinaryOp::kLogicalAnd ||
      expr->op() == ast::BinaryOp::kLogicalOr) {
    auto name = UniqueIdentifier(kTempNamePrefix);

    {
      auto pre = line();
      pre << "bool " << name << " = ";
      if (!EmitExpression(pre, expr->lhs())) {
        return false;
      }
      pre << ";";
    }

    if (expr->op() == ast::BinaryOp::kLogicalOr) {
      line() << "if (!" << name << ") {";
    } else {
      line() << "if (" << name << ") {";
    }

    {
      ScopedIndent si(this);
      auto pre = line();
      pre << name << " = ";
      if (!EmitExpression(pre, expr->rhs())) {
        return false;
      }
      pre << ";";
    }

    line() << "}";

    out << "(" << name << ")";
    return true;
  }

  out << "(";
  if (!EmitExpression(out, expr->lhs())) {
    return false;
  }
  out << " ";

  switch (expr->op()) {
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

  if (!EmitExpression(out, expr->rhs())) {
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
  if (!EmitStatementsWithIndent(stmt->statements())) {
    return false;
  }
  line() << "}";
  return true;
}

bool GeneratorImpl::EmitBreak(ast::BreakStatement*) {
  line() << "break;";
  return true;
}

bool GeneratorImpl::EmitCall(std::ostream& out, ast::CallExpression* expr) {
  const auto& params = expr->params();
  auto* ident = expr->func();
  auto* call = builder_.Sem().Get(expr);
  auto* target = call->Target();

  if (auto* func = target->As<sem::Function>()) {
    if (ast::HasDecoration<
            transform::CalculateArrayLength::BufferSizeIntrinsic>(
            func->Declaration()->decorations())) {
      // Special function generated by the CalculateArrayLength transform for
      // calling X.GetDimensions(Y)
      if (!EmitExpression(out, params[0])) {
        return false;
      }
      out << ".GetDimensions(";
      if (!EmitExpression(out, params[1])) {
        return false;
      }
      out << ")";
      return true;
    }
  }

  if (auto* intrinsic = call->Target()->As<sem::Intrinsic>()) {
    if (intrinsic->IsTexture()) {
      return EmitTextureCall(out, expr, intrinsic);
    } else if (intrinsic->Type() == sem::IntrinsicType::kSelect) {
      return EmitSelectCall(out, expr);
    } else if (intrinsic->Type() == sem::IntrinsicType::kModf) {
      return EmitModfCall(out, expr, intrinsic);
    } else if (intrinsic->Type() == sem::IntrinsicType::kFrexp) {
      return EmitFrexpCall(out, expr, intrinsic);
    } else if (intrinsic->Type() == sem::IntrinsicType::kIsNormal) {
      return EmitIsNormalCall(out, expr, intrinsic);
    } else if (intrinsic->Type() == sem::IntrinsicType::kIgnore) {
      return EmitExpression(out, expr->params()[0]);
    } else if (intrinsic->IsDataPacking()) {
      return EmitDataPackingCall(out, expr, intrinsic);
    } else if (intrinsic->IsDataUnpacking()) {
      return EmitDataUnpackingCall(out, expr, intrinsic);
    } else if (intrinsic->IsBarrier()) {
      return EmitBarrierCall(out, intrinsic);
    } else if (intrinsic->IsAtomic()) {
      return EmitWorkgroupAtomicCall(out, expr, intrinsic);
    }
    auto name = generate_builtin_name(intrinsic);
    if (name.empty()) {
      return false;
    }

    out << name << "(";

    bool first = true;
    for (auto* param : params) {
      if (!first) {
        out << ", ";
      }
      first = false;

      if (!EmitExpression(out, param)) {
        return false;
      }
    }

    out << ")";
    return true;
  }

  auto name = builder_.Symbols().NameFor(ident->symbol());
  auto caller_sym = ident->symbol();

  auto* func = builder_.AST().Functions().Find(ident->symbol());
  if (func == nullptr) {
    diagnostics_.add_error(diag::System::Writer,
                           "Unable to find function: " +
                               builder_.Symbols().NameFor(ident->symbol()));
    return false;
  }

  out << name << "(";

  bool first = true;
  for (auto* param : params) {
    if (!first) {
      out << ", ";
    }
    first = false;

    if (!EmitExpression(out, param)) {
      return false;
    }
  }

  out << ")";

  return true;
}

bool GeneratorImpl::EmitWorkgroupAtomicCall(std::ostream& out,
                                            ast::CallExpression* expr,
                                            const sem::Intrinsic* intrinsic) {
  std::string result = UniqueIdentifier("atomic_result");

  if (!intrinsic->ReturnType()->Is<sem::Void>()) {
    auto pre = line();
    if (!EmitTypeAndName(pre, intrinsic->ReturnType(), ast::StorageClass::kNone,
                         ast::Access::kUndefined, result)) {
      return false;
    }
    pre << " = ";
    if (!EmitZeroValue(pre, intrinsic->ReturnType())) {
      return false;
    }
    pre << ";";
  }

  auto call = [&](const char* name) {
    auto pre = line();
    pre << name;

    {
      ScopedParen sp(pre);
      for (size_t i = 0; i < expr->params().size(); i++) {
        auto* arg = expr->params()[i];
        if (i > 0) {
          pre << ", ";
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

  switch (intrinsic->Type()) {
    case sem::IntrinsicType::kAtomicLoad: {
      // GLSL does not have an InterlockedLoad, so we emulate it with
      // InterlockedOr using 0 as the OR value
      auto pre = line();
      pre << "InterlockedOr";
      {
        ScopedParen sp(pre);
        if (!EmitExpression(pre, expr->params()[0])) {
          return false;
        }
        pre << ", 0, " << result;
      }
      pre << ";";

      out << result;
      return true;
    }
    case sem::IntrinsicType::kAtomicStore: {
      // GLSL does not have an InterlockedStore, so we emulate it with
      // InterlockedExchange and discard the returned value
      {  // T result = 0;
        auto pre = line();
        auto* value_ty = intrinsic->Parameters()[1]->Type();
        if (!EmitTypeAndName(pre, value_ty, ast::StorageClass::kNone,
                             ast::Access::kUndefined, result)) {
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
        if (!EmitExpression(out, expr->params()[0])) {
          return false;
        }
        out << ", ";
        if (!EmitExpression(out, expr->params()[1])) {
          return false;
        }
        out << ", " << result;
      }
      return true;
    }
    case sem::IntrinsicType::kAtomicCompareExchangeWeak: {
      auto* dest = expr->params()[0];
      auto* compare_value = expr->params()[1];
      auto* value = expr->params()[2];

      std::string compare = UniqueIdentifier("atomic_compare_value");

      {  // T compare_value = <compare_value>;
        auto pre = line();
        if (!EmitTypeAndName(pre, TypeOf(compare_value),
                             ast::StorageClass::kNone, ast::Access::kUndefined,
                             compare)) {
          return false;
        }
        pre << " = ";
        if (!EmitExpression(pre, compare_value)) {
          return false;
        }
        pre << ";";
      }

      {  // InterlockedCompareExchange(dst, compare, value, result.x);
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
          pre << ", " << result << ".x";
        }
        pre << ";";
      }

      {  // result.y = result.x == compare;
        line() << result << ".y = " << result << ".x == " << compare << ";";
      }

      out << result;
      return true;
    }

    case sem::IntrinsicType::kAtomicAdd:
    case sem::IntrinsicType::kAtomicSub:
      return call("InterlockedAdd");

    case sem::IntrinsicType::kAtomicMax:
      return call("InterlockedMax");

    case sem::IntrinsicType::kAtomicMin:
      return call("InterlockedMin");

    case sem::IntrinsicType::kAtomicAnd:
      return call("InterlockedAnd");

    case sem::IntrinsicType::kAtomicOr:
      return call("InterlockedOr");

    case sem::IntrinsicType::kAtomicXor:
      return call("InterlockedXor");

    case sem::IntrinsicType::kAtomicExchange:
      return call("InterlockedExchange");

    default:
      break;
  }

  TINT_UNREACHABLE(Writer, diagnostics_)
      << "unsupported atomic intrinsic: " << intrinsic->Type();
  return false;
}

bool GeneratorImpl::EmitSelectCall(std::ostream& out,
                                   ast::CallExpression* expr) {
  auto* expr_false = expr->params()[0];
  auto* expr_true = expr->params()[1];
  auto* expr_cond = expr->params()[2];
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

bool GeneratorImpl::EmitModfCall(std::ostream& out,
                                 ast::CallExpression* expr,
                                 const sem::Intrinsic* intrinsic) {
  if (expr->params().size() == 1) {
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
          {
            auto l = line(b);
            if (!EmitType(l, intrinsic->ReturnType(), ast::StorageClass::kNone,
                          ast::Access::kUndefined, "")) {
              return false;
            }
            l << " result = {fract, whole};";
          }
          line(b) << "return result;";
          return true;
        });
  }

  // DEPRECATED
  out << "modf";
  ScopedParen sp(out);
  if (!EmitExpression(out, expr->params()[0])) {
    return false;
  }
  out << ", ";
  if (!EmitExpression(out, expr->params()[1])) {
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitFrexpCall(std::ostream& out,
                                  ast::CallExpression* expr,
                                  const sem::Intrinsic* intrinsic) {
  if (expr->params().size() == 1) {
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

          line(b) << "float" << width << " exp;";
          line(b) << "float" << width << " sig = frexp(" << in << ", exp);";
          {
            auto l = line(b);
            if (!EmitType(l, intrinsic->ReturnType(), ast::StorageClass::kNone,
                          ast::Access::kUndefined, "")) {
              return false;
            }
            l << " result = {sig, int" << width << "(exp)};";
          }
          line(b) << "return result;";
          return true;
        });
  }
  // DEPRECATED
  // Exponent is an integer in WGSL, but HLSL wants a float.
  // We need to make the call with a temporary float, and then cast.
  return CallIntrinsicHelper(
      out, expr, intrinsic,
      [&](TextBuffer* b, const std::vector<std::string>& params) {
        auto* significand_ty = intrinsic->Parameters()[0]->Type();
        auto significand = params[0];
        auto* exponent_ty = intrinsic->Parameters()[1]->Type();
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

bool GeneratorImpl::EmitIsNormalCall(std::ostream& out,
                                     ast::CallExpression* expr,
                                     const sem::Intrinsic* intrinsic) {
  // GLSL doesn't have a isNormal intrinsic, we need to emulate
  return CallIntrinsicHelper(
      out, expr, intrinsic,
      [&](TextBuffer* b, const std::vector<std::string>& params) {
        auto* input_ty = intrinsic->Parameters()[0]->Type();

        std::string width;
        if (auto* vec = input_ty->As<sem::Vector>()) {
          width = std::to_string(vec->Width());
        }

        constexpr auto* kExponentMask = "0x7f80000";
        constexpr auto* kMinNormalExponent = "0x0080000";
        constexpr auto* kMaxNormalExponent = "0x7f00000";

        line(b) << "uint" << width << " exponent = asuint(" << params[0]
                << ") & " << kExponentMask << ";";
        line(b) << "uint" << width << " clamped = "
                << "clamp(exponent, " << kMinNormalExponent << ", "
                << kMaxNormalExponent << ");";
        line(b) << "return clamped == exponent;";
        return true;
      });
}

bool GeneratorImpl::EmitDataPackingCall(std::ostream& out,
                                        ast::CallExpression* expr,
                                        const sem::Intrinsic* intrinsic) {
  return CallIntrinsicHelper(
      out, expr, intrinsic,
      [&](TextBuffer* b, const std::vector<std::string>& params) {
        uint32_t dims = 2;
        bool is_signed = false;
        uint32_t scale = 65535;
        if (intrinsic->Type() == sem::IntrinsicType::kPack4x8snorm ||
            intrinsic->Type() == sem::IntrinsicType::kPack4x8unorm) {
          dims = 4;
          scale = 255;
        }
        if (intrinsic->Type() == sem::IntrinsicType::kPack4x8snorm ||
            intrinsic->Type() == sem::IntrinsicType::kPack2x16snorm) {
          is_signed = true;
          scale = (scale - 1) / 2;
        }
        switch (intrinsic->Type()) {
          case sem::IntrinsicType::kPack4x8snorm:
          case sem::IntrinsicType::kPack4x8unorm:
          case sem::IntrinsicType::kPack2x16snorm:
          case sem::IntrinsicType::kPack2x16unorm: {
            {
              auto l = line(b);
              l << (is_signed ? "" : "u") << "int" << dims
                << " i = " << (is_signed ? "" : "u") << "int" << dims
                << "(round(clamp(" << params[0] << ", "
                << (is_signed ? "-1.0" : "0.0") << ", 1.0) * " << scale
                << ".0))";
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
          case sem::IntrinsicType::kPack2x16float: {
            line(b) << "uint2 i = f32tof16(" << params[0] << ");";
            line(b) << "return i.x | (i.y << 16);";
            break;
          }
          default:
            diagnostics_.add_error(
                diag::System::Writer,
                "Internal error: unhandled data packing intrinsic");
            return false;
        }

        return true;
      });
}

bool GeneratorImpl::EmitDataUnpackingCall(std::ostream& out,
                                          ast::CallExpression* expr,
                                          const sem::Intrinsic* intrinsic) {
  return CallIntrinsicHelper(
      out, expr, intrinsic,
      [&](TextBuffer* b, const std::vector<std::string>& params) {
        uint32_t dims = 2;
        bool is_signed = false;
        uint32_t scale = 65535;
        if (intrinsic->Type() == sem::IntrinsicType::kUnpack4x8snorm ||
            intrinsic->Type() == sem::IntrinsicType::kUnpack4x8unorm) {
          dims = 4;
          scale = 255;
        }
        if (intrinsic->Type() == sem::IntrinsicType::kUnpack4x8snorm ||
            intrinsic->Type() == sem::IntrinsicType::kUnpack2x16snorm) {
          is_signed = true;
          scale = (scale - 1) / 2;
        }
        switch (intrinsic->Type()) {
          case sem::IntrinsicType::kUnpack4x8snorm:
          case sem::IntrinsicType::kUnpack2x16snorm: {
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
            line(b) << "return clamp(float" << dims << "(i) / " << scale
                    << ".0, " << (is_signed ? "-1.0" : "0.0") << ", 1.0);";
            break;
          }
          case sem::IntrinsicType::kUnpack4x8unorm:
          case sem::IntrinsicType::kUnpack2x16unorm: {
            line(b) << "uint j = " << params[0] << ";";
            {
              auto l = line(b);
              l << "uint" << dims << " i = uint" << dims << "(";
              l << "j & " << (dims == 2 ? "0xffff" : "0xff") << ", ";
              if (dims == 4) {
                l << "(j >> " << (32 / dims)
                  << ") & 0xff, (j >> 16) & 0xff, j >> 24";
              } else {
                l << "j >> " << (32 / dims);
              }
              l << ");";
            }
            line(b) << "return float" << dims << "(i) / " << scale << ".0;";
            break;
          }
          case sem::IntrinsicType::kUnpack2x16float:
            line(b) << "uint i = " << params[0] << ";";
            line(b) << "return f16tof32(uint2(i & 0xffff, i >> 16));";
            break;
          default:
            diagnostics_.add_error(
                diag::System::Writer,
                "Internal error: unhandled data packing intrinsic");
            return false;
        }

        return true;
      });
}

bool GeneratorImpl::EmitBarrierCall(std::ostream& out,
                                    const sem::Intrinsic* intrinsic) {
  // TODO(crbug.com/tint/661): Combine sequential barriers to a single
  // instruction.
  if (intrinsic->Type() == sem::IntrinsicType::kWorkgroupBarrier) {
    out << "GroupMemoryBarrierWithGroupSync()";
  } else if (intrinsic->Type() == sem::IntrinsicType::kStorageBarrier) {
    out << "DeviceMemoryBarrierWithGroupSync()";
  } else {
    TINT_UNREACHABLE(Writer, diagnostics_)
        << "unexpected barrier intrinsic type " << sem::str(intrinsic->Type());
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitTextureCall(std::ostream& out,
                                    ast::CallExpression* expr,
                                    const sem::Intrinsic* intrinsic) {
  using Usage = sem::ParameterUsage;

  auto parameters = intrinsic->Parameters();
  auto arguments = expr->params();

  // Returns the argument with the given usage
  auto arg = [&](Usage usage) {
    int idx = sem::IndexOf(parameters, usage);
    return (idx >= 0) ? arguments[idx] : nullptr;
  };

  auto* texture = arg(Usage::kTexture);
  if (!texture) {
    TINT_ICE(Writer, diagnostics_) << "missing texture argument";
    return false;
  }

  auto* texture_type = TypeOf(texture)->UnwrapRef()->As<sem::Texture>();

  switch (intrinsic->Type()) {
    case sem::IntrinsicType::kTextureDimensions: {
      out << "textureSize(";
      if (!EmitExpression(out, texture)) {
        return false;
      }

      auto* level_arg = arg(Usage::kLevel);
      if (level_arg) {
        if (!EmitExpression(out, level_arg)) {
          return false;
        }
      }
      out << ");";
      return true;
    }
    // TODO(senorblanco): determine if this works for array textures
    case sem::IntrinsicType::kTextureNumLayers:
    case sem::IntrinsicType::kTextureNumLevels: {
      out << "textureQueryLevels(";
      if (!EmitExpression(out, texture)) {
        return false;
      }
      out << ");";
      return true;
    }
    case sem::IntrinsicType::kTextureNumSamples: {
      out << "textureSamples(";
      if (!EmitExpression(out, texture)) {
        return false;
      }
      out << ");";
      return true;
    }
    default:
      break;
  }

  if (!EmitExpression(out, texture))
    return false;

  // If pack_level_in_coords is true, then the mip level will be appended as the
  // last value of the coordinates argument. If the WGSL intrinsic overload does
  // not have a level parameter and pack_level_in_coords is true, then a zero
  // mip level will be inserted.
  bool pack_level_in_coords = false;

  uint32_t glsl_ret_width = 4u;

  switch (intrinsic->Type()) {
    case sem::IntrinsicType::kTextureSample:
      out << ".Sample(";
      break;
    case sem::IntrinsicType::kTextureSampleBias:
      out << ".SampleBias(";
      break;
    case sem::IntrinsicType::kTextureSampleLevel:
      out << ".SampleLevel(";
      break;
    case sem::IntrinsicType::kTextureSampleGrad:
      out << ".SampleGrad(";
      break;
    case sem::IntrinsicType::kTextureSampleCompare:
      out << ".SampleCmp(";
      glsl_ret_width = 1;
      break;
    case sem::IntrinsicType::kTextureSampleCompareLevel:
      out << ".SampleCmpLevelZero(";
      glsl_ret_width = 1;
      break;
    case sem::IntrinsicType::kTextureLoad:
      out << ".Load(";
      // Multisampled textures do not support mip-levels.
      if (!texture_type->Is<sem::MultisampledTexture>()) {
        pack_level_in_coords = true;
      }
      break;
    case sem::IntrinsicType::kTextureStore:
      out << "[";
      break;
    default:
      diagnostics_.add_error(
          diag::System::Writer,
          "Internal compiler error: Unhandled texture intrinsic '" +
              std::string(intrinsic->str()) + "'");
      return false;
  }

  if (auto* sampler = arg(Usage::kSampler)) {
    if (!EmitExpression(out, sampler))
      return false;
    out << ", ";
  }

  auto* param_coords = arg(Usage::kCoords);
  if (!param_coords) {
    TINT_ICE(Writer, diagnostics_) << "missing coords argument";
    return false;
  }

  auto emit_vector_appended_with_i32_zero = [&](tint::ast::Expression* vector) {
    auto* i32 = builder_.create<sem::I32>();
    auto* zero = builder_.Expr(0);
    auto* stmt = builder_.Sem().Get(vector)->Stmt();
    builder_.Sem().Add(zero, builder_.create<sem::Expression>(zero, i32, stmt,
                                                              sem::Constant{}));
    auto* packed = AppendVector(&builder_, vector, zero);
    return EmitExpression(out, packed);
  };

  auto emit_vector_appended_with_level = [&](tint::ast::Expression* vector) {
    if (auto* level = arg(Usage::kLevel)) {
      auto* packed = AppendVector(&builder_, vector, level);
      return EmitExpression(out, packed);
    }
    return emit_vector_appended_with_i32_zero(vector);
  };

  if (auto* array_index = arg(Usage::kArrayIndex)) {
    // Array index needs to be appended to the coordinates.
    auto* packed = AppendVector(&builder_, param_coords, array_index);
    if (pack_level_in_coords) {
      // Then mip level needs to be appended to the coordinates.
      if (!emit_vector_appended_with_level(packed)) {
        return false;
      }
    } else {
      if (!EmitExpression(out, packed)) {
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

  for (auto usage : {Usage::kDepthRef, Usage::kBias, Usage::kLevel, Usage::kDdx,
                     Usage::kDdy, Usage::kSampleIndex, Usage::kOffset}) {
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

  if (intrinsic->Type() == sem::IntrinsicType::kTextureStore) {
    out << "] = ";
    if (!EmitExpression(out, arg(Usage::kValue))) {
      return false;
    }
  } else {
    out << ")";

    // If the intrinsic return type does not match the number of elements of the
    // GLSL intrinsic, we need to swizzle the expression to generate the correct
    // number of components.
    uint32_t wgsl_ret_width = 1;
    if (auto* vec = intrinsic->ReturnType()->As<sem::Vector>()) {
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
          << intrinsic->Type();
      return false;
    }
  }

  return true;
}

std::string GeneratorImpl::generate_builtin_name(
    const sem::Intrinsic* intrinsic) {
  switch (intrinsic->Type()) {
    case sem::IntrinsicType::kAbs:
    case sem::IntrinsicType::kAcos:
    case sem::IntrinsicType::kAll:
    case sem::IntrinsicType::kAny:
    case sem::IntrinsicType::kAsin:
    case sem::IntrinsicType::kAtan:
    case sem::IntrinsicType::kAtan2:
    case sem::IntrinsicType::kCeil:
    case sem::IntrinsicType::kClamp:
    case sem::IntrinsicType::kCos:
    case sem::IntrinsicType::kCosh:
    case sem::IntrinsicType::kCross:
    case sem::IntrinsicType::kDeterminant:
    case sem::IntrinsicType::kDistance:
    case sem::IntrinsicType::kDot:
    case sem::IntrinsicType::kExp:
    case sem::IntrinsicType::kExp2:
    case sem::IntrinsicType::kFloor:
    case sem::IntrinsicType::kFrexp:
    case sem::IntrinsicType::kLdexp:
    case sem::IntrinsicType::kLength:
    case sem::IntrinsicType::kLog:
    case sem::IntrinsicType::kLog2:
    case sem::IntrinsicType::kMax:
    case sem::IntrinsicType::kMin:
    case sem::IntrinsicType::kModf:
    case sem::IntrinsicType::kNormalize:
    case sem::IntrinsicType::kPow:
    case sem::IntrinsicType::kReflect:
    case sem::IntrinsicType::kRefract:
    case sem::IntrinsicType::kRound:
    case sem::IntrinsicType::kSign:
    case sem::IntrinsicType::kSin:
    case sem::IntrinsicType::kSinh:
    case sem::IntrinsicType::kSqrt:
    case sem::IntrinsicType::kStep:
    case sem::IntrinsicType::kTan:
    case sem::IntrinsicType::kTanh:
    case sem::IntrinsicType::kTranspose:
    case sem::IntrinsicType::kTrunc:
      return intrinsic->str();
    case sem::IntrinsicType::kCountOneBits:
      return "countbits";
    case sem::IntrinsicType::kDpdx:
      return "ddx";
    case sem::IntrinsicType::kDpdxCoarse:
      return "ddx_coarse";
    case sem::IntrinsicType::kDpdxFine:
      return "ddx_fine";
    case sem::IntrinsicType::kDpdy:
      return "ddy";
    case sem::IntrinsicType::kDpdyCoarse:
      return "ddy_coarse";
    case sem::IntrinsicType::kDpdyFine:
      return "ddy_fine";
    case sem::IntrinsicType::kFaceForward:
      return "faceforward";
    case sem::IntrinsicType::kFract:
      return "frac";
    case sem::IntrinsicType::kFma:
      return "mad";
    case sem::IntrinsicType::kFwidth:
    case sem::IntrinsicType::kFwidthCoarse:
    case sem::IntrinsicType::kFwidthFine:
      return "fwidth";
    case sem::IntrinsicType::kInverseSqrt:
      return "rsqrt";
    case sem::IntrinsicType::kIsFinite:
      return "isfinite";
    case sem::IntrinsicType::kIsInf:
      return "isinf";
    case sem::IntrinsicType::kIsNan:
      return "isnan";
    case sem::IntrinsicType::kMix:
      return "lerp";
    case sem::IntrinsicType::kReverseBits:
      return "reversebits";
    case sem::IntrinsicType::kSmoothStep:
      return "smoothstep";
    default:
      diagnostics_.add_error(
          diag::System::Writer,
          "Unknown builtin method: " + std::string(intrinsic->str()));
  }

  return "";
}

bool GeneratorImpl::EmitCase(ast::CaseStatement* stmt) {
  if (stmt->IsDefault()) {
    line() << "default: {";
  } else {
    for (auto* selector : stmt->selectors()) {
      auto out = line();
      out << "case ";
      if (!EmitLiteral(out, selector)) {
        return false;
      }
      out << ":";
      if (selector == stmt->selectors().back()) {
        out << " {";
      }
    }
  }

  {
    ScopedIndent si(this);
    if (!EmitStatements(stmt->body()->statements())) {
      return false;
    }
    if (!last_is_break_or_fallthrough(stmt->body())) {
      line() << "break;";
    }
  }

  line() << "}";

  return true;
}

bool GeneratorImpl::EmitConstructor(std::ostream& out,
                                    ast::ConstructorExpression* expr) {
  if (auto* scalar = expr->As<ast::ScalarConstructorExpression>()) {
    return EmitScalarConstructor(out, scalar);
  }
  return EmitTypeConstructor(out, expr->As<ast::TypeConstructorExpression>());
}

bool GeneratorImpl::EmitScalarConstructor(
    std::ostream& out,
    ast::ScalarConstructorExpression* expr) {
  return EmitLiteral(out, expr->literal());
}

bool GeneratorImpl::EmitTypeConstructor(std::ostream& out,
                                        ast::TypeConstructorExpression* expr) {
  auto* type = TypeOf(expr)->UnwrapRef();

  // If the type constructor is empty then we need to construct with the zero
  // value for all components.
  if (expr->values().empty()) {
    return EmitZeroValue(out, type);
  }

  // For single-value vector initializers, swizzle the scalar to the right
  // vector dimension using .x
  const bool is_single_value_vector_init =
      type->is_scalar_vector() && expr->values().size() == 1 &&
      TypeOf(expr->values()[0])->UnwrapRef()->is_scalar();

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

  if (is_single_value_vector_init) {
    out << "(";
  }

  bool first = true;
  for (auto* e : expr->values()) {
    if (!first) {
      out << ", ";
    }
    first = false;

    if (!EmitExpression(out, e)) {
      return false;
    }
  }

  if (is_single_value_vector_init) {
    out << ")." << std::string(type->As<sem::Vector>()->Width(), 'x');
  }

  out << ")";
  return true;
}

bool GeneratorImpl::EmitContinue(ast::ContinueStatement*) {
  if (!emit_continuing_()) {
    return false;
  }
  line() << "continue;";
  return true;
}

bool GeneratorImpl::EmitDiscard(ast::DiscardStatement*) {
  // TODO(dsinclair): Verify this is correct when the discard semantics are
  // defined for WGSL (https://github.com/gpuweb/gpuweb/issues/361)
  line() << "discard;";
  return true;
}

bool GeneratorImpl::EmitExpression(std::ostream& out, ast::Expression* expr) {
  if (auto* a = expr->As<ast::ArrayAccessorExpression>()) {
    return EmitArrayAccessor(out, a);
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
  if (auto* c = expr->As<ast::ConstructorExpression>()) {
    return EmitConstructor(out, c);
  }
  if (auto* i = expr->As<ast::IdentifierExpression>()) {
    return EmitIdentifier(out, i);
  }
  if (auto* m = expr->As<ast::MemberAccessorExpression>()) {
    return EmitMemberAccessor(out, m);
  }
  if (auto* u = expr->As<ast::UnaryOpExpression>()) {
    return EmitUnaryOp(out, u);
  }

  diagnostics_.add_error(diag::System::Writer,
                         "unknown expression type: " + builder_.str(expr));
  return false;
}

bool GeneratorImpl::EmitIdentifier(std::ostream& out,
                                   ast::IdentifierExpression* expr) {
  out << builder_.Symbols().NameFor(expr->symbol());
  return true;
}

bool GeneratorImpl::EmitIf(ast::IfStatement* stmt) {
  {
    auto out = line();
    out << "if (";
    if (!EmitExpression(out, stmt->condition())) {
      return false;
    }
    out << ") {";
  }

  if (!EmitStatementsWithIndent(stmt->body()->statements())) {
    return false;
  }

  for (auto* e : stmt->else_statements()) {
    if (e->HasCondition()) {
      line() << "} else {";
      increment_indent();

      {
        auto out = line();
        out << "if (";
        if (!EmitExpression(out, e->condition())) {
          return false;
        }
        out << ") {";
      }
    } else {
      line() << "} else {";
    }

    if (!EmitStatementsWithIndent(e->body()->statements())) {
      return false;
    }
  }

  line() << "}";

  for (auto* e : stmt->else_statements()) {
    if (e->HasCondition()) {
      decrement_indent();
      line() << "}";
    }
  }
  return true;
}

bool GeneratorImpl::EmitFunction(ast::Function* func) {
  auto* sem = builder_.Sem().Get(func);

  if (ast::HasDecoration<ast::InternalDecoration>(func->decorations())) {
    // An internal function. Do not emit.
    return true;
  }

  {
    auto out = line();
    auto name = builder_.Symbols().NameFor(func->symbol());
    // If the function returns an array, then we need to declare a typedef for
    // this.
    if (sem->ReturnType()->Is<sem::Array>()) {
      auto typedef_name = UniqueIdentifier(name + "_ret");
      auto pre = line();
      pre << "typedef ";
      if (!EmitTypeAndName(pre, sem->ReturnType(), ast::StorageClass::kNone,
                           ast::Access::kReadWrite, typedef_name)) {
        return false;
      }
      pre << ";";
      out << typedef_name;
    } else {
      if (!EmitType(out, sem->ReturnType(), ast::StorageClass::kNone,
                    ast::Access::kReadWrite, "")) {
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
              builder_.Symbols().NameFor(v->Declaration()->symbol()))) {
        return false;
      }
    }
    out << ") {";
  }

  if (!EmitStatementsWithIndent(func->body()->statements())) {
    return false;
  }

  line() << "}";

  return true;
}

bool GeneratorImpl::EmitGlobalVariable(ast::Variable* global) {
  if (global->is_const()) {
    return EmitProgramConstVariable(global);
  }

  auto* sem = builder_.Sem().Get(global);
  switch (sem->StorageClass()) {
    case ast::StorageClass::kUniform:
      return EmitUniformVariable(sem);
    case ast::StorageClass::kStorage:
      return EmitStorageVariable(sem);
    case ast::StorageClass::kUniformConstant:
      return EmitHandleVariable(sem);
    case ast::StorageClass::kPrivate:
      return EmitPrivateVariable(sem);
    case ast::StorageClass::kWorkgroup:
      return EmitWorkgroupVariable(sem);
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
  auto out = line();
  if (!EmitTypeAndName(out, type, ast::StorageClass::kUniform, var->Access(),
                       builder_.Symbols().NameFor(decl->symbol()))) {
    return false;
  }
  out << ";";

  return true;
}

bool GeneratorImpl::EmitStorageVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();
  auto* type = var->Type()->UnwrapRef();
  auto out = line();
  if (!EmitTypeAndName(out, type, ast::StorageClass::kStorage, var->Access(),
                       builder_.Symbols().NameFor(decl->symbol()))) {
    return false;
  }

  out << RegisterAndSpace(var->Access() == ast::Access::kRead ? 't' : 'u',
                          decl->binding_point())
      << ";";

  return true;
}

bool GeneratorImpl::EmitHandleVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();
  auto* unwrapped_type = var->Type()->UnwrapRef();
  auto out = line();

  auto name = builder_.Symbols().NameFor(decl->symbol());
  auto* type = var->Type()->UnwrapRef();
  if (!EmitTypeAndName(out, type, var->StorageClass(), var->Access(), name)) {
    return false;
  }

  const char* register_space = nullptr;

  if (unwrapped_type->Is<sem::Texture>()) {
    register_space = "t";
    if (auto* storage_tex = unwrapped_type->As<sem::StorageTexture>()) {
      if (storage_tex->access() != ast::Access::kRead) {
        register_space = "u";
      }
    }
  } else if (unwrapped_type->Is<sem::Sampler>()) {
    register_space = "s";
  }

  if (register_space) {
    auto bp = decl->binding_point();
    out << " : register(" << register_space << bp.binding->value() << ", space"
        << bp.group->value() << ")";
  }

  out << ";";
  return true;
}

bool GeneratorImpl::EmitPrivateVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();
  auto out = line();

  out << "static ";

  auto name = builder_.Symbols().NameFor(decl->symbol());
  auto* type = var->Type()->UnwrapRef();
  if (!EmitTypeAndName(out, type, var->StorageClass(), var->Access(), name)) {
    return false;
  }

  out << " = ";
  if (auto* constructor = decl->constructor()) {
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

  out << "groupshared ";

  auto name = builder_.Symbols().NameFor(decl->symbol());
  auto* type = var->Type()->UnwrapRef();
  if (!EmitTypeAndName(out, type, var->StorageClass(), var->Access(), name)) {
    return false;
  }

  if (auto* constructor = decl->constructor()) {
    out << " = ";
    if (!EmitExpression(out, constructor)) {
      return false;
    }
  }

  out << ";";
  return true;
}

sem::Type* GeneratorImpl::builtin_type(ast::Builtin builtin) {
  switch (builtin) {
    case ast::Builtin::kPosition: {
      auto* f32 = builder_.create<sem::F32>();
      return builder_.create<sem::Vector>(f32, 4);
    }
    case ast::Builtin::kVertexIndex:
    case ast::Builtin::kInstanceIndex: {
      return builder_.create<sem::I32>();
    }
    case ast::Builtin::kFrontFacing: {
      return builder_.create<sem::Bool>();
    }
    case ast::Builtin::kFragDepth: {
      return builder_.create<sem::F32>();
    }
    case ast::Builtin::kLocalInvocationId:
    case ast::Builtin::kGlobalInvocationId:
    case ast::Builtin::kWorkgroupId: {
      auto* u32 = builder_.create<sem::U32>();
      return builder_.create<sem::Vector>(u32, 3);
    }
    case ast::Builtin::kSampleIndex: {
      return builder_.create<sem::I32>();
    }
    case ast::Builtin::kSampleMask:
    default:
      return nullptr;
  }
}

const char* GeneratorImpl::builtin_to_string(ast::Builtin builtin) const {
  switch (builtin) {
    case ast::Builtin::kPosition:
      return "gl_Position";
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
    case ast::Builtin::kWorkgroupId:
      return "gl_WorkGroupID";
    case ast::Builtin::kSampleIndex:
      return "gl_SampleID";
    case ast::Builtin::kSampleMask:
      // FIXME: is this always available?
      return "gl_SampleMask";
    default:
      return "";
  }
}

std::string GeneratorImpl::interpolation_to_modifiers(
    ast::InterpolationType type,
    ast::InterpolationSampling sampling) const {
  std::string modifiers;
  switch (type) {
    case ast::InterpolationType::kPerspective:
      modifiers += "linear ";
      break;
    case ast::InterpolationType::kLinear:
      modifiers += "noperspective ";
      break;
    case ast::InterpolationType::kFlat:
      modifiers += "nointerpolation ";
      break;
  }
  switch (sampling) {
    case ast::InterpolationSampling::kCentroid:
      modifiers += "centroid ";
      break;
    case ast::InterpolationSampling::kSample:
      modifiers += "sample ";
      break;
    case ast::InterpolationSampling::kCenter:
    case ast::InterpolationSampling::kNone:
      break;
  }
  return modifiers;
}

bool GeneratorImpl::EmitEntryPointFunction(ast::Function* func) {
  auto* func_sem = builder_.Sem().Get(func);

  {
    auto out = line();
    if (func->pipeline_stage() == ast::PipelineStage::kCompute) {
      // Emit the workgroup_size attribute.
      auto wgsize = func_sem->workgroup_size();
      out << "[numthreads(";
      for (int i = 0; i < 3; i++) {
        if (i > 0) {
          out << ", ";
        }

        if (wgsize[i].overridable_const) {
          auto* global = builder_.Sem().Get<sem::GlobalVariable>(
              wgsize[i].overridable_const);
          if (!global->IsPipelineConstant()) {
            TINT_ICE(Writer, builder_.Diagnostics())
                << "expected a pipeline-overridable constant";
          }
          out << kSpecConstantPrefix << global->ConstantId();
        } else {
          out << std::to_string(wgsize[i].value);
        }
      }
      out << ")]" << std::endl;
    }

    out << func->return_type()->FriendlyName(builder_.Symbols());

    out << " " << builder_.Symbols().NameFor(func->symbol()) << "(";

    bool first = true;

    // Emit entry point parameters.
    for (auto* var : func->params()) {
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
                           builder_.Symbols().NameFor(var->symbol()))) {
        return false;
      }
    }

    out << ") {";
  }

  {
    ScopedIndent si(this);

    if (!EmitStatements(func->body()->statements())) {
      return false;
    }

    if (!Is<ast::ReturnStatement>(func->get_last_statement())) {
      ast::ReturnStatement ret(ProgramID(), Source{});
      if (!EmitStatement(&ret)) {
        return false;
      }
    }
  }

  line() << "}";

  auto out = line();

  // Declare entry point input variables
  for (auto* var : func->params()) {
    auto* sem = builder_.Sem().Get(var);
    auto* str = sem->Type()->As<sem::Struct>();
    for (auto* member : str->Members()) {
      if (ast::HasDecoration<ast::BuiltinDecoration>(
              member->Declaration()->decorations())) {
        continue;
      }
      if (!EmitTypeAndName(
              out, member->Type(), ast::StorageClass::kInput,
              ast::Access::kReadWrite,
              builder_.Symbols().NameFor(member->Declaration()->symbol()))) {
        return false;
      }
      out << ";" << std::endl;
    }
  }

  // Declare entry point output variables
  auto* return_type = func_sem->ReturnType()->As<sem::Struct>();
  if (return_type) {
    for (auto* member : return_type->Members()) {
      if (ast::HasDecoration<ast::BuiltinDecoration>(
              member->Declaration()->decorations())) {
        continue;
      }
      if (!EmitTypeAndName(
              out, member->Type(), ast::StorageClass::kOutput,
              ast::Access::kReadWrite,
              builder_.Symbols().NameFor(member->Declaration()->symbol()))) {
        return false;
      }
      out << ";" << std::endl;
    }
  }

  // Create a main() function which calls the entry point.
  out << "void main() {" << std::endl;
  std::string printed_name;
  for (auto* var : func->params()) {
    out << "  ";
    auto* sem = builder_.Sem().Get(var);
    if (!EmitTypeAndName(out, sem->Type(), sem->StorageClass(), sem->Access(),
                         "inputs")) {
      return false;
    }
    out << ";" << std::endl;
    auto* type = sem->Type();
    auto* str = type->As<sem::Struct>();
    for (auto* member : str->Members()) {
      std::string name =
          builder_.Symbols().NameFor(member->Declaration()->symbol());
      out << "  inputs." << name << " = ";
      if (auto* builtin = ast::GetDecoration<ast::BuiltinDecoration>(
              member->Declaration()->decorations())) {
        if (builtin_type(builtin->value()) != member->Type()) {
          if (!EmitType(out, member->Type(), ast::StorageClass::kNone,
                        ast::Access::kReadWrite, "")) {
            return false;
          }
          out << "(";
          out << builtin_to_string(builtin->value());
          out << ")";
        } else {
          out << builtin_to_string(builtin->value());
        }
      } else {
        out << name;
      }
      out << ";" << std::endl;
    }
  }
  out << "  ";
  if (return_type) {
    out << return_type->FriendlyName(builder_.Symbols()) << " "
        << "outputs;" << std::endl;
    out << "  outputs = ";
  }
  out << builder_.Symbols().NameFor(func->symbol());
  if (func->params().empty()) {
    out << "()";
  } else {
    out << "(inputs)";
  }
  out << ";" << std::endl;

  auto* str = func_sem->ReturnType()->As<sem::Struct>();
  if (str) {
    for (auto* member : str->Members()) {
      std::string name =
          builder_.Symbols().NameFor(member->Declaration()->symbol());
      out << "  ";
      if (auto* builtin = ast::GetDecoration<ast::BuiltinDecoration>(
              member->Declaration()->decorations())) {
        out << builtin_to_string(builtin->value());
      } else {
        out << name;
      }
      out << " = outputs." << name << ";" << std::endl;
    }
  }

  out << "}" << std::endl << std::endl;

  return true;
}

bool GeneratorImpl::EmitLiteral(std::ostream& out, ast::Literal* lit) {
  if (auto* l = lit->As<ast::BoolLiteral>()) {
    out << (l->IsTrue() ? "true" : "false");
  } else if (auto* fl = lit->As<ast::FloatLiteral>()) {
    if (std::isinf(fl->value())) {
      out << (fl->value() >= 0 ? "asfloat(0x7f800000u)"
                               : "asfloat(0xff800000u)");
    } else if (std::isnan(fl->value())) {
      out << "asfloat(0x7fc00000u)";
    } else {
      out << FloatToString(fl->value()) << "f";
    }
  } else if (auto* sl = lit->As<ast::SintLiteral>()) {
    out << sl->value();
  } else if (auto* ul = lit->As<ast::UintLiteral>()) {
    out << ul->value() << "u";
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
    diagnostics_.add_error(
        diag::System::Writer,
        "Invalid type for zero emission: " + type->type_name());
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitLoop(ast::LoopStatement* stmt) {
  auto emit_continuing = [this, stmt]() {
    if (stmt->has_continuing()) {
      if (!EmitBlock(stmt->continuing())) {
        return false;
      }
    }
    return true;
  };

  TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);
  line() << "while (true) {";
  {
    ScopedIndent si(this);
    if (!EmitStatements(stmt->body()->statements())) {
      return false;
    }
    if (!emit_continuing()) {
      return false;
    }
  }
  line() << "}";

  return true;
}

bool GeneratorImpl::EmitForLoop(ast::ForLoopStatement* stmt) {
  // Nest a for loop with a new block. In HLSL the initializer scope is not
  // nested by the for-loop, so we may get variable redefinitions.
  line() << "{";
  increment_indent();
  TINT_DEFER({
    decrement_indent();
    line() << "}";
  });

  TextBuffer init_buf;
  if (auto* init = stmt->initializer()) {
    TINT_SCOPED_ASSIGNMENT(current_buffer_, &init_buf);
    if (!EmitStatement(init)) {
      return false;
    }
  }

  TextBuffer cond_pre;
  std::stringstream cond_buf;
  if (auto* cond = stmt->condition()) {
    TINT_SCOPED_ASSIGNMENT(current_buffer_, &cond_pre);
    if (!EmitExpression(cond_buf, cond)) {
      return false;
    }
  }

  TextBuffer cont_buf;
  if (auto* cont = stmt->continuing()) {
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
  if (init_buf.lines.size() > 1 || (stmt->initializer() && emit_as_loop)) {
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

    if (stmt->condition()) {
      current_buffer_->Append(cond_pre);
      line() << "if (!(" << cond_buf.str() << ")) { break; }";
    }

    if (!EmitStatements(stmt->body()->statements())) {
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
      if (!EmitStatementsWithIndent(stmt->body()->statements())) {
        return false;
      }
    }
    line() << "}";
  }

  return true;
}

bool GeneratorImpl::EmitMemberAccessor(std::ostream& out,
                                       ast::MemberAccessorExpression* expr) {
  if (!EmitExpression(out, expr->structure())) {
    return false;
  }
  out << ".";

  // Swizzles output the name directly
  if (builder_.Sem().Get(expr)->Is<sem::Swizzle>()) {
    out << builder_.Symbols().NameFor(expr->member()->symbol());
  } else if (!EmitExpression(out, expr->member())) {
    return false;
  }

  return true;
}

bool GeneratorImpl::EmitReturn(ast::ReturnStatement* stmt) {
  if (stmt->has_value()) {
    auto out = line();
    out << "return ";
    if (!EmitExpression(out, stmt->value())) {
      return false;
    }
    out << ";";
  } else {
    line() << "return;";
  }
  return true;
}

bool GeneratorImpl::EmitStatement(ast::Statement* stmt) {
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
    if (!TypeOf(c->expr())->Is<sem::Void>()) {
      out << "(void) ";
    }
    if (!EmitCall(out, c->expr())) {
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
    return EmitVariable(v->variable());
  }

  diagnostics_.add_error(diag::System::Writer,
                         "unknown statement type: " + builder_.str(stmt));
  return false;
}

bool GeneratorImpl::EmitSwitch(ast::SwitchStatement* stmt) {
  {  // switch(expr) {
    auto out = line();
    out << "switch(";
    if (!EmitExpression(out, stmt->condition())) {
      return false;
    }
    out << ") {";
  }

  {
    ScopedIndent si(this);
    for (auto* s : stmt->body()) {
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
    case ast::StorageClass::kUniform: {
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
      if (arr->IsRuntimeSized()) {
        TINT_ICE(Writer, diagnostics_)
            << "Runtime arrays may only exist in storage buffers, which should "
               "have been transformed into a ByteAddressBuffer";
        return false;
      }
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
      out << "[" << size << "]";
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
  } else if (auto* sampler = type->As<sem::Sampler>()) {
    out << "Sampler";
    if (sampler->IsComparison()) {
      out << "Comparison";
    }
    out << "State";
  } else if (auto* str = type->As<sem::Struct>()) {
    out << StructName(str);
  } else if (auto* tex = type->As<sem::Texture>()) {
    auto* storage = tex->As<sem::StorageTexture>();
    auto* ms = tex->As<sem::MultisampledTexture>();
    auto* depth_ms = tex->As<sem::DepthMultisampledTexture>();
    auto* sampled = tex->As<sem::SampledTexture>();

    if (storage && storage->access() != ast::Access::kRead) {
      out << "RW";
    }
    out << "Texture";

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

    if (storage) {
      auto* component = image_format_to_rwtexture_type(storage->image_format());
      if (component == nullptr) {
        TINT_ICE(Writer, diagnostics_)
            << "Unsupported StorageTexture ImageFormat: "
            << static_cast<int>(storage->image_format());
        return false;
      }
      out << "<" << component << ">";
    } else if (depth_ms) {
      out << "<float4>";
    } else if (sampled || ms) {
      auto* subtype = sampled ? sampled->type() : ms->type();
      out << "<";
      if (subtype->Is<sem::F32>()) {
        out << "float4";
      } else if (subtype->Is<sem::I32>()) {
        out << "int4";
      } else if (subtype->Is<sem::U32>()) {
        out << "uint4";
      } else {
        TINT_ICE(Writer, diagnostics_)
            << "Unsupported multisampled texture type";
        return false;
      }
      out << ">";
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
  if (storage_class_uses.size() ==
      (storage_class_uses.count(ast::StorageClass::kStorage))) {
    // The only use of the structure is as a storage buffer.
    // Structures used as storage buffer are read and written to via a
    // ByteAddressBuffer instead of true structure.
    return true;
  }

  line(b) << "struct " << StructName(str) << " {";
  {
    ScopedIndent si(b);
    for (auto* mem : str->Members()) {
      auto name = builder_.Symbols().NameFor(mem->Name());

      auto* ty = mem->Type();

      auto out = line(b);

      std::string pre, post;

      if (auto* decl = mem->Declaration()) {
        for (auto* deco : decl->decorations()) {
          if (deco->As<ast::LocationDecoration>()) {
            auto& pipeline_stage_uses = str->PipelineStageUses();
            if (pipeline_stage_uses.size() != 1) {
              TINT_ICE(Writer, diagnostics_)
                  << "invalid entry point IO struct uses";
            }
          } else if (auto* interpolate =
                         deco->As<ast::InterpolateDecoration>()) {
            auto mod = interpolation_to_modifiers(interpolate->type(),
                                                  interpolate->sampling());
            if (mod.empty()) {
              diagnostics_.add_error(diag::System::Writer,
                                     "unsupported interpolation");
              return false;
            }
          }
        }
      }

      out << pre;
      if (!EmitTypeAndName(out, ty, ast::StorageClass::kNone,
                           ast::Access::kReadWrite, name)) {
        return false;
      }
      out << post << ";";
    }
  }

  line(b) << "};";

  return true;
}

bool GeneratorImpl::EmitUnaryOp(std::ostream& out,
                                ast::UnaryOpExpression* expr) {
  switch (expr->op()) {
    case ast::UnaryOp::kIndirection:
    case ast::UnaryOp::kAddressOf:
      return EmitExpression(out, expr->expr());
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

  if (!EmitExpression(out, expr->expr())) {
    return false;
  }

  out << ")";

  return true;
}

bool GeneratorImpl::EmitVariable(ast::Variable* var) {
  auto* sem = builder_.Sem().Get(var);
  auto* type = sem->Type()->UnwrapRef();

  // TODO(dsinclair): Handle variable decorations
  if (!var->decorations().empty()) {
    diagnostics_.add_error(diag::System::Writer,
                           "Variable decorations are not handled yet");
    return false;
  }

  auto out = line();
  // TODO(senorblanco): handle const
  if (!EmitTypeAndName(out, type, sem->StorageClass(), sem->Access(),
                       builder_.Symbols().NameFor(var->symbol()))) {
    return false;
  }

  out << " = ";

  if (var->constructor()) {
    if (!EmitExpression(out, var->constructor())) {
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
  for (auto* d : var->decorations()) {
    if (!d->Is<ast::OverrideDecoration>()) {
      diagnostics_.add_error(diag::System::Writer,
                             "Decorated const values not valid");
      return false;
    }
  }
  if (!var->is_const()) {
    diagnostics_.add_error(diag::System::Writer, "Expected a const value");
    return false;
  }

  auto* sem = builder_.Sem().Get(var);
  auto* type = sem->Type();

  auto* global = sem->As<sem::GlobalVariable>();
  if (global && global->IsPipelineConstant()) {
    auto const_id = global->ConstantId();

    line() << "#ifndef " << kSpecConstantPrefix << const_id;

    if (var->constructor() != nullptr) {
      auto out = line();
      out << "#define " << kSpecConstantPrefix << const_id << " ";
      if (!EmitExpression(out, var->constructor())) {
        return false;
      }
    } else {
      line() << "#error spec constant required for constant id " << const_id;
    }
    line() << "#endif";
    {
      auto out = line();
      out << "static const ";
      if (!EmitTypeAndName(out, type, sem->StorageClass(), sem->Access(),
                           builder_.Symbols().NameFor(var->symbol()))) {
        return false;
      }
      out << " = " << kSpecConstantPrefix << const_id << ";";
    }
  } else {
    auto out = line();
    out << "static const ";
    if (!EmitTypeAndName(out, type, sem->StorageClass(), sem->Access(),
                         builder_.Symbols().NameFor(var->symbol()))) {
      return false;
    }
    out << " = ";
    if (!EmitExpression(out, var->constructor())) {
      return false;
    }
    out << ";";
  }

  return true;
}

template <typename F>
bool GeneratorImpl::CallIntrinsicHelper(std::ostream& out,
                                        ast::CallExpression* call,
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
      if (!EmitTypeAndName(decl, intrinsic->ReturnType(),
                           ast::StorageClass::kNone, ast::Access::kUndefined,
                           fn_name)) {
        return "";
      }
      {
        ScopedParen sp(decl);
        for (auto* param : intrinsic->Parameters()) {
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
    for (auto* arg : call->params()) {
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

}  // namespace glsl
}  // namespace writer
}  // namespace tint
