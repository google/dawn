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

#include "src/ast/scalar_constructor_expression.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::ScalarConstructorExpression);

namespace tint {
namespace ast {

ScalarConstructorExpression::ScalarConstructorExpression(ProgramID pid,
                                                         const Source& src,
                                                         const Literal* lit)
    : Base(pid, src), literal(lit) {
  TINT_ASSERT(AST, literal);
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, literal, program_id);
}

ScalarConstructorExpression::ScalarConstructorExpression(
    ScalarConstructorExpression&&) = default;

ScalarConstructorExpression::~ScalarConstructorExpression() = default;

const ScalarConstructorExpression* ScalarConstructorExpression::Clone(
    CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source);
  auto* lit = ctx->Clone(literal);
  return ctx->dst->create<ScalarConstructorExpression>(src, lit);
}

}  // namespace ast
}  // namespace tint
