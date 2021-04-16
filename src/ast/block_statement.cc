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

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::BlockStatement);

namespace tint {
namespace ast {

BlockStatement::BlockStatement(ProgramID program_id,
                               const Source& source,
                               const StatementList& statements)
    : Base(program_id, source), statements_(std::move(statements)) {
  for (auto* stmt : *this) {
    TINT_ASSERT(stmt);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(stmt, program_id);
  }
}

BlockStatement::BlockStatement(BlockStatement&&) = default;

BlockStatement::~BlockStatement() = default;

BlockStatement* BlockStatement::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto stmts = ctx->Clone(statements_);
  return ctx->dst->create<BlockStatement>(src, stmts);
}

void BlockStatement::to_str(const sem::Info& sem,
                            std::ostream& out,
                            size_t indent) const {
  make_indent(out, indent);
  out << "Block{" << std::endl;

  for (auto* stmt : *this) {
    stmt->to_str(sem, out, indent + 2);
  }

  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
