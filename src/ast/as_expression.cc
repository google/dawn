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

#include "src/ast/as_expression.h"

namespace tint {
namespace ast {

AsExpression::AsExpression() : Expression() {}

AsExpression::AsExpression(type::Type* type, std::unique_ptr<Expression> expr)
    : Expression(), type_(type), expr_(std::move(expr)) {}

AsExpression::AsExpression(const Source& source,
                           type::Type* type,
                           std::unique_ptr<Expression> expr)
    : Expression(source), type_(type), expr_(std::move(expr)) {}

AsExpression::AsExpression(AsExpression&&) = default;
AsExpression::~AsExpression() = default;

bool AsExpression::IsAs() const {
  return true;
}

bool AsExpression::IsValid() const {
  if (expr_ == nullptr || !expr_->IsValid())
    return false;
  return type_ != nullptr;
}

void AsExpression::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "As<" << type_->type_name() << ">{" << std::endl;
  expr_->to_str(out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
