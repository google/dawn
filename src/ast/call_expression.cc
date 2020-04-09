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

#include "src/ast/call_expression.h"

namespace tint {
namespace ast {

CallExpression::CallExpression() : Expression() {}

CallExpression::CallExpression(std::unique_ptr<Expression> func,
                               ExpressionList params)
    : Expression(), func_(std::move(func)), params_(std::move(params)) {}

CallExpression::CallExpression(const Source& source,
                               std::unique_ptr<Expression> func,
                               ExpressionList params)
    : Expression(source), func_(std::move(func)), params_(std::move(params)) {}

CallExpression::CallExpression(CallExpression&&) = default;

CallExpression::~CallExpression() = default;

bool CallExpression::IsCall() const {
  return true;
}

bool CallExpression::IsValid() const {
  if (func_ == nullptr || !func_->IsValid())
    return false;

  // All params must be valid
  for (const auto& param : params_) {
    if (param == nullptr || !param->IsValid())
      return false;
  }
  return true;
}

void CallExpression::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Call{" << std::endl;
  func_->to_str(out, indent + 2);

  make_indent(out, indent + 2);
  out << "(" << std::endl;
  for (const auto& param : params_)
    param->to_str(out, indent + 4);

  make_indent(out, indent + 2);
  out << ")" << std::endl;

  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
