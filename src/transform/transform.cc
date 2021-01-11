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

#include "src/transform/transform.h"

#include "src/ast/block_statement.h"
#include "src/ast/clone_context.h"
#include "src/ast/function.h"

namespace tint {
namespace transform {

Transform::Transform() = default;
Transform::~Transform() = default;

ast::Function* Transform::CloneWithStatementsAtStart(
    ast::CloneContext* ctx,
    ast::Function* in,
    ast::StatementList statements) {
  for (auto* s : *in->body()) {
    statements.emplace_back(ctx->Clone(s));
  }
  return ctx->mod->create<ast::Function>(
      ctx->Clone(in->source()), ctx->Clone(in->symbol()), in->name_for_clone(),
      ctx->Clone(in->params()), ctx->Clone(in->return_type()),
      ctx->mod->create<ast::BlockStatement>(ctx->Clone(in->body()->source()),
                                            statements),
      ctx->Clone(in->decorations()));
}

}  // namespace transform
}  // namespace tint
