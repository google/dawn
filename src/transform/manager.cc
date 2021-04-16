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

#include "src/transform/manager.h"

namespace tint {
namespace transform {

Manager::Manager() = default;
Manager::~Manager() = default;

Output Manager::Run(const Program* program, const DataMap& data) {
  Output out;
  if (!transforms_.empty()) {
    for (auto& transform : transforms_) {
      auto res = transform->Run(program, data);
      out.program = std::move(res.program);
      out.data.Add(std::move(res.data));
      if (!out.program.IsValid()) {
        return out;
      }
      program = &out.program;
    }
  } else {
    out.program = program->Clone();
  }

  return out;
}

}  // namespace transform
}  // namespace tint
