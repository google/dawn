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

#include "src/ast/function_decoration.h"

#include <assert.h>

#include "src/ast/stage_decoration.h"
#include "src/ast/workgroup_decoration.h"

namespace tint {
namespace ast {

constexpr const DecorationKind FunctionDecoration::Kind;

FunctionDecoration::FunctionDecoration(const Source& source)
    : Decoration(source) {}

FunctionDecoration::~FunctionDecoration() = default;

DecorationKind FunctionDecoration::GetKind() const {
  return Kind;
}

bool FunctionDecoration::IsKind(DecorationKind kind) const {
  return kind == Kind;
}

bool FunctionDecoration::IsStage() const {
  return false;
}

bool FunctionDecoration::IsWorkgroup() const {
  return false;
}

const StageDecoration* FunctionDecoration::AsStage() const {
  assert(IsStage());
  return static_cast<const StageDecoration*>(this);
}

const WorkgroupDecoration* FunctionDecoration::AsWorkgroup() const {
  assert(IsWorkgroup());
  return static_cast<const WorkgroupDecoration*>(this);
}

}  // namespace ast
}  // namespace tint
