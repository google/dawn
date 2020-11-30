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
#include <limits>
#include <utility>
#include <vector>

#include "src/ast/array_accessor_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/block_statement.h"
#include "src/ast/bool_literal.h"
#include "src/ast/break_statement.h"
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/case_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/else_statement.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/float_literal.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/location_decoration.h"
#include "src/ast/loop_statement.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/sint_literal.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/depth_texture_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/multisampled_texture_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/sampled_texture_type.h"
#include "src/ast/type/sampler_type.h"
#include "src/ast/type/storage_texture_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/uint_literal.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/writer/float_to_string.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

const char kInStructNameSuffix[] = "in";
const char kOutStructNameSuffix[] = "out";
const char kTintStructInVarPrefix[] = "tint_in";
const char kTintStructOutVarPrefix[] = "tint_out";

bool last_is_break_or_fallthrough(const ast::BlockStatement* stmts) {
  if (stmts->empty()) {
    return false;
  }

  return stmts->last()->Is<ast::BreakStatement>() ||
         stmts->last()->Is<ast::FallthroughStatement>();
}

uint32_t adjust_for_alignment(uint32_t count, uint32_t alignment) {
  const auto spill = count % alignment;
  if (spill == 0) {
    return count;
  }
  return count + alignment - spill;
}

}  // namespace

GeneratorImpl::GeneratorImpl(Context* ctx, ast::Module* module)
    : TextGenerator(ctx), module_(module) {}

GeneratorImpl::~GeneratorImpl() = default;

std::string GeneratorImpl::generate_name(const std::string& prefix) {
  std::string name = prefix;
  uint32_t i = 0;
  while (namer_.IsMapped(name)) {
    name = prefix + "_" + std::to_string(i);
    ++i;
  }
  namer_.RegisterRemappedName(name);
  return name;
}

bool GeneratorImpl::Generate() {
  out_ << "#include <metal_stdlib>" << std::endl << std::endl;

  for (auto* global : module_->global_variables()) {
    global_variables_.set(global->name(), global);
  }

  for (auto* const ty : module_->constructed_types()) {
    if (!EmitConstructedType(ty)) {
      return false;
    }
  }
  if (!module_->constructed_types().empty()) {
    out_ << std::endl;
  }

  for (auto* var : module_->global_variables()) {
    if (!var->is_const()) {
      continue;
    }
    if (!EmitProgramConstVariable(var)) {
      return false;
    }
  }

  // Make sure all entry point data is emitted before the entry point functions
  for (auto* func : module_->functions()) {
    if (!func->IsEntryPoint()) {
      continue;
    }

    if (!EmitEntryPointData(func)) {
      return false;
    }
  }

  for (auto* func : module_->functions()) {
    if (!EmitFunction(func)) {
      return false;
    }
  }

  for (auto* func : module_->functions()) {
    if (!func->IsEntryPoint()) {
      continue;
    }
    if (!EmitEntryPointFunction(func)) {
      return false;
    }
    out_ << std::endl;
  }

  return true;
}

uint32_t GeneratorImpl::calculate_largest_alignment(
    ast::type::StructType* type) {
  auto* stct = type->As<ast::type::StructType>()->impl();
  uint32_t largest_alignment = 0;
  for (auto* mem : stct->members()) {
    auto align = calculate_alignment_size(mem->type());
    if (align == 0) {
      return 0;
    }
    if (!mem->type()->Is<ast::type::StructType>()) {
      largest_alignment = std::max(largest_alignment, align);
    } else {
      largest_alignment = std::max(
          largest_alignment, calculate_largest_alignment(
                                 mem->type()->As<ast::type::StructType>()));
    }
  }
  return largest_alignment;
}

uint32_t GeneratorImpl::calculate_alignment_size(ast::type::Type* type) {
  if (type->Is<ast::type::AliasType>()) {
    return calculate_alignment_size(type->As<ast::type::AliasType>()->type());
  }
  if (type->Is<ast::type::ArrayType>()) {
    auto* ary = type->As<ast::type::ArrayType>();
    // TODO(dsinclair): Handle array stride and adjust for alignment.
    uint32_t type_size = calculate_alignment_size(ary->type());
    return ary->size() * type_size;
  }
  if (type->Is<ast::type::BoolType>()) {
    return 1;
  }
  if (type->Is<ast::type::PointerType>()) {
    return 0;
  }
  if (type->Is<ast::type::F32Type>() || type->Is<ast::type::I32Type>() ||
      type->Is<ast::type::U32Type>()) {
    return 4;
  }
  if (type->Is<ast::type::MatrixType>()) {
    auto* mat = type->As<ast::type::MatrixType>();
    // TODO(dsinclair): Handle MatrixStride
    // https://github.com/gpuweb/gpuweb/issues/773
    uint32_t type_size = calculate_alignment_size(mat->type());
    return mat->rows() * mat->columns() * type_size;
  }
  if (type->Is<ast::type::StructType>()) {
    auto* stct = type->As<ast::type::StructType>()->impl();
    uint32_t count = 0;
    uint32_t largest_alignment = 0;
    // Offset decorations in WGSL must be in increasing order.
    for (auto* mem : stct->members()) {
      for (auto* deco : mem->decorations()) {
        if (auto* offset = deco->As<ast::StructMemberOffsetDecoration>()) {
          count = offset->offset();
        }
      }
      auto align = calculate_alignment_size(mem->type());
      if (align == 0) {
        return 0;
      }
      if (!mem->type()->Is<ast::type::StructType>()) {
        largest_alignment = std::max(largest_alignment, align);
      } else {
        largest_alignment = std::max(
            largest_alignment, calculate_largest_alignment(
                                   mem->type()->As<ast::type::StructType>()));
      }

      // Round up to the alignment size
      count = adjust_for_alignment(count, align);
      count += align;
    }
    // Round struct up to largest align size
    count = adjust_for_alignment(count, largest_alignment);
    return count;
  }
  if (type->Is<ast::type::VectorType>()) {
    auto* vec = type->As<ast::type::VectorType>();
    uint32_t type_size = calculate_alignment_size(vec->type());
    if (vec->size() == 2) {
      return 2 * type_size;
    }
    return 4 * type_size;
  }
  return 0;
}

bool GeneratorImpl::EmitConstructedType(const ast::type::Type* ty) {
  make_indent();

  if (ty->Is<ast::type::AliasType>()) {
    auto* alias = ty->As<ast::type::AliasType>();

    out_ << "typedef ";
    if (!EmitType(alias->type(), "")) {
      return false;
    }
    out_ << " " << namer_.NameFor(alias->name()) << ";" << std::endl;
  } else if (ty->Is<ast::type::StructType>()) {
    if (!EmitStructType(ty->As<ast::type::StructType>())) {
      return false;
    }
  } else {
    error_ = "unknown alias type: " + ty->type_name();
    return false;
  }

  return true;
}

bool GeneratorImpl::EmitArrayAccessor(ast::ArrayAccessorExpression* expr) {
  if (!EmitExpression(expr->array())) {
    return false;
  }
  out_ << "[";

  if (!EmitExpression(expr->idx_expr())) {
    return false;
  }
  out_ << "]";

  return true;
}

bool GeneratorImpl::EmitBitcast(ast::BitcastExpression* expr) {
  out_ << "as_type<";
  if (!EmitType(expr->type(), "")) {
    return false;
  }

  out_ << ">(";
  if (!EmitExpression(expr->expr())) {
    return false;
  }

  out_ << ")";
  return true;
}

