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

#include <algorithm>

#include "src/program_builder.h"
#include "src/sem/reference_type.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::Data);

namespace tint {
namespace transform {

Data::Data() = default;
Data::Data(const Data&) = default;
Data::~Data() = default;
Data& Data::operator=(const Data&) = default;

DataMap::DataMap() = default;
DataMap::DataMap(DataMap&&) = default;
DataMap::~DataMap() = default;
DataMap& DataMap::operator=(DataMap&&) = default;

Output::Output() = default;
Output::Output(Program&& p) : program(std::move(p)) {}
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
  auto ret_decos = ctx->Clone(in->return_type_decorations());
  return ctx->dst->create<ast::Function>(source, symbol, params, return_type,
                                         body, decos, ret_decos);
}

ast::DecorationList Transform::RemoveDecorations(
    CloneContext* ctx,
    const ast::DecorationList& in,
    std::function<bool(const ast::Decoration*)> should_remove) {
  ast::DecorationList new_decorations;
  for (auto* deco : in) {
    if (!should_remove(deco)) {
      new_decorations.push_back(ctx->Clone(deco));
    }
  }
  return new_decorations;
}

ast::Type* Transform::CreateASTTypeFor(CloneContext* ctx, const sem::Type* ty) {
  if (ty->Is<sem::Void>()) {
    return ctx->dst->create<ast::Void>();
  }
  if (ty->Is<sem::I32>()) {
    return ctx->dst->create<ast::I32>();
  }
  if (ty->Is<sem::U32>()) {
    return ctx->dst->create<ast::U32>();
  }
  if (ty->Is<sem::F32>()) {
    return ctx->dst->create<ast::F32>();
  }
  if (ty->Is<sem::Bool>()) {
    return ctx->dst->create<ast::Bool>();
  }
  if (auto* m = ty->As<sem::Matrix>()) {
    auto* el = CreateASTTypeFor(ctx, m->type());
    return ctx->dst->create<ast::Matrix>(el, m->rows(), m->columns());
  }
  if (auto* v = ty->As<sem::Vector>()) {
    auto* el = CreateASTTypeFor(ctx, v->type());
    return ctx->dst->create<ast::Vector>(el, v->size());
  }
  if (auto* a = ty->As<sem::Array>()) {
    auto* el = CreateASTTypeFor(ctx, a->ElemType());
    ast::DecorationList decos;
    if (!a->IsStrideImplicit()) {
      decos.emplace_back(ctx->dst->create<ast::StrideDecoration>(a->Stride()));
    }
    return ctx->dst->create<ast::Array>(el, a->Count(), std::move(decos));
  }
  if (auto* s = ty->As<sem::Struct>()) {
    return ctx->dst->create<ast::TypeName>(
        ctx->Clone(s->Declaration()->name()));
  }
  if (auto* s = ty->As<sem::Reference>()) {
    return CreateASTTypeFor(ctx, s->StoreType());
  }
  TINT_UNREACHABLE(ctx->dst->Diagnostics())
      << "Unhandled type: " << ty->TypeInfo().name;
  return nullptr;
}

}  // namespace transform
}  // namespace tint
