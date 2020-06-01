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

#ifndef SRC_AST_EXPRESSION_H_
#define SRC_AST_EXPRESSION_H_

#include <memory>
#include <vector>

#include "src/ast/node.h"
#include "src/ast/type/type.h"

namespace tint {
namespace ast {

class ArrayAccessorExpression;
class AsExpression;
class BinaryExpression;
class CallExpression;
class CastExpression;
class IdentifierExpression;
class ConstructorExpression;
class MemberAccessorExpression;
class UnaryOpExpression;

/// Base expression class
class Expression : public Node {
 public:
  ~Expression() override;

  /// Sets the resulting type of this expression
  /// @param type the result type to set
  void set_result_type(type::Type* type);
  /// @returns the resulting type from this expression
  type::Type* result_type() const { return result_type_; }

  /// @returns true if this is an array accessor expression
  virtual bool IsArrayAccessor() const;
  /// @returns true if this is an as expression
  virtual bool IsAs() const;
  /// @returns true if this is a call expression
  virtual bool IsCall() const;
  /// @returns true if this is a cast expression
  virtual bool IsCast() const;
  /// @returns true if this is an identifier expression
  virtual bool IsIdentifier() const;
  /// @returns true if this is an constructor expression
  virtual bool IsConstructor() const;
  /// @returns true if this is a member accessor expression
  virtual bool IsMemberAccessor() const;
  /// @returns true if this is a binary expression
  virtual bool IsBinary() const;
  /// @returns true if this is a unary op expression
  virtual bool IsUnaryOp() const;

  /// @returns the expression as an array accessor
  const ArrayAccessorExpression* AsArrayAccessor() const;
  /// @returns the expression as an as
  const AsExpression* AsAs() const;
  /// @returns the expression as a call
  const CallExpression* AsCall() const;
  /// @returns the expression as a cast
  const CastExpression* AsCast() const;
  /// @returns the expression as an identifier
  const IdentifierExpression* AsIdentifier() const;
  /// @returns the expression as an constructor
  const ConstructorExpression* AsConstructor() const;
  /// @returns the expression as a member accessor
  const MemberAccessorExpression* AsMemberAccessor() const;
  /// @returns the expression as a binary expression
  const BinaryExpression* AsBinary() const;
  /// @returns the expression as a unary op expression
  const UnaryOpExpression* AsUnaryOp() const;

  /// @returns the expression as an array accessor
  ArrayAccessorExpression* AsArrayAccessor();
  /// @returns the expression as an as
  AsExpression* AsAs();
  /// @returns the expression as a call
  CallExpression* AsCall();
  /// @returns the expression as a cast
  CastExpression* AsCast();
  /// @returns the expression as an identifier
  IdentifierExpression* AsIdentifier();
  /// @returns the expression as an constructor
  ConstructorExpression* AsConstructor();
  /// @returns the expression as a member accessor
  MemberAccessorExpression* AsMemberAccessor();
  /// @returns the expression as a binary expression
  BinaryExpression* AsBinary();
  /// @returns the expression as a unary op expression
  UnaryOpExpression* AsUnaryOp();

 protected:
  /// Constructor
  Expression();
  /// Constructor
  /// @param source the source of the expression
  explicit Expression(const Source& source);
  /// Move constructor
  Expression(Expression&&) = default;

 private:
  Expression(const Expression&) = delete;

  type::Type* result_type_ = nullptr;
};

/// A list of unique expressions
using ExpressionList = std::vector<std::unique_ptr<Expression>>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_EXPRESSION_H_
