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

LoopStatement::LoopStatement()
    : Statement(),
      body_(std::make_unique<BlockStatement>()),
      continuing_(std::make_unique<BlockStatement>()) {}

LoopStatement::LoopStatement(std::unique_ptr<BlockStatement> body,
                             std::unique_ptr<BlockStatement> continuing)
    : Statement(), body_(std::move(body)), continuing_(std::move(continuing)) {}

LoopStatement::LoopStatement(const Source& source,
                             std::unique_ptr<BlockStatement> body,
                             std::unique_ptr<BlockStatement> continuing)
    : Statement(source),
      body_(std::move(body)),
      continuing_(std::move(continuing)) {}

LoopStatement::LoopStatement(LoopStatement&&) = default;

LoopStatement::~LoopStatement() = default;

bool LoopStatement::IsLoop() const {
  return true;
}

bool LoopStatement::IsValid() const {
  if (body_ == nullptr || !body_->IsValid()) {
    return false;
  }
  if (continuing_ == nullptr || !continuing_->IsValid()) {
    return false;
  }
  return true;
}

void LoopStatement::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Loop{" << std::endl;

  if (body_ != nullptr) {
    for (const auto& stmt : *body_) {
      stmt->to_str(out, indent + 2);
    }
  }

  if (continuing_ != nullptr && continuing_->size() > 0) {
    make_indent(out, indent + 2);
    out << "continuing {" << std::endl;

    for (const auto& stmt : *continuing_) {
      stmt->to_str(out, indent + 4);
    }

    make_indent(out, indent + 2);
    out << "}" << std::endl;
  }

  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
