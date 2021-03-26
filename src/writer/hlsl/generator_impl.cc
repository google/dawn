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

#include "src/writer/hlsl/generator_impl.h"

#include <utility>
#include <vector>

#include "src/ast/call_statement.h"
#include "src/ast/constant_id_decoration.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/variable_decl_statement.h"
#include "src/semantic/array.h"
#include "src/semantic/call.h"
#include "src/semantic/function.h"
#include "src/semantic/member_accessor_expression.h"
#include "src/semantic/struct.h"
#include "src/semantic/variable.h"
#include "src/type/access_control_type.h"
#include "src/type/multisampled_texture_type.h"
#include "src/type/storage_texture_type.h"
#include "src/writer/append_vector.h"
#include "src/writer/float_to_string.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

const char kInStructNameSuffix[] = "in";
const char kOutStructNameSuffix[] = "out";
const char kTintStructInVarPrefix[] = "tint_in";
const char kTintStructOutVarPrefix[] = "tint_out";
const char kTempNamePrefix[] = "_tint_tmp";

bool last_is_break_or_fallthrough(const ast::BlockStatement* stmts) {
  if (stmts->empty()) {
    return false;
  }

  return stmts->last()->Is<ast::BreakStatement>() ||
         stmts->last()->Is<ast::FallthroughStatement>();
}

const char* image_format_to_rwtexture_type(type::ImageFormat image_format) {
  switch (image_format) {
    case type::ImageFormat::kRgba8Unorm:
    case type::ImageFormat::kRgba8Snorm:
    case type::ImageFormat::kRgba16Float:
    case type::ImageFormat::kR32Float:
    case type::ImageFormat::kRg32Float:
    case type::ImageFormat::kRgba32Float:
      return "float4";
    case type::ImageFormat::kRgba8Uint:
    case type::ImageFormat::kRgba16Uint:
    case type::ImageFormat::kR32Uint:
    case type::ImageFormat::kRg32Uint:
    case type::ImageFormat::kRgba32Uint:
      return "uint4";
    case type::ImageFormat::kRgba8Sint:
    case type::ImageFormat::kRgba16Sint:
    case type::ImageFormat::kR32Sint:
    case type::ImageFormat::kRg32Sint:
    case type::ImageFormat::kRgba32Sint:
      return "int4";
    default:
      return nullptr;
  }
}

}  // namespace

GeneratorImpl::GeneratorImpl(const Program* program)
    : builder_(ProgramBuilder::Wrap(program)) {}

GeneratorImpl::~GeneratorImpl() = default;

bool GeneratorImpl::Generate(std::ostream& out) {
  for (auto* global : builder_.AST().GlobalVariables()) {
    register_global(global);
  }

  for (auto* const ty : builder_.AST().ConstructedTypes()) {
    if (!EmitConstructedType(out, ty)) {
      return false;
    }
  }
  if (!builder_.AST().ConstructedTypes().empty()) {
    out << std::endl;
  }

  for (auto* var : builder_.AST().GlobalVariables()) {
    if (!var->is_const()) {
      continue;
    }
    if (!EmitProgramConstVariable(out, var)) {
      return false;
    }
  }

  // emitted_globals is a set used to ensure that globals are emitted once even
  // if they are used by multiple entry points.
  std::unordered_set<Symbol> emitted_globals;

  // Make sure all entry point data is emitted before the entry point functions
  for (auto* func : builder_.AST().Functions()) {
    if (!func->IsEntryPoint()) {
      continue;
    }

    if (!EmitEntryPointData(out, func, emitted_globals)) {
      return false;
    }
  }

  for (auto* func : builder_.AST().Functions()) {
    if (!EmitFunction(out, func)) {
      return false;
    }
  }

  for (auto* func : builder_.AST().Functions()) {
    if (!func->IsEntryPoint()) {
      continue;
    }
    if (!EmitEntryPointFunction(out, func)) {
      return false;
    }
    out << std::endl;
  }
  return true;
}

void GeneratorImpl::register_global(ast::Variable* global) {
  auto* sem = builder_.Sem().Get(global);
  global_variables_.set(global->symbol(), sem);
}

std::string GeneratorImpl::generate_name(const std::string& prefix) {
  std::string name = prefix;
  uint32_t i = 0;
  while (namer_.IsMapped(name) || namer_.IsRemapped(name)) {
    name = prefix + "_" + std::to_string(i);
    ++i;
  }
  namer_.RegisterRemappedName(name);
  return name;
}

std::string GeneratorImpl::current_ep_var_name(VarType type) {
  std::string name = "";
  switch (type) {
    case VarType::kIn: {
      auto in_it = ep_sym_to_in_data_.find(current_ep_sym_);
      if (in_it != ep_sym_to_in_data_.end()) {
        name = in_it->second.var_name;
      }
      break;
    }
    case VarType::kOut: {
      auto outit = ep_sym_to_out_data_.find(current_ep_sym_);
      if (outit != ep_sym_to_out_data_.end()) {
        name = outit->second.var_name;
      }
      break;
    }
  }
  return name;
}

bool GeneratorImpl::EmitConstructedType(std::ostream& out,
                                        const type::Type* ty) {
  make_indent(out);

  if (auto* alias = ty->As<type::Alias>()) {
    // HLSL typedef is for intrinsic types only. For an alias'd struct,
    // generate a secondary struct with the new name.
    if (auto* str = alias->type()->As<type::Struct>()) {
      if (!EmitStructType(out, str,
                          builder_.Symbols().NameFor(alias->symbol()))) {
        return false;
      }
      return true;
    }
    out << "typedef ";
    if (!EmitType(out, alias->type(), "")) {
      return false;
    }
    out << " " << namer_.NameFor(builder_.Symbols().NameFor(alias->symbol()))
        << ";" << std::endl;
  } else if (auto* str = ty->As<type::Struct>()) {
    if (!EmitStructType(out, str, builder_.Symbols().NameFor(str->symbol()))) {
      return false;
    }
  } else {
    diagnostics_.add_error("unknown constructed type: " + ty->type_name());
    return false;
  }

  return true;
}

bool GeneratorImpl::EmitArrayAccessor(std::ostream& pre,
                                      std::ostream& out,
                                      ast::ArrayAccessorExpression* expr) {
  // Handle writing into a storage buffer array
  if (is_storage_buffer_access(expr)) {
    return EmitStorageBufferAccessor(pre, out, expr, nullptr);
  }

  if (!EmitExpression(pre, out, expr->array())) {
    return false;
  }
  out << "[";

  if (!EmitExpression(pre, out, expr->idx_expr())) {
    return false;
  }
  out << "]";

  return true;
}

bool GeneratorImpl::EmitBitcast(std::ostream& pre,
                                std::ostream& out,
                                ast::BitcastExpression* expr) {
  if (!expr->type()->is_integer_scalar() && !expr->type()->is_float_scalar()) {
    diagnostics_.add_error("Unable to do bitcast to type " +
                           expr->type()->type_name());
    return false;
  }

  out << "as";
  if (!EmitType(out, expr->type(), "")) {
    return false;
  }
  out << "(";
  if (!EmitExpression(pre, out, expr->expr())) {
    return false;
  }
  out << ")";
  return true;
}

bool GeneratorImpl::EmitAssign(std::ostream& out,
                               ast::AssignmentStatement* stmt) {
  make_indent(out);

  std::ostringstream pre;

  // If the LHS is an accessor into a storage buffer then we have to
  // emit a Store operation instead of an ='s.
  if (auto* mem = stmt->lhs()->As<ast::MemberAccessorExpression>()) {
    if (is_storage_buffer_access(mem)) {
      std::ostringstream accessor_out;
      if (!EmitStorageBufferAccessor(pre, accessor_out, mem, stmt->rhs())) {
        return false;
      }
      out << pre.str();
      out << accessor_out.str() << ";" << std::endl;
      return true;
    }
  } else if (auto* ary = stmt->lhs()->As<ast::ArrayAccessorExpression>()) {
    if (is_storage_buffer_access(ary)) {
      std::ostringstream accessor_out;
      if (!EmitStorageBufferAccessor(pre, accessor_out, ary, stmt->rhs())) {
        return false;
      }
      out << pre.str();
      out << accessor_out.str() << ";" << std::endl;
      return true;
    }
  }

  std::ostringstream lhs_out;
  if (!EmitExpression(pre, lhs_out, stmt->lhs())) {
    return false;
  }
  std::ostringstream rhs_out;
  if (!EmitExpression(pre, rhs_out, stmt->rhs())) {
    return false;
  }

  out << pre.str();
  out << lhs_out.str() << " = " << rhs_out.str() << ";" << std::endl;

  return true;
}

bool GeneratorImpl::EmitBinary(std::ostream& pre,
                               std::ostream& out,
                               ast::BinaryExpression* expr) {
  if (expr->op() == ast::BinaryOp::kLogicalAnd ||
      expr->op() == ast::BinaryOp::kLogicalOr) {
    std::ostringstream lhs_out;
    if (!EmitExpression(pre, lhs_out, expr->lhs())) {
      return false;
    }

    auto name = generate_name(kTempNamePrefix);
    make_indent(pre);
    pre << "bool " << name << " = " << lhs_out.str() << ";" << std::endl;

    make_indent(pre);
    pre << "if (";
    if (expr->op() == ast::BinaryOp::kLogicalOr) {
      pre << "!";
    }
    pre << name << ") {" << std::endl;
    increment_indent();

    std::ostringstream rhs_out;
    if (!EmitExpression(pre, rhs_out, expr->rhs())) {
      return false;
    }

    make_indent(pre);
    pre << name << " = " << rhs_out.str() << ";" << std::endl;

    decrement_indent();
    make_indent(pre);
    pre << "}" << std::endl;

    out << "(" << name << ")";
    return true;
  }

  auto* lhs_type = TypeOf(expr->lhs())->UnwrapAll();
  auto* rhs_type = TypeOf(expr->rhs())->UnwrapAll();
  // Multiplying by a matrix requires the use of `mul` in order to get the
  // type of multiply we desire.
  if (expr->op() == ast::BinaryOp::kMultiply &&
      ((lhs_type->Is<type::Vector>() && rhs_type->Is<type::Matrix>()) ||
       (lhs_type->Is<type::Matrix>() && rhs_type->Is<type::Vector>()) ||
       (lhs_type->Is<type::Matrix>() && rhs_type->Is<type::Matrix>()))) {
    out << "mul(";
    if (!EmitExpression(pre, out, expr->lhs())) {
      return false;
    }
    out << ", ";
    if (!EmitExpression(pre, out, expr->rhs())) {
      return false;
    }
    out << ")";

    return true;
  }

  out << "(";
  if (!EmitExpression(pre, out, expr->lhs())) {
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
      TINT_UNREACHABLE(diagnostics_);
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
      diagnostics_.add_error("missing binary operation type");
      return false;
  }
  out << " ";

  if (!EmitExpression(pre, out, expr->rhs())) {
    return false;
  }

  out << ")";
  return true;
}

