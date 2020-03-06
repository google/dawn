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

#include "src/ast/const_initializer_expression.h"

namespace tint {
namespace ast {

ConstInitializerExpression::ConstInitializerExpression()
    : InitializerExpression() {}

ConstInitializerExpression::ConstInitializerExpression(
    std::unique_ptr<Literal> literal)
    : InitializerExpression(), literal_(std::move(literal)) {}

ConstInitializerExpression::ConstInitializerExpression(
    const Source& source,
    std::unique_ptr<Literal> litearl)
    : InitializerExpression(source), literal_(std::move(litearl)) {}

ConstInitializerExpression::~ConstInitializerExpression() = default;

bool ConstInitializerExpression::IsValid() const {
  return literal_ != nullptr;
}

void ConstInitializerExpression::to_str(std::ostream& out,
                                        size_t indent) const {
  make_indent(out, indent);
  out << "ConstInitializer{" << literal_->to_str() << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
