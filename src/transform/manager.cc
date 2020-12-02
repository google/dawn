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

#include "src/type_determiner.h"

namespace tint {
namespace transform {

Manager::Manager() = default;

Manager::Manager(Context*, ast::Module* module) : module_(module) {}

Manager::~Manager() = default;

bool Manager::Run() {
  return Run(module_);
}

bool Manager::Run(ast::Module* module) {
  error_ = "";

  for (auto& transform : transforms_) {
    if (!transform->Run()) {
      error_ = transform->error();
      return false;
    }
  }

  if (module != nullptr) {
    // The transformed have potentially inserted nodes into the AST, so the type
    // determinater needs to be run.
    TypeDeterminer td(module);
    if (!td.Determine()) {
      error_ = td.error();
      return false;
    }
  }

  return true;
}

}  // namespace transform
}  // namespace tint
