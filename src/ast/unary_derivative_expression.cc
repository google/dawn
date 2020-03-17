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

#include "src/ast/unary_derivative_expression.h"

namespace tint {
namespace ast {

UnaryDerivativeExpression::UnaryDerivativeExpression() : Expression() {}

UnaryDerivativeExpression::UnaryDerivativeExpression(
    UnaryDerivative op,
    DerivativeModifier mod,
    std::unique_ptr<Expression> param)
    : Expression(), op_(op), modifier_(mod), param_(std::move(param)) {}

UnaryDerivativeExpression::UnaryDerivativeExpression(
    const Source& source,
    UnaryDerivative op,
    DerivativeModifier mod,
    std::unique_ptr<Expression> param)
    : Expression(source), op_(op), modifier_(mod), param_(std::move(param)) {}

UnaryDerivativeExpression::~UnaryDerivativeExpression() = default;

bool UnaryDerivativeExpression::IsValid() const {
  if (param_ == nullptr || !param_->IsValid()) {
    return false;
  }
  return true;
}

void UnaryDerivativeExpression::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "UnaryDerivative{" << std::endl;
  make_indent(out, indent + 2);
  out << op_ << std::endl;
  make_indent(out, indent + 2);
  out << modifier_ << std::endl;
  param_->to_str(out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
