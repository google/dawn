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

#include "src/ast/constructor_expression.h"

#include <assert.h>

#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type_constructor_expression.h"

namespace tint {
namespace ast {

ConstructorExpression::ConstructorExpression() = default;

ConstructorExpression::~ConstructorExpression() = default;

ConstructorExpression::ConstructorExpression(const Source& source)
    : Expression(source) {}

bool ConstructorExpression::IsConstructor() const {
  return true;
}

bool ConstructorExpression::IsScalarConstructor() const {
  return false;
}

bool ConstructorExpression::IsTypeConstructor() const {
  return false;
}

ScalarConstructorExpression* ConstructorExpression::AsScalarConstructor() {
  assert(IsScalarConstructor());
  return static_cast<ScalarConstructorExpression*>(this);
}

TypeConstructorExpression* ConstructorExpression::AsTypeConstructor() {
  assert(IsTypeConstructor());
  return static_cast<TypeConstructorExpression*>(this);
}

}  // namespace ast
}  // namespace tint