bool GeneratorImpl::EmitAssign(ast::AssignmentStatement* stmt) {
  make_indent();

  if (!EmitExpression(stmt->lhs())) {
    return false;
  }

  out_ << " = ";

  if (!EmitExpression(stmt->rhs())) {
    return false;
  }

  out_ << ";" << std::endl;

  return true;
}

bool GeneratorImpl::EmitBinary(ast::BinaryExpression* expr) {
  out_ << "(";

  if (!EmitExpression(expr->lhs())) {
    return false;
  }
  out_ << " ";

  switch (expr->op()) {
    case ast::BinaryOp::kAnd:
      out_ << "&";
      break;
    case ast::BinaryOp::kOr:
      out_ << "|";
      break;
    case ast::BinaryOp::kXor:
      out_ << "^";
      break;
    case ast::BinaryOp::kLogicalAnd:
      out_ << "&&";
      break;
    case ast::BinaryOp::kLogicalOr:
      out_ << "||";
      break;
    case ast::BinaryOp::kEqual:
      out_ << "==";
      break;
    case ast::BinaryOp::kNotEqual:
      out_ << "!=";
      break;
    case ast::BinaryOp::kLessThan:
      out_ << "<";
      break;
    case ast::BinaryOp::kGreaterThan:
      out_ << ">";
      break;
    case ast::BinaryOp::kLessThanEqual:
      out_ << "<=";
      break;
    case ast::BinaryOp::kGreaterThanEqual:
      out_ << ">=";
      break;
    case ast::BinaryOp::kShiftLeft:
      out_ << "<<";
      break;
    case ast::BinaryOp::kShiftRight:
      // TODO(dsinclair): MSL is based on C++14, and >> in C++14 has
      // implementation-defined behaviour for negative LHS.  We may have to
      // generate extra code to implement WGSL-specified behaviour for negative
      // LHS.
      out_ << R"(>>)";
      break;

    case ast::BinaryOp::kAdd:
      out_ << "+";
      break;
    case ast::BinaryOp::kSubtract:
      out_ << "-";
      break;
    case ast::BinaryOp::kMultiply:
      out_ << "*";
      break;
    case ast::BinaryOp::kDivide:
      out_ << "/";
      break;
    case ast::BinaryOp::kModulo:
      out_ << "%";
      break;
    case ast::BinaryOp::kNone:
      error_ = "missing binary operation type";
      return false;
  }
  out_ << " ";

  if (!EmitExpression(expr->rhs())) {
    return false;
  }

  out_ << ")";
  return true;
}

bool GeneratorImpl::EmitBreak(ast::BreakStatement*) {
  make_indent();
  out_ << "break;" << std::endl;
  return true;
}

std::string GeneratorImpl::current_ep_var_name(VarType type) {
  std::string name = "";
  switch (type) {
    case VarType::kIn: {
      auto in_it = ep_name_to_in_data_.find(current_ep_name_);
      if (in_it != ep_name_to_in_data_.end()) {
        name = in_it->second.var_name;
      }
      break;
    }
    case VarType::kOut: {
      auto out_it = ep_name_to_out_data_.find(current_ep_name_);
      if (out_it != ep_name_to_out_data_.end()) {
        name = out_it->second.var_name;
      }
      break;
    }
  }
  return name;
}

std::string GeneratorImpl::generate_intrinsic_name(ast::Intrinsic intrinsic) {
  if (intrinsic == ast::Intrinsic::kAny) {
    return "any";
  }
  if (intrinsic == ast::Intrinsic::kAll) {
    return "all";
  }
  if (intrinsic == ast::Intrinsic::kCountOneBits) {
    return "popcount";
  }
  if (intrinsic == ast::Intrinsic::kDot) {
    return "dot";
  }
  if (intrinsic == ast::Intrinsic::kDpdy ||
      intrinsic == ast::Intrinsic::kDpdyFine ||
      intrinsic == ast::Intrinsic::kDpdyCoarse) {
    return "dfdy";
  }
  if (intrinsic == ast::Intrinsic::kDpdx ||
      intrinsic == ast::Intrinsic::kDpdxFine ||
      intrinsic == ast::Intrinsic::kDpdxCoarse) {
    return "dfdx";
  }
  if (intrinsic == ast::Intrinsic::kFwidth ||
      intrinsic == ast::Intrinsic::kFwidthFine ||
      intrinsic == ast::Intrinsic::kFwidthCoarse) {
    return "fwidth";
  }
  if (intrinsic == ast::Intrinsic::kIsFinite) {
    return "isfinite";
  }
  if (intrinsic == ast::Intrinsic::kIsInf) {
    return "isinf";
  }
  if (intrinsic == ast::Intrinsic::kIsNan) {
    return "isnan";
  }
  if (intrinsic == ast::Intrinsic::kIsNormal) {
    return "isnormal";
  }
  if (intrinsic == ast::Intrinsic::kReverseBits) {
    return "reverse_bits";
  }
  if (intrinsic == ast::Intrinsic::kSelect) {
    return "select";
  }
  return "";
}

