// Copyright 2022 The Tint Authors.
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

#include "src/tint/transform/pad_structs.h"

#include <string>
#include <unordered_map>
#include <utility>

#include "src/tint/ast/parameter.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/module.h"
#include "src/tint/sem/type_constructor.h"

using namespace tint::number_suffixes;  // NOLINT

TINT_INSTANTIATE_TYPEINFO(tint::transform::PadStructs);

namespace tint::transform {

namespace {

void CreatePadding(utils::Vector<const ast::StructMember*, 8>* new_members,
                   utils::Hashset<const ast::StructMember*, 8>* padding_members,
                   ProgramBuilder* b,
                   uint32_t bytes) {
    for (uint32_t i = 0; i < bytes / 4u; ++i) {
        auto name = b->Symbols().New("pad");
        auto* member = b->Member(name, b->ty.u32());
        padding_members->Add(member);
        new_members->Push(member);
    }
}

}  // namespace

PadStructs::PadStructs() = default;

PadStructs::~PadStructs() = default;

void PadStructs::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    auto& sem = ctx.src->Sem();

    std::unordered_map<const ast::Struct*, const ast::Struct*> replaced_structs;
    utils::Hashset<const ast::StructMember*, 8> padding_members;

    ctx.ReplaceAll([&](const ast::Struct* ast_str) -> const ast::Struct* {
        auto* str = sem.Get<sem::Struct>(ast_str);
        if (!str || !str->IsHostShareable()) {
            return nullptr;
        }
        uint32_t offset = 0;
        bool has_runtime_sized_array = false;
        utils::Vector<const ast::StructMember*, 8> new_members;
        for (auto* mem : str->Members()) {
            auto name = ctx.src->Symbols().NameFor(mem->Name());

            if (offset < mem->Offset()) {
                CreatePadding(&new_members, &padding_members, ctx.dst, mem->Offset() - offset);
                offset = mem->Offset();
            }

            auto* ty = mem->Type();
            const ast::Type* type = CreateASTTypeFor(ctx, ty);

            new_members.Push(ctx.dst->Member(name, type));

            uint32_t size = ty->Size();
            if (ty->Is<sem::Struct>() && str->UsedAs(ast::AddressSpace::kUniform)) {
                // std140 structs should be padded out to 16 bytes.
                size = utils::RoundUp(16u, size);
            } else if (auto* array_ty = ty->As<sem::Array>()) {
                if (array_ty->IsRuntimeSized()) {
                    has_runtime_sized_array = true;
                }
            }
            offset += size;
        }

        // Add any required padding after the last member, if it's not a runtime-sized array.
        uint32_t struct_size = str->Size();
        if (str->UsedAs(ast::AddressSpace::kUniform)) {
            struct_size = utils::RoundUp(16u, struct_size);
        }
        if (offset < struct_size && !has_runtime_sized_array) {
            CreatePadding(&new_members, &padding_members, ctx.dst, struct_size - offset);
        }
        auto* new_struct = ctx.dst->create<ast::Struct>(ctx.Clone(ast_str->name),
                                                        std::move(new_members), utils::Empty);
        replaced_structs[ast_str] = new_struct;
        return new_struct;
    });

    ctx.ReplaceAll([&](const ast::CallExpression* ast_call) -> const ast::CallExpression* {
        if (ast_call->args.Length() == 0) {
            return nullptr;
        }

        auto* call = sem.Get<sem::Call>(ast_call);
        if (!call) {
            return nullptr;
        }
        auto* cons = call->Target()->As<sem::TypeConstructor>();
        if (!cons) {
            return nullptr;
        }
        auto* str = cons->ReturnType()->As<sem::Struct>();
        if (!str) {
            return nullptr;
        }

        auto* new_struct = replaced_structs[str->Declaration()];
        if (!new_struct) {
            return nullptr;
        }

        utils::Vector<const ast::Expression*, 8> new_args;

        auto* arg = ast_call->args.begin();
        for (auto* member : new_struct->members) {
            if (padding_members.Contains(member)) {
                new_args.Push(ctx.dst->Expr(0_u));
            } else {
                new_args.Push(ctx.Clone(*arg));
                arg++;
            }
        }
        return ctx.dst->Construct(CreateASTTypeFor(ctx, str), new_args);
    });

    ctx.Clone();
}

}  // namespace tint::transform
