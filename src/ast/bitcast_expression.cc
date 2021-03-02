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

#include "src/clone_context.h"
#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::BitcastExpression);

namespace tint {
namespace ast {

BitcastExpression::BitcastExpression(const Source& source,
                                     type::Type* type,
                                     Expression* expr)
    : Base(source), type_(type), expr_(expr) {}

BitcastExpression::BitcastExpression(BitcastExpression&&) = default;
BitcastExpression::~BitcastExpression() = default;

BitcastExpression* BitcastExpression::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto* ty = ctx->Clone(type_);
  auto* e = ctx->Clone(expr_);
  return ctx->dst->create<BitcastExpression>(src, ty, e);
}

bool BitcastExpression::IsValid() const {
  if (expr_ == nullptr || !expr_->IsValid())
    return false;
  return type_ != nullptr;
}

void BitcastExpression::to_str(const semantic::Info& sem,
                               std::ostream& out,
                               size_t indent) const {
  make_indent(out, indent);
  out << "Bitcast[" << result_type_str(sem) << "]<" << type_->type_name()
      << ">{" << std::endl;
  expr_->to_str(sem, out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
