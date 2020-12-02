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

#include "src/ast/binary_expression.h"

#include "src/ast/clone_context.h"
#include "src/ast/module.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::BinaryExpression);

namespace tint {
namespace ast {

BinaryExpression::BinaryExpression(BinaryOp op,
                                   Expression* lhs,
                                   Expression* rhs)
    : Base(), op_(op), lhs_(lhs), rhs_(rhs) {}

BinaryExpression::BinaryExpression(const Source& source,
                                   BinaryOp op,
                                   Expression* lhs,
                                   Expression* rhs)
    : Base(source), op_(op), lhs_(lhs), rhs_(rhs) {}

BinaryExpression::BinaryExpression(BinaryExpression&&) = default;

BinaryExpression::~BinaryExpression() = default;

BinaryExpression* BinaryExpression::Clone(CloneContext* ctx) const {
  return ctx->mod->create<BinaryExpression>(ctx->Clone(source()), op_,
                                            ctx->Clone(lhs_), ctx->Clone(rhs_));
}

bool BinaryExpression::IsValid() const {
  if (lhs_ == nullptr || !lhs_->IsValid()) {
    return false;
  }
  if (rhs_ == nullptr || !rhs_->IsValid()) {
    return false;
  }
  return op_ != BinaryOp::kNone;
}

void BinaryExpression::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Binary[" << result_type_str() << "]{" << std::endl;
  lhs_->to_str(out, indent + 2);

  make_indent(out, indent + 2);
  out << op_ << std::endl;

  rhs_->to_str(out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
