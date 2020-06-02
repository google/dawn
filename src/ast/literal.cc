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

#include "src/ast/literal.h"

#include <assert.h>

#include "src/ast/bool_literal.h"
#include "src/ast/float_literal.h"
#include "src/ast/int_literal.h"
#include "src/ast/null_literal.h"
#include "src/ast/sint_literal.h"
#include "src/ast/uint_literal.h"

namespace tint {
namespace ast {

Literal::Literal(ast::type::Type* type) : type_(type) {}

Literal::~Literal() = default;

bool Literal::IsBool() const {
  return false;
}

bool Literal::IsFloat() const {
  return false;
}

bool Literal::IsInt() const {
  return false;
}

bool Literal::IsSint() const {
  return false;
}

bool Literal::IsNull() const {
  return false;
}

bool Literal::IsUint() const {
  return false;
}

BoolLiteral* Literal::AsBool() {
  assert(IsBool());
  return static_cast<BoolLiteral*>(this);
}

FloatLiteral* Literal::AsFloat() {
  assert(IsFloat());
  return static_cast<FloatLiteral*>(this);
}

IntLiteral* Literal::AsInt() {
  assert(IsInt());
  return static_cast<IntLiteral*>(this);
}

SintLiteral* Literal::AsSint() {
  assert(IsSint());
  return static_cast<SintLiteral*>(this);
}

NullLiteral* Literal::AsNull() {
  assert(IsNull());
  return static_cast<NullLiteral*>(this);
}

UintLiteral* Literal::AsUint() {
  assert(IsUint());
  return static_cast<UintLiteral*>(this);
}

}  // namespace ast
}  // namespace tint
