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

#include "src/transform/add_spirv_block_decoration.h"

#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "src/program_builder.h"
#include "src/sem/variable.h"
#include "src/utils/map.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::AddSpirvBlockDecoration);
TINT_INSTANTIATE_TYPEINFO(
    tint::transform::AddSpirvBlockDecoration::SpirvBlockDecoration);

namespace tint {
namespace transform {

AddSpirvBlockDecoration::AddSpirvBlockDecoration() = default;

AddSpirvBlockDecoration::~AddSpirvBlockDecoration() = default;

void AddSpirvBlockDecoration::Run(CloneContext& ctx, const DataMap&, DataMap&) {
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

  // A map from a struct in the source program to a block-decorated wrapper that
  // contains it in the destination program.
  std::unordered_map<const sem::Struct*, const ast::Struct*> wrapper_structs;

  // Process global variables that are buffers.
  for (auto* var : ctx.src->AST().GlobalVariables()) {
    auto* sem_var = sem.Get<sem::GlobalVariable>(var);
    if (var->declared_storage_class != ast::StorageClass::kStorage &&
        var->declared_storage_class != ast::StorageClass::kUniform) {
      continue;
    }

    auto* str = sem.Get<sem::Struct>(var->type);
    if (!str) {
      // TODO(jrprice): We'll need to wrap these too, when WGSL supports this.
      TINT_ICE(Transform, ctx.dst->Diagnostics())
          << "non-struct buffer types are not yet supported";
      continue;
    }

    if (nested_structs.count(str)) {
      const char* kInnerStructMemberName = "inner";

      // This struct is nested somewhere else, so we need to wrap it first.
      auto* wrapper = utils::GetOrCreate(wrapper_structs, str, [&]() {
        auto* block =
            ctx.dst->ASTNodes().Create<SpirvBlockDecoration>(ctx.dst->ID());
        auto wrapper_name =
            ctx.src->Symbols().NameFor(str->Declaration()->name) + "_block";
        auto* ret = ctx.dst->create<ast::Struct>(
            ctx.dst->Symbols().New(wrapper_name),
            ast::StructMemberList{ctx.dst->Member(kInnerStructMemberName,
                                                  CreateASTTypeFor(ctx, str))},
            ast::DecorationList{block});
        ctx.InsertAfter(ctx.src->AST().GlobalDeclarations(), str->Declaration(),
                        ret);
        return ret;
      });
      ctx.Replace(var->type, ctx.dst->ty.Of(wrapper));

      // Insert a member accessor to get the original struct from the wrapper at
      // any usage of the original variable.
      for (auto* user : sem_var->Users()) {
        ctx.Replace(user->Declaration(),
                    ctx.dst->MemberAccessor(ctx.Clone(var->symbol),
                                            kInnerStructMemberName));
      }
    } else {
      // Add a block decoration to this struct directly.
      auto* block =
          ctx.dst->ASTNodes().Create<SpirvBlockDecoration>(ctx.dst->ID());
      ctx.InsertFront(str->Declaration()->decorations, block);
    }
  }

  ctx.Clone();
}

AddSpirvBlockDecoration::SpirvBlockDecoration::SpirvBlockDecoration(
    ProgramID pid)
    : Base(pid) {}
AddSpirvBlockDecoration::SpirvBlockDecoration::~SpirvBlockDecoration() =
    default;
std::string AddSpirvBlockDecoration::SpirvBlockDecoration::InternalName()
    const {
  return "spirv_block";
}

const AddSpirvBlockDecoration::SpirvBlockDecoration*
AddSpirvBlockDecoration::SpirvBlockDecoration::Clone(CloneContext* ctx) const {
  return ctx->dst->ASTNodes()
      .Create<AddSpirvBlockDecoration::SpirvBlockDecoration>(ctx->dst->ID());
}

}  // namespace transform
}  // namespace tint
