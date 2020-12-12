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

#include "src/ast/bitcast_expression.h"

#include "src/ast/clone_context.h"
#include "src/ast/module.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::BitcastExpression);

namespace tint {
namespace ast {

BitcastExpression::BitcastExpression(const Source& source,
                                     type::Type* type,
                                     Expression* expr)
    : Base(source), type_(type), expr_(expr) {}

BitcastExpression::BitcastExpression(BitcastExpression&&) = default;
BitcastExpression::~BitcastExpression() = default;

BitcastExpression* BitcastExpression::Clone(CloneContext* ctx) const {
  return ctx->mod->create<BitcastExpression>(
      ctx->Clone(source()), ctx->Clone(type_), ctx->Clone(expr_));
}

bool BitcastExpression::IsValid() const {
  if (expr_ == nullptr || !expr_->IsValid())
    return false;
  return type_ != nullptr;
}

void BitcastExpression::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Bitcast[" << result_type_str() << "]<" << type_->type_name() << ">{"
      << std::endl;
  expr_->to_str(out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
