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

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::CaseStatement);

namespace tint {
namespace ast {

CaseStatement::CaseStatement(ProgramID program_id,
                             const Source& source,
                             CaseSelectorList selectors,
                             BlockStatement* body)
    : Base(program_id, source), selectors_(selectors), body_(body) {
  TINT_ASSERT(body_);
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(body_, program_id);
  for (auto* selector : selectors) {
    TINT_ASSERT(selector);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(selector, program_id);
  }
}

CaseStatement::CaseStatement(CaseStatement&&) = default;

CaseStatement::~CaseStatement() = default;

CaseStatement* CaseStatement::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto sel = ctx->Clone(selectors_);
  auto* b = ctx->Clone(body_);
  return ctx->dst->create<CaseStatement>(src, sel, b);
}

void CaseStatement::to_str(const sem::Info& sem,
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
