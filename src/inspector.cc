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

#include "src/ast/function.h"

namespace tint {
namespace inspector {

Inspector::Inspector(const ast::Module& module) : module_(module) {}

Inspector::~Inspector() = default;

std::vector<EntryPoint> Inspector::GetEntryPoints() {
  std::vector<EntryPoint> result;
  for (const auto& func : module_.functions()) {
    if (func->IsEntryPoint()) {
      uint32_t x, y, z;
      std::tie(x, y, z) = func->workgroup_size();
      result.push_back({func->name(), func->pipeline_stage(), x, y, z});
    }
  }

  return result;
}

}  // namespace inspector
}  // namespace tint
