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

#include "src/ast/case_statement.h"

#include "src/clone_context.h"
#include "src/program_builder.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::CaseStatement);

namespace tint {
namespace ast {

CaseStatement::CaseStatement(const Source& source,
                             CaseSelectorList selectors,
                             BlockStatement* body)
    : Base(source), selectors_(selectors), body_(body) {}

CaseStatement::CaseStatement(CaseStatement&&) = default;

CaseStatement::~CaseStatement() = default;

CaseStatement* CaseStatement::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto sel = ctx->Clone(selectors_);
  auto* b = ctx->Clone(body_);
  return ctx->dst->create<CaseStatement>(src, sel, b);
}

bool CaseStatement::IsValid() const {
  return body_ != nullptr && body_->IsValid();
}

void CaseStatement::to_str(const semantic::Info& sem,
                           std::ostream& out,
                           size_t indent) const {
  make_indent(out, indent);

  if (IsDefault()) {
    out << "Default{" << std::endl;
  } else {
    out << "Case ";
    bool first = true;
    for (auto* selector : selectors_) {
      if (!first)
        out << ", ";

      first = false;
      out << selector->to_str(sem);
    }
    out << "{" << std::endl;
  }

  if (body_ != nullptr) {
    for (auto* stmt : *body_) {
      stmt->to_str(sem, out, indent + 2);
    }
  }

  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
