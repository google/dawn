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
                         StatementList body)
    : Statement(), condition_(std::move(condition)), body_(std::move(body)) {}

IfStatement::IfStatement(const Source& source,
                         std::unique_ptr<Expression> condition,
                         StatementList body)
    : Statement(source),
      condition_(std::move(condition)),
      body_(std::move(body)) {}

IfStatement::IfStatement(IfStatement&&) = default;

IfStatement::~IfStatement() = default;

bool IfStatement::IsIf() const {
  return true;
}

bool IfStatement::IsValid() const {
  if (condition_ == nullptr || !condition_->IsValid())
    return false;

  for (const auto& stmt : body_) {
    if (stmt == nullptr || !stmt->IsValid())
      return false;
  }

  bool found_else = false;
  for (const auto& el : else_statements_) {
    // Else statement must be last
    if (found_else)
      return false;

    if (el == nullptr || !el->IsValid())
      return false;

    if (el->condition() == nullptr)
      found_else = true;
  }

  for (const auto& stmt : premerge_) {
    if (stmt == nullptr || !stmt->IsValid())
      return false;
  }

  if (premerge_.size() > 0) {
    // Premerge only with a single else statement
    if (else_statements_.size() != 1)
      return false;
    // Must be an else, not an elseif
    if (else_statements_[0]->condition() != nullptr)
      return false;
  }

  return true;
}

void IfStatement::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "If{" << std::endl;

  // Open if conditional
  make_indent(out, indent + 2);
  out << "(" << std::endl;

  condition_->to_str(out, indent + 4);

  // Close if conditional
  make_indent(out, indent + 2);
  out << ")" << std::endl;

  // Open if body
  make_indent(out, indent + 2);
  out << "{" << std::endl;

  for (const auto& stmt : body_)
    stmt->to_str(out, indent + 4);

  // Close the if body
  make_indent(out, indent + 2);
  out << "}" << std::endl;

  // Close the If
  make_indent(out, indent);
  out << "}" << std::endl;

  for (const auto& e : else_statements_)
    e->to_str(out, indent);

  if (premerge_.size() > 0) {
    make_indent(out, indent);
    out << "premerge{" << std::endl;

    for (const auto& stmt : premerge_)
      stmt->to_str(out, indent + 2);

    make_indent(out, indent);
    out << "}" << std::endl;
  }
}

}  // namespace ast
}  // namespace tint
