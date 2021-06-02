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

#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {

namespace {

const char* dxc_path = nullptr;

}  // namespace

void EnableHLSLValidation(const char* dxc) {
  dxc_path = dxc;
}

val::Result Validate(Program* program, GeneratorImpl* generator) {
  if (!dxc_path) {
    return val::Result{};
  }

  std::ostringstream hlsl;
  if (!generator->Generate(hlsl)) {
    return {true, generator->error(), ""};
  }
  return val::Hlsl(dxc_path, hlsl.str(), program);
}

}  // namespace hlsl
}  // namespace writer
}  // namespace tint
