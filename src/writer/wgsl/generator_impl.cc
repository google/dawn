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
#include "src/ast/else_statement.h"
#include "src/ast/float_literal.h"
#include "src/ast/group_decoration.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/location_decoration.h"
#include "src/ast/loop_statement.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/statement.h"
#include "src/ast/stride_decoration.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/workgroup_decoration.h"
#include "src/type/access_control_type.h"
#include "src/type/array_type.h"
#include "src/type/bool_type.h"
#include "src/type/depth_texture_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/matrix_type.h"
#include "src/type/multisampled_texture_type.h"
#include "src/type/pointer_type.h"
#include "src/type/sampled_texture_type.h"
#include "src/type/sampler_type.h"
#include "src/type/storage_texture_type.h"
#include "src/type/struct_type.h"
#include "src/type/u32_type.h"
#include "src/type/vector_type.h"
#include "src/type/void_type.h"
#include "src/writer/float_to_string.h"

namespace tint {
namespace writer {
namespace wgsl {

GeneratorImpl::GeneratorImpl(ast::Module* module)
    : TextGenerator(), module_(*module) {}

GeneratorImpl::~GeneratorImpl() = default;

bool GeneratorImpl::Generate() {
  for (auto* const ty : module_.constructed_types()) {
    if (!EmitConstructedType(ty)) {
      return false;
    }
  }
  if (!module_.constructed_types().empty())
    out_ << std::endl;

  for (auto* var : module_.global_variables()) {
    if (!EmitVariable(var)) {
      return false;
    }
  }
  if (!module_.global_variables().empty()) {
    out_ << std::endl;
  }

  for (auto* func : module_.functions()) {
    if (!EmitFunction(func)) {
      return false;
    }
    out_ << std::endl;
  }

  return true;
}

bool GeneratorImpl::GenerateEntryPoint(ast::PipelineStage stage,
                                       const std::string& name) {
  auto* func =
      module_.FindFunctionBySymbolAndStage(module_.GetSymbol(name), stage);
  if (func == nullptr) {
    error_ = "Unable to find requested entry point: " + name;
    return false;
  }

  // TODO(dsinclair): We always emit constructed types even if they aren't
  // strictly needed
  for (auto* const ty : module_.constructed_types()) {
    if (!EmitConstructedType(ty)) {
      return false;
    }
  }
  if (!module_.constructed_types().empty()) {
    out_ << std::endl;
  }

  // TODO(dsinclair): This should be smarter and only emit needed const
  // variables
  for (auto* var : module_.global_variables()) {
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

  for (auto* f : module_.functions()) {
    if (!f->HasAncestorEntryPoint(module_.GetSymbol(name))) {
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

bool GeneratorImpl::EmitConstructedType(const type::Type* ty) {
  make_indent();
  if (auto* alias = ty->As<type::Alias>()) {
    out_ << "type " << module_.SymbolToName(alias->symbol()) << " = ";
    if (!EmitType(alias->type())) {
      return false;
    }
    out_ << ";" << std::endl;
  } else if (auto* str = ty->As<type::Struct>()) {
    if (!EmitStructType(str)) {
      return false;
    }
  } else {
    error_ = "unknown constructed type: " + ty->type_name();
    return false;
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
  if (auto* i = expr->As<ast::IdentifierExpression>()) {
    return EmitIdentifier(i);
  }
  if (auto* c = expr->As<ast::ConstructorExpression>()) {
    return EmitConstructor(c);
  }
  if (auto* m = expr->As<ast::MemberAccessorExpression>()) {
    return EmitMemberAccessor(m);
  }
  if (auto* u = expr->As<ast::UnaryOpExpression>()) {
    return EmitUnaryOp(u);
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
  if (auto* scalar = expr->As<ast::ScalarConstructorExpression>()) {
    return EmitScalarConstructor(scalar);
  }
  return EmitTypeConstructor(expr->As<ast::TypeConstructorExpression>());
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
  if (auto* bl = lit->As<ast::BoolLiteral>()) {
    out_ << (bl->IsTrue() ? "true" : "false");
  } else if (auto* fl = lit->As<ast::FloatLiteral>()) {
    out_ << FloatToString(fl->value());
  } else if (auto* sl = lit->As<ast::SintLiteral>()) {
    out_ << sl->value();
  } else if (auto* ul = lit->As<ast::UintLiteral>()) {
    out_ << ul->value() << "u";
  } else {
    error_ = "unknown literal type";
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitIdentifier(ast::IdentifierExpression* expr) {
  auto* ident = expr->As<ast::IdentifierExpression>();
  out_ << module_.SymbolToName(ident->symbol());
  return true;
}

bool GeneratorImpl::EmitFunction(ast::Function* func) {
  for (auto* deco : func->decorations()) {
    make_indent();
    out_ << "[[";
    if (auto* workgroup = deco->As<ast::WorkgroupDecoration>()) {
      uint32_t x = 0;
      uint32_t y = 0;
      uint32_t z = 0;
      std::tie(x, y, z) = workgroup->values();
      out_ << "workgroup_size(" << std::to_string(x) << ", "
           << std::to_string(y) << ", " << std::to_string(z) << ")";
    }
    if (auto* stage = deco->As<ast::StageDecoration>()) {
      out_ << "stage(" << stage->value() << ")";
    }
    out_ << "]]" << std::endl;
  }

  make_indent();
  out_ << "fn " << module_.SymbolToName(func->symbol()) << "(";

  bool first = true;
  for (auto* v : func->params()) {
    if (!first) {
      out_ << ", ";
    }
    first = false;

    out_ << module_.SymbolToName(v->symbol()) << " : ";

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

bool GeneratorImpl::EmitImageFormat(const type::ImageFormat fmt) {
  switch (fmt) {
    case type::ImageFormat::kNone:
      error_ = "unknown image format";
      return false;
    default:
      out_ << fmt;
  }
  return true;
}

bool GeneratorImpl::EmitType(type::Type* type) {
  std::string storage_texture_access = "";
  if (auto* ac = type->As<type::AccessControl>()) {
    out_ << "[[access(";
    if (ac->IsReadOnly()) {
      out_ << "read";
    } else if (ac->IsWriteOnly()) {
      out_ << "write";
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
    return true;
  } else if (auto* alias = type->As<type::Alias>()) {
    out_ << module_.SymbolToName(alias->symbol());
  } else if (auto* ary = type->As<type::Array>()) {
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
  } else if (type->Is<type::Bool>()) {
    out_ << "bool";
  } else if (type->Is<type::F32>()) {
    out_ << "f32";
  } else if (type->Is<type::I32>()) {
    out_ << "i32";
  } else if (auto* mat = type->As<type::Matrix>()) {
    out_ << "mat" << mat->columns() << "x" << mat->rows() << "<";
    if (!EmitType(mat->type())) {
      return false;
    }
    out_ << ">";
  } else if (auto* ptr = type->As<type::Pointer>()) {
    out_ << "ptr<" << ptr->storage_class() << ", ";
    if (!EmitType(ptr->type())) {
      return false;
    }
    out_ << ">";
  } else if (auto* sampler = type->As<type::Sampler>()) {
    out_ << "sampler";

    if (sampler->IsComparison()) {
      out_ << "_comparison";
    }
  } else if (auto* str = type->As<type::Struct>()) {
    // The struct, as a type, is just the name. We should have already emitted
    // the declaration through a call to |EmitStructType| earlier.
    out_ << module_.SymbolToName(str->symbol());
  } else if (auto* texture = type->As<type::Texture>()) {
    out_ << "texture_";
    if (texture->Is<type::DepthTexture>()) {
      out_ << "depth_";
    } else if (texture->Is<type::SampledTexture>()) {
      /* nothing to emit */
    } else if (texture->Is<type::MultisampledTexture>()) {
      out_ << "multisampled_";
    } else if (texture->Is<type::StorageTexture>()) {
      out_ << "storage_";
    } else {
      error_ = "unknown texture type";
      return false;
    }

    switch (texture->dim()) {
      case type::TextureDimension::k1d:
        out_ << "1d";
        break;
      case type::TextureDimension::k1dArray:
        out_ << "1d_array";
        break;
      case type::TextureDimension::k2d:
        out_ << "2d";
        break;
      case type::TextureDimension::k2dArray:
        out_ << "2d_array";
        break;
      case type::TextureDimension::k3d:
        out_ << "3d";
        break;
      case type::TextureDimension::kCube:
        out_ << "cube";
        break;
      case type::TextureDimension::kCubeArray:
        out_ << "cube_array";
        break;
      default:
        error_ = "unknown texture dimension";
        return false;
    }

    if (auto* sampled = texture->As<type::SampledTexture>()) {
      out_ << "<";
      if (!EmitType(sampled->type())) {
        return false;
      }
      out_ << ">";
    } else if (auto* ms = texture->As<type::MultisampledTexture>()) {
      out_ << "<";
      if (!EmitType(ms->type())) {
        return false;
      }
      out_ << ">";
    } else if (auto* storage = texture->As<type::StorageTexture>()) {
      out_ << "<";
      if (!EmitImageFormat(storage->image_format())) {
        return false;
      }
      out_ << ">";
    }

  } else if (type->Is<type::U32>()) {
    out_ << "u32";
  } else if (auto* vec = type->As<type::Vector>()) {
    out_ << "vec" << vec->size() << "<";
    if (!EmitType(vec->type())) {
      return false;
    }
    out_ << ">";
  } else if (type->Is<type::Void>()) {
    out_ << "void";
  } else {
    error_ = "unknown type in EmitType: " + type->type_name();
    return false;
  }

  return true;
}

bool GeneratorImpl::EmitStructType(const type::Struct* str) {
  auto* impl = str->impl();
  for (auto* deco : impl->decorations()) {
    out_ << "[[";
    deco->to_str(out_, 0);
    out_ << "]]" << std::endl;
  }
  out_ << "struct " << module_.SymbolToName(str->symbol()) << " {" << std::endl;

  increment_indent();
  for (auto* mem : impl->members()) {
    for (auto* deco : mem->decorations()) {
      make_indent();

      // TODO(dsinclair): Split this out when we have more then one
      auto* offset = deco->As<ast::StructMemberOffsetDecoration>();
      assert(offset != nullptr);
      out_ << "[[offset(" << offset->offset() << ")]]" << std::endl;
    }
    make_indent();
    out_ << module_.SymbolToName(mem->symbol()) << " : ";
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

  if (!var->decorations().empty() && !EmitVariableDecorations(var)) {
    return false;
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

  out_ << " " << module_.SymbolToName(var->symbol()) << " : ";
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

bool GeneratorImpl::EmitVariableDecorations(ast::Variable* var) {
  out_ << "[[";
  bool first = true;
  for (auto* deco : var->decorations()) {
    if (!first) {
      out_ << ", ";
    }
    first = false;

    if (auto* binding = deco->As<ast::BindingDecoration>()) {
      out_ << "binding(" << binding->value() << ")";
    } else if (auto* group = deco->As<ast::GroupDecoration>()) {
      out_ << "group(" << group->value() << ")";
    } else if (auto* location = deco->As<ast::LocationDecoration>()) {
      out_ << "location(" << location->value() << ")";
    } else if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
      out_ << "builtin(" << builtin->value() << ")";
    } else if (auto* constant = deco->As<ast::ConstantIdDecoration>()) {
      out_ << "constant_id(" << constant->value() << ")";
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
    return EmitFallthrough(f);
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
    return EmitVariable(v->variable());
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
