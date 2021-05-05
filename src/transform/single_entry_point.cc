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

TINT_INSTANTIATE_TYPEINFO(tint::transform::SingleEntryPoint::Config);

namespace tint {
namespace transform {

SingleEntryPoint::SingleEntryPoint() = default;

SingleEntryPoint::~SingleEntryPoint() = default;

Output SingleEntryPoint::Run(const Program* in, const DataMap& data) {
  ProgramBuilder out;

  auto* cfg = data.Get<Config>();
  if (cfg == nullptr) {
    out.Diagnostics().add_error("missing transform data for SingleEntryPoint");
    return Output(Program(std::move(out)));
  }

  // Find the target entry point.
  ast::Function* entry_point = nullptr;
  for (auto* f : in->AST().Functions()) {
    if (!f->IsEntryPoint()) {
      continue;
    }
    if (in->Symbols().NameFor(f->symbol()) == cfg->entry_point_name) {
      entry_point = f;
      break;
    }
  }
  if (entry_point == nullptr) {
    out.Diagnostics().add_error("entry point '" + cfg->entry_point_name +
                                "' not found");
    return Output(Program(std::move(out)));
  }

  CloneContext ctx(&out, in);

  auto* sem = in->Sem().Get(entry_point);

  // Build set of referenced module-scope variables for faster lookups later.
  std::unordered_set<const ast::Variable*> referenced_vars;
  for (auto* var : sem->ReferencedModuleVariables()) {
    referenced_vars.emplace(var->Declaration());
  }

  // Clone any module-scope variables, types, and functions that are statically
  // referenced by the target entry point.
  for (auto* decl : in->AST().GlobalDeclarations()) {
    if (auto* ty = decl->As<ast::NamedType>()) {
      // TODO(jrprice): Strip unused types.
      out.AST().AddConstructedType(ctx.Clone(ty));
    } else if (auto* var = decl->As<ast::Variable>()) {
      if (var->is_const() || referenced_vars.count(var)) {
        out.AST().AddGlobalVariable(ctx.Clone(var));
      }
    } else if (auto* func = decl->As<ast::Function>()) {
      if (in->Sem().Get(func)->HasAncestorEntryPoint(entry_point->symbol())) {
        out.AST().AddFunction(ctx.Clone(func));
      }
    } else {
      TINT_UNREACHABLE(out.Diagnostics())
          << "unhandled global declaration: " << decl->TypeInfo().name;
      return Output(Program(std::move(out)));
    }
  }

  // Clone the entry point.
  out.AST().AddFunction(ctx.Clone(entry_point));

  return Output(Program(std::move(out)));
}

SingleEntryPoint::Config::Config(std::string entry_point)
    : entry_point_name(entry_point) {}

SingleEntryPoint::Config::Config(const Config&) = default;
SingleEntryPoint::Config::~Config() = default;
SingleEntryPoint::Config& SingleEntryPoint::Config::operator=(const Config&) =
    default;

}  // namespace transform
}  // namespace tint
