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

#include "src/ast/regardless_statement.h"

namespace tint {
namespace ast {

RegardlessStatement::RegardlessStatement() : Statement() {}

RegardlessStatement::RegardlessStatement(std::unique_ptr<Expression> condition,
                                         StatementList body)
    : Statement(), condition_(std::move(condition)), body_(std::move(body)) {}

RegardlessStatement::RegardlessStatement(const Source& source,
                                         std::unique_ptr<Expression> condition,
                                         StatementList body)
    : Statement(source),
      condition_(std::move(condition)),
      body_(std::move(body)) {}

RegardlessStatement::RegardlessStatement(RegardlessStatement&&) = default;
RegardlessStatement::~RegardlessStatement() = default;

bool RegardlessStatement::IsRegardless() const {
  return true;
}

bool RegardlessStatement::IsValid() const {
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

void RegardlessStatement::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Regardless{" << std::endl;

  condition_->to_str(out, indent + 2);
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
