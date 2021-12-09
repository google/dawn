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

#include "src/transform/single_entry_point.h"

#include <unordered_set>
#include <utility>

#include "src/program_builder.h"
#include "src/sem/function.h"
#include "src/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::SingleEntryPoint);
TINT_INSTANTIATE_TYPEINFO(tint::transform::SingleEntryPoint::Config);

namespace tint {
namespace transform {

SingleEntryPoint::SingleEntryPoint() = default;

SingleEntryPoint::~SingleEntryPoint() = default;

void SingleEntryPoint::Run(CloneContext& ctx, const DataMap& inputs, DataMap&) {
  auto* cfg = inputs.Get<Config>();
  if (cfg == nullptr) {
    ctx.dst->Diagnostics().add_error(
        diag::System::Transform,
        "missing transform data for " + std::string(TypeInfo().name));

    return;
  }

  // Find the target entry point.
  const ast::Function* entry_point = nullptr;
  for (auto* f : ctx.src->AST().Functions()) {
    if (!f->IsEntryPoint()) {
      continue;
    }
    if (ctx.src->Symbols().NameFor(f->symbol) == cfg->entry_point_name) {
      entry_point = f;
      break;
    }
  }
  if (entry_point == nullptr) {
    ctx.dst->Diagnostics().add_error(
        diag::System::Transform,
        "entry point '" + cfg->entry_point_name + "' not found");
    return;
  }

  auto& sem = ctx.src->Sem();

  // Build set of referenced module-scope variables for faster lookups later.
  std::unordered_set<const ast::Variable*> referenced_vars;
  for (auto* var : sem.Get(entry_point)->TransitivelyReferencedGlobals()) {
    referenced_vars.emplace(var->Declaration());
  }

  // Clone any module-scope variables, types, and functions that are statically
  // referenced by the target entry point.
  for (auto* decl : ctx.src->AST().GlobalDeclarations()) {
    if (auto* ty = decl->As<ast::TypeDecl>()) {
      // TODO(jrprice): Strip unused types.
      ctx.dst->AST().AddTypeDecl(ctx.Clone(ty));
    } else if (auto* var = decl->As<ast::Variable>()) {
      if (referenced_vars.count(var)) {
        if (var->is_const) {
          if (auto* deco = ast::GetDecoration<ast::OverrideDecoration>(
                  var->decorations)) {
            // It is an overridable constant
            if (!deco->has_value) {
              // If the decoration doesn't have numeric ID specified explicitly
              // Make their ids explicitly assigned in the decoration so that
              // they won't be affected by other stripped away constants
              auto* global = sem.Get(var)->As<sem::GlobalVariable>();
              const auto* new_deco =
                  ctx.dst->Override(deco->source, global->ConstantId());
              ctx.Replace(deco, new_deco);
            }
          }
        }
        ctx.dst->AST().AddGlobalVariable(ctx.Clone(var));
      }
    } else if (auto* func = decl->As<ast::Function>()) {
      if (sem.Get(func)->HasAncestorEntryPoint(entry_point->symbol)) {
        ctx.dst->AST().AddFunction(ctx.Clone(func));
      }
    } else {
      TINT_UNREACHABLE(Transform, ctx.dst->Diagnostics())
          << "unhandled global declaration: " << decl->TypeInfo().name;
      return;
    }
  }

  // Clone the entry point.
  ctx.dst->AST().AddFunction(ctx.Clone(entry_point));

  // Retain the list of applied transforms.
  // We need to do this manually since we are not going to use the top-level
  // ctx.Clone() function.
  ctx.dst->SetTransformApplied(ctx.src->TransformsApplied());
}

SingleEntryPoint::Config::Config(std::string entry_point)
    : entry_point_name(entry_point) {}

SingleEntryPoint::Config::Config(const Config&) = default;
SingleEntryPoint::Config::~Config() = default;
SingleEntryPoint::Config& SingleEntryPoint::Config::operator=(const Config&) =
    default;

}  // namespace transform
}  // namespace tint
