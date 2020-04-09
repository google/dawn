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

#include "src/ast/unary_op_expression.h"

namespace tint {
namespace ast {

UnaryOpExpression::UnaryOpExpression() : Expression() {}

UnaryOpExpression::UnaryOpExpression(UnaryOp op,
                                     std::unique_ptr<Expression> expr)
    : Expression(), op_(op), expr_(std::move(expr)) {}

UnaryOpExpression::UnaryOpExpression(const Source& source,
                                     UnaryOp op,
                                     std::unique_ptr<Expression> expr)
    : Expression(source), op_(op), expr_(std::move(expr)) {}

UnaryOpExpression::UnaryOpExpression(UnaryOpExpression&&) = default;

UnaryOpExpression::~UnaryOpExpression() = default;

bool UnaryOpExpression::IsUnaryOp() const {
  return true;
}

bool UnaryOpExpression::IsValid() const {
  return expr_ != nullptr && expr_->IsValid();
}

void UnaryOpExpression::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "UnaryOp{" << std::endl;
  make_indent(out, indent + 2);
  out << op_ << std::endl;
  expr_->to_str(out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
