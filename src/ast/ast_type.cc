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

#include "src/ast/type.h"

#include "src/ast/alias.h"
#include "src/ast/bool.h"
#include "src/ast/f32.h"
#include "src/ast/i32.h"
#include "src/ast/matrix.h"
#include "src/ast/pointer.h"
#include "src/ast/sampler.h"
#include "src/ast/texture.h"
#include "src/ast/u32.h"
#include "src/ast/vector.h"
#include "src/symbol_table.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Type);

namespace tint {
namespace ast {

Type::Type(ProgramID pid, const Source& src) : Base(pid, src) {}

Type::Type(Type&&) = default;

Type::~Type() = default;

}  // namespace ast
}  // namespace tint
