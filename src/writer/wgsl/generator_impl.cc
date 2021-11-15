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

#include "src/ast/access.h"
#include "src/ast/alias.h"
#include "src/ast/array.h"
#include "src/ast/atomic.h"
#include "src/ast/bool.h"
#include "src/ast/bool_literal_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/depth_texture.h"
#include "src/ast/external_texture.h"
#include "src/ast/f32.h"
#include "src/ast/float_literal_expression.h"
#include "src/ast/i32.h"
#include "src/ast/internal_decoration.h"
#include "src/ast/interpolate_decoration.h"
#include "src/ast/invariant_decoration.h"
#include "src/ast/matrix.h"
#include "src/ast/module.h"
#include "src/ast/multisampled_texture.h"
#include "src/ast/override_decoration.h"
#include "src/ast/pointer.h"
#include "src/ast/sampled_texture.h"
#include "src/ast/sint_literal_expression.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/storage_texture.h"
#include "src/ast/stride_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_member_align_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/struct_member_size_decoration.h"
#include "src/ast/type_name.h"
#include "src/ast/u32.h"
#include "src/ast/uint_literal_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/vector.h"
#include "src/ast/void.h"
#include "src/ast/workgroup_decoration.h"
#include "src/sem/struct.h"
#include "src/utils/math.h"
#include "src/utils/scoped_assignment.h"
#include "src/writer/float_to_string.h"