bool GeneratorImpl::EmitCall(ast::CallExpression* expr) {
  auto* ident = expr->func()->As<ast::IdentifierExpression>();

  if (ident == nullptr) {
    error_ = "invalid function name";
    return 0;
  }

  if (ident->IsIntrinsic()) {
    const auto& params = expr->params();
    if (ident->intrinsic() == ast::Intrinsic::kOuterProduct) {
      error_ = "outer_product not supported yet";
      return false;
      // TODO(dsinclair): This gets tricky. We need to generate two variables to
      // hold the outer_product expressions, but we maybe inside an expression
      // ourselves. So, this will need to, possibly, output the variables
      // _before_ the expression which contains the outer product.
      //
      // This then has the follow on, what if we have `(false &&
      // outer_product())` in that case, we shouldn't evaluate the expressions
      // at all because of short circuting.
      //
      // So .... this turns out to be hard ...

      // // We create variables to hold the two parameters in case they're
      // // function calls with side effects.
      // auto* param0 = param[0].get();
      // auto* name0 = generate_name("outer_product_expr_0");

      // auto* param1 = param[1].get();
      // auto* name1 = generate_name("outer_product_expr_1");

      // make_indent();
      // if (!EmitType(expr->result_type(), "")) {
      //   return false;
      // }
      // out_ << "(";

      // auto param1_type = params[1]->result_type()->UnwrapPtrIfNeeded();
      // if (!param1_type->Is<ast::type::VectorType>()) {
      //   error_ = "invalid param type in outer_product got: " +
      //            param1_type->type_name();
      //   return false;
      // }

      // for (uint32_t i = 0; i <
      // param1_type->As<ast::type::VectorType>()->size(); ++i) {
      //   if (i > 0) {
      //     out_ << ", ";
      //   }

      //   if (!EmitExpression(params[0].get())) {
      //     return false;
      //   }
      //   out_ << " * ";

      //   if (!EmitExpression(params[1].get())) {
      //     return false;
      //   }
      //   out_ << "[" << i << "]";
      // }

      // out_ << ")";
    } else {
      auto name = generate_intrinsic_name(ident->intrinsic());
      if (name.empty()) {
        if (ast::intrinsic::IsTextureIntrinsic(ident->intrinsic())) {
          return EmitTextureCall(expr);
        }
        name = generate_builtin_name(ident);
        if (name.empty()) {
          return false;
        }
      }

      make_indent();
      out_ << name << "(";

      bool first = true;
      for (auto* param : params) {
        if (!first) {
          out_ << ", ";
        }
        first = false;

        if (!EmitExpression(param)) {
          return false;
        }
      }

      out_ << ")";
    }
    return true;
  }

  auto name = ident->name();
  auto it = ep_func_name_remapped_.find(current_ep_name_ + "_" + name);
  if (it != ep_func_name_remapped_.end()) {
    name = it->second;
  }

  auto* func = module_->FindFunctionByName(ident->name());
  if (func == nullptr) {
    error_ = "Unable to find function: " + name;
    return false;
  }

  out_ << name << "(";

  bool first = true;
  if (has_referenced_in_var_needing_struct(func)) {
    auto var_name = current_ep_var_name(VarType::kIn);
    if (!var_name.empty()) {
      out_ << var_name;
      first = false;
    }
  }
  if (has_referenced_out_var_needing_struct(func)) {
    auto var_name = current_ep_var_name(VarType::kOut);
    if (!var_name.empty()) {
      if (!first) {
        out_ << ", ";
      }
      first = false;
      out_ << var_name;
    }
  }

  for (const auto& data : func->referenced_builtin_variables()) {
    auto* var = data.first;
    if (var->storage_class() != ast::StorageClass::kInput) {
      continue;
    }
    if (!first) {
      out_ << ", ";
    }
    first = false;
    out_ << var->name();
  }

  for (const auto& data : func->referenced_uniform_variables()) {
    auto* var = data.first;
    if (!first) {
      out_ << ", ";
    }
    first = false;
    out_ << var->name();
  }

  for (const auto& data : func->referenced_storagebuffer_variables()) {
    auto* var = data.first;
    if (!first) {
      out_ << ", ";
    }
    first = false;
    out_ << var->name();
  }

  const auto& params = expr->params();
  for (auto* param : params) {
    if (!first) {
      out_ << ", ";
    }
    first = false;

    if (!EmitExpression(param)) {
      return false;
    }
  }

  out_ << ")";

  return true;
}

bool GeneratorImpl::EmitTextureCall(ast::CallExpression* expr) {
  auto* ident = expr->func()->As<ast::IdentifierExpression>();

  auto params = expr->params();
  auto* signature = static_cast<const ast::intrinsic::TextureSignature*>(
      ident->intrinsic_signature());
  auto& pidx = signature->params.idx;
  auto const kNotUsed = ast::intrinsic::TextureSignature::Parameters::kNotUsed;

  if (!EmitExpression(params[pidx.texture]))
    return false;

  switch (ident->intrinsic()) {
    case ast::Intrinsic::kTextureSample:
    case ast::Intrinsic::kTextureSampleBias:
    case ast::Intrinsic::kTextureSampleLevel:
    case ast::Intrinsic::kTextureSampleGrad:
      out_ << ".sample(";
      break;
    case ast::Intrinsic::kTextureSampleCompare:
      out_ << ".sample_compare(";
      break;
    default:
      error_ = "Internal compiler error: Unhandled texture intrinsic '" +
               ident->name() + "'";
      break;
  }

  if (!EmitExpression(params[pidx.sampler])) {
    return false;
  }

  for (auto idx : {pidx.coords, pidx.array_index, pidx.depth_ref}) {
    if (idx != kNotUsed) {
      out_ << ", ";
      if (!EmitExpression(params[idx]))
        return false;
    }
  }

  if (pidx.bias != kNotUsed) {
    out_ << ", bias(";
    if (!EmitExpression(params[pidx.bias])) {
      return false;
    }
    out_ << ")";
  }
  if (pidx.level != kNotUsed) {
    out_ << ", level(";
    if (!EmitExpression(params[pidx.level])) {
      return false;
    }
    out_ << ")";
  }
  if (pidx.ddx != kNotUsed) {
    auto dim = params[pidx.texture]
                   ->result_type()
                   ->UnwrapPtrIfNeeded()
                   ->As<ast::type::TextureType>()
                   ->dim();
    switch (dim) {
      case ast::type::TextureDimension::k2d:
      case ast::type::TextureDimension::k2dArray:
        out_ << ", gradient2d(";
        break;
      case ast::type::TextureDimension::k3d:
        out_ << ", gradient3d(";
        break;
      case ast::type::TextureDimension::kCube:
      case ast::type::TextureDimension::kCubeArray:
        out_ << ", gradientcube(";
        break;
      default: {
        std::stringstream err;
        err << "MSL does not support gradients for " << dim << " textures";
        error_ = err.str();
        return false;
      }
    }
    if (!EmitExpression(params[pidx.ddx])) {
      return false;
    }
    out_ << ", ";
    if (!EmitExpression(params[pidx.ddy])) {
      return false;
    }
    out_ << ")";
  }

  if (pidx.offset != kNotUsed) {
    out_ << ", ";
    if (!EmitExpression(params[pidx.offset])) {
      return false;
    }
  }

  out_ << ")";

  return true;
}

std::string GeneratorImpl::generate_builtin_name(
    ast::IdentifierExpression* ident) {
  std::string out = "metal::";
  switch (ident->intrinsic()) {
    case ast::Intrinsic::kAcos:
    case ast::Intrinsic::kAsin:
    case ast::Intrinsic::kAtan:
    case ast::Intrinsic::kAtan2:
    case ast::Intrinsic::kCeil:
    case ast::Intrinsic::kCos:
    case ast::Intrinsic::kCosh:
    case ast::Intrinsic::kCross:
    case ast::Intrinsic::kDeterminant:
    case ast::Intrinsic::kDistance:
    case ast::Intrinsic::kExp:
    case ast::Intrinsic::kExp2:
    case ast::Intrinsic::kFloor:
    case ast::Intrinsic::kFma:
    case ast::Intrinsic::kFract:
    case ast::Intrinsic::kLength:
    case ast::Intrinsic::kLog:
    case ast::Intrinsic::kLog2:
    case ast::Intrinsic::kMix:
    case ast::Intrinsic::kNormalize:
    case ast::Intrinsic::kPow:
    case ast::Intrinsic::kReflect:
    case ast::Intrinsic::kRound:
    case ast::Intrinsic::kSin:
    case ast::Intrinsic::kSinh:
    case ast::Intrinsic::kSqrt:
    case ast::Intrinsic::kStep:
    case ast::Intrinsic::kTan:
    case ast::Intrinsic::kTanh:
    case ast::Intrinsic::kTrunc:
    case ast::Intrinsic::kSign:
    case ast::Intrinsic::kClamp:
      out += ident->name();
      break;
    case ast::Intrinsic::kAbs:
      if (ident->result_type()->Is<ast::type::F32Type>()) {
        out += "fabs";
      } else if (ident->result_type()->Is<ast::type::U32Type>() ||
                 ident->result_type()->Is<ast::type::I32Type>()) {
        out += "abs";
      }
      break;
    case ast::Intrinsic::kMax:
      if (ident->result_type()->Is<ast::type::F32Type>()) {
        out += "fmax";
      } else if (ident->result_type()->Is<ast::type::U32Type>() ||
                 ident->result_type()->Is<ast::type::I32Type>()) {
        out += "max";
      }
      break;
    case ast::Intrinsic::kMin:
      if (ident->result_type()->Is<ast::type::F32Type>()) {
        out += "fmin";
      } else if (ident->result_type()->Is<ast::type::U32Type>() ||
                 ident->result_type()->Is<ast::type::I32Type>()) {
        out += "min";
      }
      break;
    case ast::Intrinsic::kFaceForward:
      out += "faceforward";
      break;
    case ast::Intrinsic::kSmoothStep:
      out += "smoothstep";
      break;
    case ast::Intrinsic::kInverseSqrt:
      out += "rsqrt";
      break;
    default:
      error_ = "Unknown import method: " + ident->name();
      return "";
  }
  return out;
}

