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

#include "src/ast/identifier_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"

namespace tint {
namespace writer {
namespace msl {

GeneratorImpl::GeneratorImpl() = default;

GeneratorImpl::~GeneratorImpl() = default;

bool GeneratorImpl::Generate(const ast::Module&) {
  return true;
}

bool GeneratorImpl::EmitExpression(ast::Expression* expr) {
  if (expr->IsIdentifier()) {
    return EmitIdentifier(expr->AsIdentifier());
  }

  error_ = "unknown expression type";
  return false;
}

bool GeneratorImpl::EmitIdentifier(ast::IdentifierExpression* expr) {
  auto* ident = expr->AsIdentifier();
  if (ident->has_path()) {
    // TODO(dsinclair): Handle identifier with path
    error_ = "Identifier paths not handled yet.";
    return false;
  }
  out_ << ident->name();
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

bool GeneratorImpl::EmitStatementBlock(const ast::StatementList& statements) {
  out_ << " {" << std::endl;

  increment_indent();

  for (const auto& s : statements) {
    if (!EmitStatement(s.get())) {
      return false;
    }
  }

  decrement_indent();
  make_indent();
  out_ << "}";

  return true;
}

bool GeneratorImpl::EmitStatementBlockAndNewline(
    const ast::StatementList& statements) {
  const bool result = EmitStatementBlock(statements);
  if (result) {
    out_ << std::endl;
  }
  return result;
}

bool GeneratorImpl::EmitStatement(ast::Statement* stmt) {
  if (stmt->IsReturn()) {
    return EmitReturn(stmt->AsReturn());
  }

  error_ = "unknown statement type";
  return false;
}

bool GeneratorImpl::EmitType(ast::type::Type* type, const std::string& name) {
  if (type->IsAlias()) {
    auto* alias = type->AsAlias();
    out_ << alias->name();
  } else if (type->IsArray()) {
    auto* ary = type->AsArray();

    if (!EmitType(ary->type(), "")) {
      return false;
    }
    if (!name.empty()) {
      out_ << " " << name;
    }
    out_ << "[";
    if (ary->IsRuntimeArray()) {
      out_ << "1";
    } else {
      out_ << std::to_string(ary->size());
    }

    out_ << "]";
  } else if (type->IsBool()) {
    out_ << "bool";
  } else if (type->IsF32()) {
    out_ << "float";
  } else if (type->IsI32()) {
    out_ << "int";
  } else if (type->IsMatrix()) {
    auto* mat = type->AsMatrix();
    if (!EmitType(mat->type(), "")) {
      return false;
    }
    out_ << mat->columns() << "x" << mat->rows();
  } else if (type->IsPointer()) {
    auto* ptr = type->AsPointer();
    // TODO(dsinclair): Storage class?
    if (!EmitType(ptr->type(), "")) {
      return false;
    }
    out_ << "*";
  } else if (type->IsStruct()) {
    auto* str = type->AsStruct()->impl();
    // TODO(dsinclair): Block decoration?
    // if (str->decoration() != ast::StructDecoration::kNone) {
    // }
    out_ << "struct {" << std::endl;

    increment_indent();
    for (const auto& mem : str->members()) {
      make_indent();
      // TODO(dsinclair): Member decorations?
      // if (!mem->decorations().empty()) {
      // }

      if (!EmitType(mem->type(), mem->name())) {
        return false;
      }
      // Array member name will be output with the type
      if (!mem->type()->IsArray()) {
        out_ << mem->name();
      }
      out_ << ";" << std::endl;
    }
    decrement_indent();
    make_indent();

    out_ << "}";
  } else if (type->IsU32()) {
    out_ << "uint";
  } else if (type->IsVector()) {
    auto* vec = type->AsVector();
    if (!EmitType(vec->type(), "")) {
      return false;
    }
    out_ << vec->size();
  } else if (type->IsVoid()) {
    out_ << "void";
  } else {
    error_ = "unknown type in EmitType";
    return false;
  }

  return true;
}

}  // namespace msl
}  // namespace writer
}  // namespace tint
