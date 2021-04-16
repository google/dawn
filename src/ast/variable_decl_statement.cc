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

#include "src/ast/variable_decl_statement.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::VariableDeclStatement);

namespace tint {
namespace ast {

VariableDeclStatement::VariableDeclStatement(ProgramID program_id,
                                             const Source& source,
                                             Variable* variable)
    : Base(program_id, source), variable_(variable) {
  TINT_ASSERT(variable_);
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(variable_, program_id);
}

VariableDeclStatement::VariableDeclStatement(VariableDeclStatement&&) = default;

VariableDeclStatement::~VariableDeclStatement() = default;

VariableDeclStatement* VariableDeclStatement::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto* var = ctx->Clone(variable());
  return ctx->dst->create<VariableDeclStatement>(src, var);
}

void VariableDeclStatement::to_str(const sem::Info& sem,
                                   std::ostream& out,
                                   size_t indent) const {
  make_indent(out, indent);
  out << "VariableDeclStatement{" << std::endl;
  variable_->to_str(sem, out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
