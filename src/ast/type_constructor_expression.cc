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

#include "src/ast/type_constructor_expression.h"

#include "src/clone_context.h"
#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::TypeConstructorExpression);

namespace tint {
namespace ast {

TypeConstructorExpression::TypeConstructorExpression(const Source& source,
                                                     type::Type* type,
                                                     ExpressionList values)
    : Base(source), type_(type), values_(std::move(values)) {}

TypeConstructorExpression::TypeConstructorExpression(
    TypeConstructorExpression&&) = default;

TypeConstructorExpression::~TypeConstructorExpression() = default;

TypeConstructorExpression* TypeConstructorExpression::Clone(
    CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto* ty = ctx->Clone(type());
  auto vals = ctx->Clone(values());
  return ctx->dst->create<TypeConstructorExpression>(src, ty, vals);
}

bool TypeConstructorExpression::IsValid() const {
  if (values_.empty()) {
    return true;
  }
  if (type_ == nullptr) {
    return false;
  }
  for (auto* val : values_) {
    if (val == nullptr || !val->IsValid()) {
      return false;
    }
  }
  return true;
}

void TypeConstructorExpression::to_str(const semantic::Info& sem,
                                       std::ostream& out,
                                       size_t indent) const {
  make_indent(out, indent);
  out << "TypeConstructor[" << result_type_str(sem) << "]{" << std::endl;
  make_indent(out, indent + 2);
  out << type_->type_name() << std::endl;

  for (auto* val : values_) {
    val->to_str(sem, out, indent + 2);
  }
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