bool GeneratorImpl::EmitBlock(std::ostream& out,
                              const ast::BlockStatement* stmt) {
  out << "{" << std::endl;
  increment_indent();

  for (auto* s : *stmt) {
    if (!EmitStatement(out, s)) {
      return false;
    }
  }

  decrement_indent();
  make_indent(out);
  out << "}";

  return true;
}

bool GeneratorImpl::EmitBlockAndNewline(std::ostream& out,
                                        const ast::BlockStatement* stmt) {
  const bool result = EmitBlock(out, stmt);
  if (result) {
    out << std::endl;
  }
  return result;
}

bool GeneratorImpl::EmitIndentedBlockAndNewline(std::ostream& out,
                                                ast::BlockStatement* stmt) {
  make_indent(out);
  const bool result = EmitBlock(out, stmt);
  if (result) {
    out << std::endl;
  }
  return result;
}

bool GeneratorImpl::EmitBreak(std::ostream& out, ast::BreakStatement*) {
  make_indent(out);
  out << "break;" << std::endl;
  return true;
}

bool GeneratorImpl::EmitCall(std::ostream& pre,
                             std::ostream& out,
                             ast::CallExpression* expr) {
  auto* ident = expr->func()->As<ast::IdentifierExpression>();
  if (ident == nullptr) {
    diagnostics_.add_error("invalid function name");
    return 0;
  }

  auto* call = builder_.Sem().Get(expr);
  if (auto* intrinsic = call->Target()->As<semantic::Intrinsic>()) {
    if (intrinsic->IsTexture()) {
      return EmitTextureCall(pre, out, expr, intrinsic);
    }
    const auto& params = expr->params();
    if (intrinsic->Type() == semantic::IntrinsicType::kSelect) {
      diagnostics_.add_error("select not supported in HLSL backend yet");
      return false;
    } else if (intrinsic->Type() == semantic::IntrinsicType::kIsNormal) {
      diagnostics_.add_error("is_normal not supported in HLSL backend yet");
      return false;
    } else if (intrinsic->IsDataPacking()) {
      return EmitDataPackingCall(pre, out, expr, intrinsic);
    } else if (intrinsic->IsDataUnpacking()) {
      return EmitDataUnpackingCall(pre, out, expr, intrinsic);
    } else if (intrinsic->IsBarrier()) {
      return EmitBarrierCall(pre, out, intrinsic);
    }
    auto name = generate_builtin_name(intrinsic);
    if (name.empty()) {
      return false;
    }

    make_indent(out);
    out << name << "(";

    bool first = true;
    for (auto* param : params) {
      if (!first) {
        out << ", ";
      }
      first = false;

      if (!EmitExpression(pre, out, param)) {
        return false;
      }
    }

    out << ")";
    return true;
  }

  auto name = builder_.Symbols().NameFor(ident->symbol());
  auto caller_sym = ident->symbol();
  auto it = ep_func_name_remapped_.find(current_ep_sym_.to_str() + "_" +
                                        caller_sym.to_str());
  if (it != ep_func_name_remapped_.end()) {
    name = it->second;
  }

  auto* func = builder_.AST().Functions().Find(ident->symbol());
  if (func == nullptr) {
    diagnostics_.add_error("Unable to find function: " +
                           builder_.Symbols().NameFor(ident->symbol()));
    return false;
  }

  out << name << "(";

  auto* func_sem = builder_.Sem().Get(func);

  bool first = true;
  if (has_referenced_in_var_needing_struct(func_sem)) {
    auto var_name = current_ep_var_name(VarType::kIn);
    if (!var_name.empty()) {
      out << var_name;
      first = false;
    }
  }
  if (has_referenced_out_var_needing_struct(func_sem)) {
    auto var_name = current_ep_var_name(VarType::kOut);
    if (!var_name.empty()) {
      if (!first) {
        out << ", ";
      }
      first = false;
      out << var_name;
    }
  }

  const auto& params = expr->params();
  for (auto* param : params) {
    if (!first) {
      out << ", ";
    }
    first = false;

    if (!EmitExpression(pre, out, param)) {
      return false;
    }
  }

  out << ")";

  return true;
}

bool GeneratorImpl::EmitDataPackingCall(std::ostream& pre,
                                        std::ostream& out,
                                        ast::CallExpression* expr,
                                        const semantic::Intrinsic* intrinsic) {
  auto* param = expr->params()[0];
  auto tmp_name = generate_name(kTempNamePrefix);
  std::ostringstream expr_out;
  if (!EmitExpression(pre, expr_out, param)) {
    return false;
  }
  uint32_t dims = 2;
  bool is_signed = false;
  uint32_t scale = 65535;
  if (intrinsic->Type() == semantic::IntrinsicType::kPack4x8Snorm ||
      intrinsic->Type() == semantic::IntrinsicType::kPack4x8Unorm) {
    dims = 4;
    scale = 255;
  }
  if (intrinsic->Type() == semantic::IntrinsicType::kPack4x8Snorm ||
      intrinsic->Type() == semantic::IntrinsicType::kPack2x16Snorm) {
    is_signed = true;
    scale = (scale - 1) / 2;
  }
  switch (intrinsic->Type()) {
    case semantic::IntrinsicType::kPack4x8Snorm:
    case semantic::IntrinsicType::kPack4x8Unorm:
    case semantic::IntrinsicType::kPack2x16Snorm:
    case semantic::IntrinsicType::kPack2x16Unorm:
      pre << (is_signed ? "" : "u") << "int" << dims << " " << tmp_name << " = "
          << (is_signed ? "" : "u") << "int" << dims << "(round(clamp("
          << expr_out.str() << ", " << (is_signed ? "-1.0" : "0.0")
          << ", 1.0) * " << scale << ".0))";
      if (is_signed) {
        pre << " & " << (dims == 4 ? "0xff" : "0xffff");
      }
      pre << ";\n";
      if (is_signed) {
        out << "asuint";
      }
      out << "(";
      out << tmp_name << ".x | " << tmp_name << ".y << " << (32 / dims);
      if (dims == 4) {
        out << " | " << tmp_name << ".z << 16 | " << tmp_name << ".w << 24";
      }
      out << ")";
      break;
    case semantic::IntrinsicType::kPack2x16Float:
      pre << "uint2 " << tmp_name << " = f32tof16(" << expr_out.str() << ");\n";
      out << "(" << tmp_name << ".x | " << tmp_name << ".y << 16)";
      break;
    default:
      diagnostics_.add_error(
          "Internal error: unhandled data packing intrinsic");
      return false;
  }

  return true;
}

bool GeneratorImpl::EmitDataUnpackingCall(
    std::ostream& pre,
    std::ostream& out,
    ast::CallExpression* expr,
    const semantic::Intrinsic* intrinsic) {
  auto* param = expr->params()[0];
  auto tmp_name = generate_name(kTempNamePrefix);
  std::ostringstream expr_out;
  if (!EmitExpression(pre, expr_out, param)) {
    return false;
  }
  uint32_t dims = 2;
  bool is_signed = false;
  uint32_t scale = 65535;
  if (intrinsic->Type() == semantic::IntrinsicType::kUnpack4x8Snorm ||
      intrinsic->Type() == semantic::IntrinsicType::kUnpack4x8Unorm) {
    dims = 4;
    scale = 255;
  }
  if (intrinsic->Type() == semantic::IntrinsicType::kUnpack4x8Snorm ||
      intrinsic->Type() == semantic::IntrinsicType::kUnpack2x16Snorm) {
    is_signed = true;
    scale = (scale - 1) / 2;
  }
  switch (intrinsic->Type()) {
    case semantic::IntrinsicType::kUnpack4x8Snorm:
    case semantic::IntrinsicType::kUnpack2x16Snorm: {
      auto tmp_name2 = generate_name(kTempNamePrefix);
      pre << "int " << tmp_name2 << " = int(" << expr_out.str() << ");\n";
      // Perform sign extension on the converted values.
      pre << "int" << dims << " " << tmp_name << " = int" << dims << "(";
      if (dims == 2) {
        pre << tmp_name2 << " << 16, " << tmp_name2 << ") >> 16";
      } else {
        pre << tmp_name2 << " << 24, " << tmp_name2 << " << 16, " << tmp_name2
            << " << 8, " << tmp_name2 << ") >> 24";
      }
      pre << ";\n";
      out << "clamp(float" << dims << "(" << tmp_name << ") / " << scale
          << ".0, " << (is_signed ? "-1.0" : "0.0") << ", 1.0)";
      break;
    }
    case semantic::IntrinsicType::kUnpack4x8Unorm:
    case semantic::IntrinsicType::kUnpack2x16Unorm: {
      auto tmp_name2 = generate_name(kTempNamePrefix);
      pre << "uint " << tmp_name2 << " = " << expr_out.str() << ";\n";
      pre << "uint" << dims << " " << tmp_name << " = uint" << dims << "(";
      pre << tmp_name2 << " & " << (dims == 2 ? "0xffff" : "0xff") << ", ";
      if (dims == 4) {
        pre << "(" << tmp_name2 << " >> " << (32 / dims) << ") & 0xff, ("
            << tmp_name2 << " >> 16) & 0xff, " << tmp_name2 << " >> 24";
      } else {
        pre << tmp_name2 << " >> " << (32 / dims);
      }
      pre << ");\n";
      out << "float" << dims << "(" << tmp_name << ") / " << scale << ".0";
      break;
    }
    case semantic::IntrinsicType::kUnpack2x16Float:
      pre << "uint " << tmp_name << " = " << expr_out.str() << ";\n";
      out << "f16tof32(uint2(" << tmp_name << " & 0xffff, " << tmp_name
          << " >> 16))";
      break;
    default:
      diagnostics_.add_error(
          "Internal error: unhandled data packing intrinsic");
      return false;
  }

  return true;
}

