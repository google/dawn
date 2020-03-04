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

#include "src/validator_impl.h"

namespace tint {

ValidatorImpl::ValidatorImpl() = default;

ValidatorImpl::~ValidatorImpl() = default;

void ValidatorImpl::set_error(const Source& src, const std::string& msg) {
  error_ =
      std::to_string(src.line) + ":" + std::to_string(src.column) + ": " + msg;
}

bool ValidatorImpl::Validate(const ast::Module& module) {
  if (!CheckImports(module))
    return false;
  return true;
}

bool ValidatorImpl::CheckImports(const ast::Module& module) {
  for (const auto& import : module.imports()) {
    if (import->path() != "GLSL.std.450") {
      set_error(import->source(), "v-0001: unknown import: " + import->path());
      return false;
    }
  }
  return true;
}

}  // namespace tint
