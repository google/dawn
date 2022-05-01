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

#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/variable.h"
#include "src/tint/utils/map.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::AddSpirvBlockAttribute);
TINT_INSTANTIATE_TYPEINFO(tint::transform::AddSpirvBlockAttribute::SpirvBlockAttribute);

namespace tint::transform {

AddSpirvBlockAttribute::AddSpirvBlockAttribute() = default;

AddSpirvBlockAttribute::~AddSpirvBlockAttribute() = default;

void AddSpirvBlockAttribute::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    auto& sem = ctx.src->Sem();

    // Collect the set of structs that are nested in other types.
    std::unordered_set<const sem::Struct*> nested_structs;
    for (auto* node : ctx.src->ASTNodes().Objects()) {
        if (auto* arr = sem.Get<sem::Array>(node->As<ast::Array>())) {
            if (auto* nested_str = arr->ElemType()->As<sem::Struct>()) {
                nested_structs.insert(nested_str);
            }
        } else if (auto* str = sem.Get<sem::Struct>(node->As<ast::Struct>())) {
            for (auto* member : str->Members()) {
                if (auto* nested_str = member->Type()->As<sem::Struct>()) {
                    nested_structs.insert(nested_str);
                }
            }
        }
    }

    // A map from a type in the source program to a block-decorated wrapper that
    // contains it in the destination program.
    std::unordered_map<const sem::Type*, const ast::Struct*> wrapper_structs;

    // Process global variables that are buffers.
    for (auto* var : ctx.src->AST().GlobalVariables()) {
        auto* sem_var = sem.Get<sem::GlobalVariable>(var);
        if (var->declared_storage_class != ast::StorageClass::kStorage &&
            var->declared_storage_class != ast::StorageClass::kUniform) {
            continue;
        }

        auto* ty = sem.Get(var->type);
        auto* str = ty->As<sem::Struct>();
        if (!str || nested_structs.count(str)) {
            const char* kMemberName = "inner";

            // This is a non-struct or a struct that is nested somewhere else, so we
            // need to wrap it first.
            auto* wrapper = utils::GetOrCreate(wrapper_structs, ty, [&]() {
                auto* block = ctx.dst->ASTNodes().Create<SpirvBlockAttribute>(ctx.dst->ID());
                auto wrapper_name = ctx.src->Symbols().NameFor(var->symbol) + "_block";
                auto* ret = ctx.dst->create<ast::Struct>(
                    ctx.dst->Symbols().New(wrapper_name),
                    ast::StructMemberList{ctx.dst->Member(kMemberName, CreateASTTypeFor(ctx, ty))},
                    ast::AttributeList{block});
                ctx.InsertBefore(ctx.src->AST().GlobalDeclarations(), var, ret);
                return ret;
            });
            ctx.Replace(var->type, ctx.dst->ty.Of(wrapper));

            // Insert a member accessor to get the original type from the wrapper at
            // any usage of the original variable.
            for (auto* user : sem_var->Users()) {
                ctx.Replace(user->Declaration(),
                            ctx.dst->MemberAccessor(ctx.Clone(var->symbol), kMemberName));
            }
        } else {
            // Add a block attribute to this struct directly.
            auto* block = ctx.dst->ASTNodes().Create<SpirvBlockAttribute>(ctx.dst->ID());
            ctx.InsertFront(str->Declaration()->attributes, block);
        }
    }

    ctx.Clone();
}

AddSpirvBlockAttribute::SpirvBlockAttribute::SpirvBlockAttribute(ProgramID pid) : Base(pid) {}
AddSpirvBlockAttribute::SpirvBlockAttribute::~SpirvBlockAttribute() = default;
std::string AddSpirvBlockAttribute::SpirvBlockAttribute::InternalName() const {
    return "spirv_block";
}

const AddSpirvBlockAttribute::SpirvBlockAttribute*
AddSpirvBlockAttribute::SpirvBlockAttribute::Clone(CloneContext* ctx) const {
    return ctx->dst->ASTNodes().Create<AddSpirvBlockAttribute::SpirvBlockAttribute>(ctx->dst->ID());
}

}  // namespace tint::transform
