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

#include "src/ast/return_statement.h"

namespace tint {
namespace ast {

ReturnStatement::ReturnStatement() : Statement() {}

ReturnStatement::ReturnStatement(const Source& source) : Statement(source) {}

ReturnStatement::ReturnStatement(std::unique_ptr<Expression> value)
    : Statement(), value_(std::move(value)) {}

ReturnStatement::ReturnStatement(const Source& source,
                                 std::unique_ptr<Expression> value)
    : Statement(source), value_(std::move(value)) {}

ReturnStatement::ReturnStatement(ReturnStatement&&) = default;

ReturnStatement::~ReturnStatement() = default;

bool ReturnStatement::IsReturn() const {
  return true;
}

bool ReturnStatement::IsValid() const {
  if (value_ != nullptr) {
    return value_->IsValid();
  }
  return true;
}

void ReturnStatement::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Return{";

  if (value_) {
    out << std::endl;

    make_indent(out, indent + 2);
    out << "{" << std::endl;

    value_->to_str(out, indent + 4);

    make_indent(out, indent + 2);
    out << "}" << std::endl;

    make_indent(out, indent);
  }
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
