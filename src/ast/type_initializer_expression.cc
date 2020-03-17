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

#include "src/ast/type_initializer_expression.h"

namespace tint {
namespace ast {

TypeInitializerExpression::TypeInitializerExpression()
    : InitializerExpression() {}

TypeInitializerExpression::TypeInitializerExpression(
    type::Type* type,
    std::vector<std::unique_ptr<Expression>> values)
    : InitializerExpression(), type_(type), values_(std::move(values)) {}

TypeInitializerExpression::TypeInitializerExpression(
    const Source& source,
    type::Type* type,
    std::vector<std::unique_ptr<Expression>> values)
    : InitializerExpression(source), type_(type), values_(std::move(values)) {}

TypeInitializerExpression::~TypeInitializerExpression() = default;

bool TypeInitializerExpression::IsValid() const {
  if (values_.empty()) {
    return false;
  }
  if (type_ == nullptr) {
    return false;
  }
  for (const auto& val : values_) {
    if (val == nullptr || !val->IsValid()) {
      return false;
    }
  }
  return true;
}

void TypeInitializerExpression::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "TypeInitializer{" << std::endl;
  make_indent(out, indent + 2);
  out << type_->type_name() << std::endl;

  for (const auto& val : values_) {
    val->to_str(out, indent + 2);
  }
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
