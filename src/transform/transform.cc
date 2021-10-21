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
#include <string>

#include "src/program_builder.h"
#include "src/sem/atomic_type.h"
#include "src/sem/block_statement.h"
#include "src/sem/depth_multisampled_texture_type.h"
#include "src/sem/for_loop_statement.h"
#include "src/sem/reference_type.h"
#include "src/sem/sampler_type.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::Transform);
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

Output Transform::Run(const Program* program, const DataMap& data /* = {} */) {
  ProgramBuilder builder;
  CloneContext ctx(&builder, program);
  Output output;
  Run(ctx, data, output.data);
  builder.SetTransformApplied(this);
  output.program = Program(std::move(builder));
  return output;
}

void Transform::Run(CloneContext& ctx, const DataMap&, DataMap&) {
  TINT_UNIMPLEMENTED(Transform, ctx.dst->Diagnostics())
      << "Transform::Run() unimplemented for " << TypeInfo().name;
}

bool Transform::Requires(CloneContext& ctx,
                         std::initializer_list<const ::tint::TypeInfo*> deps) {
  for (auto* dep : deps) {
    if (!ctx.src->HasTransformApplied(dep)) {
      ctx.dst->Diagnostics().add_error(
          diag::System::Transform, std::string(TypeInfo().name) +
                                       " depends on " + std::string(dep->name) +
                                       " but the dependency was not run");
      return false;
    }
  }
  return true;
}

void Transform::RemoveStatement(CloneContext& ctx, const ast::Statement* stmt) {
  auto* sem = ctx.src->Sem().Get(stmt);
  if (auto* block = tint::As<sem::BlockStatement>(sem->Parent())) {
    ctx.Remove(block->Declaration()->statements, stmt);
    return;
  }
  if (tint::Is<sem::ForLoopStatement>(sem->Parent())) {
    ctx.Replace(stmt, static_cast<ast::Expression*>(nullptr));
    return;
  }
  TINT_ICE(Transform, ctx.dst->Diagnostics())
      << "unable to remove statement from parent of type "
      << sem->TypeInfo().name;
}

const ast::Type* Transform::CreateASTTypeFor(CloneContext& ctx,
                                             const sem::Type* ty) {
  if (ty->Is<sem::Void>()) {
    return ctx.dst->create<ast::Void>();
  }
  if (ty->Is<sem::I32>()) {
    return ctx.dst->create<ast::I32>();
  }
  if (ty->Is<sem::U32>()) {
    return ctx.dst->create<ast::U32>();
  }
  if (ty->Is<sem::F32>()) {
    return ctx.dst->create<ast::F32>();
  }
  if (ty->Is<sem::Bool>()) {
    return ctx.dst->create<ast::Bool>();
  }
  if (auto* m = ty->As<sem::Matrix>()) {
    auto* el = CreateASTTypeFor(ctx, m->type());
    return ctx.dst->create<ast::Matrix>(el, m->rows(), m->columns());
  }
  if (auto* v = ty->As<sem::Vector>()) {
    auto* el = CreateASTTypeFor(ctx, v->type());
    return ctx.dst->create<ast::Vector>(el, v->Width());
  }
  if (auto* a = ty->As<sem::Array>()) {
    auto* el = CreateASTTypeFor(ctx, a->ElemType());
    ast::DecorationList decos;
    if (!a->IsStrideImplicit()) {
      decos.emplace_back(ctx.dst->create<ast::StrideDecoration>(a->Stride()));
    }
    if (a->IsRuntimeSized()) {
      return ctx.dst->ty.array(el, nullptr, std::move(decos));
    } else {
      return ctx.dst->ty.array(el, a->Count(), std::move(decos));
    }
  }
  if (auto* s = ty->As<sem::Struct>()) {
    return ctx.dst->create<ast::TypeName>(ctx.Clone(s->Declaration()->name));
  }
  if (auto* s = ty->As<sem::Reference>()) {
    return CreateASTTypeFor(ctx, s->StoreType());
  }
  if (auto* a = ty->As<sem::Atomic>()) {
    return ctx.dst->create<ast::Atomic>(CreateASTTypeFor(ctx, a->Type()));
  }
  if (auto* t = ty->As<sem::DepthTexture>()) {
    return ctx.dst->create<ast::DepthTexture>(t->dim());
  }
  if (auto* t = ty->As<sem::DepthMultisampledTexture>()) {
    return ctx.dst->create<ast::DepthMultisampledTexture>(t->dim());
  }
  if (auto* t = ty->As<sem::MultisampledTexture>()) {
    return ctx.dst->create<ast::MultisampledTexture>(
        t->dim(), CreateASTTypeFor(ctx, t->type()));
  }
  if (auto* t = ty->As<sem::SampledTexture>()) {
    return ctx.dst->create<ast::SampledTexture>(
        t->dim(), CreateASTTypeFor(ctx, t->type()));
  }
  if (auto* t = ty->As<sem::StorageTexture>()) {
    return ctx.dst->create<ast::StorageTexture>(
        t->dim(), t->image_format(), CreateASTTypeFor(ctx, t->type()),
        t->access());
  }
  if (auto* s = ty->As<sem::Sampler>()) {
    return ctx.dst->create<ast::Sampler>(s->kind());
  }
  TINT_UNREACHABLE(Transform, ctx.dst->Diagnostics())
      << "Unhandled type: " << ty->TypeInfo().name;
  return nullptr;
}

}  // namespace transform
}  // namespace tint
