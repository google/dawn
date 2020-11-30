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
#include <string>
#include <vector>

#include "src/ast/node.h"
#include "src/ast/type/type.h"

namespace tint {
namespace ast {

/// Base expression class
class Expression : public Castable<Expression, Node> {
 public:
  ~Expression() override;

  /// Sets the resulting type of this expression
  /// @param type the result type to set
  void set_result_type(type::Type* type);
  /// @returns the resulting type from this expression
  type::Type* result_type() const { return result_type_; }

  /// @returns a string representation of the result type or 'not set' if no
  /// result type present
  std::string result_type_str() const {
    return result_type_ ? result_type_->type_name() : "not set";
  }

 protected:
  /// Constructor
  Expression();
  /// Constructor
  /// @param source the source of the expression
  explicit Expression(const Source& source);
  /// Move constructor
  Expression(Expression&&);

 private:
  Expression(const Expression&) = delete;

  type::Type* result_type_ = nullptr;
};

/// A list of expressions
using ExpressionList = std::vector<Expression*>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_EXPRESSION_H_
