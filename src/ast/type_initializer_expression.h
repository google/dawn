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

#ifndef SRC_AST_TYPE_INITIALIZER_EXPRESSION_H_
#define SRC_AST_TYPE_INITIALIZER_EXPRESSION_H_

#include <memory>
#include <utility>
#include <vector>

#include "src/ast/initializer_expression.h"
#include "src/ast/type/type.h"

namespace tint {
namespace ast {

/// A type specific initializer
class TypeInitializerExpression : public InitializerExpression {
 public:
  TypeInitializerExpression();
  /// Constructor
  /// @param type the type
  /// @param values the values
  explicit TypeInitializerExpression(
      type::Type* type,
      std::vector<std::unique_ptr<Expression>> values);
  /// Constructor
  /// @param source the initializer source
  /// @param type the type
  /// @param values the initializer values
  TypeInitializerExpression(const Source& source,
                            type::Type* type,
                            std::vector<std::unique_ptr<Expression>> values);
  /// Move constructor
  TypeInitializerExpression(TypeInitializerExpression&&) = default;
  ~TypeInitializerExpression() override;

  /// @returns true if this is a type initializer
  bool IsTypeInitializer() const override { return true; }

  /// Set the type
  /// @param type the type
  void set_type(type::Type* type) { type_ = type; }
  /// @returns the type
  type::Type* type() const { return type_; }

  /// Set the values
  /// @param values the values
  void set_values(std::vector<std::unique_ptr<Expression>> values) {
    values_ = std::move(values);
  }
  /// @returns the values
  const std::vector<std::unique_ptr<Expression>>& values() const {
    return values_;
  }

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  TypeInitializerExpression(const TypeInitializerExpression&) = delete;

  type::Type* type_ = nullptr;
  std::vector<std::unique_ptr<Expression>> values_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_INITIALIZER_EXPRESSION_H_
