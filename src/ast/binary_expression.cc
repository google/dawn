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

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::BinaryExpression);

namespace tint {
namespace ast {

BinaryExpression::BinaryExpression(ProgramID program_id,
                                   const Source& source,
                                   BinaryOp op,
                                   Expression* lhs,
                                   Expression* rhs)
    : Base(program_id, source), op_(op), lhs_(lhs), rhs_(rhs) {
  TINT_ASSERT(lhs_);
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(lhs_, program_id);
  TINT_ASSERT(rhs_);
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(rhs_, program_id);
  TINT_ASSERT(op_ != BinaryOp::kNone);
}

BinaryExpression::BinaryExpression(BinaryExpression&&) = default;

BinaryExpression::~BinaryExpression() = default;

BinaryExpression* BinaryExpression::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto* l = ctx->Clone(lhs_);
  auto* r = ctx->Clone(rhs_);
  return ctx->dst->create<BinaryExpression>(src, op_, l, r);
}

void BinaryExpression::to_str(const sem::Info& sem,
                              std::ostream& out,
                              size_t indent) const {
  make_indent(out, indent);
  out << "Binary[" << result_type_str(sem) << "]{" << std::endl;
  lhs_->to_str(sem, out, indent + 2);

  make_indent(out, indent + 2);
  out << op_ << std::endl;

  rhs_->to_str(sem, out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
