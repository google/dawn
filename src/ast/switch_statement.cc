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

#include "src/ast/switch_statement.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::SwitchStatement);

namespace tint {
namespace ast {

SwitchStatement::SwitchStatement(ProgramID program_id,
                                 const Source& source,
                                 Expression* condition,
                                 CaseStatementList body)
    : Base(program_id, source), condition_(condition), body_(body) {
  TINT_ASSERT(condition_);
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(condition_, program_id);
  for (auto* stmt : body_) {
    TINT_ASSERT(stmt);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(stmt, program_id);
  }
}

SwitchStatement::SwitchStatement(SwitchStatement&&) = default;

SwitchStatement::~SwitchStatement() = default;

SwitchStatement* SwitchStatement::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto* cond = ctx->Clone(condition());
  auto b = ctx->Clone(body());
  return ctx->dst->create<SwitchStatement>(src, cond, b);
}

void SwitchStatement::to_str(const sem::Info& sem,
                             std::ostream& out,
                             size_t indent) const {
  make_indent(out, indent);
  out << "Switch{" << std::endl;
  condition_->to_str(sem, out, indent + 2);

  make_indent(out, indent + 2);
  out << "{" << std::endl;

  for (auto* stmt : body_) {
    stmt->to_str(sem, out, indent + 4);
  }

  make_indent(out, indent + 2);
  out << "}" << std::endl;

  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
