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

ArrayAccessorExpression::ArrayAccessorExpression(ProgramID pid,
                                                 const Source& src,
                                                 const Expression* arr,
                                                 const Expression* idx)
    : Base(pid, src), array(arr), index(idx) {
  TINT_ASSERT(AST, array);
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, array, program_id);
  TINT_ASSERT(AST, idx);
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, idx, program_id);
}

ArrayAccessorExpression::ArrayAccessorExpression(ArrayAccessorExpression&&) =
    default;

ArrayAccessorExpression::~ArrayAccessorExpression() = default;

const ArrayAccessorExpression* ArrayAccessorExpression::Clone(
    CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source);
  auto* arr = ctx->Clone(array);
  auto* idx = ctx->Clone(index);
  return ctx->dst->create<ArrayAccessorExpression>(src, arr, idx);
}

}  // namespace ast
}  // namespace tint
