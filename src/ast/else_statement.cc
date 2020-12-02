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

#include "src/ast/clone_context.h"
#include "src/ast/module.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::ElseStatement);

namespace tint {
namespace ast {

ElseStatement::ElseStatement(BlockStatement* body) : Base(), body_(body) {}

ElseStatement::ElseStatement(Expression* condition, BlockStatement* body)
    : Base(), condition_(condition), body_(body) {}

ElseStatement::ElseStatement(const Source& source, BlockStatement* body)
    : Base(source), body_(body) {}

ElseStatement::ElseStatement(const Source& source,
                             Expression* condition,
                             BlockStatement* body)
    : Base(source), condition_(condition), body_(body) {}

ElseStatement::ElseStatement(ElseStatement&&) = default;

ElseStatement::~ElseStatement() = default;

ElseStatement* ElseStatement::Clone(CloneContext* ctx) const {
  return ctx->mod->create<ElseStatement>(
      ctx->Clone(source()), ctx->Clone(condition_), ctx->Clone(body_));
}

bool ElseStatement::IsValid() const {
  if (body_ == nullptr || !body_->IsValid()) {
    return false;
  }
  return condition_ == nullptr || condition_->IsValid();
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

  if (body_ != nullptr) {
    for (auto* stmt : *body_) {
      stmt->to_str(out, indent + 4);
    }
  }

  make_indent(out, indent + 2);
  out << "}" << std::endl;

  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
