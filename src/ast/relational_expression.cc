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

#include "src/ast/relational_expression.h"

namespace tint {
namespace ast {

RelationalExpression::RelationalExpression() : Expression() {}

RelationalExpression::RelationalExpression(Relation relation,
                                           std::unique_ptr<Expression> lhs,
                                           std::unique_ptr<Expression> rhs)
    : Expression(),
      relation_(relation),
      lhs_(std::move(lhs)),
      rhs_(std::move(rhs)) {}

RelationalExpression::RelationalExpression(const Source& source,
                                           Relation relation,
                                           std::unique_ptr<Expression> lhs,
                                           std::unique_ptr<Expression> rhs)
    : Expression(source),
      relation_(relation),
      lhs_(std::move(lhs)),
      rhs_(std::move(rhs)) {}

RelationalExpression::~RelationalExpression() = default;

bool RelationalExpression::IsValid() const {
  if (lhs_ == nullptr || !lhs_->IsValid()) {
    return false;
  }
  if (rhs_ == nullptr || !rhs_->IsValid()) {
    return false;
  }
  return relation_ != Relation::kNone;
}

void RelationalExpression::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Relation{" << std::endl;
  lhs_->to_str(out, indent + 2);

  make_indent(out, indent + 2);
  out << relation_ << std::endl;

  rhs_->to_str(out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
