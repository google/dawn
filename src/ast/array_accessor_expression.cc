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

#include "src/ast/array_accessor_expression.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::ArrayAccessorExpression);

namespace tint {
namespace ast {

ArrayAccessorExpression::ArrayAccessorExpression(ProgramID program_id,
                                                 const Source& source,
                                                 Expression* array,
                                                 Expression* idx_expr)
    : Base(program_id, source), array_(array), idx_expr_(idx_expr) {
  TINT_ASSERT(array_);
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(array_, program_id);
  TINT_ASSERT(idx_expr_);
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(idx_expr_, program_id);
}

ArrayAccessorExpression::ArrayAccessorExpression(ArrayAccessorExpression&&) =
    default;

ArrayAccessorExpression::~ArrayAccessorExpression() = default;

ArrayAccessorExpression* ArrayAccessorExpression::Clone(
    CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto* arr = ctx->Clone(array_);
  auto* idx = ctx->Clone(idx_expr_);
  return ctx->dst->create<ArrayAccessorExpression>(src, arr, idx);
}

void ArrayAccessorExpression::to_str(const sem::Info& sem,
                                     std::ostream& out,
                                     size_t indent) const {
  make_indent(out, indent);
  out << "ArrayAccessor[" << result_type_str(sem) << "]{" << std::endl;
  array_->to_str(sem, out, indent + 2);
  idx_expr_->to_str(sem, out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
