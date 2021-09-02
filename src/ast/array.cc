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

#include "src/ast/array.h"

#include <cmath>

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Array);

namespace tint {
namespace ast {

namespace {
// Returns the string representation of an array size expression.
std::string SizeExprToString(const ast::Expression* size,
                             const SymbolTable* symbols = nullptr) {
  if (auto* ident = size->As<ast::IdentifierExpression>()) {
    if (symbols) {
      return symbols->NameFor(ident->symbol());
    } else {
      return ident->symbol().to_str();
    }
  } else if (auto* scalar = size->As<ast::ScalarConstructorExpression>()) {
    auto* literal = scalar->literal()->As<ast::IntLiteral>();
    if (literal) {
      return std::to_string(literal->value_as_u32());
    }
  }
  // This will never be exposed to the user as the Resolver will reject this
  // expression for array size.
  return "<invalid>";
}
}  // namespace

Array::Array(ProgramID program_id,
             const Source& source,
             Type* subtype,
             ast::Expression* size,
             ast::DecorationList decorations)
    : Base(program_id, source),
      subtype_(subtype),
      size_(size),
      decos_(decorations) {}

Array::Array(Array&&) = default;

Array::~Array() = default;

std::string Array::type_name() const {
  TINT_ASSERT(AST, subtype_);

  std::string type_name = "__array" + subtype_->type_name();
  if (!IsRuntimeArray()) {
    type_name += "_" + SizeExprToString(size_);
  }
  for (auto* deco : decos_) {
    if (auto* stride = deco->As<ast::StrideDecoration>()) {
      type_name += "_stride_" + std::to_string(stride->stride());
    }
  }

  return type_name;
}

std::string Array::FriendlyName(const SymbolTable& symbols) const {
  std::ostringstream out;
  for (auto* deco : decos_) {
    if (auto* stride = deco->As<ast::StrideDecoration>()) {
      out << "[[stride(" << stride->stride() << ")]] ";
    }
  }
  out << "array<" << subtype_->FriendlyName(symbols);
  if (!IsRuntimeArray()) {
    out << ", " << SizeExprToString(size_, &symbols);
  }
  out << ">";
  return out.str();
}

Array* Array::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto* ty = ctx->Clone(type());
  auto* size = ctx->Clone(Size());
  auto decos = ctx->Clone(decorations());
  return ctx->dst->create<Array>(src, ty, size, decos);
}

}  // namespace ast
}  // namespace tint
