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

#include "src/ast/return_statement.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::ReturnStatement);

namespace tint {
namespace ast {

ReturnStatement::ReturnStatement(ProgramID program_id, const Source& source)
    : Base(program_id, source), value_(nullptr) {}

ReturnStatement::ReturnStatement(ProgramID program_id,
                                 const Source& source,
                                 Expression* value)
    : Base(program_id, source), value_(value) {
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(value_, program_id);
}

ReturnStatement::ReturnStatement(ReturnStatement&&) = default;

ReturnStatement::~ReturnStatement() = default;

ReturnStatement* ReturnStatement::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto* ret = ctx->Clone(value());
  return ctx->dst->create<ReturnStatement>(src, ret);
}

void ReturnStatement::to_str(const sem::Info& sem,
                             std::ostream& out,
                             size_t indent) const {
  make_indent(out, indent);
  out << "Return{";

  if (value_) {
    out << std::endl;

    make_indent(out, indent + 2);
    out << "{" << std::endl;

    value_->to_str(sem, out, indent + 4);

    make_indent(out, indent + 2);
    out << "}" << std::endl;

    make_indent(out, indent);
  }
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
