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

namespace tint {
namespace ast {

CaseStatement::CaseStatement() : Statement() {}

CaseStatement::CaseStatement(CaseSelectorList conditions, StatementList body)
    : Statement(), conditions_(std::move(conditions)), body_(std::move(body)) {}

CaseStatement::CaseStatement(const Source& source,
                             CaseSelectorList conditions,
                             StatementList body)
    : Statement(source),
      conditions_(std::move(conditions)),
      body_(std::move(body)) {}

CaseStatement::CaseStatement(CaseStatement&&) = default;

CaseStatement::~CaseStatement() = default;

bool CaseStatement::IsCase() const {
  return true;
}

bool CaseStatement::IsValid() const {
  for (const auto& stmt : body_) {
    if (stmt == nullptr || !stmt->IsValid())
      return false;
  }
  return true;
}

void CaseStatement::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);

  if (IsDefault()) {
    out << "Default{" << std::endl;
  } else {
    out << "Case ";
    bool first = true;
    for (const auto& lit : conditions_) {
      if (!first)
        out << ", ";

      first = false;
      out << lit->to_str();
    }
    out << "{" << std::endl;
  }

  for (const auto& stmt : body_)
    stmt->to_str(out, indent + 2);

  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
