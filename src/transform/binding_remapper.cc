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

#include "src/transform/binding_remapper.h"

#include <utility>

#include "src/program_builder.h"
#include "src/semantic/variable.h"
#include "src/type/access_control_type.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::BindingRemapper::Remappings);

namespace tint {
namespace transform {

BindingRemapper::Remappings::Remappings(BindingPoints bp, AccessControls ac)
    : binding_points(std::move(bp)), access_controls(std::move(ac)) {}

BindingRemapper::Remappings::Remappings(const Remappings&) = default;
BindingRemapper::Remappings::~Remappings() = default;

BindingRemapper::BindingRemapper() = default;
BindingRemapper::~BindingRemapper() = default;

Transform::Output BindingRemapper::Run(const Program* in,
                                       const DataMap& datamap) {
  ProgramBuilder out;
  auto* remappings = datamap.Get<Remappings>();
  if (!remappings) {
    out.Diagnostics().add_error(
        "BindingRemapper did not find the remapping data");
    return Output(Program(std::move(out)));
  }
  CloneContext ctx(&out, in);
  for (auto* var : in->AST().GlobalVariables()) {
    if (auto binding_point = var->binding_point()) {
      BindingPoint from{binding_point.group->value(),
                        binding_point.binding->value()};

      // Replace any group or binding decorations.
      // Note: This has to be performed *before* remapping access controls, as
      // `ctx.Clone(var->decorations())` depend on these replacements.
      auto bp_it = remappings->binding_points.find(from);
      if (bp_it != remappings->binding_points.end()) {
        BindingPoint to = bp_it->second;
        auto* new_group = out.create<ast::GroupDecoration>(to.group);
        auto* new_binding = out.create<ast::BindingDecoration>(to.binding);
        ctx.Replace(binding_point.group, new_group);
        ctx.Replace(binding_point.binding, new_binding);
      }

      // Replace any access controls.
      auto ac_it = remappings->access_controls.find(from);
      if (ac_it != remappings->access_controls.end()) {
        ast::AccessControl ac = ac_it->second;
        auto* var_ty = in->Sem().Get(var)->Type();
        auto* ty = var_ty->UnwrapAliasIfNeeded();
        type::Type* inner_ty = nullptr;
        if (auto* old_ac = ty->As<type::AccessControl>()) {
          inner_ty = ctx.Clone(old_ac->type());
        } else {
          inner_ty = ctx.Clone(ty);
        }
        auto* new_ty = ctx.dst->create<type::AccessControl>(ac, inner_ty);
        auto* new_var = ctx.dst->create<ast::Variable>(
            ctx.Clone(var->source()), ctx.Clone(var->symbol()),
            var->declared_storage_class(), new_ty, var->is_const(),
            ctx.Clone(var->constructor()), ctx.Clone(var->decorations()));
        ctx.Replace(var, new_var);
      }
    }
  }
  ctx.Clone();
  return Output(Program(std::move(out)));
}

}  // namespace transform
}  // namespace tint
