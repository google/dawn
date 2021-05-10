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

#include "src/transform/msl.h"

#include <utility>

#include "src/transform/canonicalize_entry_point_io.h"
#include "src/transform/external_texture_transform.h"
#include "src/transform/manager.h"

namespace tint {
namespace transform {

Msl::Msl() = default;
Msl::~Msl() = default;

Output Msl::Run(const Program* in, const DataMap& data) {
  Manager manager;
  manager.Add<CanonicalizeEntryPointIO>();
  manager.Add<ExternalTextureTransform>();
  auto out = manager.Run(in, data);
  if (!out.program.IsValid()) {
    return out;
  }
  return Output{Program(std::move(out.program))};
}

}  // namespace transform
}  // namespace tint
