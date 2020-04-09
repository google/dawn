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

#include "src/ast/unary_method_expression.h"

namespace tint {
namespace ast {

UnaryMethodExpression::UnaryMethodExpression() : Expression() {}

UnaryMethodExpression::UnaryMethodExpression(UnaryMethod op,
                                             ExpressionList params)
    : Expression(), op_(op), params_(std::move(params)) {}

UnaryMethodExpression::UnaryMethodExpression(const Source& source,
                                             UnaryMethod op,
                                             ExpressionList params)
    : Expression(source), op_(op), params_(std::move(params)) {}

UnaryMethodExpression::UnaryMethodExpression(UnaryMethodExpression&&) = default;

UnaryMethodExpression::~UnaryMethodExpression() = default;

bool UnaryMethodExpression::IsUnaryMethod() const {
  return true;
}

bool UnaryMethodExpression::IsValid() const {
  if (params_.empty()) {
    return false;
  }
  for (const auto& p : params_) {
    if (p == nullptr || !p->IsValid()) {
      return false;
    }
  }
  return true;
}

void UnaryMethodExpression::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);

  out << "UnaryMethod{" << std::endl;
  make_indent(out, indent + 2);
  out << op_ << std::endl;
  for (const auto& param : params_) {
    param->to_str(out, indent + 2);
  }
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
