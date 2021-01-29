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

#include "src/ast/case_statement.h"
#include "src/clone_context.h"
#include "src/program_builder.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::SwitchStatement);

namespace tint {
namespace ast {

SwitchStatement::SwitchStatement(const Source& source,
                                 Expression* condition,
                                 CaseStatementList body)
    : Base(source), condition_(condition), body_(body) {}

SwitchStatement::SwitchStatement(SwitchStatement&&) = default;

SwitchStatement::~SwitchStatement() = default;

SwitchStatement* SwitchStatement::Clone(CloneContext* ctx) const {
  return ctx->dst->create<SwitchStatement>(
      ctx->Clone(source()), ctx->Clone(condition_), ctx->Clone(body_));
}

bool SwitchStatement::IsValid() const {
  if (condition_ == nullptr || !condition_->IsValid()) {
    return false;
  }
  for (auto* stmt : body_) {
    if (stmt == nullptr || !stmt->IsValid()) {
      return false;
    }
  }
  return true;
}

void SwitchStatement::to_str(const semantic::Info& sem,
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
