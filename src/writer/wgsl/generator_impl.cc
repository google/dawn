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
#include "src/ast/binding_decoration.h"
#include "src/ast/bool_literal.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/call_expression.h"
#include "src/ast/cast_expression.h"
#include "src/ast/const_initializer_expression.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/initializer_expression.h"
#include "src/ast/int_literal.h"
#include "src/ast/location_decoration.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/set_decoration.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_initializer_expression.h"
#include "src/ast/uint_literal.h"

namespace tint {
namespace writer {
namespace wgsl {

GeneratorImpl::GeneratorImpl() = default;

GeneratorImpl::~GeneratorImpl() = default;

bool GeneratorImpl::Generate(const ast::Module& module) {
  for (const auto& import : module.imports()) {
    if (!EmitImport(import.get())) {
      return false;
    }
  }
  if (!module.imports().empty()) {
    out_ << std::endl;
  }

  for (const auto& ep : module.entry_points()) {
    if (!EmitEntryPoint(ep.get())) {
      return false;
    }
  }
  if (!module.entry_points().empty())
    out_ << std::endl;

  for (const auto& alias : module.alias_types()) {
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

  return true;
}

void GeneratorImpl::make_indent() {
  for (size_t i = 0; i < indent_; i++) {
    out_ << " ";
  }
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

bool GeneratorImpl::EmitEntryPoint(const ast::EntryPoint* ep) {
  make_indent();
  out_ << "entry_point " << ep->stage() << " ";
  if (!ep->name().empty() && ep->name() != ep->function_name()) {
    out_ << R"(as ")" << ep->name() << R"(" )";
  }
  out_ << "= " << ep->function_name() << ";" << std::endl;

  return true;
}

bool GeneratorImpl::EmitExpression(ast::Expression* expr) {
  if (expr->IsArrayAccessor()) {
    return EmitArrayAccessor(expr->AsArrayAccessor());
  }
  if (expr->IsAs()) {
    return EmitAs(expr->AsAs());
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
  if (expr->IsInitializer()) {
    return EmitInitializer(expr->AsInitializer());
  }
  if (expr->IsMemberAccessor()) {
    return EmitMemberAccessor(expr->AsMemberAccessor());
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

bool GeneratorImpl::EmitInitializer(ast::InitializerExpression* expr) {
  if (expr->IsConstInitializer()) {
    return EmitConstInitializer(expr->AsConstInitializer());
  }
  return EmitTypeInitializer(expr->AsTypeInitializer());
}

bool GeneratorImpl::EmitTypeInitializer(ast::TypeInitializerExpression* expr) {
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

bool GeneratorImpl::EmitConstInitializer(
    ast::ConstInitializerExpression* expr) {
  auto lit = expr->literal();
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
  } else if (lit->IsInt()) {
    out_ << lit->AsInt()->value();
  } else if (lit->IsUint()) {
    out_ << lit->AsUint()->value() << "u";
  } else {
    error_ = "unknown literal type";
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitIdentifier(ast::IdentifierExpression* expr) {
  bool first = true;
  for (const auto& part : expr->AsIdentifier()->name()) {
    if (!first) {
      out_ << "::";
    }
    first = false;
    out_ << part;
  }
  return true;
}

bool GeneratorImpl::EmitImport(const ast::Import* import) {
  make_indent();
  out_ << R"(import ")" << import->path() << R"(" as )" << import->name() << ";"
       << std::endl;
  return true;
}

bool GeneratorImpl::EmitType(ast::type::Type* type) {
  if (type->IsAlias()) {
    auto alias = type->AsAlias();
    out_ << alias->name();
  } else if (type->IsArray()) {
    auto ary = type->AsArray();
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
    auto mat = type->AsMatrix();
    out_ << "mat" << mat->columns() << "x" << mat->rows() << "<";
    if (!EmitType(mat->type())) {
      return false;
    }
    out_ << ">";
  } else if (type->IsPointer()) {
    auto ptr = type->AsPointer();
    out_ << "ptr<" << ptr->storage_class() << ", ";
    if (!EmitType(ptr->type())) {
      return false;
    }
    out_ << ">";
  } else if (type->IsStruct()) {
    auto str = type->AsStruct()->impl();
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

          out_ << "offset " << deco->AsOffset()->offset();
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
  } else if (type->IsU32()) {
    out_ << "u32";
  } else if (type->IsVector()) {
    auto vec = type->AsVector();
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
    if (var->storage_class() != ast::StorageClass::kNone) {
      out_ << "<" << var->storage_class() << ">";
    }
  }

  out_ << " " << var->name() << " : ";
  if (!EmitType(var->type())) {
    return false;
  }

  if (var->initializer() != nullptr) {
    out_ << " = ";
    if (!EmitExpression(var->initializer())) {
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
      out_ << "binding " << deco->AsBinding()->value();
    } else if (deco->IsSet()) {
      out_ << "set " << deco->AsSet()->value();
    } else if (deco->IsLocation()) {
      out_ << "location " << deco->AsLocation()->value();
    } else if (deco->IsBuiltin()) {
      out_ << "builtin " << deco->AsBuiltin()->value();
    } else {
      error_ = "unknown variable decoration";
      return false;
    }
  }
  out_ << "]] ";

  return true;
}

}  // namespace wgsl
}  // namespace writer
}  // namespace tint