bool GeneratorImpl::EmitCase(ast::CaseStatement* stmt) {
  make_indent();

  if (stmt->IsDefault()) {
    out_ << "default:";
  } else {
    bool first = true;
    for (auto* selector : stmt->selectors()) {
      if (!first) {
        out_ << std::endl;
        make_indent();
      }
      first = false;

      out_ << "case ";
      if (!EmitLiteral(selector)) {
        return false;
      }
      out_ << ":";
    }
  }

  out_ << " {" << std::endl;

  increment_indent();

  for (auto* s : *stmt->body()) {
    if (!EmitStatement(s)) {
      return false;
    }
  }

  if (!last_is_break_or_fallthrough(stmt->body())) {
    make_indent();
    out_ << "break;" << std::endl;
  }

  decrement_indent();
  make_indent();
  out_ << "}" << std::endl;

  return true;
}

bool GeneratorImpl::EmitConstructor(ast::ConstructorExpression* expr) {
  if (auto* scalar = expr->As<ast::ScalarConstructorExpression>()) {
    return EmitScalarConstructor(scalar);
  }
  return EmitTypeConstructor(expr->As<ast::TypeConstructorExpression>());
}

bool GeneratorImpl::EmitContinue(ast::ContinueStatement*) {
  make_indent();
  out_ << "continue;" << std::endl;
  return true;
}

bool GeneratorImpl::EmitTypeConstructor(ast::TypeConstructorExpression* expr) {
  if (expr->type()->Is<ast::type::ArrayType>()) {
    out_ << "{";
  } else {
    if (!EmitType(expr->type(), "")) {
      return false;
    }
    out_ << "(";
  }

  // If the type constructor is empty then we need to construct with the zero
  // value for all components.
  if (expr->values().empty()) {
    if (!EmitZeroValue(expr->type())) {
      return false;
    }
  } else {
    bool first = true;
    for (auto* e : expr->values()) {
      if (!first) {
        out_ << ", ";
      }
      first = false;

      if (!EmitExpression(e)) {
        return false;
      }
    }
  }

  if (expr->type()->Is<ast::type::ArrayType>()) {
    out_ << "}";
  } else {
    out_ << ")";
  }
  return true;
}

