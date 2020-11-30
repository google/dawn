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
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/binding_decoration.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/block_statement.h"
#include "src/ast/bool_literal.h"
#include "src/ast/break_statement.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/case_statement.h"
#include "src/ast/constant_id_decoration.h"
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
#include "src/ast/stride_decoration.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type/access_control_type.h"
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
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/workgroup_decoration.h"
#include "src/writer/float_to_string.h"

namespace tint {
namespace writer {
namespace wgsl {

GeneratorImpl::GeneratorImpl(Context* ctx) : TextGenerator(ctx) {}

GeneratorImpl::~GeneratorImpl() = default;

bool GeneratorImpl::Generate(const ast::Module& module) {
  for (auto* const ty : module.constructed_types()) {
    if (!EmitConstructedType(ty)) {
      return false;
    }
  }
  if (!module.constructed_types().empty())
    out_ << std::endl;

  for (auto* var : module.global_variables()) {
    if (!EmitVariable(var)) {
      return false;
    }
  }
  if (!module.global_variables().empty()) {
    out_ << std::endl;
  }

  for (auto* func : module.functions()) {
    if (!EmitFunction(func)) {
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

  // TODO(dsinclair): We always emit constructed types even if they aren't
  // strictly needed
  for (auto* const ty : module.constructed_types()) {
    if (!EmitConstructedType(ty)) {
      return false;
    }
  }
  if (!module.constructed_types().empty()) {
    out_ << std::endl;
  }

  // TODO(dsinclair): This should be smarter and only emit needed const
  // variables
  for (auto* var : module.global_variables()) {
    if (!var->is_const()) {
      continue;
    }
    if (!EmitVariable(var)) {
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

  for (auto* f : module.functions()) {
    if (!f->HasAncestorEntryPoint(name)) {
      continue;
    }

    if (!EmitFunction(f)) {
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

bool GeneratorImpl::EmitConstructedType(const ast::type::Type* ty) {
  make_indent();
  if (ty->Is<ast::type::AliasType>()) {
    auto* alias = ty->As<ast::type::AliasType>();
    out_ << "type " << alias->name() << " = ";
    if (!EmitType(alias->type())) {
      return false;
    }
    out_ << ";" << std::endl;
  } else if (ty->Is<ast::type::StructType>()) {
    if (!EmitStructType(ty->As<ast::type::StructType>())) {
      return false;
    }
  } else {
    error_ = "unknown constructed type: " + ty->type_name();
    return false;
  }

  return true;
}

bool GeneratorImpl::EmitExpression(ast::Expression* expr) {
  if (expr->IsArrayAccessor()) {
    return EmitArrayAccessor(expr->AsArrayAccessor());
  }
  if (expr->IsBinary()) {
    return EmitBinary(expr->AsBinary());
  }
  if (expr->IsBitcast()) {
    return EmitBitcast(expr->AsBitcast());
  }
  if (expr->IsCall()) {
    return EmitCall(expr->AsCall());
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

bool GeneratorImpl::EmitBitcast(ast::BitcastExpression* expr) {
  out_ << "bitcast<";
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
  for (auto* e : expr->values()) {
    if (!first) {
      out_ << ", ";
    }
    first = false;

    if (!EmitExpression(e)) {
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
    out_ << FloatToString(lit->AsFloat()->value());
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
  for (auto* deco : func->decorations()) {
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
  for (auto* v : func->params()) {
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
    case ast::type::ImageFormat::kNone:
      error_ = "unknown image format";
      return false;
    default:
      out_ << fmt;
  }
  return true;
}

bool GeneratorImpl::EmitType(ast::type::Type* type) {
  if (type->Is<ast::type::AccessControlType>()) {
    auto* ac = type->As<ast::type::AccessControlType>();

    out_ << "[[access(";
    if (ac->IsReadOnly()) {
      out_ << "read";
    } else if (ac->IsReadWrite()) {
      out_ << "read_write";
    } else {
      error_ = "invalid access control";
      return false;
    }
    out_ << ")]]" << std::endl;
    if (!EmitType(ac->type())) {
      return false;
    }
  } else if (type->Is<ast::type::AliasType>()) {
    out_ << type->As<ast::type::AliasType>()->name();
  } else if (type->Is<ast::type::ArrayType>()) {
    auto* ary = type->As<ast::type::ArrayType>();

    for (auto* deco : ary->decorations()) {
      if (auto* stride = deco->As<ast::StrideDecoration>()) {
        out_ << "[[stride(" << stride->stride() << ")]] ";
      }
    }

    out_ << "array<";
    if (!EmitType(ary->type())) {
      return false;
    }

    if (!ary->IsRuntimeArray())
      out_ << ", " << ary->size();

    out_ << ">";
  } else if (type->Is<ast::type::BoolType>()) {
    out_ << "bool";
  } else if (type->Is<ast::type::F32Type>()) {
    out_ << "f32";
  } else if (type->Is<ast::type::I32Type>()) {
    out_ << "i32";
  } else if (type->Is<ast::type::MatrixType>()) {
    auto* mat = type->As<ast::type::MatrixType>();
    out_ << "mat" << mat->columns() << "x" << mat->rows() << "<";
    if (!EmitType(mat->type())) {
      return false;
    }
    out_ << ">";
  } else if (type->Is<ast::type::PointerType>()) {
    auto* ptr = type->As<ast::type::PointerType>();
    out_ << "ptr<" << ptr->storage_class() << ", ";
    if (!EmitType(ptr->type())) {
      return false;
    }
    out_ << ">";
  } else if (type->Is<ast::type::SamplerType>()) {
    auto* sampler = type->As<ast::type::SamplerType>();
    out_ << "sampler";

    if (sampler->IsComparison()) {
      out_ << "_comparison";
    }
  } else if (type->Is<ast::type::StructType>()) {
    // The struct, as a type, is just the name. We should have already emitted
    // the declaration through a call to |EmitStructType| earlier.
    out_ << type->As<ast::type::StructType>()->name();
  } else if (type->Is<ast::type::TextureType>()) {
    auto* texture = type->As<ast::type::TextureType>();

    out_ << "texture_";
    if (texture->Is<ast::type::DepthTextureType>()) {
      out_ << "depth_";
    } else if (texture->Is<ast::type::SampledTextureType>()) {
      /* nothing to emit */
    } else if (texture->Is<ast::type::MultisampledTextureType>()) {
      out_ << "multisampled_";
    } else if (texture->Is<ast::type::StorageTextureType>()) {
      out_ << "storage_";

      auto* storage = texture->As<ast::type::StorageTextureType>();
      if (storage->access() == ast::AccessControl::kReadOnly) {
        out_ << "ro_";
      } else if (storage->access() == ast::AccessControl::kWriteOnly) {
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

    if (texture->Is<ast::type::SampledTextureType>()) {
      auto* sampled = texture->As<ast::type::SampledTextureType>();

      out_ << "<";
      if (!EmitType(sampled->type())) {
        return false;
      }
      out_ << ">";
    } else if (texture->Is<ast::type::MultisampledTextureType>()) {
      auto* sampled = texture->As<ast::type::MultisampledTextureType>();

      out_ << "<";
      if (!EmitType(sampled->type())) {
        return false;
      }
      out_ << ">";
    } else if (texture->Is<ast::type::StorageTextureType>()) {
      auto* storage = texture->As<ast::type::StorageTextureType>();

      out_ << "<";
      if (!EmitImageFormat(storage->image_format())) {
        return false;
      }
      out_ << ">";
    }

  } else if (type->Is<ast::type::U32Type>()) {
    out_ << "u32";
  } else if (type->Is<ast::type::VectorType>()) {
    auto* vec = type->As<ast::type::VectorType>();
    out_ << "vec" << vec->size() << "<";
    if (!EmitType(vec->type())) {
      return false;
    }
    out_ << ">";
  } else if (type->Is<ast::type::VoidType>()) {
    out_ << "void";
  } else {
    error_ = "unknown type in EmitType: " + type->type_name();
    return false;
  }

  return true;
}

bool GeneratorImpl::EmitStructType(const ast::type::StructType* str) {
  auto* impl = str->impl();
  for (auto* deco : impl->decorations()) {
    out_ << "[[";
    deco->to_str(out_, 0);
    out_ << "]]" << std::endl;
  }
  out_ << "struct " << str->name() << " {" << std::endl;

  increment_indent();
  for (auto* mem : impl->members()) {
    for (auto* deco : mem->decorations()) {
      make_indent();

      // TODO(dsinclair): Split this out when we have more then one
      assert(deco->IsOffset());
      out_ << "[[offset(" << deco->AsOffset()->offset() << ")]]" << std::endl;
    }
    make_indent();
    out_ << mem->name() << " : ";
    if (!EmitType(mem->type())) {
      return false;
    }
    out_ << ";" << std::endl;
  }
  decrement_indent();
  make_indent();

  out_ << "};" << std::endl;
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
  for (auto* deco : var->decorations()) {
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
    } else if (deco->IsConstantId()) {
      out_ << "constant_id(" << deco->AsConstantId()->value() << ")";
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
    for (auto* selector : stmt->selectors()) {
      if (!first) {
        out_ << ", ";
      }

      first = false;
      if (!EmitLiteral(selector)) {
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

  for (auto* e : stmt->else_statements()) {
    if (!EmitElse(e)) {
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

  for (auto* s : *(stmt->body())) {
    if (!EmitStatement(s)) {
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

}  // namespace wgsl
}  // namespace writer
}  // namespace tint
