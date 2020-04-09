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

#include "src/ast/else_statement.h"

namespace tint {
namespace ast {

ElseStatement::ElseStatement() : Statement() {}

ElseStatement::ElseStatement(StatementList body)
    : Statement(), body_(std::move(body)) {}

ElseStatement::ElseStatement(std::unique_ptr<Expression> condition,
                             StatementList body)
    : Statement(), condition_(std::move(condition)), body_(std::move(body)) {}

ElseStatement::ElseStatement(const Source& source, StatementList body)
    : Statement(source), body_(std::move(body)) {}

ElseStatement::ElseStatement(const Source& source,
                             std::unique_ptr<Expression> condition,
                             StatementList body)
    : Statement(source),
      condition_(std::move(condition)),
      body_(std::move(body)) {}

ElseStatement::ElseStatement(ElseStatement&&) = default;

ElseStatement::~ElseStatement() = default;

bool ElseStatement::IsElse() const {
  return true;
}

bool ElseStatement::IsValid() const {
  for (const auto& stmt : body_) {
    if (stmt == nullptr || !stmt->IsValid())
      return false;
  }
  if (condition_)
    return condition_->IsValid();

  return true;
}

void ElseStatement::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Else{" << std::endl;
  if (condition_ != nullptr) {
    make_indent(out, indent + 2);
    out << "(" << std::endl;

    condition_->to_str(out, indent + 4);

    make_indent(out, indent + 2);
    out << ")" << std::endl;
  }

  make_indent(out, indent + 2);
  out << "{" << std::endl;

  for (const auto& stmt : body_)
    stmt->to_str(out, indent + 4);

  make_indent(out, indent + 2);
  out << "}" << std::endl;

  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
