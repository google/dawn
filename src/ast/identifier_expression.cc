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

#include "src/ast/identifier_expression.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::IdentifierExpression);

namespace tint {
namespace ast {

IdentifierExpression::IdentifierExpression(ProgramID program_id,
                                           const Source& source,
                                           Symbol sym)
    : Base(program_id, source), sym_(sym) {
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(sym_, program_id);
  TINT_ASSERT(sym_.IsValid());
}

IdentifierExpression::IdentifierExpression(IdentifierExpression&&) = default;

IdentifierExpression::~IdentifierExpression() = default;

IdentifierExpression* IdentifierExpression::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto sym = ctx->Clone(symbol());
  return ctx->dst->create<IdentifierExpression>(src, sym);
}

void IdentifierExpression::to_str(const sem::Info& sem,
                                  std::ostream& out,
                                  size_t indent) const {
  make_indent(out, indent);
  out << "Identifier[" << result_type_str(sem) << "]{" << sym_.to_str() << "}"
      << std::endl;
}

}  // namespace ast
}  // namespace tint
