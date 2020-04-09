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

#include "src/ast/array_accessor_expression.h"

namespace tint {
namespace ast {

ArrayAccessorExpression::ArrayAccessorExpression() : Expression() {}

ArrayAccessorExpression::ArrayAccessorExpression(
    std::unique_ptr<Expression> array,
    std::unique_ptr<Expression> idx_expr)
    : Expression(), array_(std::move(array)), idx_expr_(std::move(idx_expr)) {}

ArrayAccessorExpression::ArrayAccessorExpression(
    const Source& source,
    std::unique_ptr<Expression> array,
    std::unique_ptr<Expression> idx_expr)
    : Expression(source),
      array_(std::move(array)),
      idx_expr_(std::move(idx_expr)) {}

ArrayAccessorExpression::ArrayAccessorExpression(ArrayAccessorExpression&&) =
    default;

ArrayAccessorExpression::~ArrayAccessorExpression() = default;

bool ArrayAccessorExpression::IsArrayAccessor() const {
  return true;
}

bool ArrayAccessorExpression::IsValid() const {
  if (array_ == nullptr || !array_->IsValid())
    return false;
  if (idx_expr_ == nullptr || !idx_expr_->IsValid())
    return false;

  return true;
}

void ArrayAccessorExpression::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "ArrayAccessor{" << std::endl;
  array_->to_str(out, indent + 2);
  idx_expr_->to_str(out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
