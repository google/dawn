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

#include "src/ast/if_statement.h"

#include "src/ast/else_statement.h"

namespace tint {
namespace ast {

IfStatement::IfStatement() : Statement() {}

IfStatement::IfStatement(std::unique_ptr<Expression> condition,
                         std::vector<std::unique_ptr<Statement>> body)
    : Statement(), condition_(std::move(condition)), body_(std::move(body)) {}

IfStatement::IfStatement(const Source& source,
                         std::unique_ptr<Expression> condition,
                         std::vector<std::unique_ptr<Statement>> body)
    : Statement(source),
      condition_(std::move(condition)),
      body_(std::move(body)) {}

IfStatement::~IfStatement() = default;

bool IfStatement::IsValid() const {
  if (condition_ == nullptr)
    return false;

  if (premerge_.size() > 0 && else_statements_.size() > 1)
    return false;

  return true;
}

void IfStatement::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "If{" << std::endl;
  condition_->to_str(out, indent + 2);
  out << std::endl;
  make_indent(out, indent + 2);
  out << "{" << std::endl;

  for (const auto& stmt : body_)
    stmt->to_str(out, indent + 4);

  make_indent(out, indent + 2);
  out << "}" << std::endl;

  for (const auto& e : else_statements_)
    e->to_str(out, indent + 2);

  if (premerge_.size() > 0) {
    make_indent(out, indent + 2);
    out << "premerge{" << std::endl;

    for (const auto& stmt : premerge_)
      stmt->to_str(out, indent + 4);

    make_indent(out, indent + 2);
    out << "}" << std::endl;
  }
}

}  // namespace ast
}  // namespace tint
