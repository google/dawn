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

#include "src/tint/transform/transform.h"

#include <algorithm>
#include <string>

#include "src/tint/program_builder.h"
#include "src/tint/sem/atomic.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/depth_multisampled_texture.h"
#include "src/tint/sem/for_loop_statement.h"
#include "src/tint/sem/reference.h"
#include "src/tint/sem/sampler.h"
#include "src/tint/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::Transform);
TINT_INSTANTIATE_TYPEINFO(tint::transform::Data);

namespace tint::transform {

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

Output Transform::Run(const Program* src, const DataMap& data /* = {} */) const {
    Output output;
    if (auto program = Apply(src, data, output.data)) {
        output.program = std::move(program.value());
    } else {
        ProgramBuilder b;
        CloneContext ctx{&b, src, /* auto_clone_symbols */ true};
        ctx.Clone();
        output.program = Program(std::move(b));
    }
    return output;
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
        << "unable to remove statement from parent of type " << sem->TypeInfo().name;
}

const ast::Type* Transform::CreateASTTypeFor(CloneContext& ctx, const sem::Type* ty) {
    if (ty->Is<sem::Void>()) {
        return ctx.dst->create<ast::Void>();
    }
    if (ty->Is<sem::I32>()) {
        return ctx.dst->create<ast::I32>();
    }
    if (ty->Is<sem::U32>()) {
        return ctx.dst->create<ast::U32>();
    }
    if (ty->Is<sem::F16>()) {
        return ctx.dst->create<ast::F16>();
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
        utils::Vector<const ast::Attribute*, 1> attrs;
        if (!a->IsStrideImplicit()) {
            attrs.Push(ctx.dst->create<ast::StrideAttribute>(a->Stride()));
        }
        if (a->IsRuntimeSized()) {
            return ctx.dst->ty.array(el, nullptr, std::move(attrs));
        }
        if (auto* override = std::get_if<sem::NamedOverrideArrayCount>(&a->Count())) {
            auto* count = ctx.Clone(override->variable->Declaration());
            return ctx.dst->ty.array(el, count, std::move(attrs));
        }
        if (auto* override = std::get_if<sem::UnnamedOverrideArrayCount>(&a->Count())) {
            // If the array count is an unnamed (complex) override expression, then its not safe to
            // redeclare this type as we'd end up with two types that would not compare equal.
            // See crbug.com/tint/1764.
            // Look for a type alias for this array.
            for (auto* type_decl : ctx.src->AST().TypeDecls()) {
                if (auto* alias = type_decl->As<ast::Alias>()) {
                    if (ty == ctx.src->Sem().Get(alias)) {
                        // Alias found. Use the alias name to ensure types compare equal.
                        return ctx.dst->ty.type_name(ctx.Clone(alias->name));
                    }
                }
            }
            // Array is not aliased. Rebuild the array.
            auto* count = ctx.Clone(override->expr->Declaration());
            return ctx.dst->ty.array(el, count, std::move(attrs));
        }
        if (auto count = a->ConstantCount()) {
            return ctx.dst->ty.array(el, u32(count.value()), std::move(attrs));
        }
        TINT_ICE(Transform, ctx.dst->Diagnostics()) << sem::Array::kErrExpectedConstantCount;
        return ctx.dst->ty.array(el, u32(1), std::move(attrs));
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
    if (ty->Is<sem::ExternalTexture>()) {
        return ctx.dst->create<ast::ExternalTexture>();
    }
    if (auto* t = ty->As<sem::MultisampledTexture>()) {
        return ctx.dst->create<ast::MultisampledTexture>(t->dim(),
                                                         CreateASTTypeFor(ctx, t->type()));
    }
    if (auto* t = ty->As<sem::SampledTexture>()) {
        return ctx.dst->create<ast::SampledTexture>(t->dim(), CreateASTTypeFor(ctx, t->type()));
    }
    if (auto* t = ty->As<sem::StorageTexture>()) {
        return ctx.dst->create<ast::StorageTexture>(t->dim(), t->texel_format(),
                                                    CreateASTTypeFor(ctx, t->type()), t->access());
    }
    if (auto* s = ty->As<sem::Sampler>()) {
        return ctx.dst->create<ast::Sampler>(s->kind());
    }
    TINT_UNREACHABLE(Transform, ctx.dst->Diagnostics())
        << "Unhandled type: " << ty->TypeInfo().name;
    return nullptr;
}

}  // namespace tint::transform
