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

#ifndef SRC_AST_TYPE_CONSTRUCTOR_EXPRESSION_H_
#define SRC_AST_TYPE_CONSTRUCTOR_EXPRESSION_H_

#include <memory>
#include <utility>

#include "src/ast/constructor_expression.h"
#include "src/ast/type/type.h"

namespace tint {
namespace ast {

/// A type specific constructor
class TypeConstructorExpression : public ConstructorExpression {
 public:
  TypeConstructorExpression();
  /// Constructor
  /// @param type the type
  /// @param values the values
  explicit TypeConstructorExpression(type::Type* type, ExpressionList values);
  /// Constructor
  /// @param source the constructor source
  /// @param type the type
  /// @param values the constructor values
  TypeConstructorExpression(const Source& source,
                            type::Type* type,
                            ExpressionList values);
  /// Move constructor
  TypeConstructorExpression(TypeConstructorExpression&&);
  ~TypeConstructorExpression() override;

  /// @returns true if this is a type constructor
  bool IsTypeConstructor() const override;

  /// Set the type
  /// @param type the type
  void set_type(type::Type* type) { type_ = type; }
  /// @returns the type
  type::Type* type() const { return type_; }

  /// Set the values
  /// @param values the values
  void set_values(ExpressionList values) { values_ = std::move(values); }
  /// @returns the values
  const ExpressionList& values() const { return values_; }

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  TypeConstructorExpression(const TypeConstructorExpression&) = delete;

  type::Type* type_ = nullptr;
  ExpressionList values_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_CONSTRUCTOR_EXPRESSION_H_