bool GeneratorImpl::EmitZeroValue(ast::type::Type* type) {
  if (type->Is<ast::type::BoolType>()) {
    out_ << "false";
  } else if (type->Is<ast::type::F32Type>()) {
    out_ << "0.0f";
  } else if (type->Is<ast::type::I32Type>()) {
    out_ << "0";
  } else if (type->Is<ast::type::U32Type>()) {
    out_ << "0u";
  } else if (type->Is<ast::type::VectorType>()) {
    return EmitZeroValue(type->As<ast::type::VectorType>()->type());
  } else if (type->Is<ast::type::MatrixType>()) {
    return EmitZeroValue(type->As<ast::type::MatrixType>()->type());
  } else if (type->Is<ast::type::ArrayType>()) {
    out_ << "{";
    if (!EmitZeroValue(type->As<ast::type::ArrayType>()->type())) {
      return false;
    }
    out_ << "}";
  } else if (type->Is<ast::type::StructType>()) {
    out_ << "{}";
  } else {
    error_ = "Invalid type for zero emission: " + type->type_name();
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitScalarConstructor(
    ast::ScalarConstructorExpression* expr) {
  return EmitLiteral(expr->literal());
}

bool GeneratorImpl::EmitLiteral(ast::Literal* lit) {
  if (lit->Is<ast::BoolLiteral>()) {
    out_ << (lit->As<ast::BoolLiteral>()->IsTrue() ? "true" : "false");
  } else if (lit->Is<ast::FloatLiteral>()) {
    out_ << FloatToString(lit->As<ast::FloatLiteral>()->value()) << "f";
  } else if (lit->Is<ast::SintLiteral>()) {
    out_ << lit->As<ast::SintLiteral>()->value();
  } else if (lit->Is<ast::UintLiteral>()) {
    out_ << lit->As<ast::UintLiteral>()->value() << "u";
  } else {
    error_ = "unknown literal type";
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitEntryPointData(ast::Function* func) {
  std::vector<std::pair<ast::Variable*, uint32_t>> in_locations;
  std::vector<std::pair<ast::Variable*, ast::VariableDecoration*>>
      out_variables;
  for (auto data : func->referenced_location_variables()) {
    auto* var = data.first;
    auto* deco = data.second;

    if (var->storage_class() == ast::StorageClass::kInput) {
      in_locations.push_back({var, deco->value()});
    } else if (var->storage_class() == ast::StorageClass::kOutput) {
      out_variables.push_back({var, deco});
    }
  }

  for (auto data : func->referenced_builtin_variables()) {
    auto* var = data.first;
    auto* deco = data.second;

    if (var->storage_class() == ast::StorageClass::kOutput) {
      out_variables.push_back({var, deco});
    }
  }

  if (!in_locations.empty()) {
    auto in_struct_name =
        generate_name(func->name() + "_" + kInStructNameSuffix);
    auto in_var_name = generate_name(kTintStructInVarPrefix);
    ep_name_to_in_data_[func->name()] = {in_struct_name, in_var_name};

    make_indent();
    out_ << "struct " << in_struct_name << " {" << std::endl;

    increment_indent();

    for (auto& data : in_locations) {
      auto* var = data.first;
      uint32_t loc = data.second;

      make_indent();
      if (!EmitType(var->type(), var->name())) {
        return false;
      }

      out_ << " " << var->name() << " [[";
      if (func->pipeline_stage() == ast::PipelineStage::kVertex) {
        out_ << "attribute(" << loc << ")";
      } else if (func->pipeline_stage() == ast::PipelineStage::kFragment) {
        out_ << "user(locn" << loc << ")";
      } else {
        error_ = "invalid location variable for pipeline stage";
        return false;
      }
      out_ << "]];" << std::endl;
    }
    decrement_indent();
    make_indent();

    out_ << "};" << std::endl << std::endl;
  }

  if (!out_variables.empty()) {
    auto out_struct_name =
        generate_name(func->name() + "_" + kOutStructNameSuffix);
    auto out_var_name = generate_name(kTintStructOutVarPrefix);
    ep_name_to_out_data_[func->name()] = {out_struct_name, out_var_name};

    make_indent();
    out_ << "struct " << out_struct_name << " {" << std::endl;

    increment_indent();
    for (auto& data : out_variables) {
      auto* var = data.first;
      auto* deco = data.second;

      make_indent();
      if (!EmitType(var->type(), var->name())) {
        return false;
      }

      out_ << " " << var->name() << " [[";

      if (auto* location = deco->As<ast::LocationDecoration>()) {
        auto loc = location->value();
        if (func->pipeline_stage() == ast::PipelineStage::kVertex) {
          out_ << "user(locn" << loc << ")";
        } else if (func->pipeline_stage() == ast::PipelineStage::kFragment) {
          out_ << "color(" << loc << ")";
        } else {
          error_ = "invalid location variable for pipeline stage";
          return false;
        }
      } else if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
        auto attr = builtin_to_attribute(builtin->value());
        if (attr.empty()) {
          error_ = "unsupported builtin";
          return false;
        }
        out_ << attr;
      } else {
        error_ = "unsupported variable decoration for entry point output";
        return false;
      }
      out_ << "]];" << std::endl;
    }
    decrement_indent();
    make_indent();
    out_ << "};" << std::endl << std::endl;
  }

  return true;
}

bool GeneratorImpl::EmitExpression(ast::Expression* expr) {
  if (auto* a = expr->As<ast::ArrayAccessorExpression>()) {
    return EmitArrayAccessor(a);
  }
  if (auto* b = expr->As<ast::BinaryExpression>()) {
    return EmitBinary(b);
  }
  if (auto* b = expr->As<ast::BitcastExpression>()) {
    return EmitBitcast(b);
  }
  if (auto* c = expr->As<ast::CallExpression>()) {
    return EmitCall(c);
  }
  if (auto* c = expr->As<ast::ConstructorExpression>()) {
    return EmitConstructor(c);
  }
  if (auto* i = expr->As<ast::IdentifierExpression>()) {
    return EmitIdentifier(i);
  }
  if (auto* m = expr->As<ast::MemberAccessorExpression>()) {
    return EmitMemberAccessor(m);
  }
  if (auto* u = expr->As<ast::UnaryOpExpression>()) {
    return EmitUnaryOp(u);
  }

  error_ = "unknown expression type: " + expr->str();
  return false;
}

void GeneratorImpl::EmitStage(ast::PipelineStage stage) {
  switch (stage) {
    case ast::PipelineStage::kFragment:
      out_ << "fragment";
      break;
    case ast::PipelineStage::kVertex:
      out_ << "vertex";
      break;
    case ast::PipelineStage::kCompute:
      out_ << "kernel";
      break;
    case ast::PipelineStage::kNone:
      break;
  }
  return;
}

bool GeneratorImpl::has_referenced_in_var_needing_struct(ast::Function* func) {
  for (auto data : func->referenced_location_variables()) {
    auto* var = data.first;
    if (var->storage_class() == ast::StorageClass::kInput) {
      return true;
    }
  }
  return false;
}

bool GeneratorImpl::has_referenced_out_var_needing_struct(ast::Function* func) {
  for (auto data : func->referenced_location_variables()) {
    auto* var = data.first;
    if (var->storage_class() == ast::StorageClass::kOutput) {
      return true;
    }
  }

  for (auto data : func->referenced_builtin_variables()) {
    auto* var = data.first;
    if (var->storage_class() == ast::StorageClass::kOutput) {
      return true;
    }
  }
  return false;
}

bool GeneratorImpl::has_referenced_var_needing_struct(ast::Function* func) {
  return has_referenced_in_var_needing_struct(func) ||
         has_referenced_out_var_needing_struct(func);
}

bool GeneratorImpl::EmitFunction(ast::Function* func) {
  make_indent();

  // Entry points will be emitted later, skip for now.
  if (func->IsEntryPoint()) {
    return true;
  }

  // TODO(dsinclair): This could be smarter. If the input/outputs for multiple
  // entry points are the same we could generate a single struct and then have
  // this determine it's the same struct and just emit once.
  bool emit_duplicate_functions = func->ancestor_entry_points().size() > 0 &&
                                  has_referenced_var_needing_struct(func);

  if (emit_duplicate_functions) {
    for (const auto& ep_name : func->ancestor_entry_points()) {
      if (!EmitFunctionInternal(func, emit_duplicate_functions, ep_name)) {
        return false;
      }
      out_ << std::endl;
    }
  } else {
    // Emit as non-duplicated
    if (!EmitFunctionInternal(func, false, "")) {
      return false;
    }
    out_ << std::endl;
  }

  return true;
}

bool GeneratorImpl::EmitFunctionInternal(ast::Function* func,
                                         bool emit_duplicate_functions,
                                         const std::string& ep_name) {
  auto name = func->name();

  if (!EmitType(func->return_type(), "")) {
    return false;
  }

  out_ << " ";
  if (emit_duplicate_functions) {
    name = generate_name(name + "_" + ep_name);
    ep_func_name_remapped_[ep_name + "_" + func->name()] = name;
  } else {
    name = namer_.NameFor(name);
  }
  out_ << name << "(";

  bool first = true;

  // If we're emitting duplicate functions that means the function takes
  // the stage_in or stage_out value from the entry point, emit them.
  //
  // We emit both of them if they're there regardless of if they're both used.
  if (emit_duplicate_functions) {
    auto in_it = ep_name_to_in_data_.find(ep_name);
    if (in_it != ep_name_to_in_data_.end()) {
      out_ << "thread " << in_it->second.struct_name << "& "
           << in_it->second.var_name;
      first = false;
    }

    auto out_it = ep_name_to_out_data_.find(ep_name);
    if (out_it != ep_name_to_out_data_.end()) {
      if (!first) {
        out_ << ", ";
      }
      out_ << "thread " << out_it->second.struct_name << "& "
           << out_it->second.var_name;
      first = false;
    }
  }

  for (const auto& data : func->referenced_builtin_variables()) {
    auto* var = data.first;
    if (var->storage_class() != ast::StorageClass::kInput) {
      continue;
    }
    if (!first) {
      out_ << ", ";
    }
    first = false;

    out_ << "thread ";
    if (!EmitType(var->type(), "")) {
      return false;
    }
    out_ << "& " << var->name();
  }

  for (const auto& data : func->referenced_uniform_variables()) {
    auto* var = data.first;
    if (!first) {
      out_ << ", ";
    }
    first = false;

    out_ << "constant ";
    // TODO(dsinclair): Can arrays be uniform? If so, fix this ...
    if (!EmitType(var->type(), "")) {
      return false;
    }
    out_ << "& " << var->name();
  }

  for (const auto& data : func->referenced_storagebuffer_variables()) {
    auto* var = data.first;
    if (!first) {
      out_ << ", ";
    }
    first = false;

    if (!var->type()->Is<ast::type::AccessControlType>()) {
      error_ = "invalid type for storage buffer, expected access control";
      return false;
    }
    auto* ac = var->type()->As<ast::type::AccessControlType>();
    if (ac->IsReadOnly()) {
      out_ << "const ";
    }

    out_ << "device ";
    if (!EmitType(ac->type(), "")) {
      return false;
    }
    out_ << "& " << var->name();
  }

  for (auto* v : func->params()) {
    if (!first) {
      out_ << ", ";
    }
    first = false;

    if (!EmitType(v->type(), v->name())) {
      return false;
    }
    // Array name is output as part of the type
    if (!v->type()->Is<ast::type::ArrayType>()) {
      out_ << " " << v->name();
    }
  }

  out_ << ") ";

  current_ep_name_ = ep_name;

  if (!EmitBlockAndNewline(func->body())) {
    return false;
  }

  current_ep_name_ = "";

  return true;
}

std::string GeneratorImpl::builtin_to_attribute(ast::Builtin builtin) const {
  switch (builtin) {
    case ast::Builtin::kPosition:
      return "position";
    case ast::Builtin::kVertexIdx:
      return "vertex_id";
    case ast::Builtin::kInstanceIdx:
      return "instance_id";
    case ast::Builtin::kFrontFacing:
      return "front_facing";
    case ast::Builtin::kFragCoord:
      return "position";
    case ast::Builtin::kFragDepth:
      return "depth(any)";
    case ast::Builtin::kLocalInvocationId:
      return "thread_position_in_threadgroup";
    case ast::Builtin::kLocalInvocationIdx:
      return "thread_index_in_threadgroup";
    case ast::Builtin::kGlobalInvocationId:
      return "thread_position_in_grid";
    default:
      break;
  }
  return "";
}

bool GeneratorImpl::EmitEntryPointFunction(ast::Function* func) {
  make_indent();

  current_ep_name_ = func->name();

  EmitStage(func->pipeline_stage());
  out_ << " ";

  // This is an entry point, the return type is the entry point output structure
  // if one exists, or void otherwise.
  auto out_data = ep_name_to_out_data_.find(current_ep_name_);
  bool has_out_data = out_data != ep_name_to_out_data_.end();
  if (has_out_data) {
    out_ << out_data->second.struct_name;
  } else {
    out_ << "void";
  }
  out_ << " " << namer_.NameFor(current_ep_name_) << "(";

  bool first = true;
  auto in_data = ep_name_to_in_data_.find(current_ep_name_);
  if (in_data != ep_name_to_in_data_.end()) {
    out_ << in_data->second.struct_name << " " << in_data->second.var_name
         << " [[stage_in]]";
    first = false;
  }

  for (auto data : func->referenced_builtin_variables()) {
    auto* var = data.first;
    if (var->storage_class() != ast::StorageClass::kInput) {
      continue;
    }

    if (!first) {
      out_ << ", ";
    }
    first = false;

    auto* builtin = data.second;

    if (!EmitType(var->type(), "")) {
      return false;
    }

    auto attr = builtin_to_attribute(builtin->value());
    if (attr.empty()) {
      error_ = "unknown builtin";
      return false;
    }
    out_ << " " << var->name() << " [[" << attr << "]]";
  }

  for (auto data : func->referenced_uniform_variables()) {
    if (!first) {
      out_ << ", ";
    }
    first = false;

    auto* var = data.first;
    // TODO(dsinclair): We're using the binding to make up the buffer number but
    // we should instead be using a provided mapping that uses both buffer and
    // set. https://bugs.chromium.org/p/tint/issues/detail?id=104
    auto* binding = data.second.binding;
    if (binding == nullptr) {
      error_ = "unable to find binding information for uniform: " + var->name();
      return false;
    }
    // auto* set = data.second.set;

    out_ << "constant ";
    // TODO(dsinclair): Can you have a uniform array? If so, this needs to be
    // updated to handle arrays property.
    if (!EmitType(var->type(), "")) {
      return false;
    }
    out_ << "& " << var->name() << " [[buffer(" << binding->value() << ")]]";
  }

  for (auto data : func->referenced_storagebuffer_variables()) {
    if (!first) {
      out_ << ", ";
    }
    first = false;

    auto* var = data.first;
    // TODO(dsinclair): We're using the binding to make up the buffer number but
    // we should instead be using a provided mapping that uses both buffer and
    // set. https://bugs.chromium.org/p/tint/issues/detail?id=104
    auto* binding = data.second.binding;
    // auto* set = data.second.set;

    if (!var->type()->Is<ast::type::AccessControlType>()) {
      error_ = "invalid type for storage buffer, expected access control";
      return false;
    }
    auto* ac = var->type()->As<ast::type::AccessControlType>();
    if (ac->IsReadOnly()) {
      out_ << "const ";
    }

    out_ << "device ";
    if (!EmitType(ac->type(), "")) {
      return false;
    }
    out_ << "& " << var->name() << " [[buffer(" << binding->value() << ")]]";
  }

  out_ << ") {" << std::endl;

  increment_indent();

  if (has_out_data) {
    make_indent();
    out_ << out_data->second.struct_name << " " << out_data->second.var_name
         << " = {};" << std::endl;
  }

  generating_entry_point_ = true;
  for (auto* s : *func->body()) {
    if (!EmitStatement(s)) {
      return false;
    }
  }
  generating_entry_point_ = false;

  decrement_indent();
  make_indent();
  out_ << "}" << std::endl;

  current_ep_name_ = "";
  return true;
}

bool GeneratorImpl::global_is_in_struct(ast::Variable* var) const {
  bool in_or_out_struct_has_location =
      var->Is<ast::DecoratedVariable>() &&
      var->As<ast::DecoratedVariable>()->HasLocationDecoration() &&
      (var->storage_class() == ast::StorageClass::kInput ||
       var->storage_class() == ast::StorageClass::kOutput);
  bool in_struct_has_builtin =
      var->Is<ast::DecoratedVariable>() &&
      var->As<ast::DecoratedVariable>()->HasBuiltinDecoration() &&
      var->storage_class() == ast::StorageClass::kOutput;
  return in_or_out_struct_has_location || in_struct_has_builtin;
}

bool GeneratorImpl::EmitIdentifier(ast::IdentifierExpression* expr) {
  auto* ident = expr->As<ast::IdentifierExpression>();
  ast::Variable* var = nullptr;
  if (global_variables_.get(ident->name(), &var)) {
    if (global_is_in_struct(var)) {
      auto var_type = var->storage_class() == ast::StorageClass::kInput
                          ? VarType::kIn
                          : VarType::kOut;
      auto name = current_ep_var_name(var_type);
      if (name.empty()) {
        error_ = "unable to find entry point data for variable";
        return false;
      }
      out_ << name << ".";
    }
  }
  out_ << namer_.NameFor(ident->name());

  return true;
}

bool GeneratorImpl::EmitLoop(ast::LoopStatement* stmt) {
  loop_emission_counter_++;

  std::string guard = namer_.NameFor("tint_msl_is_first_" +
                                     std::to_string(loop_emission_counter_));

  if (stmt->has_continuing()) {
    make_indent();

    // Continuing variables get their own scope.
    out_ << "{" << std::endl;
    increment_indent();

    make_indent();
    out_ << "bool " << guard << " = true;" << std::endl;

    // A continuing block may use variables declared in the method body. As a
    // first pass, if we have a continuing, we pull all declarations outside
    // the for loop into the continuing scope. Then, the variable declarations
    // will be turned into assignments.
    for (auto* s : *(stmt->body())) {
      if (auto* decl = s->As<ast::VariableDeclStatement>()) {
        if (!EmitVariable(decl->variable(), true)) {
          return false;
        }
      }
    }
  }

  make_indent();
  out_ << "for(;;) {" << std::endl;
  increment_indent();

  if (stmt->has_continuing()) {
    make_indent();
    out_ << "if (!" << guard << ") ";

    if (!EmitBlockAndNewline(stmt->continuing())) {
      return false;
    }

    make_indent();
    out_ << guard << " = false;" << std::endl;
    out_ << std::endl;
  }

  for (auto* s : *(stmt->body())) {
    // If we have a continuing block we've already emitted the variable
    // declaration before the loop, so treat it as an assignment.
    auto* decl = s->As<ast::VariableDeclStatement>();
    if (decl != nullptr && stmt->has_continuing()) {
      make_indent();

      auto* var = decl->variable();
      out_ << var->name() << " = ";
      if (var->constructor() != nullptr) {
        if (!EmitExpression(var->constructor())) {
          return false;
        }
      } else {
        if (!EmitZeroValue(var->type())) {
          return false;
        }
      }
      out_ << ";" << std::endl;
      continue;
    }

    if (!EmitStatement(s)) {
      return false;
    }
  }

  decrement_indent();
  make_indent();
  out_ << "}" << std::endl;

  // Close the scope for any continuing variables.
  if (stmt->has_continuing()) {
    decrement_indent();
    make_indent();
    out_ << "}" << std::endl;
  }

  return true;
}

bool GeneratorImpl::EmitDiscard(ast::DiscardStatement*) {
  make_indent();
  // TODO(dsinclair): Verify this is correct when the discard semantics are
  // defined for WGSL (https://github.com/gpuweb/gpuweb/issues/361)
  out_ << "discard_fragment();" << std::endl;
  return true;
}

bool GeneratorImpl::EmitElse(ast::ElseStatement* stmt) {
  if (stmt->HasCondition()) {
    out_ << " else if (";
    if (!EmitExpression(stmt->condition())) {
      return false;
    }
    out_ << ") ";
  } else {
    out_ << " else ";
  }

  return EmitBlock(stmt->body());
}

bool GeneratorImpl::EmitIf(ast::IfStatement* stmt) {
  make_indent();

  out_ << "if (";
  if (!EmitExpression(stmt->condition())) {
    return false;
  }
  out_ << ") ";

  if (!EmitBlock(stmt->body())) {
    return false;
  }

  for (auto* e : stmt->else_statements()) {
    if (!EmitElse(e)) {
      return false;
    }
  }
  out_ << std::endl;

  return true;
}

bool GeneratorImpl::EmitMemberAccessor(ast::MemberAccessorExpression* expr) {
  if (!EmitExpression(expr->structure())) {
    return false;
  }

  out_ << ".";

  return EmitExpression(expr->member());
}

bool GeneratorImpl::EmitReturn(ast::ReturnStatement* stmt) {
  make_indent();

  out_ << "return";

  if (generating_entry_point_) {
    auto out_data = ep_name_to_out_data_.find(current_ep_name_);
    if (out_data != ep_name_to_out_data_.end()) {
      out_ << " " << out_data->second.var_name;
    }
  } else if (stmt->has_value()) {
    out_ << " ";
    if (!EmitExpression(stmt->value())) {
      return false;
    }
  }
  out_ << ";" << std::endl;
  return true;
}

bool GeneratorImpl::EmitBlock(const ast::BlockStatement* stmt) {
  out_ << "{" << std::endl;
  increment_indent();

  for (auto* s : *stmt) {
    if (!EmitStatement(s)) {
      return false;
    }
  }

  decrement_indent();
  make_indent();
  out_ << "}";

  return true;
}

bool GeneratorImpl::EmitBlockAndNewline(const ast::BlockStatement* stmt) {
  const bool result = EmitBlock(stmt);
  if (result) {
    out_ << std::endl;
  }
  return result;
}

bool GeneratorImpl::EmitIndentedBlockAndNewline(ast::BlockStatement* stmt) {
  make_indent();
  const bool result = EmitBlock(stmt);
  if (result) {
    out_ << std::endl;
  }
  return result;
}

bool GeneratorImpl::EmitStatement(ast::Statement* stmt) {
  if (auto* a = stmt->As<ast::AssignmentStatement>()) {
    return EmitAssign(a);
  }
  if (auto* b = stmt->As<ast::BlockStatement>()) {
    return EmitIndentedBlockAndNewline(b);
  }
  if (auto* b = stmt->As<ast::BreakStatement>()) {
    return EmitBreak(b);
  }
  if (auto* c = stmt->As<ast::CallStatement>()) {
    make_indent();
    if (!EmitCall(c->expr())) {
      return false;
    }
    out_ << ";" << std::endl;
    return true;
  }
  if (auto* c = stmt->As<ast::ContinueStatement>()) {
    return EmitContinue(c);
  }
  if (auto* d = stmt->As<ast::DiscardStatement>()) {
    return EmitDiscard(d);
  }
  if (auto* f = stmt->As<ast::FallthroughStatement>()) {
    make_indent();
    out_ << "/* fallthrough */" << std::endl;
    return true;
  }
  if (auto* i = stmt->As<ast::IfStatement>()) {
    return EmitIf(i);
  }
  if (auto* l = stmt->As<ast::LoopStatement>()) {
    return EmitLoop(l);
  }
  if (auto* r = stmt->As<ast::ReturnStatement>()) {
    return EmitReturn(r);
  }
  if (auto* s = stmt->As<ast::SwitchStatement>()) {
    return EmitSwitch(s);
  }
  if (auto* v = stmt->As<ast::VariableDeclStatement>()) {
    return EmitVariable(v->variable(), false);
  }

  error_ = "unknown statement type: " + stmt->str();
  return false;
}

bool GeneratorImpl::EmitSwitch(ast::SwitchStatement* stmt) {
  make_indent();

  out_ << "switch(";
  if (!EmitExpression(stmt->condition())) {
    return false;
  }
  out_ << ") {" << std::endl;

  increment_indent();

  for (auto* s : stmt->body()) {
    if (!EmitCase(s)) {
      return false;
    }
  }

  decrement_indent();
  make_indent();
  out_ << "}" << std::endl;

  return true;
}

bool GeneratorImpl::EmitType(ast::type::Type* type, const std::string& name) {
  if (type->Is<ast::type::AliasType>()) {
    auto* alias = type->As<ast::type::AliasType>();
    out_ << namer_.NameFor(alias->name());
  } else if (type->Is<ast::type::ArrayType>()) {
    auto* ary = type->As<ast::type::ArrayType>();

    ast::type::Type* base_type = ary;
    std::vector<uint32_t> sizes;
    while (base_type->Is<ast::type::ArrayType>()) {
      if (base_type->As<ast::type::ArrayType>()->IsRuntimeArray()) {
        sizes.push_back(1);
      } else {
        sizes.push_back(base_type->As<ast::type::ArrayType>()->size());
      }
      base_type = base_type->As<ast::type::ArrayType>()->type();
    }
    if (!EmitType(base_type, "")) {
      return false;
    }
    if (!name.empty()) {
      out_ << " " << namer_.NameFor(name);
    }
    for (uint32_t size : sizes) {
      out_ << "[" << size << "]";
    }
  } else if (type->Is<ast::type::BoolType>()) {
    out_ << "bool";
  } else if (type->Is<ast::type::F32Type>()) {
    out_ << "float";
  } else if (type->Is<ast::type::I32Type>()) {
    out_ << "int";
  } else if (type->Is<ast::type::MatrixType>()) {
    auto* mat = type->As<ast::type::MatrixType>();
    if (!EmitType(mat->type(), "")) {
      return false;
    }
    out_ << mat->columns() << "x" << mat->rows();
  } else if (type->Is<ast::type::PointerType>()) {
    auto* ptr = type->As<ast::type::PointerType>();
    // TODO(dsinclair): Storage class?
    if (!EmitType(ptr->type(), "")) {
      return false;
    }
    out_ << "*";
  } else if (type->Is<ast::type::SamplerType>()) {
    out_ << "sampler";
  } else if (type->Is<ast::type::StructType>()) {
    // The struct type emits as just the name. The declaration would be emitted
    // as part of emitting the constructed types.
    out_ << type->As<ast::type::StructType>()->name();
  } else if (type->Is<ast::type::TextureType>()) {
    auto* tex = type->As<ast::type::TextureType>();

    if (tex->Is<ast::type::DepthTextureType>()) {
      out_ << "depth";
    } else {
      out_ << "texture";
    }

    switch (tex->dim()) {
      case ast::type::TextureDimension::k1d:
        out_ << "1d";
        break;
      case ast::type::TextureDimension::k1dArray:
        out_ << "1d_array";
        break;
      case ast::type::TextureDimension::k2d:
        out_ << "2d";
        break;
      case ast::type::TextureDimension::k2dArray:
        out_ << "2d_array";
        break;
      case ast::type::TextureDimension::k3d:
        out_ << "3d";
        break;
      case ast::type::TextureDimension::kCube:
        out_ << "cube";
        break;
      case ast::type::TextureDimension::kCubeArray:
        out_ << "cube_array";
        break;
      default:
        error_ = "Invalid texture dimensions";
        return false;
    }
    if (tex->Is<ast::type::MultisampledTextureType>()) {
      out_ << "_ms";
    }
    out_ << "<";
    if (tex->Is<ast::type::DepthTextureType>()) {
      out_ << "float, access::sample";
    } else if (tex->Is<ast::type::StorageTextureType>()) {
      auto* storage = tex->As<ast::type::StorageTextureType>();
      if (!EmitType(storage->type(), "")) {
        return false;
      }
      out_ << ", access::";
      if (storage->access() == ast::AccessControl::kReadOnly) {
        out_ << "read";
      } else if (storage->access() == ast::AccessControl::kWriteOnly) {
        out_ << "write";
      } else {
        error_ = "Invalid access control for storage texture";
        return false;
      }
    } else if (tex->Is<ast::type::MultisampledTextureType>()) {
      if (!EmitType(tex->As<ast::type::MultisampledTextureType>()->type(),
                    "")) {
        return false;
      }
      out_ << ", access::sample";
    } else if (tex->Is<ast::type::SampledTextureType>()) {
      if (!EmitType(tex->As<ast::type::SampledTextureType>()->type(), "")) {
        return false;
      }
      out_ << ", access::sample";
    } else {
      error_ = "invalid texture type";
      return false;
    }
    out_ << ">";

  } else if (type->Is<ast::type::U32Type>()) {
    out_ << "uint";
  } else if (type->Is<ast::type::VectorType>()) {
    auto* vec = type->As<ast::type::VectorType>();
    if (!EmitType(vec->type(), "")) {
      return false;
    }
    out_ << vec->size();
  } else if (type->Is<ast::type::VoidType>()) {
    out_ << "void";
  } else {
    error_ = "unknown type in EmitType: " + type->type_name();
    return false;
  }

  return true;
}

bool GeneratorImpl::EmitStructType(const ast::type::StructType* str) {
  // TODO(dsinclair): Block decoration?
  // if (str->impl()->decoration() != ast::StructDecoration::kNone) {
  // }
  out_ << "struct " << str->name() << " {" << std::endl;

  increment_indent();
  uint32_t current_offset = 0;
  uint32_t pad_count = 0;
  for (auto* mem : str->impl()->members()) {
    make_indent();
    for (auto* deco : mem->decorations()) {
      if (auto* o = deco->As<ast::StructMemberOffsetDecoration>()) {
        uint32_t offset = o->offset();
        if (offset != current_offset) {
          out_ << "int8_t pad_" << pad_count << "[" << (offset - current_offset)
               << "];" << std::endl;
          pad_count++;
          make_indent();
        }
        current_offset = offset;
      } else {
        error_ = "unsupported member decoration: " + deco->str();
        return false;
      }
    }

    if (!EmitType(mem->type(), mem->name())) {
      return false;
    }
    auto size = calculate_alignment_size(mem->type());
    if (size == 0) {
      error_ = "unable to calculate byte size for: " + mem->type()->type_name();
      return false;
    }
    current_offset += size;

    // Array member name will be output with the type
    if (!mem->type()->Is<ast::type::ArrayType>()) {
      out_ << " " << namer_.NameFor(mem->name());
    }
    out_ << ";" << std::endl;
  }
  decrement_indent();
  make_indent();

  out_ << "};" << std::endl;
  return true;
}

bool GeneratorImpl::EmitUnaryOp(ast::UnaryOpExpression* expr) {
  switch (expr->op()) {
    case ast::UnaryOp::kNot:
      out_ << "!";
      break;
    case ast::UnaryOp::kNegation:
      out_ << "-";
      break;
  }
  out_ << "(";

  if (!EmitExpression(expr->expr())) {
    return false;
  }

  out_ << ")";

  return true;
}

bool GeneratorImpl::EmitVariable(ast::Variable* var, bool skip_constructor) {
  make_indent();

  // TODO(dsinclair): Handle variable decorations
  if (var->Is<ast::DecoratedVariable>()) {
    error_ = "Variable decorations are not handled yet";
    return false;
  }

  if (var->is_const()) {
    out_ << "const ";
  }
  if (!EmitType(var->type(), var->name())) {
    return false;
  }
  if (!var->type()->Is<ast::type::ArrayType>()) {
    out_ << " " << var->name();
  }

  if (!skip_constructor) {
    out_ << " = ";
    if (var->constructor() != nullptr) {
      if (!EmitExpression(var->constructor())) {
        return false;
      }
    } else if (var->storage_class() == ast::StorageClass::kPrivate ||
               var->storage_class() == ast::StorageClass::kFunction ||
               var->storage_class() == ast::StorageClass::kNone ||
               var->storage_class() == ast::StorageClass::kOutput) {
      if (!EmitZeroValue(var->type())) {
        return false;
      }
    }
  }
  out_ << ";" << std::endl;

  return true;
}

bool GeneratorImpl::EmitProgramConstVariable(const ast::Variable* var) {
  make_indent();

  if (var->Is<ast::DecoratedVariable>() &&
      !var->As<ast::DecoratedVariable>()->HasConstantIdDecoration()) {
    error_ = "Decorated const values not valid";
    return false;
  }
  if (!var->is_const()) {
    error_ = "Expected a const value";
    return false;
  }

  out_ << "constant ";
  if (!EmitType(var->type(), var->name())) {
    return false;
  }
  if (!var->type()->Is<ast::type::ArrayType>()) {
    out_ << " " << var->name();
  }

  if (var->Is<ast::DecoratedVariable>() &&
      var->As<ast::DecoratedVariable>()->HasConstantIdDecoration()) {
    out_ << " [[function_constant("
         << var->As<ast::DecoratedVariable>()->constant_id() << ")]]";
  } else if (var->constructor() != nullptr) {
    out_ << " = ";
    if (!EmitExpression(var->constructor())) {
      return false;
    }
  }
  out_ << ";" << std::endl;

  return true;
}

}  // namespace msl
}  // namespace writer
}  // namespace tint
