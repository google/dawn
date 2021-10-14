// Copyright 2021 The Tint Authors.
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

#include "src/ast/for_loop_statement.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::ForLoopStatement);

namespace tint {
namespace ast {

ForLoopStatement::ForLoopStatement(ProgramID program_id,
                                   const Source& source,
                                   Statement* initializer,
                                   Expression* condition,
                                   Statement* continuing,
                                   BlockStatement* body)
    : Base(program_id, source),
      initializer_(initializer),
      condition_(condition),
      continuing_(continuing),
      body_(body) {
  TINT_ASSERT(AST, body_);

  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, initializer_, program_id);
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, condition_, program_id);
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, continuing_, program_id);
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, body_, program_id);
}

ForLoopStatement::ForLoopStatement(ForLoopStatement&&) = default;

ForLoopStatement::~ForLoopStatement() = default;

ForLoopStatement* ForLoopStatement::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());

  auto* init = ctx->Clone(initializer_);
  auto* cond = ctx->Clone(condition_);
  auto* cont = ctx->Clone(continuing_);
  auto* b = ctx->Clone(body_);
  return ctx->dst->create<ForLoopStatement>(src, init, cond, cont, b);
}

}  // namespace ast
}  // namespace tint
