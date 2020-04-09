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

namespace tint {
namespace ast {

SwitchStatement::SwitchStatement() : Statement() {}

SwitchStatement::SwitchStatement(std::unique_ptr<Expression> condition,
                                 CaseStatementList body)
    : Statement(), condition_(std::move(condition)), body_(std::move(body)) {}

SwitchStatement::SwitchStatement(const Source& source,
                                 std::unique_ptr<Expression> condition,
                                 CaseStatementList body)
    : Statement(source),
      condition_(std::move(condition)),
      body_(std::move(body)) {}

bool SwitchStatement::IsSwitch() const {
  return true;
}

SwitchStatement::SwitchStatement(SwitchStatement&&) = default;

SwitchStatement::~SwitchStatement() = default;

bool SwitchStatement::IsValid() const {
  if (condition_ == nullptr || !condition_->IsValid()) {
    return false;
  }
  for (const auto& stmt : body_) {
    if (stmt == nullptr || !stmt->IsValid()) {
      return false;
    }
  }
  return true;
}

void SwitchStatement::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Switch{" << std::endl;
  condition_->to_str(out, indent + 2);

  make_indent(out, indent + 2);
  out << "{" << std::endl;

  for (const auto& stmt : body_) {
    stmt->to_str(out, indent + 4);
  }

  make_indent(out, indent + 2);
  out << "}" << std::endl;

  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
