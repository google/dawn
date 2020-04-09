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

#ifndef SRC_AST_CONSTRUCTOR_EXPRESSION_H_
#define SRC_AST_CONSTRUCTOR_EXPRESSION_H_

#include "src/ast/expression.h"

namespace tint {
namespace ast {

class ScalarConstructorExpression;
class TypeConstructorExpression;

/// Base class for constructor style expressions
class ConstructorExpression : public Expression {
 public:
  ~ConstructorExpression() override;

  /// @returns true if this is an constructor expression
  bool IsConstructor() const override;

  /// @returns true if this is a scalar constructor
  virtual bool IsScalarConstructor() const;
  /// @returns true if this is a type constructor
  virtual bool IsTypeConstructor() const;

  /// @returns this as a scalar constructor expression
  ScalarConstructorExpression* AsScalarConstructor();
  /// @returns this as a type constructor expression
  TypeConstructorExpression* AsTypeConstructor();

 protected:
  /// Constructor
  ConstructorExpression();
  /// Constructor
  /// @param source the constructor source
  explicit ConstructorExpression(const Source& source);
  /// Move constructor
  ConstructorExpression(ConstructorExpression&&) = default;

 private:
  ConstructorExpression(const ConstructorExpression&) = delete;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_CONSTRUCTOR_EXPRESSION_H_
