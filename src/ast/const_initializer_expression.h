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

#ifndef SRC_AST_CONST_INITIALIZER_EXPRESSION_H_
#define SRC_AST_CONST_INITIALIZER_EXPRESSION_H_

#include <memory>
#include <utility>

#include "src/ast/initializer_expression.h"
#include "src/ast/literal.h"

namespace tint {
namespace ast {

/// A constant initializer
class ConstInitializerExpression : public InitializerExpression {
 public:
  /// Constructor
  /// @param literal the const literal
  explicit ConstInitializerExpression(std::unique_ptr<Literal> literal);
  /// Constructor
  /// @param source the initializer source
  /// @param literal the const literal
  ConstInitializerExpression(const Source& source,
                             std::unique_ptr<Literal> literal);
  /// Move constructor
  ConstInitializerExpression(ConstInitializerExpression&&) = default;
  ~ConstInitializerExpression() override;

  /// @returns true if this is a constant initializer
  bool IsConstInitializer() const override { return true; }

  /// Set the literal value
  /// @param literal the literal
  void set_literal(std::unique_ptr<Literal> literal) {
    literal_ = std::move(literal);
  }
  /// @returns the literal value
  Literal* literal() const { return literal_.get(); }

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  ConstInitializerExpression(const ConstInitializerExpression&) = delete;

  std::unique_ptr<Literal> literal_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_CONST_INITIALIZER_EXPRESSION_H_
