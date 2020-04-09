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

#include "src/ast/variable_decoration.h"

#include <assert.h>

#include "src/ast/binding_decoration.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/location_decoration.h"
#include "src/ast/set_decoration.h"

namespace tint {
namespace ast {

VariableDecoration::VariableDecoration() = default;

VariableDecoration::~VariableDecoration() = default;

bool VariableDecoration::IsBinding() const {
  return false;
}

bool VariableDecoration::IsBuiltin() const {
  return false;
}

bool VariableDecoration::IsLocation() const {
  return false;
}

bool VariableDecoration::IsSet() const {
  return false;
}

BindingDecoration* VariableDecoration::AsBinding() {
  assert(IsBinding());
  return static_cast<BindingDecoration*>(this);
}

BuiltinDecoration* VariableDecoration::AsBuiltin() {
  assert(IsBuiltin());
  return static_cast<BuiltinDecoration*>(this);
}

LocationDecoration* VariableDecoration::AsLocation() {
  assert(IsLocation());
  return static_cast<LocationDecoration*>(this);
}

SetDecoration* VariableDecoration::AsSet() {
  assert(IsSet());
  return static_cast<SetDecoration*>(this);
}

}  // namespace ast
}  // namespace tint
