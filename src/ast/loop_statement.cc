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

#include "src/clone_context.h"
#include "src/program.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::LoopStatement);

namespace tint {
namespace ast {

LoopStatement::LoopStatement(const Source& source,
                             BlockStatement* body,
                             BlockStatement* continuing)
    : Base(source), body_(body), continuing_(continuing) {}

LoopStatement::LoopStatement(LoopStatement&&) = default;

LoopStatement::~LoopStatement() = default;

LoopStatement* LoopStatement::Clone(CloneContext* ctx) const {
  return ctx->dst->create<LoopStatement>(
      ctx->Clone(source()), ctx->Clone(body_), ctx->Clone(continuing_));
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
    for (auto* stmt : *body_) {
      stmt->to_str(out, indent + 2);
    }
  }

  if (continuing_ != nullptr && continuing_->size() > 0) {
    make_indent(out, indent + 2);
    out << "continuing {" << std::endl;

    for (auto* stmt : *continuing_) {
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
