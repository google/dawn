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

#include "src/ast/initializer_expression.h"

#include <assert.h>

#include "src/ast/const_initializer_expression.h"
#include "src/ast/type_initializer_expression.h"

namespace tint {
namespace ast {

InitializerExpression::InitializerExpression() = default;

InitializerExpression::~InitializerExpression() = default;

InitializerExpression::InitializerExpression(const Source& source)
    : Expression(source) {}

ConstInitializerExpression* InitializerExpression::AsConstInitializer() {
  assert(IsConstInitializer());
  return static_cast<ConstInitializerExpression*>(this);
}

TypeInitializerExpression* InitializerExpression::AsTypeInitializer() {
  assert(IsTypeInitializer());
  return static_cast<TypeInitializerExpression*>(this);
}

}  // namespace ast
}  // namespace tint
