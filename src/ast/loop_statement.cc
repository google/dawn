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

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::LoopStatement);

namespace tint {
namespace ast {

LoopStatement::LoopStatement(const Source& source,
                             BlockStatement* body,
                             BlockStatement* continuing)
    : Base(source), body_(body), continuing_(continuing) {
  TINT_ASSERT(body_);
}

LoopStatement::LoopStatement(LoopStatement&&) = default;

LoopStatement::~LoopStatement() = default;

LoopStatement* LoopStatement::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto* b = ctx->Clone(body_);
  auto* cont = ctx->Clone(continuing_);
  return ctx->dst->create<LoopStatement>(src, b, cont);
}

void LoopStatement::to_str(const semantic::Info& sem,
                           std::ostream& out,
                           size_t indent) const {
  make_indent(out, indent);
  out << "Loop{" << std::endl;

  if (body_ != nullptr) {
    for (auto* stmt : *body_) {
      stmt->to_str(sem, out, indent + 2);
    }
  }

  if (continuing_ != nullptr && continuing_->size() > 0) {
    make_indent(out, indent + 2);
    out << "continuing {" << std::endl;

    for (auto* stmt : *continuing_) {
      stmt->to_str(sem, out, indent + 4);
    }

    make_indent(out, indent + 2);
    out << "}" << std::endl;
  }

  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
