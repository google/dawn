// Copyright 2021 The Tint Authors.
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

#include <algorithm>
#include <limits>

#include "src/ast/access_control.h"
#include "src/ast/alias.h"
#include "src/ast/array.h"
#include "src/ast/bool.h"
#include "src/ast/bool_literal.h"
#include "src/ast/call_statement.h"
#include "src/ast/depth_texture.h"
#include "src/ast/f32.h"
#include "src/ast/float_literal.h"
#include "src/ast/i32.h"
#include "src/ast/internal_decoration.h"
#include "src/ast/matrix.h"
#include "src/ast/module.h"
#include "src/ast/multisampled_texture.h"
#include "src/ast/override_decoration.h"
#include "src/ast/pointer.h"
#include "src/ast/sampled_texture.h"
#include "src/ast/sint_literal.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/storage_texture.h"
#include "src/ast/stride_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_member_align_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/struct_member_size_decoration.h"
#include "src/ast/type_name.h"
#include "src/ast/u32.h"
#include "src/ast/uint_literal.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/vector.h"
#include "src/ast/void.h"
#include "src/ast/workgroup_decoration.h"
#include "src/sem/struct.h"
#include "src/utils/math.h"
#include "src/writer/float_to_string.h"

namespace tint {
namespace writer {
namespace wgsl {

GeneratorImpl::GeneratorImpl(const Program* program)
    : TextGenerator(), program_(program) {}

GeneratorImpl::~GeneratorImpl() = default;

bool GeneratorImpl::Generate() {
  // Generate global declarations in the order they appear in the module.
  for (auto* decl : program_->AST().GlobalDeclarations()) {
    if (auto* ty = decl->As<ast::Type>()) {
      if (!EmitConstructedType(ty)) {
        return false;
      }
    } else if (auto* func = decl->As<ast::Function>()) {
      if (!EmitFunction(func)) {
        return false;
      }
    } else if (auto* var = decl->As<ast::Variable>()) {
      if (!EmitVariable(var)) {
        return false;
      }
    } else {
      TINT_UNREACHABLE(diagnostics_);
      return false;
    }

    if (decl != program_->AST().GlobalDeclarations().back()) {
      out_ << std::endl;
    }
  }

  return true;
}

bool GeneratorImpl::EmitConstructedType(const ast::Type* ty) {
  make_indent();

  if (auto* alias = ty->As<ast::Alias>()) {
    out_ << "type " << program_->Symbols().NameFor(alias->symbol()) << " = ";
    if (!EmitType(alias->type())) {
      return false;
    }
    out_ << ";" << std::endl;
  } else if (auto* str = ty->As<ast::Struct>()) {
    if (!EmitStructType(str)) {
      return false;
    }
  } else {
    diagnostics_.add_error("unknown constructed type: " + ty->type_name());
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

  diagnostics_.add_error("unknown expression type");
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
  bool paren_lhs =
      !expr->structure()
           ->IsAnyOf<ast::ArrayAccessorExpression, ast::CallExpression,
                     ast::IdentifierExpression, ast::MemberAccessorExpression,
                     ast::TypeConstructorExpression>();
  if (paren_lhs) {
    out_ << "(";
  }
  if (!EmitExpression(expr->structure())) {
    return false;
  }
  if (paren_lhs) {
    out_ << ")";
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
    diagnostics_.add_error("unknown literal type");
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitIdentifier(ast::IdentifierExpression* expr) {
  auto* ident = expr->As<ast::IdentifierExpression>();
  out_ << program_->Symbols().NameFor(ident->symbol());
  return true;
}

bool GeneratorImpl::EmitFunction(ast::Function* func) {
  if (func->decorations().size()) {
    make_indent();
    if (!EmitDecorations(func->decorations())) {
      return false;
    }
    out_ << std::endl;
  }

  make_indent();
  out_ << "fn " << program_->Symbols().NameFor(func->symbol()) << "(";

  bool first = true;
  for (auto* v : func->params()) {
    if (!first) {
      out_ << ", ";
    }
    first = false;

    if (!v->decorations().empty()) {
      if (!EmitDecorations(v->decorations())) {
        return false;
      }
      out_ << " ";
    }

    out_ << program_->Symbols().NameFor(v->symbol()) << " : ";

    if (!EmitType(v->type())) {
      return false;
    }
  }

  out_ << ")";

  if (!func->return_type()->Is<ast::Void>() ||
      !func->return_type_decorations().empty()) {
    out_ << " -> ";

    if (!func->return_type_decorations().empty()) {
      if (!EmitDecorations(func->return_type_decorations())) {
        return false;
      }
      out_ << " ";
    }

    if (!EmitType(func->return_type())) {
      return false;
    }
  }

  if (func->body()) {
    out_ << " ";
    return EmitBlockAndNewline(func->body());
  } else {
    out_ << std::endl;
  }

  return true;
}

bool GeneratorImpl::EmitImageFormat(const ast::ImageFormat fmt) {
  switch (fmt) {
    case ast::ImageFormat::kNone:
      diagnostics_.add_error("unknown image format");
      return false;
    default:
      out_ << fmt;
  }
  return true;
}

bool GeneratorImpl::EmitType(const ast::Type* ty) {
  if (auto* ac = ty->As<ast::AccessControl>()) {
    out_ << "[[access(";
    if (ac->IsReadOnly()) {
      out_ << "read";
    } else if (ac->IsWriteOnly()) {
      out_ << "write";
    } else if (ac->IsReadWrite()) {
      out_ << "read_write";
    } else {
      diagnostics_.add_error("invalid access control");
      return false;
    }
    out_ << ")]] ";
    if (!EmitType(ac->type())) {
      return false;
    }
    return true;
  } else if (auto* alias = ty->As<ast::Alias>()) {
    out_ << program_->Symbols().NameFor(alias->symbol());
  } else if (auto* ary = ty->As<ast::Array>()) {
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
  } else if (ty->Is<ast::Bool>()) {
    out_ << "bool";
  } else if (ty->Is<ast::F32>()) {
    out_ << "f32";
  } else if (ty->Is<ast::I32>()) {
    out_ << "i32";
  } else if (auto* mat = ty->As<ast::Matrix>()) {
    out_ << "mat" << mat->columns() << "x" << mat->rows() << "<";
    if (!EmitType(mat->type())) {
      return false;
    }
    out_ << ">";
  } else if (auto* ptr = ty->As<ast::Pointer>()) {
    out_ << "ptr<" << ptr->storage_class() << ", ";
    if (!EmitType(ptr->type())) {
      return false;
    }
    out_ << ">";
  } else if (auto* sampler = ty->As<ast::Sampler>()) {
    out_ << "sampler";

    if (sampler->IsComparison()) {
      out_ << "_comparison";
    }
  } else if (auto* str = ty->As<ast::Struct>()) {
    // The struct, as a type, is just the name. We should have already emitted
    // the declaration through a call to |EmitStructType| earlier.
    out_ << program_->Symbols().NameFor(str->name());
  } else if (auto* texture = ty->As<ast::Texture>()) {
    out_ << "texture_";
    if (texture->Is<ast::DepthTexture>()) {
      out_ << "depth_";
    } else if (texture->Is<ast::SampledTexture>()) {
      /* nothing to emit */
    } else if (texture->Is<ast::MultisampledTexture>()) {
      out_ << "multisampled_";
    } else if (texture->Is<ast::StorageTexture>()) {
      out_ << "storage_";
    } else {
      diagnostics_.add_error("unknown texture type");
      return false;
    }

    switch (texture->dim()) {
      case ast::TextureDimension::k1d:
        out_ << "1d";
        break;
      case ast::TextureDimension::k2d:
        out_ << "2d";
        break;
      case ast::TextureDimension::k2dArray:
        out_ << "2d_array";
        break;
      case ast::TextureDimension::k3d:
        out_ << "3d";
        break;
      case ast::TextureDimension::kCube:
        out_ << "cube";
        break;
      case ast::TextureDimension::kCubeArray:
        out_ << "cube_array";
        break;
      default:
        diagnostics_.add_error("unknown texture dimension");
        return false;
    }

    if (auto* sampled = texture->As<ast::SampledTexture>()) {
      out_ << "<";
      if (!EmitType(sampled->type())) {
        return false;
      }
      out_ << ">";
    } else if (auto* ms = texture->As<ast::MultisampledTexture>()) {
      out_ << "<";
      if (!EmitType(ms->type())) {
        return false;
      }
      out_ << ">";
    } else if (auto* storage = texture->As<ast::StorageTexture>()) {
      out_ << "<";
      if (!EmitImageFormat(storage->image_format())) {
        return false;
      }
      out_ << ">";
    }

  } else if (ty->Is<ast::U32>()) {
    out_ << "u32";
  } else if (auto* vec = ty->As<ast::Vector>()) {
    out_ << "vec" << vec->size() << "<";
    if (!EmitType(vec->type())) {
      return false;
    }
    out_ << ">";
  } else if (ty->Is<ast::Void>()) {
    out_ << "void";
  } else if (auto* tn = ty->As<ast::TypeName>()) {
    out_ << program_->Symbols().NameFor(tn->name());
  } else {
    diagnostics_.add_error("unknown type in EmitType: " + ty->type_name());
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitStructType(const ast::Struct* str) {
  if (str->decorations().size()) {
    if (!EmitDecorations(str->decorations())) {
      return false;
    }
    out_ << std::endl;
  }
  out_ << "struct " << program_->Symbols().NameFor(str->name()) << " {"
       << std::endl;

  auto add_padding = [&](uint32_t size) {
    make_indent();
    out_ << "[[size(" << size << ")]]" << std::endl;
    make_indent();
    // Note: u32 is the smallest primitive we currently support. When WGSL
    // supports smaller types, this will need to be updated.
    out_ << UniqueIdentifier("padding") << " : u32;" << std::endl;
  };

  increment_indent();
  uint32_t offset = 0;
  for (auto* mem : str->members()) {
    // TODO(crbug.com/tint/798) move the [[offset]] decoration handling to the
    // transform::Wgsl sanitizer.
    if (auto* mem_sem = program_->Sem().Get(mem)) {
      offset = utils::RoundUp(mem_sem->Align(), offset);
      if (uint32_t padding = mem_sem->Offset() - offset) {
        add_padding(padding);
        offset += padding;
      }
      offset += mem_sem->Size();
    }

    // Offset decorations no longer exist in the WGSL spec, but are emitted
    // by the SPIR-V reader and are consumed by the Resolver(). These should not
    // be emitted, but instead struct padding fields should be emitted.
    ast::DecorationList decorations_sanitized;
    decorations_sanitized.reserve(mem->decorations().size());
    for (auto* deco : mem->decorations()) {
      if (!deco->Is<ast::StructMemberOffsetDecoration>()) {
        decorations_sanitized.emplace_back(deco);
      }
    }

    if (!decorations_sanitized.empty()) {
      make_indent();
      if (!EmitDecorations(decorations_sanitized)) {
        return false;
      }
      out_ << std::endl;
    }

    make_indent();
    out_ << program_->Symbols().NameFor(mem->symbol()) << " : ";
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

  if (!var->decorations().empty()) {
    if (!EmitDecorations(var->decorations())) {
      return false;
    }
    out_ << " ";
  }

  if (var->is_const()) {
    out_ << "let";
  } else {
    out_ << "var";
    auto sc = var->declared_storage_class();
    if (sc != ast::StorageClass::kNone) {
      out_ << "<" << sc << ">";
    }
  }

  out_ << " " << program_->Symbols().NameFor(var->symbol());

  if (auto* ty = var->type()) {
    out_ << " : ";
    if (!EmitType(ty)) {
      return false;
    }
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

bool GeneratorImpl::EmitDecorations(const ast::DecorationList& decos) {
  out_ << "[[";
  bool first = true;
  for (auto* deco : decos) {
    if (!first) {
      out_ << ", ";
    }
    first = false;

    if (auto* workgroup = deco->As<ast::WorkgroupDecoration>()) {
      auto values = workgroup->values();
      out_ << "workgroup_size(";
      for (int i = 0; i < 3; i++) {
        if (values[i]) {
          if (i > 0) {
            out_ << ", ";
          }
          if (auto* ident = values[i]->As<ast::IdentifierExpression>()) {
            if (!EmitIdentifier(ident)) {
              return false;
            }
          } else if (auto* scalar =
                         values[i]->As<ast::ScalarConstructorExpression>()) {
            if (!EmitScalarConstructor(scalar)) {
              return false;
            }
          } else {
            TINT_ICE(diagnostics_) << "Unsupported workgroup_size expression";
          }
        }
      }
      out_ << ")";
    } else if (deco->Is<ast::StructBlockDecoration>()) {
      out_ << "block";
    } else if (auto* stage = deco->As<ast::StageDecoration>()) {
      out_ << "stage(" << stage->value() << ")";
    } else if (auto* binding = deco->As<ast::BindingDecoration>()) {
      out_ << "binding(" << binding->value() << ")";
    } else if (auto* group = deco->As<ast::GroupDecoration>()) {
      out_ << "group(" << group->value() << ")";
    } else if (auto* location = deco->As<ast::LocationDecoration>()) {
      out_ << "location(" << location->value() << ")";
    } else if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
      out_ << "builtin(" << builtin->value() << ")";
    } else if (auto* override_deco = deco->As<ast::OverrideDecoration>()) {
      out_ << "override";
      if (override_deco->HasValue()) {
        out_ << "(" << override_deco->value() << ")";
      }
    } else if (auto* size = deco->As<ast::StructMemberSizeDecoration>()) {
      out_ << "size(" << size->size() << ")";
    } else if (auto* align = deco->As<ast::StructMemberAlignDecoration>()) {
      out_ << "align(" << align->align() << ")";
    } else if (auto* internal = deco->As<ast::InternalDecoration>()) {
      out_ << "internal(" << internal->Name() << ")";
    } else {
      TINT_ICE(diagnostics_)
          << "Unsupported decoration '" << deco->TypeInfo().name << "'";
      return false;
    }
  }
  out_ << "]]";

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
      diagnostics_.add_error("missing binary operation type");
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
    case ast::UnaryOp::kAddressOf:
      out_ << "&";
      break;
    case ast::UnaryOp::kIndirection:
      out_ << "*";
      break;
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

  diagnostics_.add_error("unknown statement type: " + program_->str(stmt));
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

std::string GeneratorImpl::UniqueIdentifier(const std::string& suffix) {
  auto const limit =
      std::numeric_limits<decltype(next_unique_identifier_suffix)>::max();
  while (next_unique_identifier_suffix < limit) {
    auto ident = "tint_" + std::to_string(next_unique_identifier_suffix);
    if (!suffix.empty()) {
      ident += "_" + suffix;
    }
    next_unique_identifier_suffix++;
    if (!program_->Symbols().Get(ident).IsValid()) {
      return ident;
    }
  }
  diagnostics_.add_error("Unable to generate a unique WGSL identifier");
  return "<invalid-ident>";
}

}  // namespace wgsl
}  // namespace writer
}  // namespace tint
