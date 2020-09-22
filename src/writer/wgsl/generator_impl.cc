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

#include "src/writer/wgsl/generator_impl.h"

#include <cassert>
#include <limits>

#include "src/ast/array_accessor_expression.h"
#include "src/ast/as_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/binding_decoration.h"
#include "src/ast/block_statement.h"
#include "src/ast/bool_literal.h"
#include "src/ast/break_statement.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/case_statement.h"
#include "src/ast/cast_expression.h"
#include "src/ast/constructor_expression.h"
#include "src/ast/continue_statement.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/else_statement.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/location_decoration.h"
#include "src/ast/loop_statement.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/set_decoration.h"
#include "src/ast/sint_literal.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/statement.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/depth_texture_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/multisampled_texture_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/sampled_texture_type.h"
#include "src/ast/type/sampler_type.h"
#include "src/ast/type/storage_texture_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/workgroup_decoration.h"

namespace tint {
namespace writer {
namespace wgsl {

GeneratorImpl::GeneratorImpl() = default;

GeneratorImpl::~GeneratorImpl() = default;

bool GeneratorImpl::Generate(const ast::Module& module) {
  for (auto* const alias : module.alias_types()) {
    if (!EmitAliasType(alias)) {
      return false;
    }
  }
  if (!module.alias_types().empty())
    out_ << std::endl;

  for (const auto& var : module.global_variables()) {
    if (!EmitVariable(var.get())) {
      return false;
    }
  }
  if (!module.global_variables().empty()) {
    out_ << std::endl;
  }

  for (const auto& func : module.functions()) {
    if (!EmitFunction(func.get())) {
      return false;
    }
    out_ << std::endl;
  }

  return true;
}

bool GeneratorImpl::GenerateEntryPoint(const ast::Module& module,
                                       ast::PipelineStage stage,
                                       const std::string& name) {
  auto* func = module.FindFunctionByNameAndStage(name, stage);
  if (func == nullptr) {
    error_ = "Unable to find requested entry point: " + name;
    return false;
  }

  // TODO(dsinclair): We always emit aliases even if they aren't strictly needed
  for (auto* const alias : module.alias_types()) {
    if (!EmitAliasType(alias)) {
      return false;
    }
  }
  if (!module.alias_types().empty()) {
    out_ << std::endl;
  }

  // TODO(dsinclair): This should be smarter and only emit needed const
  // variables
  for (const auto& var : module.global_variables()) {
    if (!var->is_const()) {
      continue;
    }
    if (!EmitVariable(var.get())) {
      return false;
    }
  }

  bool found_func_variable = false;
  for (auto* var : func->referenced_module_variables()) {
    if (!EmitVariable(var)) {
      return false;
    }
    found_func_variable = true;
  }
  if (found_func_variable) {
    out_ << std::endl;
  }

  for (const auto& f : module.functions()) {
    if (!f->HasAncestorEntryPoint(name)) {
      continue;
    }

    if (!EmitFunction(f.get())) {
      return false;
    }
    out_ << std::endl;
  }

  if (!EmitFunction(func)) {
    return false;
  }
  out_ << std::endl;

  return true;
}

bool GeneratorImpl::EmitAliasType(const ast::type::AliasType* alias) {
  make_indent();
  out_ << "type " << alias->name() << " = ";
  if (!EmitType(alias->type())) {
    return false;
  }
  out_ << ";" << std::endl;

  return true;
}

bool GeneratorImpl::EmitExpression(ast::Expression* expr) {
  if (expr->IsArrayAccessor()) {
    return EmitArrayAccessor(expr->AsArrayAccessor());
  }
  if (expr->IsAs()) {
    return EmitAs(expr->AsAs());
  }
  if (expr->IsBinary()) {
    return EmitBinary(expr->AsBinary());
  }
  if (expr->IsCall()) {
    return EmitCall(expr->AsCall());
  }
  if (expr->IsCast()) {
    return EmitCast(expr->AsCast());
  }
  if (expr->IsIdentifier()) {
    return EmitIdentifier(expr->AsIdentifier());
  }
  if (expr->IsConstructor()) {
    return EmitConstructor(expr->AsConstructor());
  }
  if (expr->IsMemberAccessor()) {
    return EmitMemberAccessor(expr->AsMemberAccessor());
  }
  if (expr->IsUnaryOp()) {
    return EmitUnaryOp(expr->AsUnaryOp());
  }

  error_ = "unknown expression type";
  return false;
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

bool GeneratorImpl::EmitMemberAccessor(ast::MemberAccessorExpression* expr) {
  if (!EmitExpression(expr->structure())) {
    return false;
  }

  out_ << ".";

  return EmitExpression(expr->member());
}

bool GeneratorImpl::EmitAs(ast::AsExpression* expr) {
  out_ << "as<";
  if (!EmitType(expr->type())) {
    return false;
  }

  out_ << ">(";
  if (!EmitExpression(expr->expr())) {
    return false;
  }

  out_ << ")";
  return true;
}

bool GeneratorImpl::EmitCall(ast::CallExpression* expr) {
  if (!EmitExpression(expr->func())) {
    return false;
  }
  out_ << "(";

  bool first = true;
  const auto& params = expr->params();
  for (const auto& param : params) {
    if (!first) {
      out_ << ", ";
    }
    first = false;

    if (!EmitExpression(param.get())) {
      return false;
    }
  }

  out_ << ")";

  return true;
}

bool GeneratorImpl::EmitCast(ast::CastExpression* expr) {
  out_ << "cast<";
  if (!EmitType(expr->type())) {
    return false;
  }

  out_ << ">(";
  if (!EmitExpression(expr->expr())) {
    return false;
  }

  out_ << ")";
  return true;
}

bool GeneratorImpl::EmitConstructor(ast::ConstructorExpression* expr) {
  if (expr->IsScalarConstructor()) {
    return EmitScalarConstructor(expr->AsScalarConstructor());
  }
  return EmitTypeConstructor(expr->AsTypeConstructor());
}

bool GeneratorImpl::EmitTypeConstructor(ast::TypeConstructorExpression* expr) {
  if (!EmitType(expr->type())) {
    return false;
  }

  out_ << "(";

  bool first = true;
  for (const auto& e : expr->values()) {
    if (!first) {
      out_ << ", ";
    }
    first = false;

    if (!EmitExpression(e.get())) {
      return false;
    }
  }

  out_ << ")";
  return true;
}

bool GeneratorImpl::EmitScalarConstructor(
    ast::ScalarConstructorExpression* expr) {
  return EmitLiteral(expr->literal());
}

bool GeneratorImpl::EmitLiteral(ast::Literal* lit) {
  if (lit->IsBool()) {
    out_ << (lit->AsBool()->IsTrue() ? "true" : "false");
  } else if (lit->IsFloat()) {
    auto flags = out_.flags();
    auto precision = out_.precision();

    out_.flags(flags | std::ios_base::showpoint);
    out_.precision(std::numeric_limits<float>::max_digits10);

    out_ << lit->AsFloat()->value();

    out_.precision(precision);
    out_.flags(flags);
  } else if (lit->IsSint()) {
    out_ << lit->AsSint()->value();
  } else if (lit->IsUint()) {
    out_ << lit->AsUint()->value() << "u";
  } else {
    error_ = "unknown literal type";
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitIdentifier(ast::IdentifierExpression* expr) {
  auto* ident = expr->AsIdentifier();
  out_ << ident->name();
  return true;
}

bool GeneratorImpl::EmitFunction(ast::Function* func) {
  for (auto& deco : func->decorations()) {
    make_indent();
    out_ << "[[";
    if (deco->IsWorkgroup()) {
      uint32_t x = 0;
      uint32_t y = 0;
      uint32_t z = 0;
      std::tie(x, y, z) = deco->AsWorkgroup()->values();
      out_ << "workgroup_size(" << std::to_string(x) << ", "
           << std::to_string(y) << ", " << std::to_string(z) << ")";
    }
    if (deco->IsStage()) {
      out_ << "stage(" << deco->AsStage()->value() << ")";
    }
    out_ << "]]" << std::endl;
  }

  make_indent();
  out_ << "fn " << func->name() << "(";

  bool first = true;
  for (const auto& v : func->params()) {
    if (!first) {
      out_ << ", ";
    }
    first = false;

    out_ << v->name() << " : ";

    if (!EmitType(v->type())) {
      return false;
    }
  }

  out_ << ") -> ";

  if (!EmitType(func->return_type())) {
    return false;
  }

  out_ << " ";
  return EmitBlockAndNewline(func->body());
}

bool GeneratorImpl::EmitImageFormat(const ast::type::ImageFormat fmt) {
  switch (fmt) {
    case ast::type::ImageFormat::kBgra8Unorm:
      out_ << "bgra8unorm";
      break;
    case ast::type::ImageFormat::kBgra8UnormSrgb:
      out_ << "bgra8unorm_srgb";
      break;
    case ast::type::ImageFormat::kR16Float:
      out_ << "r16float";
      break;
    case ast::type::ImageFormat::kR16Sint:
      out_ << "r16sint";
      break;
    case ast::type::ImageFormat::kR16Uint:
      out_ << "r16uint";
      break;
    case ast::type::ImageFormat::kR32Float:
      out_ << "r32float";
      break;
    case ast::type::ImageFormat::kR32Sint:
      out_ << "r32sint";
      break;
    case ast::type::ImageFormat::kR32Uint:
      out_ << "r32uint";
      break;
    case ast::type::ImageFormat::kR8Sint:
      out_ << "r8sint";
      break;
    case ast::type::ImageFormat::kR8Snorm:
      out_ << "r8snorm";
      break;
    case ast::type::ImageFormat::kR8Uint:
      out_ << "r8uint";
      break;
    case ast::type::ImageFormat::kR8Unorm:
      out_ << "r8unorm";
      break;
    case ast::type::ImageFormat::kRg11B10Float:
      out_ << "rg11b10float";
      break;
    case ast::type::ImageFormat::kRg16Float:
      out_ << "rg16float";
      break;
    case ast::type::ImageFormat::kRg16Sint:
      out_ << "rg16sint";
      break;
    case ast::type::ImageFormat::kRg16Uint:
      out_ << "rg16uint";
      break;
    case ast::type::ImageFormat::kRg32Float:
      out_ << "rg32float";
      break;
    case ast::type::ImageFormat::kRg32Sint:
      out_ << "rg32sint";
      break;
    case ast::type::ImageFormat::kRg32Uint:
      out_ << "rg32uint";
      break;
    case ast::type::ImageFormat::kRg8Sint:
      out_ << "rg8sint";
      break;
    case ast::type::ImageFormat::kRg8Snorm:
      out_ << "rg8snorm";
      break;
    case ast::type::ImageFormat::kRg8Uint:
      out_ << "rg8uint";
      break;
    case ast::type::ImageFormat::kRg8Unorm:
      out_ << "rg8unorm";
      break;
    case ast::type::ImageFormat::kRgb10A2Unorm:
      out_ << "rgb10a2unorm";
      break;
    case ast::type::ImageFormat::kRgba16Float:
      out_ << "rgba16float";
      break;
    case ast::type::ImageFormat::kRgba16Sint:
      out_ << "rgba16sint";
      break;
    case ast::type::ImageFormat::kRgba16Uint:
      out_ << "rgba16uint";
      break;
    case ast::type::ImageFormat::kRgba32Float:
      out_ << "rgba32float";
      break;
    case ast::type::ImageFormat::kRgba32Sint:
      out_ << "rgba32sint";
      break;
    case ast::type::ImageFormat::kRgba32Uint:
      out_ << "rgba32uint";
      break;
    case ast::type::ImageFormat::kRgba8Sint:
      out_ << "rgba8sint";
      break;
    case ast::type::ImageFormat::kRgba8Snorm:
      out_ << "rgba8snorm";
      break;
    case ast::type::ImageFormat::kRgba8Uint:
      out_ << "rgba8uint";
      break;
    case ast::type::ImageFormat::kRgba8Unorm:
      out_ << "rgba8unorm";
      break;
    case ast::type::ImageFormat::kRgba8UnormSrgb:
      out_ << "rgba8unorm_srgb";
      break;
    default:
      error_ = "unknown image format";
      return false;
  }
  return true;
}

bool GeneratorImpl::EmitType(ast::type::Type* type) {
  if (type->IsAlias()) {
    auto* alias = type->AsAlias();
    out_ << alias->name();
  } else if (type->IsArray()) {
    auto* ary = type->AsArray();

    if (ary->has_array_stride()) {
      out_ << "[[stride(" << ary->array_stride() << ")]] ";
    }

    out_ << "array<";
    if (!EmitType(ary->type())) {
      return false;
    }

    if (!ary->IsRuntimeArray())
      out_ << ", " << ary->size();

    out_ << ">";
  } else if (type->IsBool()) {
    out_ << "bool";
  } else if (type->IsF32()) {
    out_ << "f32";
  } else if (type->IsI32()) {
    out_ << "i32";
  } else if (type->IsMatrix()) {
    auto* mat = type->AsMatrix();
    out_ << "mat" << mat->columns() << "x" << mat->rows() << "<";
    if (!EmitType(mat->type())) {
      return false;
    }
    out_ << ">";
  } else if (type->IsPointer()) {
    auto* ptr = type->AsPointer();
    out_ << "ptr<" << ptr->storage_class() << ", ";
    if (!EmitType(ptr->type())) {
      return false;
    }
    out_ << ">";
  } else if (type->IsSampler()) {
    auto* sampler = type->AsSampler();
    out_ << "sampler";

    if (sampler->IsComparison()) {
      out_ << "_comparison";
    }
  } else if (type->IsStruct()) {
    auto* str = type->AsStruct()->impl();
    if (str->decoration() != ast::StructDecoration::kNone) {
      out_ << "[[" << str->decoration() << "]] ";
    }
    out_ << "struct {" << std::endl;

    increment_indent();
    for (const auto& mem : str->members()) {
      make_indent();
      if (!mem->decorations().empty()) {
        out_ << "[[";
        bool first = true;
        for (const auto& deco : mem->decorations()) {
          if (!first) {
            out_ << ", ";
          }

          first = false;
          // TODO(dsinclair): Split this out when we have more then one
          assert(deco->IsOffset());

          out_ << "offset(" << deco->AsOffset()->offset() << ")";
        }
        out_ << "]] ";
      }

      out_ << mem->name() << " : ";
      if (!EmitType(mem->type())) {
        return false;
      }
      out_ << ";" << std::endl;
    }
    decrement_indent();
    make_indent();

    out_ << "}";
  } else if (type->IsTexture()) {
    auto* texture = type->AsTexture();

    out_ << "texture_";
    if (texture->IsDepth()) {
      out_ << "depth_";
    } else if (texture->IsSampled()) {
      out_ << "sampled_";
    } else if (texture->IsMultisampled()) {
      out_ << "multisampled_";
    } else if (texture->IsStorage()) {
      auto* storage = texture->AsStorage();

      if (storage->access() == ast::type::StorageAccess::kRead) {
        out_ << "ro_";
      } else if (storage->access() == ast::type::StorageAccess::kWrite) {
        out_ << "wo_";
      } else {
        error_ = "unknown storage texture access";
        return false;
      }
    } else {
      error_ = "unknown texture type";
      return false;
    }

    switch (texture->dim()) {
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
        error_ = "unknown texture dimension";
        return false;
    }

    if (texture->IsSampled()) {
      auto* sampled = texture->AsSampled();

      out_ << "<";
      if (!EmitType(sampled->type())) {
        return false;
      }
      out_ << ">";
    } else if (texture->IsMultisampled()) {
      auto* sampled = texture->AsMultisampled();

      out_ << "<";
      if (!EmitType(sampled->type())) {
        return false;
      }
      out_ << ">";
    } else if (texture->IsStorage()) {
      auto* storage = texture->AsStorage();

      out_ << "<";
      if (!EmitImageFormat(storage->image_format())) {
        return false;
      }
      out_ << ">";
    }

  } else if (type->IsU32()) {
    out_ << "u32";
  } else if (type->IsVector()) {
    auto* vec = type->AsVector();
    out_ << "vec" << vec->size() << "<";
    if (!EmitType(vec->type())) {
      return false;
    }
    out_ << ">";
  } else if (type->IsVoid()) {
    out_ << "void";
  } else {
    error_ = "unknown type in EmitType";
    return false;
  }

  return true;
}

bool GeneratorImpl::EmitVariable(ast::Variable* var) {
  make_indent();

  if (var->IsDecorated()) {
    if (!EmitVariableDecorations(var->AsDecorated())) {
      return false;
    }
  }

  if (var->is_const()) {
    out_ << "const";
  } else {
    out_ << "var";
    if (var->storage_class() != ast::StorageClass::kNone &&
        var->storage_class() != ast::StorageClass::kFunction) {
      out_ << "<" << var->storage_class() << ">";
    }
  }

  out_ << " " << var->name() << " : ";
  if (!EmitType(var->type())) {
    return false;
  }

  if (var->constructor() != nullptr) {
    out_ << " = ";
    if (!EmitExpression(var->constructor())) {
      return false;
    }
  }
  out_ << ";" << std::endl;

  return true;
}

bool GeneratorImpl::EmitVariableDecorations(ast::DecoratedVariable* var) {
  out_ << "[[";
  bool first = true;
  for (const auto& deco : var->decorations()) {
    if (!first) {
      out_ << ", ";
    }
    first = false;

    if (deco->IsBinding()) {
      out_ << "binding(" << deco->AsBinding()->value() << ")";
    } else if (deco->IsSet()) {
      out_ << "set(" << deco->AsSet()->value() << ")";
    } else if (deco->IsLocation()) {
      out_ << "location(" << deco->AsLocation()->value() << ")";
    } else if (deco->IsBuiltin()) {
      out_ << "builtin(" << deco->AsBuiltin()->value() << ")";
    } else {
      error_ = "unknown variable decoration";
      return false;
    }
  }
  out_ << "]] ";

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
      out_ << ">>";
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

bool GeneratorImpl::EmitBlock(const ast::BlockStatement* stmt) {
  out_ << "{" << std::endl;
  increment_indent();

  for (const auto& s : *stmt) {
    if (!EmitStatement(s.get())) {
      return false;
    }
  }

  decrement_indent();
  make_indent();
  out_ << "}";

  return true;
}

bool GeneratorImpl::EmitIndentedBlockAndNewline(
    const ast::BlockStatement* stmt) {
  make_indent();
  const bool result = EmitBlock(stmt);
  if (result) {
    out_ << std::endl;
  }
  return result;
}

bool GeneratorImpl::EmitBlockAndNewline(const ast::BlockStatement* stmt) {
  const bool result = EmitBlock(stmt);
  if (result) {
    out_ << std::endl;
  }
  return result;
}

bool GeneratorImpl::EmitStatement(ast::Statement* stmt) {
  if (stmt->IsAssign()) {
    return EmitAssign(stmt->AsAssign());
  }
  if (stmt->IsBlock()) {
    return EmitIndentedBlockAndNewline(stmt->AsBlock());
  }
  if (stmt->IsBreak()) {
    return EmitBreak(stmt->AsBreak());
  }
  if (stmt->IsCall()) {
    make_indent();
    if (!EmitCall(stmt->AsCall()->expr())) {
      return false;
    }
    out_ << ";" << std::endl;
    return true;
  }
  if (stmt->IsContinue()) {
    return EmitContinue(stmt->AsContinue());
  }
  if (stmt->IsDiscard()) {
    return EmitDiscard(stmt->AsDiscard());
  }
  if (stmt->IsFallthrough()) {
    return EmitFallthrough(stmt->AsFallthrough());
  }
  if (stmt->IsIf()) {
    return EmitIf(stmt->AsIf());
  }
  if (stmt->IsLoop()) {
    return EmitLoop(stmt->AsLoop());
  }
  if (stmt->IsReturn()) {
    return EmitReturn(stmt->AsReturn());
  }
  if (stmt->IsSwitch()) {
    return EmitSwitch(stmt->AsSwitch());
  }
  if (stmt->IsVariableDecl()) {
    return EmitVariable(stmt->AsVariableDecl()->variable());
  }

  error_ = "unknown statement type: " + stmt->str();
  return false;
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

bool GeneratorImpl::EmitBreak(ast::BreakStatement*) {
  make_indent();
  out_ << "break;" << std::endl;
  return true;
}

bool GeneratorImpl::EmitCase(ast::CaseStatement* stmt) {
  make_indent();

  if (stmt->IsDefault()) {
    out_ << "default";
  } else {
    out_ << "case ";

    bool first = true;
    for (const auto& selector : stmt->selectors()) {
      if (!first) {
        out_ << ", ";
      }

      first = false;
      if (!EmitLiteral(selector.get())) {
        return false;
      }
    }
  }
  out_ << ": ";

  return EmitBlockAndNewline(stmt->body());
}

bool GeneratorImpl::EmitContinue(ast::ContinueStatement*) {
  make_indent();
  out_ << "continue;" << std::endl;
  return true;
}

bool GeneratorImpl::EmitElse(ast::ElseStatement* stmt) {
  if (stmt->HasCondition()) {
    out_ << " elseif (";
    if (!EmitExpression(stmt->condition())) {
      return false;
    }
    out_ << ") ";
  } else {
    out_ << " else ";
  }

  return EmitBlock(stmt->body());
}

bool GeneratorImpl::EmitFallthrough(ast::FallthroughStatement*) {
  make_indent();
  out_ << "fallthrough;" << std::endl;
  return true;
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

  for (const auto& e : stmt->else_statements()) {
    if (!EmitElse(e.get())) {
      return false;
    }
  }
  out_ << std::endl;

  return true;
}

bool GeneratorImpl::EmitDiscard(ast::DiscardStatement*) {
  make_indent();
  out_ << "discard;" << std::endl;
  return true;
}

bool GeneratorImpl::EmitLoop(ast::LoopStatement* stmt) {
  make_indent();

  out_ << "loop {" << std::endl;
  increment_indent();

  for (const auto& s : *(stmt->body())) {
    if (!EmitStatement(s.get())) {
      return false;
    }
  }

  if (stmt->has_continuing()) {
    out_ << std::endl;

    make_indent();
    out_ << "continuing ";

    if (!EmitBlockAndNewline(stmt->continuing())) {
      return false;
    }
  }

  decrement_indent();
  make_indent();
  out_ << "}" << std::endl;

  return true;
}

bool GeneratorImpl::EmitReturn(ast::ReturnStatement* stmt) {
  make_indent();

  out_ << "return";
  if (stmt->has_value()) {
    out_ << " ";
    if (!EmitExpression(stmt->value())) {
      return false;
    }
  }
  out_ << ";" << std::endl;
  return true;
}

bool GeneratorImpl::EmitSwitch(ast::SwitchStatement* stmt) {
  make_indent();

  out_ << "switch(";
  if (!EmitExpression(stmt->condition())) {
    return false;
  }
  out_ << ") {" << std::endl;

  increment_indent();

  for (const auto& s : stmt->body()) {
    if (!EmitCase(s.get())) {
      return false;
    }
  }

  decrement_indent();
  make_indent();
  out_ << "}" << std::endl;

  return true;
}

}  // namespace wgsl
}  // namespace writer
}  // namespace tint
