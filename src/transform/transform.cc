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
#include "src/ast/function.h"
#include "src/clone_context.h"
#include "src/program_builder.h"

TINT_INSTANTIATE_CLASS_ID(tint::transform::Data);

namespace tint {
namespace transform {

Data::Data() = default;

Data::Data(const Data&) = default;

Data::~Data() = default;

DataMap::DataMap() = default;

DataMap::DataMap(DataMap&&) = default;

DataMap::~DataMap() = default;

Transform::Output::Output() = default;

Transform::Output::Output(Program&& p) : program(std::move(p)) {}

Transform::Transform() = default;

Transform::~Transform() = default;

ast::Function* Transform::CloneWithStatementsAtStart(
    CloneContext* ctx,
    ast::Function* in,
    ast::StatementList statements) {
  for (auto* s : *in->body()) {
    statements.emplace_back(ctx->Clone(s));
  }
  // Clone arguments outside of create() call to have deterministic ordering
  auto source = ctx->Clone(in->source());
  auto symbol = ctx->Clone(in->symbol());
  auto params = ctx->Clone(in->params());
  auto* return_type = ctx->Clone(in->return_type());
  auto* body = ctx->dst->create<ast::BlockStatement>(
      ctx->Clone(in->body()->source()), statements);
  auto decos = ctx->Clone(in->decorations());
  return ctx->dst->create<ast::Function>(source, symbol, params, return_type,
                                         body, decos);
}

}  // namespace transform
}  // namespace tint
