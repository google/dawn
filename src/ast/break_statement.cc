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

#include "src/ast/break_statement.h"

namespace tint {
namespace ast {

BreakStatement::BreakStatement() : Statement() {}

BreakStatement::BreakStatement(const Source& source) : Statement(source) {}

BreakStatement::BreakStatement(StatementCondition condition,
                               std::unique_ptr<Expression> conditional)
    : Statement(),
      condition_(condition),
      conditional_(std::move(conditional)) {}

BreakStatement::BreakStatement(const Source& source,
                               StatementCondition condition,
                               std::unique_ptr<Expression> conditional)
    : Statement(source),
      condition_(condition),
      conditional_(std::move(conditional)) {}

BreakStatement::BreakStatement(BreakStatement&&) = default;

BreakStatement::~BreakStatement() = default;

bool BreakStatement::IsBreak() const {
  return true;
}

bool BreakStatement::IsValid() const {
  if (condition_ == StatementCondition::kNone)
    return conditional_ == nullptr;

  return conditional_ != nullptr && conditional_->IsValid();
}

void BreakStatement::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Break{";

  if (condition_ != StatementCondition::kNone) {
    out << std::endl;

    make_indent(out, indent + 2);
    out << condition_ << std::endl;
    conditional_->to_str(out, indent + 2);

    make_indent(out, indent);
  }
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
