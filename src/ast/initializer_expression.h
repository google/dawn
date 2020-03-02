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

#ifndef SRC_AST_INITIALIZER_EXPRESSION_H_
#define SRC_AST_INITIALIZER_EXPRESSION_H_

#include "src/ast/expression.h"

namespace tint {
namespace ast {

class ConstInitializerExpression;
class TypeInitializerExpression;

/// Base class for initializer style expressions
class InitializerExpression : public Expression {
 public:
  ~InitializerExpression() override;

  /// @returns true if this is an initializer expression
  bool IsInitializer() const override { return true; }

  /// @returns true if this is a constant initializer
  virtual bool IsConstInitializer() const { return false; }
  /// @returns true if this is a type initializer
  virtual bool IsTypeInitializer() const { return false; }

  /// @returns this as a const initializer expression
  ConstInitializerExpression* AsConstInitializer();
  /// @returns this as a type initializer expression
  TypeInitializerExpression* AsTypeInitializer();

 protected:
  /// Constructor
  InitializerExpression();
  /// Constructor
  /// @param source the initializer source
  explicit InitializerExpression(const Source& source);
  /// Move constructor
  InitializerExpression(InitializerExpression&&) = default;

 private:
  InitializerExpression(const InitializerExpression&) = delete;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_INITIALIZER_EXPRESSION_H_
