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

#include "src/ast/expression.h"

#include <assert.h>

#include "src/ast/array_accessor_expression.h"
#include "src/ast/as_expression.h"
#include "src/ast/binary_expression.h"
#include "src/ast/call_expression.h"
#include "src/ast/cast_expression.h"
#include "src/ast/constructor_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/unary_derivative_expression.h"
#include "src/ast/unary_method_expression.h"
#include "src/ast/unary_op_expression.h"

namespace tint {
namespace ast {

Expression::Expression() = default;

Expression::Expression(const Source& source) : Node(source) {}

Expression::~Expression() = default;

ArrayAccessorExpression* Expression::AsArrayAccessor() {
  assert(IsArrayAccessor());
  return static_cast<ArrayAccessorExpression*>(this);
}

AsExpression* Expression::AsAs() {
  assert(IsAs());
  return static_cast<AsExpression*>(this);
}

BinaryExpression* Expression::AsBinary() {
  assert(IsBinary());
  return static_cast<BinaryExpression*>(this);
}

CallExpression* Expression::AsCall() {
  assert(IsCall());
  return static_cast<CallExpression*>(this);
}

CastExpression* Expression::AsCast() {
  assert(IsCast());
  return static_cast<CastExpression*>(this);
}

ConstructorExpression* Expression::AsConstructor() {
  assert(IsConstructor());
  return static_cast<ConstructorExpression*>(this);
}

IdentifierExpression* Expression::AsIdentifier() {
  assert(IsIdentifier());
  return static_cast<IdentifierExpression*>(this);
}

MemberAccessorExpression* Expression::AsMemberAccessor() {
  assert(IsMemberAccessor());
  return static_cast<MemberAccessorExpression*>(this);
}

UnaryDerivativeExpression* Expression::AsUnaryDerivative() {
  assert(IsUnaryDerivative());
  return static_cast<UnaryDerivativeExpression*>(this);
}

UnaryMethodExpression* Expression::AsUnaryMethod() {
  assert(IsUnaryMethod());
  return static_cast<UnaryMethodExpression*>(this);
}

UnaryOpExpression* Expression::AsUnaryOp() {
  assert(IsUnaryOp());
  return static_cast<UnaryOpExpression*>(this);
}

}  // namespace ast
}  // namespace tint
