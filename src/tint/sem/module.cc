// Copyright 2022 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#include "src/tint/sem/module.h"

#include <utility>
#include <vector>

TINT_INSTANTIATE_TYPEINFO(tint::sem::Module);

namespace tint::sem {

Module::Module(utils::VectorRef<const ast::Node*> dep_ordered_decls, builtin::Extensions extensions)
    : dep_ordered_decls_(std::move(dep_ordered_decls)), extensions_(std::move(extensions)) {}

Module::~Module() = default;

}  // namespace tint::sem
