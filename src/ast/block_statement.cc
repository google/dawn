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

#include "src/ast/block_statement.h"

namespace tint {
namespace ast {

BlockStatement::BlockStatement() : Statement() {}

BlockStatement::BlockStatement(const Source& source) : Statement(source) {}

BlockStatement::BlockStatement(BlockStatement&&) = default;

BlockStatement::~BlockStatement() = default;

bool BlockStatement::IsBlock() const {
  return true;
}

bool BlockStatement::IsValid() const {
  for (const auto& stmt : *this) {
    if (stmt == nullptr || !stmt->IsValid()) {
      return false;
    }
  }
  return true;
}

void BlockStatement::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Block{" << std::endl;

  for (const auto& stmt : *this) {
    stmt->to_str(out, indent + 2);
  }

  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
