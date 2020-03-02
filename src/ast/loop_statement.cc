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

#include "src/ast/loop_statement.h"

namespace tint {
namespace ast {

LoopStatement::LoopStatement(std::vector<std::unique_ptr<Statement>> body,
                             std::vector<std::unique_ptr<Statement>> continuing)
    : Statement(), body_(std::move(body)), continuing_(std::move(continuing)) {}

LoopStatement::LoopStatement(const Source& source,
                             std::vector<std::unique_ptr<Statement>> body,
                             std::vector<std::unique_ptr<Statement>> continuing)
    : Statement(source),
      body_(std::move(body)),
      continuing_(std::move(continuing)) {}

LoopStatement::~LoopStatement() = default;

bool LoopStatement::IsValid() const {
  return true;
}

void LoopStatement::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Loop{" << std::endl;

  for (const auto& stmt : body_)
    stmt->to_str(out, indent + 2);

  make_indent(out, indent + 2);
  out << "continuing {" << std::endl;

  for (const auto& stmt : continuing_)
    stmt->to_str(out, indent + 4);

  make_indent(out, indent + 2);
  out << "}" << std::endl;

  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
