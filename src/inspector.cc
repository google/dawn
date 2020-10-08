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

#include "src/inspector.h"

#include <algorithm>

#include "src/ast/function.h"

namespace tint {
namespace inspector {

EntryPoint::EntryPoint() = default;
EntryPoint::EntryPoint(EntryPoint&) = default;
EntryPoint::EntryPoint(EntryPoint&&) = default;
EntryPoint::~EntryPoint() = default;

Inspector::Inspector(const ast::Module& module) : module_(module) {}

Inspector::~Inspector() = default;

std::vector<EntryPoint> Inspector::GetEntryPoints() {
  std::vector<EntryPoint> result;

  for (const auto& func : module_.functions()) {
    if (func->IsEntryPoint()) {
      EntryPoint entry_point;
      entry_point.name = func->name();
      entry_point.stage = func->pipeline_stage();
      std::tie(entry_point.workgroup_size_x, entry_point.workgroup_size_y,
               entry_point.workgroup_size_z) = func->workgroup_size();

      for (auto* var : func->referenced_module_variables()) {
        if (var->storage_class() == ast::StorageClass::kInput) {
          entry_point.input_variables.push_back(var->name());
        } else {
          entry_point.output_variables.push_back(var->name());
        }
      }
      result.push_back(std::move(entry_point));
    }
  }

  return result;
}

}  // namespace inspector
}  // namespace tint
