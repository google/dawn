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

#include "src/tint/transform/add_spirv_block_attribute.h"

#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/variable.h"
#include "src/tint/utils/hashmap.h"
#include "src/tint/utils/hashset.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::AddSpirvBlockAttribute);
TINT_INSTANTIATE_TYPEINFO(tint::transform::AddSpirvBlockAttribute::SpirvBlockAttribute);

namespace tint::transform {

AddSpirvBlockAttribute::AddSpirvBlockAttribute() = default;

AddSpirvBlockAttribute::~AddSpirvBlockAttribute() = default;

void AddSpirvBlockAttribute::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    auto& sem = ctx.src->Sem();

    // Collect the set of structs that are nested in other types.
    utils::Hashset<const sem::Struct*, 8> nested_structs;
    for (auto* ty : ctx.src->Types()) {
        Switch(
            ty,
            [&](const sem::Array* arr) {
                if (auto* nested_str = arr->ElemType()->As<sem::Struct>()) {
                    nested_structs.Add(nested_str);
                }
            },
            [&](const sem::Struct* str) {
                for (auto* member : str->Members()) {
                    if (auto* nested_str = member->Type()->As<sem::Struct>()) {
                        nested_structs.Add(nested_str);
                    }
                }
            });
    }

    // A map from a type in the source program to a block-decorated wrapper that contains it in the
    // destination program.
    utils::Hashmap<const sem::Type*, const ast::Struct*, 8> wrapper_structs;

    // Process global 'var' declarations that are buffers.
    for (auto* global : ctx.src->AST().GlobalVariables()) {
        auto* var = sem.Get(global);
        if (!ast::IsHostShareable(var->StorageClass())) {
            // Not declared in a host-sharable storage class
            continue;
        }

        auto* ty = var->Type()->UnwrapRef();
        auto* str = ty->As<sem::Struct>();
        bool needs_wrapping = !str ||                        // Type is not a structure
                              nested_structs.Contains(str);  // Structure is nested by another type

        if (needs_wrapping) {
            const char* kMemberName = "inner";

            // This is a non-struct or a struct that is nested somewhere else, so we
            // need to wrap it first.
            auto* wrapper = wrapper_structs.GetOrCreate(ty, [&] {
                auto* block = ctx.dst->ASTNodes().Create<SpirvBlockAttribute>(
                    ctx.dst->ID(), ctx.dst->AllocateNodeID());
                auto wrapper_name = ctx.src->Symbols().NameFor(global->symbol) + "_block";
                auto* ret = ctx.dst->create<ast::Struct>(
                    ctx.dst->Symbols().New(wrapper_name),
                    utils::Vector{ctx.dst->Member(kMemberName, CreateASTTypeFor(ctx, ty))},
                    utils::Vector{block});
                ctx.InsertBefore(ctx.src->AST().GlobalDeclarations(), global, ret);
                return ret;
            });
            ctx.Replace(global->type, ctx.dst->ty.Of(wrapper));

            // Insert a member accessor to get the original type from the wrapper at
            // any usage of the original variable.
            for (auto* user : var->Users()) {
                ctx.Replace(user->Declaration(),
                            ctx.dst->MemberAccessor(ctx.Clone(global->symbol), kMemberName));
            }
        } else {
            // Add a block attribute to this struct directly.
            auto* block = ctx.dst->ASTNodes().Create<SpirvBlockAttribute>(
                ctx.dst->ID(), ctx.dst->AllocateNodeID());
            ctx.InsertFront(str->Declaration()->attributes, block);
        }
    }

    ctx.Clone();
}

AddSpirvBlockAttribute::SpirvBlockAttribute::SpirvBlockAttribute(ProgramID pid, ast::NodeID nid)
    : Base(pid, nid) {}
AddSpirvBlockAttribute::SpirvBlockAttribute::~SpirvBlockAttribute() = default;
std::string AddSpirvBlockAttribute::SpirvBlockAttribute::InternalName() const {
    return "spirv_block";
}

const AddSpirvBlockAttribute::SpirvBlockAttribute*
AddSpirvBlockAttribute::SpirvBlockAttribute::Clone(CloneContext* ctx) const {
    return ctx->dst->ASTNodes().Create<AddSpirvBlockAttribute::SpirvBlockAttribute>(
        ctx->dst->ID(), ctx->dst->AllocateNodeID());
}

}  // namespace tint::transform
