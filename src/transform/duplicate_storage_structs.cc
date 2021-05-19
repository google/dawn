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

#include "src/transform/duplicate_storage_structs.h"

#include <unordered_set>
#include <utility>
#include <vector>

#include "src/program_builder.h"
#include "src/sem/variable.h"
#include "src/utils/get_or_create.h"
#include "src/utils/hash.h"

namespace {
struct StructAccessPair {
  tint::sem::Struct const* str;
  tint::ast::AccessControl::Access access;

  bool operator==(const StructAccessPair& rhs) const {
    return str == rhs.str && access == rhs.access;
  }
};
}  // namespace

namespace std {
/// Custom std::hash specialization for StructAccessPair so
/// StructAccessPairs can be used as keys for std::unordered_map and
/// std::unordered_set.
template <>
class hash<StructAccessPair> {
 public:
  /// @param struct_access_pair the StructAccessPair to create a hash for
  /// @return the hash value
  inline std::size_t operator()(
      const StructAccessPair& struct_access_pair) const {
    return tint::utils::Hash(struct_access_pair.str, struct_access_pair.access);
  }
};
}  // namespace std

namespace tint {
namespace transform {
DuplicateStorageStructs::DuplicateStorageStructs() = default;
DuplicateStorageStructs::~DuplicateStorageStructs() = default;

Output DuplicateStorageStructs::Run(const Program* in, const DataMap&) {
  ProgramBuilder out;
  CloneContext ctx(&out, in);

  // Create struct duplicates of storage buffers per access type.
  // We do this by finding all storage variables, and creating a copy of the
  // Struct they point to per struct/access pair.

  std::unordered_map<StructAccessPair, ast::Struct*> sap_to_new_struct;

  for (auto* node : in->ASTNodes().Objects()) {
    if (auto* ast_var = node->As<ast::Variable>()) {
      auto* var = in->Sem().Get(ast_var);
      if (auto* str = var->Type()->UnwrapRef()->As<sem::Struct>()) {
        // Skip non-storage structs
        if (!str->UsedAs(ast::StorageClass::kStorage)) {
          continue;
        }

        auto sap = StructAccessPair{str, var->AccessControl()};

        auto* new_str = utils::GetOrCreate(sap_to_new_struct, sap, [&] {
          // For this ast::Struct/Access pair, create a clone of the existing
          // ast::Struct with a new name.
          auto* old_str = str->Declaration();
          std::string old_name = ctx.src->Symbols().NameFor(old_str->name());

          auto* new_struct = [&] {
            Symbol new_name = ctx.dst->Symbols().New(old_name);
            auto new_members = ctx.Clone(old_str->members());
            auto new_decorations = ctx.Clone(old_str->decorations());
            return ctx.dst->create<ast::Struct>(new_name, new_members,
                                                new_decorations);
          }();

          // Insert it after the original struct
          ctx.InsertAfter(ctx.src->AST().GlobalDeclarations(),
                          str->Declaration(), new_struct);

          // Remove the original struct
          ctx.Remove(ctx.src->AST().GlobalDeclarations(), old_str);

          return new_struct;
        });

        // Replace the existing variable with a new one who's type is an
        // AccessControl<TypeName<"New Struct">>.
        auto* new_var = [&] {
          auto new_name = ctx.Clone(ast_var->symbol());
          auto* new_ty = ctx.dst->ty.access(
              var->AccessControl(), ctx.dst->ty.type_name(new_str->name()));
          auto* new_constructor = ctx.Clone(ast_var->constructor());
          auto new_decorations = ctx.Clone(ast_var->decorations());
          return ctx.dst->create<ast::Variable>(
              new_name, ast_var->declared_storage_class(), new_ty,
              ast_var->is_const(), new_constructor, new_decorations);
        }();
        ctx.Replace(ast_var, new_var);
      }
    }
  }

  ctx.Clone();

  return Output(Program(std::move(out)));
}

}  // namespace transform
}  // namespace tint