bool GeneratorImpl::EmitBarrierCall(std::ostream&,
                                    std::ostream& out,
                                    const semantic::Intrinsic* intrinsic) {
  // TODO(crbug.com/tint/661): Combine sequential barriers to a single
  // instruction.
  if (intrinsic->Type() == semantic::IntrinsicType::kWorkgroupBarrier) {
    out << "GroupMemoryBarrierWithGroupSync()";
  } else if (intrinsic->Type() == semantic::IntrinsicType::kStorageBarrier) {
    out << "DeviceMemoryBarrierWithGroupSync()";
  } else {
    TINT_UNREACHABLE(diagnostics_) << "unexpected barrier intrinsic type "
                                   << semantic::str(intrinsic->Type());
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitTextureCall(std::ostream& pre,
                                    std::ostream& out,
                                    ast::CallExpression* expr,
                                    const semantic::Intrinsic* intrinsic) {
  using Usage = semantic::Parameter::Usage;

  auto parameters = intrinsic->Parameters();
  auto arguments = expr->params();

  // Returns the argument with the given usage
  auto arg = [&](Usage usage) {
    int idx = semantic::IndexOf(parameters, usage);
    return (idx >= 0) ? arguments[idx] : nullptr;
  };

  auto* texture = arg(Usage::kTexture);
  assert(texture);

  auto* texture_type = TypeOf(texture)->UnwrapAll()->As<type::Texture>();

  switch (intrinsic->Type()) {
    case semantic::IntrinsicType::kTextureDimensions:
    case semantic::IntrinsicType::kTextureNumLayers:
    case semantic::IntrinsicType::kTextureNumLevels:
    case semantic::IntrinsicType::kTextureNumSamples: {
      // All of these intrinsics use the GetDimensions() method on the texture
      bool is_ms = texture_type->Is<type::MultisampledTexture>();
      int num_dimensions = 0;
      std::string swizzle;

      switch (intrinsic->Type()) {
        case semantic::IntrinsicType::kTextureDimensions:
          switch (texture_type->dim()) {
            case type::TextureDimension::kNone:
              TINT_ICE(diagnostics_) << "texture dimension is kNone";
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
              // width == height == depth for cubes
              // See https://github.com/gpuweb/gpuweb/issues/1345
              num_dimensions = 2;
              swizzle = ".xyy";  // [width, height, height]
              break;
            case type::TextureDimension::kCubeArray:
              // width == height == depth for cubes
              // See https://github.com/gpuweb/gpuweb/issues/1345
              num_dimensions = 3;
              swizzle = ".xyy";  // [width, height, height]
              break;
          }
          break;
        case semantic::IntrinsicType::kTextureNumLayers:
          switch (texture_type->dim()) {
            default:
              TINT_ICE(diagnostics_) << "texture dimension is not arrayed";
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
        case semantic::IntrinsicType::kTextureNumLevels:
          switch (texture_type->dim()) {
            default:
              TINT_ICE(diagnostics_)
                  << "texture dimension does not support mips";
              return false;
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
        case semantic::IntrinsicType::kTextureNumSamples:
          switch (texture_type->dim()) {
            default:
              TINT_ICE(diagnostics_)
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
          TINT_ICE(diagnostics_) << "unexpected intrinsic";
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

      if (num_dimensions > 4) {
        TINT_ICE(diagnostics_)
            << "Texture query intrinsic temporary vector has " << num_dimensions
            << " dimensions";
        return false;
      }

      // Declare a variable to hold the queried texture info
      auto dims = generate_name(kTempNamePrefix);

      if (num_dimensions == 1) {
        pre << "int " << dims << ";";
      } else {
        pre << "int" << num_dimensions << " " << dims << ";";
      }

      pre << std::endl;
      make_indent(pre);

      if (!EmitExpression(pre, pre, texture)) {
        return false;
      }
      pre << ".GetDimensions(";

      if (level_arg) {
        if (!EmitExpression(pre, pre, level_arg)) {
          return false;
        }
        pre << ", ";
      } else if (intrinsic->Type() ==
                 semantic::IntrinsicType::kTextureNumLevels) {
        pre << "0, ";
      }

      if (num_dimensions == 1) {
        pre << dims;
      } else {
        static constexpr char xyzw[] = {'x', 'y', 'z', 'w'};
        assert(num_dimensions > 0);
        assert(num_dimensions <= 4);
        for (int i = 0; i < num_dimensions; i++) {
          if (i > 0) {
            pre << ", ";
          }
          pre << dims << "." << xyzw[i];
        }
      }

      pre << ");" << std::endl;
      make_indent(pre);

      // The out parameters of the GetDimensions() call is now in temporary
      // `dims` variable. This may be packed with other data, so the final
      // expression may require a swizzle.
      out << dims << swizzle;
      return true;
    }
    default:
      break;
  }

  if (!EmitExpression(pre, out, texture))
    return false;

  bool pack_mip_in_coords = false;

  switch (intrinsic->Type()) {
    case semantic::IntrinsicType::kTextureSample:
      out << ".Sample(";
      break;
    case semantic::IntrinsicType::kTextureSampleBias:
      out << ".SampleBias(";
      break;
    case semantic::IntrinsicType::kTextureSampleLevel:
      out << ".SampleLevel(";
      break;
    case semantic::IntrinsicType::kTextureSampleGrad:
      out << ".SampleGrad(";
      break;
    case semantic::IntrinsicType::kTextureSampleCompare:
      out << ".SampleCmp(";
      break;
    case semantic::IntrinsicType::kTextureLoad:
      out << ".Load(";
      if (!texture_type->Is<type::StorageTexture>()) {
        pack_mip_in_coords = true;
      }
      break;
    case semantic::IntrinsicType::kTextureStore:
      out << "[";
      break;
    default:
      diagnostics_.add_error(
          "Internal compiler error: Unhandled texture intrinsic '" +
          std::string(intrinsic->str()) + "'");
      return false;
  }

  if (auto* sampler = arg(Usage::kSampler)) {
    if (!EmitExpression(pre, out, sampler))
      return false;
    out << ", ";
  }

  auto* param_coords = arg(Usage::kCoords);
  assert(param_coords);

  auto emit_vector_appended_with_i32_zero = [&](tint::ast::Expression* vector) {
    auto* i32 = builder_.create<type::I32>();
    auto* zero = builder_.Expr(0);
    auto* stmt = builder_.Sem().Get(vector)->Stmt();
    builder_.Sem().Add(zero,
                       builder_.create<semantic::Expression>(zero, i32, stmt));
    auto* packed = AppendVector(&builder_, vector, zero);
    return EmitExpression(pre, out, packed);
  };

  if (auto* array_index = arg(Usage::kArrayIndex)) {
    // Array index needs to be appended to the coordinates.
    auto* packed = AppendVector(&builder_, param_coords, array_index);
    if (pack_mip_in_coords) {
      if (!emit_vector_appended_with_i32_zero(packed)) {
        return false;
      }
    } else {
      if (!EmitExpression(pre, out, packed)) {
        return false;
      }
    }
  } else if (pack_mip_in_coords) {
    // Mip level needs to be appended to the coordinates, but is always zero.
    if (!emit_vector_appended_with_i32_zero(param_coords))
      return false;
  } else {
    if (!EmitExpression(pre, out, param_coords))
      return false;
  }

  for (auto usage : {Usage::kDepthRef, Usage::kBias, Usage::kLevel, Usage::kDdx,
                     Usage::kDdy, Usage::kSampleIndex, Usage::kOffset}) {
    if (auto* e = arg(usage)) {
      out << ", ";
      if (!EmitExpression(pre, out, e))
        return false;
    }
  }

  if (intrinsic->Type() == semantic::IntrinsicType::kTextureStore) {
    out << "] = ";
    if (!EmitExpression(pre, out, arg(Usage::kValue)))
      return false;
  } else {
    out << ")";
  }

  return true;
}  // namespace hlsl

std::string GeneratorImpl::generate_builtin_name(
    const semantic::Intrinsic* intrinsic) {
  std::string out;
  switch (intrinsic->Type()) {
    case semantic::IntrinsicType::kAcos:
    case semantic::IntrinsicType::kAny:
    case semantic::IntrinsicType::kAll:
    case semantic::IntrinsicType::kAsin:
    case semantic::IntrinsicType::kAtan:
    case semantic::IntrinsicType::kAtan2:
    case semantic::IntrinsicType::kCeil:
    case semantic::IntrinsicType::kCos:
    case semantic::IntrinsicType::kCosh:
    case semantic::IntrinsicType::kCross:
    case semantic::IntrinsicType::kDeterminant:
    case semantic::IntrinsicType::kDistance:
    case semantic::IntrinsicType::kDot:
    case semantic::IntrinsicType::kExp:
    case semantic::IntrinsicType::kExp2:
    case semantic::IntrinsicType::kFloor:
    case semantic::IntrinsicType::kFma:
    case semantic::IntrinsicType::kLdexp:
    case semantic::IntrinsicType::kLength:
    case semantic::IntrinsicType::kLog:
    case semantic::IntrinsicType::kLog2:
    case semantic::IntrinsicType::kNormalize:
    case semantic::IntrinsicType::kPow:
    case semantic::IntrinsicType::kReflect:
    case semantic::IntrinsicType::kRound:
    case semantic::IntrinsicType::kSin:
    case semantic::IntrinsicType::kSinh:
    case semantic::IntrinsicType::kSqrt:
    case semantic::IntrinsicType::kStep:
    case semantic::IntrinsicType::kTan:
    case semantic::IntrinsicType::kTanh:
    case semantic::IntrinsicType::kTrunc:
    case semantic::IntrinsicType::kMix:
    case semantic::IntrinsicType::kSign:
    case semantic::IntrinsicType::kAbs:
    case semantic::IntrinsicType::kMax:
    case semantic::IntrinsicType::kMin:
    case semantic::IntrinsicType::kClamp:
      out = intrinsic->str();
      break;
    case semantic::IntrinsicType::kCountOneBits:
      out = "countbits";
      break;
    case semantic::IntrinsicType::kDpdx:
      out = "ddx";
      break;
    case semantic::IntrinsicType::kDpdxCoarse:
      out = "ddx_coarse";
      break;
    case semantic::IntrinsicType::kDpdxFine:
      out = "ddx_fine";
      break;
    case semantic::IntrinsicType::kDpdy:
      out = "ddy";
      break;
    case semantic::IntrinsicType::kDpdyCoarse:
      out = "ddy_coarse";
      break;
    case semantic::IntrinsicType::kDpdyFine:
      out = "ddy_fine";
      break;
    case semantic::IntrinsicType::kFaceForward:
      out = "faceforward";
      break;
    case semantic::IntrinsicType::kFract:
      out = "frac";
      break;
    case semantic::IntrinsicType::kFwidth:
    case semantic::IntrinsicType::kFwidthCoarse:
    case semantic::IntrinsicType::kFwidthFine:
      out = "fwidth";
      break;
    case semantic::IntrinsicType::kInverseSqrt:
      out = "rsqrt";
      break;
    case semantic::IntrinsicType::kIsFinite:
      out = "isfinite";
      break;
    case semantic::IntrinsicType::kIsInf:
      out = "isinf";
      break;
    case semantic::IntrinsicType::kIsNan:
      out = "isnan";
      break;
    case semantic::IntrinsicType::kReverseBits:
      out = "reversebits";
      break;
    case semantic::IntrinsicType::kSmoothStep:
      out = "smoothstep";
      break;
    default:
      diagnostics_.add_error("Unknown builtin method: " +
                             std::string(intrinsic->str()));
      return "";
  }

  return out;
}

bool GeneratorImpl::EmitCase(std::ostream& out, ast::CaseStatement* stmt) {
  make_indent(out);

  if (stmt->IsDefault()) {
    out << "default:";
  } else {
    bool first = true;
    for (auto* selector : stmt->selectors()) {
      if (!first) {
        out << std::endl;
        make_indent(out);
      }
      first = false;

      out << "case ";
      if (!EmitLiteral(out, selector)) {
        return false;
      }
      out << ":";
    }
  }

  out << " {" << std::endl;

  increment_indent();

  for (auto* s : *stmt->body()) {
    if (!EmitStatement(out, s)) {
      return false;
    }
  }

  if (!last_is_break_or_fallthrough(stmt->body())) {
    make_indent(out);
    out << "break;" << std::endl;
  }

  decrement_indent();
  make_indent(out);
  out << "}" << std::endl;

  return true;
}

bool GeneratorImpl::EmitConstructor(std::ostream& pre,
                                    std::ostream& out,
                                    ast::ConstructorExpression* expr) {
  if (auto* scalar = expr->As<ast::ScalarConstructorExpression>()) {
    return EmitScalarConstructor(pre, out, scalar);
  }
  return EmitTypeConstructor(pre, out,
                             expr->As<ast::TypeConstructorExpression>());
}

bool GeneratorImpl::EmitScalarConstructor(
    std::ostream&,
    std::ostream& out,
    ast::ScalarConstructorExpression* expr) {
  return EmitLiteral(out, expr->literal());
}

bool GeneratorImpl::EmitTypeConstructor(std::ostream& pre,
                                        std::ostream& out,
                                        ast::TypeConstructorExpression* expr) {
  if (expr->type()->Is<type::Array>()) {
    out << "{";
  } else {
    if (!EmitType(out, expr->type(), "")) {
      return false;
    }
    out << "(";
  }

  // If the type constructor is empty then we need to construct with the zero
  // value for all components.
  if (expr->values().empty()) {
    if (!EmitZeroValue(out, expr->type())) {
      return false;
    }
  } else {
    bool first = true;
    for (auto* e : expr->values()) {
      if (!first) {
        out << ", ";
      }
      first = false;

      if (!EmitExpression(pre, out, e)) {
        return false;
      }
    }
  }

  if (expr->type()->Is<type::Array>()) {
    out << "}";
  } else {
    out << ")";
  }
  return true;
}

bool GeneratorImpl::EmitContinue(std::ostream& out, ast::ContinueStatement*) {
  make_indent(out);
  out << "continue;" << std::endl;
  return true;
}

bool GeneratorImpl::EmitDiscard(std::ostream& out, ast::DiscardStatement*) {
  make_indent(out);
  // TODO(dsinclair): Verify this is correct when the discard semantics are
  // defined for WGSL (https://github.com/gpuweb/gpuweb/issues/361)
  out << "discard;" << std::endl;
  return true;
}

bool GeneratorImpl::EmitExpression(std::ostream& pre,
                                   std::ostream& out,
                                   ast::Expression* expr) {
  if (auto* a = expr->As<ast::ArrayAccessorExpression>()) {
    return EmitArrayAccessor(pre, out, a);
  }
  if (auto* b = expr->As<ast::BinaryExpression>()) {
    return EmitBinary(pre, out, b);
  }
  if (auto* b = expr->As<ast::BitcastExpression>()) {
    return EmitBitcast(pre, out, b);
  }
  if (auto* c = expr->As<ast::CallExpression>()) {
    return EmitCall(pre, out, c);
  }
  if (auto* c = expr->As<ast::ConstructorExpression>()) {
    return EmitConstructor(pre, out, c);
  }
  if (auto* i = expr->As<ast::IdentifierExpression>()) {
    return EmitIdentifier(pre, out, i);
  }
  if (auto* m = expr->As<ast::MemberAccessorExpression>()) {
    return EmitMemberAccessor(pre, out, m);
  }
  if (auto* u = expr->As<ast::UnaryOpExpression>()) {
    return EmitUnaryOp(pre, out, u);
  }

  diagnostics_.add_error("unknown expression type: " + builder_.str(expr));
  return false;
}

bool GeneratorImpl::global_is_in_struct(const semantic::Variable* var) const {
  if (var->Declaration()->HasLocationDecoration() ||
      var->Declaration()->HasBuiltinDecoration()) {
    return var->StorageClass() == ast::StorageClass::kInput ||
           var->StorageClass() == ast::StorageClass::kOutput;
  }
  return false;
}

bool GeneratorImpl::EmitIdentifier(std::ostream&,
                                   std::ostream& out,
                                   ast::IdentifierExpression* expr) {
  auto* ident = expr->As<ast::IdentifierExpression>();
  const semantic::Variable* var = nullptr;
  if (global_variables_.get(ident->symbol(), &var)) {
    if (global_is_in_struct(var)) {
      auto var_type = var->StorageClass() == ast::StorageClass::kInput
                          ? VarType::kIn
                          : VarType::kOut;
      auto name = current_ep_var_name(var_type);
      if (name.empty()) {
        diagnostics_.add_error("unable to find entry point data for variable");
        return false;
      }
      out << name << ".";
    }
  }

  out << namer_.NameFor(builder_.Symbols().NameFor(ident->symbol()));

  return true;
}

bool GeneratorImpl::EmitIf(std::ostream& out, ast::IfStatement* stmt) {
  make_indent(out);

  std::ostringstream pre;
  std::ostringstream cond;
  if (!EmitExpression(pre, cond, stmt->condition())) {
    return false;
  }

  std::ostringstream if_out;
  if_out << "if (" << cond.str() << ") ";
  if (!EmitBlock(if_out, stmt->body())) {
    return false;
  }

  for (auto* e : stmt->else_statements()) {
    if (e->HasCondition()) {
      if_out << " else {" << std::endl;

      increment_indent();

      std::ostringstream else_pre;
      std::ostringstream else_cond_out;
      if (!EmitExpression(else_pre, else_cond_out, e->condition())) {
        return false;
      }
      if_out << else_pre.str();

      make_indent(if_out);
      if_out << "if (" << else_cond_out.str() << ") ";
    } else {
      if_out << " else ";
    }

    if (!EmitBlock(if_out, e->body())) {
      return false;
    }
  }
  if_out << std::endl;

  for (auto* e : stmt->else_statements()) {
    if (!e->HasCondition()) {
      continue;
    }

    decrement_indent();
    make_indent(if_out);
    if_out << "}" << std::endl;
  }

  out << pre.str();
  out << if_out.str();
  return true;
}

bool GeneratorImpl::has_referenced_in_var_needing_struct(
    const semantic::Function* func) {
  for (auto data : func->ReferencedLocationVariables()) {
    auto* var = data.first;
    if (var->StorageClass() == ast::StorageClass::kInput) {
      return true;
    }
  }

  for (auto data : func->ReferencedBuiltinVariables()) {
    auto* var = data.first;
    if (var->StorageClass() == ast::StorageClass::kInput) {
      return true;
    }
  }
  return false;
}

bool GeneratorImpl::has_referenced_out_var_needing_struct(
    const semantic::Function* func) {
  for (auto data : func->ReferencedLocationVariables()) {
    auto* var = data.first;
    if (var->StorageClass() == ast::StorageClass::kOutput) {
      return true;
    }
  }

  for (auto data : func->ReferencedBuiltinVariables()) {
    auto* var = data.first;
    if (var->StorageClass() == ast::StorageClass::kOutput) {
      return true;
    }
  }
  return false;
}

bool GeneratorImpl::has_referenced_var_needing_struct(
    const semantic::Function* func) {
  for (auto data : func->ReferencedLocationVariables()) {
    auto* var = data.first;
    if (var->StorageClass() == ast::StorageClass::kOutput ||
        var->StorageClass() == ast::StorageClass::kInput) {
      return true;
    }
  }

  for (auto data : func->ReferencedBuiltinVariables()) {
    auto* var = data.first;
    if (var->StorageClass() == ast::StorageClass::kOutput ||
        var->StorageClass() == ast::StorageClass::kInput) {
      return true;
    }
  }
  return false;
}

bool GeneratorImpl::EmitFunction(std::ostream& out, ast::Function* func) {
  make_indent(out);

  // Entry points will be emitted later, skip for now.
  if (func->IsEntryPoint()) {
    return true;
  }

  auto* func_sem = builder_.Sem().Get(func);

  // TODO(dsinclair): This could be smarter. If the input/outputs for multiple
  // entry points are the same we could generate a single struct and then have
  // this determine it's the same struct and just emit once.
  bool emit_duplicate_functions = func_sem->AncestorEntryPoints().size() > 0 &&
                                  has_referenced_var_needing_struct(func_sem);

  if (emit_duplicate_functions) {
    for (const auto& ep_sym : func_sem->AncestorEntryPoints()) {
      if (!EmitFunctionInternal(out, func, emit_duplicate_functions, ep_sym)) {
        return false;
      }
      out << std::endl;
    }
  } else {
    // Emit as non-duplicated
    if (!EmitFunctionInternal(out, func, false, Symbol())) {
      return false;
    }
    out << std::endl;
  }

  return true;
}

bool GeneratorImpl::EmitFunctionInternal(std::ostream& out,
                                         ast::Function* func,
                                         bool emit_duplicate_functions,
                                         Symbol ep_sym) {
  auto name = func->symbol().to_str();

  if (!EmitType(out, func->return_type(), "")) {
    return false;
  }

  out << " ";

  if (emit_duplicate_functions) {
    auto func_name = name;
    auto ep_name = ep_sym.to_str();
    // TODO(dsinclair): The SymbolToName should go away and just use
    // to_str() here when the conversion is complete.
    name = generate_name(builder_.Symbols().NameFor(func->symbol()) + "_" +
                         builder_.Symbols().NameFor(ep_sym));
    ep_func_name_remapped_[ep_name + "_" + func_name] = name;
  } else {
    // TODO(dsinclair): this should be updated to a remapped name
    name = namer_.NameFor(builder_.Symbols().NameFor(func->symbol()));
  }

  out << name << "(";

  bool first = true;

  // If we're emitting duplicate functions that means the function takes
  // the stage_in or stage_out value from the entry point, emit them.
  //
  // We emit both of them if they're there regardless of if they're both used.
  if (emit_duplicate_functions) {
    auto in_it = ep_sym_to_in_data_.find(ep_sym);
    if (in_it != ep_sym_to_in_data_.end()) {
      out << "in " << in_it->second.struct_name << " "
          << in_it->second.var_name;
      first = false;
    }

    auto outit = ep_sym_to_out_data_.find(ep_sym);
    if (outit != ep_sym_to_out_data_.end()) {
      if (!first) {
        out << ", ";
      }
      out << "out " << outit->second.struct_name << " "
          << outit->second.var_name;
      first = false;
    }
  }

  for (auto* v : func->params()) {
    if (!first) {
      out << ", ";
    }
    first = false;

    auto* type = builder_.Sem().Get(v)->Type();

    if (!EmitType(out, type, builder_.Symbols().NameFor(v->symbol()))) {
      return false;
    }
    // Array name is output as part of the type
    if (!type->Is<type::Array>()) {
      out << " " << builder_.Symbols().NameFor(v->symbol());
    }
  }

  out << ") ";

  current_ep_sym_ = ep_sym;

  if (!EmitBlockAndNewline(out, func->body())) {
    return false;
  }

  current_ep_sym_ = Symbol();

  return true;
}

bool GeneratorImpl::EmitEntryPointData(
    std::ostream& out,
    ast::Function* func,
    std::unordered_set<Symbol>& emitted_globals) {
  std::vector<std::pair<const ast::Variable*, ast::Decoration*>> in_variables;
  std::vector<std::pair<const ast::Variable*, ast::Decoration*>> outvariables;
  auto* func_sem = builder_.Sem().Get(func);
  auto func_sym = func->symbol();

  // TODO(jrprice): Remove this when we remove support for entry point
  // inputs/outputs as module-scope globals.
  for (auto data : func_sem->ReferencedLocationVariables()) {
    auto* var = data.first;
    auto* decl = var->Declaration();
    auto* deco = data.second;

    if (var->StorageClass() == ast::StorageClass::kInput) {
      in_variables.push_back({decl, deco});
    } else if (var->StorageClass() == ast::StorageClass::kOutput) {
      outvariables.push_back({decl, deco});
    }
  }

  // TODO(jrprice): Remove this when we remove support for entry point
  // inputs/outputs as module-scope globals.
  for (auto data : func_sem->ReferencedBuiltinVariables()) {
    auto* var = data.first;
    auto* decl = var->Declaration();
    auto* deco = data.second;

    if (var->StorageClass() == ast::StorageClass::kInput) {
      in_variables.push_back({decl, deco});
    } else if (var->StorageClass() == ast::StorageClass::kOutput) {
      outvariables.push_back({decl, deco});
    }
  }

  bool emitted_uniform = false;
  for (auto data : func_sem->ReferencedUniformVariables()) {
    auto* var = data.first;
    auto* decl = var->Declaration();

    if (!emitted_globals.emplace(decl->symbol()).second) {
      continue;  // Global already emitted
    }

    // TODO(dsinclair): We're using the binding to make up the buffer number but
    // we should instead be using a provided mapping that uses both buffer and
    // set. https://bugs.chromium.org/p/tint/issues/detail?id=104
    auto* binding = data.second.binding;
    if (binding == nullptr) {
      diagnostics_.add_error(
          "unable to find binding information for uniform: " +
          builder_.Symbols().NameFor(decl->symbol()));
      return false;
    }
    // auto* set = data.second.set;

    auto* type = var->Type()->UnwrapIfNeeded();
    if (auto* strct = type->As<type::Struct>()) {
      out << "ConstantBuffer<" << builder_.Symbols().NameFor(strct->symbol())
          << "> " << builder_.Symbols().NameFor(decl->symbol())
          << " : register(b" << binding->value() << ");" << std::endl;
    } else {
      // TODO(dsinclair): There is outstanding spec work to require all uniform
      // buffers to be [[block]] decorated, which means structs. This is
      // currently not the case, so this code handles the cases where the data
      // is not a block.
      // Relevant: https://github.com/gpuweb/gpuweb/issues/1004
      //           https://github.com/gpuweb/gpuweb/issues/1008
      auto name = "cbuffer_" + builder_.Symbols().NameFor(decl->symbol());
      out << "cbuffer " << name << " : register(b" << binding->value() << ") {"
          << std::endl;

      increment_indent();
      make_indent(out);
      if (!EmitType(out, type, "")) {
        return false;
      }
      out << " " << builder_.Symbols().NameFor(decl->symbol()) << ";"
          << std::endl;
      decrement_indent();
      out << "};" << std::endl;
    }

    emitted_uniform = true;
  }
  if (emitted_uniform) {
    out << std::endl;
  }

  bool emitted_storagebuffer = false;
  for (auto data : func_sem->ReferencedStorageBufferVariables()) {
    auto* var = data.first;
    auto* decl = var->Declaration();

    if (!emitted_globals.emplace(decl->symbol()).second) {
      continue;  // Global already emitted
    }

    auto* binding = data.second.binding;
    auto* ac = var->Type()->As<type::AccessControl>();
    if (ac == nullptr) {
      diagnostics_.add_error("access control type required for storage buffer");
      return false;
    }

    if (!ac->IsReadOnly()) {
      out << "RW";
    }
    out << "ByteAddressBuffer " << builder_.Symbols().NameFor(decl->symbol())
        << " : register(" << (ac->IsReadOnly() ? "t" : "u") << binding->value()
        << ");" << std::endl;
    emitted_storagebuffer = true;
  }
  if (emitted_storagebuffer) {
    out << std::endl;
  }

  // TODO(jrprice): Remove this when we remove support for entry point inputs as
  // module-scope globals.
  if (!in_variables.empty()) {
    auto in_struct_name = generate_name(builder_.Symbols().NameFor(func_sym) +
                                        "_" + kInStructNameSuffix);
    auto in_var_name = generate_name(kTintStructInVarPrefix);
    ep_sym_to_in_data_[func_sym] = {in_struct_name, in_var_name};

    make_indent(out);
    out << "struct " << in_struct_name << " {" << std::endl;

    increment_indent();

    for (auto& data : in_variables) {
      auto* var = data.first;
      auto* deco = data.second;
      auto* type = builder_.Sem().Get(var)->Type();

      make_indent(out);
      if (!EmitType(out, type, builder_.Symbols().NameFor(var->symbol()))) {
        return false;
      }

      out << " " << builder_.Symbols().NameFor(var->symbol()) << " : ";
      if (auto* location = deco->As<ast::LocationDecoration>()) {
        if (func->pipeline_stage() == ast::PipelineStage::kCompute) {
          diagnostics_.add_error(
              "invalid location variable for pipeline stage");
          return false;
        }
        out << "TEXCOORD" << location->value();
      } else if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
        auto attr = builtin_to_attribute(builtin->value());
        if (attr.empty()) {
          diagnostics_.add_error("unsupported builtin");
          return false;
        }
        out << attr;
      } else {
        diagnostics_.add_error(
            "unsupported variable decoration for entry point output");
        return false;
      }
      out << ";" << std::endl;
    }
    decrement_indent();
    make_indent(out);

    out << "};" << std::endl << std::endl;
  }

  // TODO(jrprice): Remove this when we remove support for entry point outputs
  // as module-scope globals.
  if (!outvariables.empty()) {
    auto outstruct_name = generate_name(builder_.Symbols().NameFor(func_sym) +
                                        "_" + kOutStructNameSuffix);
    auto outvar_name = generate_name(kTintStructOutVarPrefix);
    ep_sym_to_out_data_[func_sym] = {outstruct_name, outvar_name};

    make_indent(out);
    out << "struct " << outstruct_name << " {" << std::endl;

    increment_indent();
    for (auto& data : outvariables) {
      auto* var = data.first;
      auto* deco = data.second;
      auto* type = builder_.Sem().Get(var)->Type();

      make_indent(out);
      if (!EmitType(out, type, builder_.Symbols().NameFor(var->symbol()))) {
        return false;
      }

      out << " " << builder_.Symbols().NameFor(var->symbol()) << " : ";

      if (auto* location = deco->As<ast::LocationDecoration>()) {
        auto loc = location->value();
        if (func->pipeline_stage() == ast::PipelineStage::kVertex) {
          out << "TEXCOORD" << loc;
        } else if (func->pipeline_stage() == ast::PipelineStage::kFragment) {
          out << "SV_Target" << loc << "";
        } else {
          diagnostics_.add_error(
              "invalid location variable for pipeline stage");
          return false;
        }
      } else if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
        auto attr = builtin_to_attribute(builtin->value());
        if (attr.empty()) {
          diagnostics_.add_error("unsupported builtin");
          return false;
        }
        out << attr;
      } else {
        diagnostics_.add_error(
            "unsupported variable decoration for entry point output");
        return false;
      }
      out << ";" << std::endl;
    }
    decrement_indent();
    make_indent(out);
    out << "};" << std::endl << std::endl;
  }

  {
    bool add_newline = false;
    for (auto* var : func_sem->ReferencedModuleVariables()) {
      auto* decl = var->Declaration();

      auto* unwrapped_type = var->Type()->UnwrapAll();
      if (!unwrapped_type->Is<type::Texture>() &&
          !unwrapped_type->Is<type::Sampler>()) {
        continue;  // Not interested in this type
      }

      if (!emitted_globals.emplace(decl->symbol()).second) {
        continue;  // Global already emitted
      }

      if (!EmitType(out, var->Type(), "")) {
        return false;
      }
      out << " " << namer_.NameFor(builder_.Symbols().NameFor(decl->symbol()))
          << ";" << std::endl;

      add_newline = true;
    }
    if (add_newline) {
      out << std::endl;
    }
  }

  return true;
}

std::string GeneratorImpl::builtin_to_attribute(ast::Builtin builtin) const {
  switch (builtin) {
    case ast::Builtin::kPosition:
      return "SV_Position";
    case ast::Builtin::kVertexIndex:
      return "SV_VertexID";
    case ast::Builtin::kInstanceIndex:
      return "SV_InstanceID";
    case ast::Builtin::kFrontFacing:
      return "SV_IsFrontFacing";
    case ast::Builtin::kFragCoord:
      return "SV_Position";
    case ast::Builtin::kFragDepth:
      return "SV_Depth";
    case ast::Builtin::kLocalInvocationId:
      return "SV_GroupThreadID";
    case ast::Builtin::kLocalInvocationIndex:
      return "SV_GroupIndex";
    case ast::Builtin::kGlobalInvocationId:
      return "SV_DispatchThreadID";
    case ast::Builtin::kSampleIndex:
      return "SV_SampleIndex";
    case ast::Builtin::kSampleMaskIn:
      return "SV_Coverage";
    case ast::Builtin::kSampleMaskOut:
      return "SV_Coverage";
    default:
      break;
  }
  return "";
}

bool GeneratorImpl::EmitEntryPointFunction(std::ostream& out,
                                           ast::Function* func) {
  make_indent(out);

  current_ep_sym_ = func->symbol();

  if (func->pipeline_stage() == ast::PipelineStage::kCompute) {
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t z = 0;
    std::tie(x, y, z) = func->workgroup_size();
    out << "[numthreads(" << std::to_string(x) << ", " << std::to_string(y)
        << ", " << std::to_string(z) << ")]" << std::endl;
    make_indent(out);
  }

  auto outdata = ep_sym_to_out_data_.find(current_ep_sym_);
  bool has_outdata = outdata != ep_sym_to_out_data_.end();
  if (has_outdata) {
    out << outdata->second.struct_name;
  } else {
    out << "void";
  }
  // TODO(dsinclair): This should output the remapped name
  out << " " << namer_.NameFor(builder_.Symbols().NameFor(current_ep_sym_))
      << "(";

  bool first = true;
  // TODO(jrprice): Remove this when we remove support for inputs as globals.
  auto in_data = ep_sym_to_in_data_.find(current_ep_sym_);
  if (in_data != ep_sym_to_in_data_.end()) {
    out << in_data->second.struct_name << " " << in_data->second.var_name;
    first = false;
  }

  // Emit entry point parameters.
  for (auto* var : func->params()) {
    auto* type = builder_.Sem().Get(var)->Type();
    if (!type->Is<type::Struct>()) {
      TINT_ICE(diagnostics_) << "Unsupported non-struct entry point parameter";
    }

    if (!first) {
      out << ", ";
    }
    first = false;

    if (!EmitType(out, type, "")) {
      return false;
    }

    out << " " << builder_.Symbols().NameFor(var->symbol());
  }

  out << ") {" << std::endl;

  increment_indent();

  if (has_outdata) {
    make_indent(out);
    out << outdata->second.struct_name << " " << outdata->second.var_name << ";"
        << std::endl;
  }

  generating_entry_point_ = true;
  for (auto* s : *func->body()) {
    if (!EmitStatement(out, s)) {
      return false;
    }
  }
  auto* last_statement = func->get_last_statement();
  if (last_statement == nullptr ||
      !last_statement->Is<ast::ReturnStatement>()) {
    ast::ReturnStatement ret(Source{});
    if (!EmitStatement(out, &ret)) {
      return false;
    }
  }
  generating_entry_point_ = false;

  decrement_indent();
  make_indent(out);
  out << "}" << std::endl;

  current_ep_sym_ = Symbol();

  return true;
}

bool GeneratorImpl::EmitLiteral(std::ostream& out, ast::Literal* lit) {
  if (auto* l = lit->As<ast::BoolLiteral>()) {
    out << (l->IsTrue() ? "true" : "false");
  } else if (auto* fl = lit->As<ast::FloatLiteral>()) {
    out << FloatToString(fl->value()) << "f";
  } else if (auto* sl = lit->As<ast::SintLiteral>()) {
    out << sl->value();
  } else if (auto* ul = lit->As<ast::UintLiteral>()) {
    out << ul->value() << "u";
  } else {
    diagnostics_.add_error("unknown literal type");
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitZeroValue(std::ostream& out, type::Type* type) {
  if (type->Is<type::Bool>()) {
    out << "false";
  } else if (type->Is<type::F32>()) {
    out << "0.0f";
  } else if (type->Is<type::I32>()) {
    out << "0";
  } else if (type->Is<type::U32>()) {
    out << "0u";
  } else if (auto* vec = type->As<type::Vector>()) {
    return EmitZeroValue(out, vec->type());
  } else if (auto* mat = type->As<type::Matrix>()) {
    for (uint32_t i = 0; i < (mat->rows() * mat->columns()); i++) {
      if (i != 0) {
        out << ", ";
      }
      if (!EmitZeroValue(out, mat->type())) {
        return false;
      }
    }
  } else {
    diagnostics_.add_error("Invalid type for zero emission: " +
                           type->type_name());
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitLoop(std::ostream& out, ast::LoopStatement* stmt) {
  loop_emission_counter_++;

  std::string guard = namer_.NameFor("tint_hlsl_is_first_" +
                                     std::to_string(loop_emission_counter_));

  if (stmt->has_continuing()) {
    make_indent(out);

    // Continuing variables get their own scope.
    out << "{" << std::endl;
    increment_indent();

    make_indent(out);
    out << "bool " << guard << " = true;" << std::endl;

    // A continuing block may use variables declared in the method body. As a
    // first pass, if we have a continuing, we pull all declarations outside
    // the for loop into the continuing scope. Then, the variable declarations
    // will be turned into assignments.
    for (auto* s : *stmt->body()) {
      if (auto* v = s->As<ast::VariableDeclStatement>()) {
        if (!EmitVariable(out, v->variable(), true)) {
          return false;
        }
      }
    }
  }

  make_indent(out);
  out << "for(;;) {" << std::endl;
  increment_indent();

  if (stmt->has_continuing()) {
    make_indent(out);
    out << "if (!" << guard << ") ";

    if (!EmitBlockAndNewline(out, stmt->continuing())) {
      return false;
    }

    make_indent(out);
    out << guard << " = false;" << std::endl;
    out << std::endl;
  }

  for (auto* s : *(stmt->body())) {
    // If we have a continuing block we've already emitted the variable
    // declaration before the loop, so treat it as an assignment.
    if (auto* decl = s->As<ast::VariableDeclStatement>()) {
      if (stmt->has_continuing()) {
        make_indent(out);

        auto* var = decl->variable();

        std::ostringstream pre;
        std::ostringstream constructor_out;
        if (var->constructor() != nullptr) {
          if (!EmitExpression(pre, constructor_out, var->constructor())) {
            return false;
          }
        }
        out << pre.str();

        out << builder_.Symbols().NameFor(var->symbol()) << " = ";
        if (var->constructor() != nullptr) {
          out << constructor_out.str();
        } else {
          if (!EmitZeroValue(out, builder_.Sem().Get(var)->Type())) {
            return false;
          }
        }
        out << ";" << std::endl;
        continue;
      }
    }

    if (!EmitStatement(out, s)) {
      return false;
    }
  }

  decrement_indent();
  make_indent(out);
  out << "}" << std::endl;

  // Close the scope for any continuing variables.
  if (stmt->has_continuing()) {
    decrement_indent();
    make_indent(out);
    out << "}" << std::endl;
  }

  return true;
}

std::string GeneratorImpl::generate_storage_buffer_index_expression(
    std::ostream& pre,
    ast::Expression* expr) {
  std::ostringstream out;
  bool first = true;
  for (;;) {
    if (expr->Is<ast::IdentifierExpression>()) {
      break;
    }

    if (!first) {
      out << " + ";
    }
    first = false;
    if (auto* mem = expr->As<ast::MemberAccessorExpression>()) {
      auto* res_type = TypeOf(mem->structure())->UnwrapAll();
      if (auto* str = res_type->As<type::Struct>()) {
        auto* str_type = str->impl();
        auto* str_member = str_type->get_member(mem->member()->symbol());

        auto* sem_mem = builder_.Sem().Get(str_member);
        if (!sem_mem) {
          TINT_ICE(diagnostics_) << "struct member missing semantic info";
          return "";
        }

        out << sem_mem->Offset();

      } else if (res_type->Is<type::Vector>()) {
        auto swizzle = builder_.Sem().Get(mem)->Swizzle();

        // TODO(dsinclair): Swizzle stuff
        //
        // This must be a single element swizzle if we've got a vector at this
        // point.
        if (swizzle.size() != 1) {
          TINT_ICE(diagnostics_)
              << "Encountered multi-element swizzle when should have only one "
                 "level";
          return "";
        }

        // TODO(dsinclair): All our types are currently 4 bytes (f32, i32, u32)
        // so this is assuming 4. This will need to be fixed when we get f16 or
        // f64 types.
        out << "(4 * " << swizzle[0] << ")";
      } else {
        TINT_ICE(diagnostics_) << "Invalid result type for member accessor: "
                               << res_type->type_name();
        return "";
      }

      expr = mem->structure();
    } else if (auto* ary = expr->As<ast::ArrayAccessorExpression>()) {
      auto* ary_type = TypeOf(ary->array())->UnwrapAll();

      out << "(";
      if (auto* arr = ary_type->As<type::Array>()) {
        auto* sem_arr = builder_.Sem().Get(arr);
        if (!sem_arr) {
          TINT_ICE(diagnostics_) << "array type missing semantic info";
          return "";
        }
        out << sem_arr->Stride();
      } else if (ary_type->Is<type::Vector>()) {
        // TODO(dsinclair): This is a hack. Our vectors can only be f32, i32
        // or u32 which are all 4 bytes. When we get f16 or other types we'll
        // have to ask the type for the byte size.
        out << "4";
      } else if (auto* mat = ary_type->As<type::Matrix>()) {
        if (mat->columns() == 2) {
          out << "8";
        } else {
          out << "16";
        }
      } else {
        diagnostics_.add_error("Invalid array type in storage buffer access");
        return "";
      }
      out << " * ";
      if (!EmitExpression(pre, out, ary->idx_expr())) {
        return "";
      }
      out << ")";

      expr = ary->array();
    } else {
      diagnostics_.add_error("error emitting storage buffer access");
      return "";
    }
  }

  return out.str();
}

// TODO(dsinclair): This currently only handles loading of 4, 8, 12 or 16 byte
// members. If we need to support larger we'll need to do the loading into
// chunks.
//
// TODO(dsinclair): Need to support loading through a pointer. The pointer is
// just a memory address in the storage buffer, so need to do the correct
// calculation.
bool GeneratorImpl::EmitStorageBufferAccessor(std::ostream& pre,
                                              std::ostream& out,
                                              ast::Expression* expr,
                                              ast::Expression* rhs) {
  auto* result_type = TypeOf(expr)->UnwrapAll();
  bool is_store = rhs != nullptr;

  std::string access_method = is_store ? "Store" : "Load";
  if (auto* vec = result_type->As<type::Vector>()) {
    access_method += std::to_string(vec->size());
  } else if (auto* mat = result_type->As<type::Matrix>()) {
    access_method += std::to_string(mat->rows());
  }

  // If we aren't storing then we need to put in the outer cast.
  if (!is_store) {
    if (result_type->is_float_scalar_or_vector() ||
        result_type->Is<type::Matrix>()) {
      out << "asfloat(";
    } else if (result_type->is_signed_scalar_or_vector()) {
      out << "asint(";
    } else if (result_type->is_unsigned_scalar_or_vector()) {
      out << "asuint(";
    }
  }

  auto buffer_name = get_buffer_name(expr);
  if (buffer_name.empty()) {
    diagnostics_.add_error("error emitting storage buffer access");
    return false;
  }

  auto idx = generate_storage_buffer_index_expression(pre, expr);
  if (idx.empty()) {
    return false;
  }

  if (auto* mat = result_type->As<type::Matrix>()) {
    // TODO(dsinclair): This is assuming 4 byte elements. Will need to be fixed
    // if we get matrixes of f16 or f64.
    uint32_t stride = mat->rows() == 2 ? 8 : 16;

    if (is_store) {
      if (!EmitType(out, mat, "")) {
        return false;
      }

      auto name = generate_name(kTempNamePrefix);
      out << " " << name << " = ";
      if (!EmitExpression(pre, out, rhs)) {
        return false;
      }
      out << ";" << std::endl;

      for (uint32_t i = 0; i < mat->columns(); i++) {
        if (i > 0) {
          out << ";" << std::endl;
        }

        make_indent(out);
        out << buffer_name << "." << access_method << "(" << idx << " + "
            << (i * stride) << ", asuint(" << name << "[" << i << "]))";
      }

      return true;
    }

    out << "uint" << mat->rows() << "x" << mat->columns() << "(";

    for (uint32_t i = 0; i < mat->columns(); i++) {
      if (i != 0) {
        out << ", ";
      }

      out << buffer_name << "." << access_method << "(" << idx << " + "
          << (i * stride) << ")";
    }

    // Close the matrix type and outer cast
    out << "))";

    return true;
  }

  out << buffer_name << "." << access_method << "(" << idx;
  if (is_store) {
    out << ", asuint(";
    if (!EmitExpression(pre, out, rhs)) {
      return false;
    }
    out << ")";
  }

  out << ")";

  // Close the outer cast.
  if (!is_store) {
    out << ")";
  }

  return true;
}

bool GeneratorImpl::is_storage_buffer_access(
    ast::ArrayAccessorExpression* expr) {
  // We only care about array so we can get to the next part of the expression.
  // If it isn't an array or a member accessor we can stop looking as it won't
  // be a storage buffer.
  auto* ary = expr->array();
  if (auto* member = ary->As<ast::MemberAccessorExpression>()) {
    return is_storage_buffer_access(member);
  } else if (auto* array = ary->As<ast::ArrayAccessorExpression>()) {
    return is_storage_buffer_access(array);
  }
  return false;
}

bool GeneratorImpl::is_storage_buffer_access(
    ast::MemberAccessorExpression* expr) {
  auto* structure = expr->structure();
  auto* data_type = TypeOf(structure)->UnwrapAll();
  // TODO(dsinclair): Swizzle
  //
  // If the data is a multi-element swizzle then we will not load the swizzle
  // portion through the Load command.
  if (data_type->Is<type::Vector>() &&
      builder_.Symbols().NameFor(expr->member()->symbol()).size() > 1) {
    return false;
  }

  // Check if this is a storage buffer variable
  if (auto* ident = expr->structure()->As<ast::IdentifierExpression>()) {
    const semantic::Variable* var = nullptr;
    if (!global_variables_.get(ident->symbol(), &var)) {
      return false;
    }
    return var->StorageClass() == ast::StorageClass::kStorage;
  } else if (auto* member = structure->As<ast::MemberAccessorExpression>()) {
    return is_storage_buffer_access(member);
  } else if (auto* array = structure->As<ast::ArrayAccessorExpression>()) {
    return is_storage_buffer_access(array);
  }

  // Technically I don't think this is possible, but if we don't have a struct
  // or array accessor then we can't have a storage buffer I believe.
  return false;
}

bool GeneratorImpl::EmitMemberAccessor(std::ostream& pre,
                                       std::ostream& out,
                                       ast::MemberAccessorExpression* expr) {
  // Look for storage buffer accesses as we have to convert them into Load
  // expressions. Stores will be identified in the assignment emission and a
  // member accessor store of a storage buffer will not get here.
  if (is_storage_buffer_access(expr)) {
    return EmitStorageBufferAccessor(pre, out, expr, nullptr);
  }

  if (!EmitExpression(pre, out, expr->structure())) {
    return false;
  }
  out << ".";

  // Swizzles output the name directly
  if (builder_.Sem().Get(expr)->IsSwizzle()) {
    out << builder_.Symbols().NameFor(expr->member()->symbol());
  } else if (!EmitExpression(pre, out, expr->member())) {
    return false;
  }

  return true;
}

bool GeneratorImpl::EmitReturn(std::ostream& out, ast::ReturnStatement* stmt) {
  make_indent(out);

  if (generating_entry_point_) {
    out << "return";
    auto outdata = ep_sym_to_out_data_.find(current_ep_sym_);
    if (outdata != ep_sym_to_out_data_.end()) {
      out << " " << outdata->second.var_name;
    }
  } else if (stmt->has_value()) {
    std::ostringstream pre;
    std::ostringstream ret_out;
    if (!EmitExpression(pre, ret_out, stmt->value())) {
      return false;
    }
    out << pre.str();
    out << "return " << ret_out.str();
  } else {
    out << "return";
  }
  out << ";" << std::endl;
  return true;
}

bool GeneratorImpl::EmitStatement(std::ostream& out, ast::Statement* stmt) {
  if (auto* a = stmt->As<ast::AssignmentStatement>()) {
    return EmitAssign(out, a);
  }
  if (auto* b = stmt->As<ast::BlockStatement>()) {
    return EmitIndentedBlockAndNewline(out, b);
  }
  if (auto* b = stmt->As<ast::BreakStatement>()) {
    return EmitBreak(out, b);
  }
  if (auto* c = stmt->As<ast::CallStatement>()) {
    make_indent(out);
    std::ostringstream pre;
    std::ostringstream call_out;
    if (!EmitCall(pre, call_out, c->expr())) {
      return false;
    }
    out << pre.str();
    if (!TypeOf(c->expr())->Is<type::Void>()) {
      out << "(void) ";
    }
    out << call_out.str() << ";" << std::endl;
    return true;
  }
  if (auto* c = stmt->As<ast::ContinueStatement>()) {
    return EmitContinue(out, c);
  }
  if (auto* d = stmt->As<ast::DiscardStatement>()) {
    return EmitDiscard(out, d);
  }
  if (stmt->As<ast::FallthroughStatement>()) {
    make_indent(out);
    out << "/* fallthrough */" << std::endl;
    return true;
  }
  if (auto* i = stmt->As<ast::IfStatement>()) {
    return EmitIf(out, i);
  }
  if (auto* l = stmt->As<ast::LoopStatement>()) {
    return EmitLoop(out, l);
  }
  if (auto* r = stmt->As<ast::ReturnStatement>()) {
    return EmitReturn(out, r);
  }
  if (auto* s = stmt->As<ast::SwitchStatement>()) {
    return EmitSwitch(out, s);
  }
  if (auto* v = stmt->As<ast::VariableDeclStatement>()) {
    return EmitVariable(out, v->variable(), false);
  }

  diagnostics_.add_error("unknown statement type: " + builder_.str(stmt));
  return false;
}

bool GeneratorImpl::EmitSwitch(std::ostream& out, ast::SwitchStatement* stmt) {
  make_indent(out);

  std::ostringstream pre;
  std::ostringstream cond;
  if (!EmitExpression(pre, cond, stmt->condition())) {
    return false;
  }

  out << pre.str();
  out << "switch(" << cond.str() << ") {" << std::endl;

  increment_indent();

  for (auto* s : stmt->body()) {
    if (!EmitCase(out, s)) {
      return false;
    }
  }

  decrement_indent();
  make_indent(out);
  out << "}" << std::endl;

  return true;
}

bool GeneratorImpl::EmitType(std::ostream& out,
                             type::Type* type,
                             const std::string& name) {
  // HLSL doesn't have the read/write only markings so just unwrap the access
  // control type.
  if (auto* ac = type->As<type::AccessControl>()) {
    return EmitType(out, ac->type(), name);
  }

  if (auto* alias = type->As<type::Alias>()) {
    out << namer_.NameFor(builder_.Symbols().NameFor(alias->symbol()));
  } else if (auto* ary = type->As<type::Array>()) {
    type::Type* base_type = ary;
    std::vector<uint32_t> sizes;
    while (auto* arr = base_type->As<type::Array>()) {
      if (arr->IsRuntimeArray()) {
        // TODO(dsinclair): Support runtime arrays
        // https://bugs.chromium.org/p/tint/issues/detail?id=185
        diagnostics_.add_error("runtime array not supported yet.");
        return false;
      } else {
        sizes.push_back(arr->size());
      }
      base_type = arr->type();
    }
    if (!EmitType(out, base_type, "")) {
      return false;
    }
    if (!name.empty()) {
      out << " " << namer_.NameFor(name);
    }
    for (uint32_t size : sizes) {
      out << "[" << size << "]";
    }
  } else if (type->Is<type::Bool>()) {
    out << "bool";
  } else if (type->Is<type::F32>()) {
    out << "float";
  } else if (type->Is<type::I32>()) {
    out << "int";
  } else if (auto* mat = type->As<type::Matrix>()) {
    if (!EmitType(out, mat->type(), "")) {
      return false;
    }
    out << mat->rows() << "x" << mat->columns();
  } else if (type->Is<type::Pointer>()) {
    // TODO(dsinclair): What do we do with pointers in HLSL?
    // https://bugs.chromium.org/p/tint/issues/detail?id=183
    diagnostics_.add_error("pointers not supported in HLSL");
    return false;
  } else if (auto* sampler = type->As<type::Sampler>()) {
    out << "Sampler";
    if (sampler->IsComparison()) {
      out << "Comparison";
    }
    out << "State";
  } else if (auto* str = type->As<type::Struct>()) {
    out << builder_.Symbols().NameFor(str->symbol());
  } else if (auto* tex = type->As<type::Texture>()) {
    if (tex->Is<type::StorageTexture>()) {
      out << "RW";
    }
    out << "Texture";

    auto* ms = tex->As<type::MultisampledTexture>();

    switch (tex->dim()) {
      case type::TextureDimension::k1d:
        out << "1D";
        break;
      case type::TextureDimension::k2d:
        out << (ms ? "2DMS" : "2D");
        break;
      case type::TextureDimension::k2dArray:
        out << (ms ? "2DMSArray" : "2DArray");
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
        TINT_UNREACHABLE(diagnostics_)
            << "unexpected TextureDimension " << tex->dim();
        return false;
    }

    if (ms) {
      out << "<";
      if (ms->type()->Is<type::F32>()) {
        out << "float4";
      } else if (ms->type()->Is<type::I32>()) {
        out << "int4";
      } else if (ms->type()->Is<type::U32>()) {
        out << "uint4";
      } else {
        TINT_ICE(diagnostics_) << "Unsupported multisampled texture type";
        return false;
      }

      // TODO(ben-clayton): The HLSL docs claim that the MS texture type should
      // also contain the number of samples, which is not part of the WGSL type.
      // However, DXC seems to consider this optional.
      // See: https://github.com/gpuweb/gpuweb/issues/1445

      out << ">";
    } else if (auto* st = tex->As<type::StorageTexture>()) {
      auto* component = image_format_to_rwtexture_type(st->image_format());
      if (component == nullptr) {
        TINT_ICE(diagnostics_) << "Unsupported StorageTexture ImageFormat: "
                               << static_cast<int>(st->image_format());
        return false;
      }
      out << "<" << component << ">";
    }
  } else if (type->Is<type::U32>()) {
    out << "uint";
  } else if (auto* vec = type->As<type::Vector>()) {
    auto size = vec->size();
    if (vec->type()->Is<type::F32>() && size >= 1 && size <= 4) {
      out << "float" << size;
    } else if (vec->type()->Is<type::I32>() && size >= 1 && size <= 4) {
      out << "int" << size;
    } else if (vec->type()->Is<type::U32>() && size >= 1 && size <= 4) {
      out << "uint" << size;
    } else {
      out << "vector<";
      if (!EmitType(out, vec->type(), "")) {
        return false;
      }
      out << ", " << size << ">";
    }
  } else if (type->Is<type::Void>()) {
    out << "void";
  } else {
    diagnostics_.add_error("unknown type in EmitType");
    return false;
  }

  return true;
}

bool GeneratorImpl::EmitStructType(std::ostream& out,
                                   const type::Struct* str,
                                   const std::string& name) {
  // TODO(dsinclair): Block decoration?
  // if (str->impl()->decoration() != ast::Decoration::kNone) {
  // }
  out << "struct " << name << " {" << std::endl;

  increment_indent();
  for (auto* mem : str->impl()->members()) {
    make_indent(out);
    // TODO(dsinclair): Handle [[offset]] annotation on structs
    // https://bugs.chromium.org/p/tint/issues/detail?id=184

    if (!EmitType(out, mem->type(),
                  builder_.Symbols().NameFor(mem->symbol()))) {
      return false;
    }
    // Array member name will be output with the type
    if (!mem->type()->Is<type::Array>()) {
      out << " " << namer_.NameFor(builder_.Symbols().NameFor(mem->symbol()));
    }

    if (mem->decorations().size() > 0) {
      auto* deco = mem->decorations()[0];
      if (auto* location = deco->As<ast::LocationDecoration>()) {
        out << " : TEXCOORD" << location->value();
      } else if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
        auto attr = builtin_to_attribute(builtin->value());
        if (attr.empty()) {
          diagnostics_.add_error("unsupported builtin");
          return false;
        }
        out << " : " << attr;
      } else if (deco->Is<ast::StructMemberOffsetDecoration>()) {
        // Nothing to do, offsets are handled at the point of access.
      } else {
        diagnostics_.add_error("unsupported struct member decoration");
        return false;
      }
    }

    out << ";" << std::endl;
  }
  decrement_indent();
  make_indent(out);

  out << "};" << std::endl;

  return true;
}

bool GeneratorImpl::EmitUnaryOp(std::ostream& pre,
                                std::ostream& out,
                                ast::UnaryOpExpression* expr) {
  switch (expr->op()) {
    case ast::UnaryOp::kNot:
      out << "!";
      break;
    case ast::UnaryOp::kNegation:
      out << "-";
      break;
  }
  out << "(";

  if (!EmitExpression(pre, out, expr->expr())) {
    return false;
  }

  out << ")";

  return true;
}

bool GeneratorImpl::EmitVariable(std::ostream& out,
                                 ast::Variable* var,
                                 bool skip_constructor) {
  make_indent(out);

  // TODO(dsinclair): Handle variable decorations
  if (!var->decorations().empty()) {
    diagnostics_.add_error("Variable decorations are not handled yet");
    return false;
  }

  std::ostringstream constructor_out;
  if (!skip_constructor && var->constructor() != nullptr) {
    constructor_out << " = ";

    std::ostringstream pre;
    if (!EmitExpression(pre, constructor_out, var->constructor())) {
      return false;
    }
    out << pre.str();
  }

  if (var->is_const()) {
    out << "const ";
  }
  auto* type = builder_.Sem().Get(var)->Type();
  if (!EmitType(out, type, builder_.Symbols().NameFor(var->symbol()))) {
    return false;
  }
  if (!type->Is<type::Array>()) {
    out << " " << builder_.Symbols().NameFor(var->symbol());
  }
  out << constructor_out.str() << ";" << std::endl;

  return true;
}

bool GeneratorImpl::EmitProgramConstVariable(std::ostream& out,
                                             const ast::Variable* var) {
  make_indent(out);

  for (auto* d : var->decorations()) {
    if (!d->Is<ast::ConstantIdDecoration>()) {
      diagnostics_.add_error("Decorated const values not valid");
      return false;
    }
  }
  if (!var->is_const()) {
    diagnostics_.add_error("Expected a const value");
    return false;
  }

  std::ostringstream constructor_out;
  if (var->constructor() != nullptr) {
    std::ostringstream pre;
    if (!EmitExpression(pre, constructor_out, var->constructor())) {
      return false;
    }
    out << pre.str();
  }

  auto* type = builder_.Sem().Get(var)->Type();

  if (var->HasConstantIdDecoration()) {
    auto const_id = var->constant_id();

    out << "#ifndef WGSL_SPEC_CONSTANT_" << const_id << std::endl;

    if (var->constructor() != nullptr) {
      out << "#define WGSL_SPEC_CONSTANT_" << const_id << " "
          << constructor_out.str() << std::endl;
    } else {
      out << "#error spec constant required for constant id " << const_id
          << std::endl;
    }
    out << "#endif" << std::endl;
    out << "static const ";
    if (!EmitType(out, type, builder_.Symbols().NameFor(var->symbol()))) {
      return false;
    }
    out << " " << builder_.Symbols().NameFor(var->symbol())
        << " = WGSL_SPEC_CONSTANT_" << const_id << ";" << std::endl;
    out << "#undef WGSL_SPEC_CONSTANT_" << const_id << std::endl;
  } else {
    out << "static const ";
    if (!EmitType(out, type, builder_.Symbols().NameFor(var->symbol()))) {
      return false;
    }
    if (!type->Is<type::Array>()) {
      out << " " << builder_.Symbols().NameFor(var->symbol());
    }

    if (var->constructor() != nullptr) {
      out << " = " << constructor_out.str();
    }
    out << ";" << std::endl;
  }

  return true;
}

std::string GeneratorImpl::get_buffer_name(ast::Expression* expr) {
  for (;;) {
    if (auto* ident = expr->As<ast::IdentifierExpression>()) {
      return builder_.Symbols().NameFor(ident->symbol());
    } else if (auto* member = expr->As<ast::MemberAccessorExpression>()) {
      expr = member->structure();
    } else if (auto* array = expr->As<ast::ArrayAccessorExpression>()) {
      expr = array->array();
    } else {
      break;
    }
  }
  return "";
}

}  // namespace hlsl
}  // namespace writer
}  // namespace tint