namespace tint {
namespace writer {
namespace wgsl {

GeneratorImpl::GeneratorImpl(const Program* program) : TextGenerator(program) {}

GeneratorImpl::~GeneratorImpl() = default;

bool GeneratorImpl::Generate() {
  // Generate global declarations in the order they appear in the module.
  for (auto* decl : program_->AST().GlobalDeclarations()) {
    if (auto* td = decl->As<ast::TypeDecl>()) {
      if (!EmitTypeDecl(td)) {
        return false;
      }
    } else if (auto* func = decl->As<ast::Function>()) {
      if (!EmitFunction(func)) {
        return false;
      }
    } else if (auto* var = decl->As<ast::Variable>()) {
      if (!EmitVariable(line(), var)) {
        return false;
      }
    } else {
      TINT_UNREACHABLE(Writer, diagnostics_);
      return false;
    }

    if (decl != program_->AST().GlobalDeclarations().back()) {
      line();
    }
  }

  return true;
}

bool GeneratorImpl::EmitTypeDecl(const ast::TypeDecl* ty) {
  if (auto* alias = ty->As<ast::Alias>()) {
    auto out = line();
    out << "type " << program_->Symbols().NameFor(alias->name) << " = ";
    if (!EmitType(out, alias->type)) {
      return false;
    }
    out << ";";
  } else if (auto* str = ty->As<ast::Struct>()) {
    if (!EmitStructType(str)) {
      return false;
    }
  } else {
    diagnostics_.add_error(
        diag::System::Writer,
        "unknown declared type: " + std::string(ty->TypeInfo().name));
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
  if (expr->Is<ast::PhonyExpression>()) {
    out << "_";
    return true;
  }
  if (auto* u = expr->As<ast::UnaryOpExpression>()) {
    return EmitUnaryOp(out, u);
  }

  diagnostics_.add_error(diag::System::Writer, "unknown expression type");
  return false;
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

bool GeneratorImpl::EmitMemberAccessor(
    std::ostream& out,
    const ast::MemberAccessorExpression* expr) {
  bool paren_lhs =
      !expr->structure->IsAnyOf<ast::IndexAccessorExpression,
                                ast::CallExpression, ast::IdentifierExpression,
                                ast::MemberAccessorExpression>();
  if (paren_lhs) {
    out << "(";
  }
  if (!EmitExpression(out, expr->structure)) {
    return false;
  }
  if (paren_lhs) {
    out << ")";
  }

  out << ".";

  return EmitExpression(out, expr->member);
}

bool GeneratorImpl::EmitBitcast(std::ostream& out,
                                const ast::BitcastExpression* expr) {
  out << "bitcast<";
  if (!EmitType(out, expr->type)) {
    return false;
  }

  out << ">(";
  if (!EmitExpression(out, expr->expr)) {
    return false;
  }

  out << ")";
  return true;
}

bool GeneratorImpl::EmitCall(std::ostream& out,
                             const ast::CallExpression* expr) {
  if (expr->target.name) {
    if (!EmitExpression(out, expr->target.name)) {
      return false;
    }
  } else if (expr->target.type) {
    if (!EmitType(out, expr->target.type)) {
      return false;
    }
  } else {
    TINT_ICE(Writer, diagnostics_)
        << "CallExpression target had neither a name or type";
    return false;
  }
  out << "(";

  bool first = true;
  const auto& args = expr->args;
  for (auto* arg : args) {
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

bool GeneratorImpl::EmitLiteral(std::ostream& out,
                                const ast::LiteralExpression* lit) {
  if (auto* bl = lit->As<ast::BoolLiteralExpression>()) {
    out << (bl->value ? "true" : "false");
  } else if (auto* fl = lit->As<ast::FloatLiteralExpression>()) {
    out << FloatToBitPreservingString(fl->value);
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

bool GeneratorImpl::EmitIdentifier(std::ostream& out,
                                   const ast::IdentifierExpression* expr) {
  out << program_->Symbols().NameFor(expr->symbol);
  return true;
}

bool GeneratorImpl::EmitFunction(const ast::Function* func) {
  if (func->decorations.size()) {
    if (!EmitDecorations(line(), func->decorations)) {
      return false;
    }
  }
  {
    auto out = line();
    out << "fn " << program_->Symbols().NameFor(func->symbol) << "(";

    bool first = true;
    for (auto* v : func->params) {
      if (!first) {
        out << ", ";
      }
      first = false;

      if (!v->decorations.empty()) {
        if (!EmitDecorations(out, v->decorations)) {
          return false;
        }
        out << " ";
      }

      out << program_->Symbols().NameFor(v->symbol) << " : ";

      if (!EmitType(out, v->type)) {
        return false;
      }
    }

    out << ")";

    if (!func->return_type->Is<ast::Void>() ||
        !func->return_type_decorations.empty()) {
      out << " -> ";

      if (!func->return_type_decorations.empty()) {
        if (!EmitDecorations(out, func->return_type_decorations)) {
          return false;
        }
        out << " ";
      }

      if (!EmitType(out, func->return_type)) {
        return false;
      }
    }

    if (func->body) {
      out << " {";
    }
  }

  if (func->body) {
    if (!EmitStatementsWithIndent(func->body->statements)) {
      return false;
    }
    line() << "}";
  }

  return true;
}

bool GeneratorImpl::EmitImageFormat(std::ostream& out,
                                    const ast::ImageFormat fmt) {
  switch (fmt) {
    case ast::ImageFormat::kNone:
      diagnostics_.add_error(diag::System::Writer, "unknown image format");
      return false;
    default:
      out << fmt;
  }
  return true;
}

bool GeneratorImpl::EmitAccess(std::ostream& out, const ast::Access access) {
  switch (access) {
    case ast::Access::kRead:
      out << "read";
      return true;
    case ast::Access::kWrite:
      out << "write";
      return true;
    case ast::Access::kReadWrite:
      out << "read_write";
      return true;
    default:
      break;
  }
  diagnostics_.add_error(diag::System::Writer, "unknown access");
  return false;
}

bool GeneratorImpl::EmitType(std::ostream& out, const ast::Type* ty) {
  if (auto* ary = ty->As<ast::Array>()) {
    for (auto* deco : ary->decorations) {
      if (auto* stride = deco->As<ast::StrideDecoration>()) {
        out << "[[stride(" << stride->stride << ")]] ";
      }
    }

    out << "array<";
    if (!EmitType(out, ary->type)) {
      return false;
    }

    if (!ary->IsRuntimeArray()) {
      out << ", ";
      if (!EmitExpression(out, ary->count)) {
        return false;
      }
    }

    out << ">";
  } else if (ty->Is<ast::Bool>()) {
    out << "bool";
  } else if (ty->Is<ast::F32>()) {
    out << "f32";
  } else if (ty->Is<ast::I32>()) {
    out << "i32";
  } else if (auto* mat = ty->As<ast::Matrix>()) {
    out << "mat" << mat->columns << "x" << mat->rows << "<";
    if (!EmitType(out, mat->type)) {
      return false;
    }
    out << ">";
  } else if (auto* ptr = ty->As<ast::Pointer>()) {
    out << "ptr<" << ptr->storage_class << ", ";
    if (!EmitType(out, ptr->type)) {
      return false;
    }
    if (ptr->access != ast::Access::kUndefined) {
      out << ", ";
      if (!EmitAccess(out, ptr->access)) {
        return false;
      }
    }
    out << ">";
  } else if (auto* atomic = ty->As<ast::Atomic>()) {
    out << "atomic<";
    if (!EmitType(out, atomic->type)) {
      return false;
    }
    out << ">";
  } else if (auto* sampler = ty->As<ast::Sampler>()) {
    out << "sampler";

    if (sampler->IsComparison()) {
      out << "_comparison";
    }
  } else if (ty->Is<ast::ExternalTexture>()) {
    out << "texture_external";
  } else if (auto* texture = ty->As<ast::Texture>()) {
    out << "texture_";
    if (texture->Is<ast::DepthTexture>()) {
      out << "depth_";
    } else if (texture->Is<ast::DepthMultisampledTexture>()) {
      out << "depth_multisampled_";
    } else if (texture->Is<ast::SampledTexture>()) {
      /* nothing to emit */
    } else if (texture->Is<ast::MultisampledTexture>()) {
      out << "multisampled_";
    } else if (texture->Is<ast::StorageTexture>()) {
      out << "storage_";
    } else {
      diagnostics_.add_error(diag::System::Writer, "unknown texture type");
      return false;
    }

    switch (texture->dim) {
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
                               "unknown texture dimension");
        return false;
    }

    if (auto* sampled = texture->As<ast::SampledTexture>()) {
      out << "<";
      if (!EmitType(out, sampled->type)) {
        return false;
      }
      out << ">";
    } else if (auto* ms = texture->As<ast::MultisampledTexture>()) {
      out << "<";
      if (!EmitType(out, ms->type)) {
        return false;
      }
      out << ">";
    } else if (auto* storage = texture->As<ast::StorageTexture>()) {
      out << "<";
      if (!EmitImageFormat(out, storage->format)) {
        return false;
      }
      out << ", ";
      if (!EmitAccess(out, storage->access)) {
        return false;
      }
      out << ">";
    }

  } else if (ty->Is<ast::U32>()) {
    out << "u32";
  } else if (auto* vec = ty->As<ast::Vector>()) {
    out << "vec" << vec->width << "<";
    if (!EmitType(out, vec->type)) {
      return false;
    }
    out << ">";
  } else if (ty->Is<ast::Void>()) {
    out << "void";
  } else if (auto* tn = ty->As<ast::TypeName>()) {
    out << program_->Symbols().NameFor(tn->name);
  } else {
    diagnostics_.add_error(
        diag::System::Writer,
        "unknown type in EmitType: " + std::string(ty->TypeInfo().name));
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitStructType(const ast::Struct* str) {
  if (str->decorations.size()) {
    if (!EmitDecorations(line(), str->decorations)) {
      return false;
    }
  }
  line() << "struct " << program_->Symbols().NameFor(str->name) << " {";

  auto add_padding = [&](uint32_t size) {
    line() << "[[size(" << size << ")]]";

    // Note: u32 is the smallest primitive we currently support. When WGSL
    // supports smaller types, this will need to be updated.
    line() << UniqueIdentifier("padding") << " : u32;";
  };

  increment_indent();
  uint32_t offset = 0;
  for (auto* mem : str->members) {
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
    decorations_sanitized.reserve(mem->decorations.size());
    for (auto* deco : mem->decorations) {
      if (!deco->Is<ast::StructMemberOffsetDecoration>()) {
        decorations_sanitized.emplace_back(deco);
      }
    }

    if (!decorations_sanitized.empty()) {
      if (!EmitDecorations(line(), decorations_sanitized)) {
        return false;
      }
    }

    auto out = line();
    out << program_->Symbols().NameFor(mem->symbol) << " : ";
    if (!EmitType(out, mem->type)) {
      return false;
    }
    out << ";";
  }
  decrement_indent();

  line() << "};";
  return true;
}

bool GeneratorImpl::EmitVariable(std::ostream& out, const ast::Variable* var) {
  if (!var->decorations.empty()) {
    if (!EmitDecorations(out, var->decorations)) {
      return false;
    }
    out << " ";
  }

  if (var->is_const) {
    out << "let";
  } else {
    out << "var";
    auto sc = var->declared_storage_class;
    auto ac = var->declared_access;
    if (sc != ast::StorageClass::kNone || ac != ast::Access::kUndefined) {
      out << "<" << sc;
      if (ac != ast::Access::kUndefined) {
        out << ", ";
        if (!EmitAccess(out, ac)) {
          return false;
        }
      }
      out << ">";
    }
  }

  out << " " << program_->Symbols().NameFor(var->symbol);

  if (auto* ty = var->type) {
    out << " : ";
    if (!EmitType(out, ty)) {
      return false;
    }
  }

  if (var->constructor != nullptr) {
    out << " = ";
    if (!EmitExpression(out, var->constructor)) {
      return false;
    }
  }
  out << ";";

  return true;
}

bool GeneratorImpl::EmitDecorations(std::ostream& out,
                                    const ast::DecorationList& decos) {
  out << "[[";
  bool first = true;
  for (auto* deco : decos) {
    if (!first) {
      out << ", ";
    }
    first = false;

    if (auto* workgroup = deco->As<ast::WorkgroupDecoration>()) {
      auto values = workgroup->Values();
      out << "workgroup_size(";
      for (int i = 0; i < 3; i++) {
        if (values[i]) {
          if (i > 0) {
            out << ", ";
          }
          if (!EmitExpression(out, values[i])) {
            return false;
          }
        }
      }
      out << ")";
    } else if (deco->Is<ast::StructBlockDecoration>()) {
      out << "block";
    } else if (auto* stage = deco->As<ast::StageDecoration>()) {
      out << "stage(" << stage->stage << ")";
    } else if (auto* binding = deco->As<ast::BindingDecoration>()) {
      out << "binding(" << binding->value << ")";
    } else if (auto* group = deco->As<ast::GroupDecoration>()) {
      out << "group(" << group->value << ")";
    } else if (auto* location = deco->As<ast::LocationDecoration>()) {
      out << "location(" << location->value << ")";
    } else if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
      out << "builtin(" << builtin->builtin << ")";
    } else if (auto* interpolate = deco->As<ast::InterpolateDecoration>()) {
      out << "interpolate(" << interpolate->type;
      if (interpolate->sampling != ast::InterpolationSampling::kNone) {
        out << ", " << interpolate->sampling;
      }
      out << ")";
    } else if (deco->Is<ast::InvariantDecoration>()) {
      out << "invariant";
    } else if (auto* override_deco = deco->As<ast::OverrideDecoration>()) {
      out << "override";
      if (override_deco->has_value) {
        out << "(" << override_deco->value << ")";
      }
    } else if (auto* size = deco->As<ast::StructMemberSizeDecoration>()) {
      out << "size(" << size->size << ")";
    } else if (auto* align = deco->As<ast::StructMemberAlignDecoration>()) {
      out << "align(" << align->align << ")";
    } else if (auto* stride = deco->As<ast::StrideDecoration>()) {
      out << "stride(" << stride->stride << ")";
    } else if (auto* internal = deco->As<ast::InternalDecoration>()) {
      out << "internal(" << internal->InternalName() << ")";
    } else {
      TINT_ICE(Writer, diagnostics_)
          << "Unsupported decoration '" << deco->TypeInfo().name << "'";
      return false;
    }
  }
  out << "]]";

  return true;
}

bool GeneratorImpl::EmitBinary(std::ostream& out,
                               const ast::BinaryExpression* expr) {
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
      out << ">>";
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

bool GeneratorImpl::EmitUnaryOp(std::ostream& out,
                                const ast::UnaryOpExpression* expr) {
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
  if (auto* f = stmt->As<ast::FallthroughStatement>()) {
    return EmitFallthrough(f);
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
    return EmitVariable(line(), v->variable);
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

bool GeneratorImpl::EmitBreak(const ast::BreakStatement*) {
  line() << "break;";
  return true;
}

bool GeneratorImpl::EmitCase(const ast::CaseStatement* stmt) {
  if (stmt->IsDefault()) {
    line() << "default: {";
  } else {
    auto out = line();
    out << "case ";

    bool first = true;
    for (auto* selector : stmt->selectors) {
      if (!first) {
        out << ", ";
      }

      first = false;
      if (!EmitLiteral(out, selector)) {
        return false;
      }
    }
    out << ": {";
  }

  if (!EmitStatementsWithIndent(stmt->body->statements)) {
    return false;
  }

  line() << "}";
  return true;
}

bool GeneratorImpl::EmitContinue(const ast::ContinueStatement*) {
  line() << "continue;";
  return true;
}

bool GeneratorImpl::EmitFallthrough(const ast::FallthroughStatement*) {
  line() << "fallthrough;";
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
      auto out = line();
      out << "} elseif (";
      if (!EmitExpression(out, e->condition)) {
        return false;
      }
      out << ") {";
    } else {
      line() << "} else {";
    }

    if (!EmitStatementsWithIndent(e->body->statements)) {
      return false;
    }
  }

  line() << "}";

  return true;
}

bool GeneratorImpl::EmitDiscard(const ast::DiscardStatement*) {
  line() << "discard;";
  return true;
}

bool GeneratorImpl::EmitLoop(const ast::LoopStatement* stmt) {
  line() << "loop {";
  increment_indent();

  if (!EmitStatements(stmt->body->statements)) {
    return false;
  }

  if (stmt->continuing && !stmt->continuing->Empty()) {
    line();
    line() << "continuing {";
    if (!EmitStatementsWithIndent(stmt->continuing->statements)) {
      return false;
    }
    line() << "}";
  }

  decrement_indent();
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

  TextBuffer cont_buf;
  if (auto* cont = stmt->continuing) {
    TINT_SCOPED_ASSIGNMENT(current_buffer_, &cont_buf);
    if (!EmitStatement(cont)) {
      return false;
    }
  }

  {
    auto out = line();
    out << "for";
    {
      ScopedParen sp(out);
      switch (init_buf.lines.size()) {
        case 0:  // No initializer
          break;
        case 1:  // Single line initializer statement
          out << TrimSuffix(init_buf.lines[0].content, ";");
          break;
        default:  // Block initializer statement
          for (size_t i = 1; i < init_buf.lines.size(); i++) {
            // Indent all by the first line
            init_buf.lines[i].indent += current_buffer_->current_indent;
          }
          out << TrimSuffix(init_buf.String(), "\n");
          break;
      }

      out << "; ";

      if (auto* cond = stmt->condition) {
        if (!EmitExpression(out, cond)) {
          return false;
        }
      }

      out << "; ";

      switch (cont_buf.lines.size()) {
        case 0:  // No continuing
          break;
        case 1:  // Single line continuing statement
          out << TrimSuffix(cont_buf.lines[0].content, ";");
          break;
        default:  // Block continuing statement
          for (size_t i = 1; i < cont_buf.lines.size(); i++) {
            // Indent all by the first line
            cont_buf.lines[i].indent += current_buffer_->current_indent;
          }
          out << TrimSuffix(cont_buf.String(), "\n");
          break;
      }
    }
    out << " {";
  }

  if (!EmitStatementsWithIndent(stmt->body->statements)) {
    return false;
  }

  line() << "}";

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

}  // namespace wgsl
}  // namespace writer
}  // namespace tint
