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

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::IfStatement);

namespace tint {
namespace ast {

IfStatement::IfStatement(ProgramID program_id,
                         const Source& source,
                         Expression* condition,
                         BlockStatement* body,
                         ElseStatementList else_stmts)
    : Base(program_id, source),
      condition_(condition),
      body_(body),
      else_statements_(std::move(else_stmts)) {
  TINT_ASSERT(condition_);
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(condition_, program_id);
  TINT_ASSERT(body_);
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(body_, program_id);
  for (auto* el : else_statements_) {
    TINT_ASSERT(el);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(el, program_id);
  }
}

IfStatement::IfStatement(IfStatement&&) = default;

IfStatement::~IfStatement() = default;

IfStatement* IfStatement::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto* cond = ctx->Clone(condition_);
  auto* b = ctx->Clone(body_);
  auto el = ctx->Clone(else_statements_);
  return ctx->dst->create<IfStatement>(src, cond, b, el);
}

void IfStatement::to_str(const sem::Info& sem,
                         std::ostream& out,
                         size_t indent) const {
  make_indent(out, indent);
  out << "If{" << std::endl;

  // Open if conditional
  make_indent(out, indent + 2);
  out << "(" << std::endl;

  condition_->to_str(sem, out, indent + 4);

  // Close if conditional
  make_indent(out, indent + 2);
  out << ")" << std::endl;

  // Open if body
  make_indent(out, indent + 2);
  out << "{" << std::endl;

  if (body_ != nullptr) {
    for (auto* stmt : *body_) {
      stmt->to_str(sem, out, indent + 4);
    }
  }

  // Close the if body
  make_indent(out, indent + 2);
  out << "}" << std::endl;

  // Close the If
  make_indent(out, indent);
  out << "}" << std::endl;

  for (auto* e : else_statements_) {
    e->to_str(sem, out, indent);
  }
}

}  // namespace ast
}  // namespace tint
