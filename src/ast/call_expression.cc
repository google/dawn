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

#include "src/ast/module.h"
#include "src/clone_context.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::CallExpression);

namespace tint {
namespace ast {

CallExpression::CallExpression(const Source& source,
                               Expression* func,
                               ExpressionList params)
    : Base(source), func_(func), params_(params) {}

CallExpression::CallExpression(CallExpression&&) = default;

CallExpression::~CallExpression() = default;

CallExpression* CallExpression::Clone(CloneContext* ctx) const {
  return ctx->dst->create<CallExpression>(
      ctx->Clone(source()), ctx->Clone(func_), ctx->Clone(params_));
}

bool CallExpression::IsValid() const {
  if (func_ == nullptr || !func_->IsValid())
    return false;

  // All params must be valid
  for (auto* param : params_) {
    if (param == nullptr || !param->IsValid())
      return false;
  }
  return true;
}

void CallExpression::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Call[" << result_type_str() << "]{" << std::endl;
  func_->to_str(out, indent + 2);

  make_indent(out, indent + 2);
  out << "(" << std::endl;
  for (auto* param : params_)
    param->to_str(out, indent + 4);

  make_indent(out, indent + 2);
  out << ")" << std::endl;

  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
