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

#ifndef SRC_AST_AS_EXPRESSION_H_
#define SRC_AST_AS_EXPRESSION_H_

#include <memory>
#include <utility>

#include "src/ast/expression.h"
#include "src/ast/literal.h"
#include "src/ast/type/type.h"

namespace tint {
namespace ast {

/// An as expression
class AsExpression : public Expression {
 public:
  /// Constructor
  AsExpression();
  /// Constructor
  /// @param type the type
  /// @param expr the expr
  AsExpression(type::Type* type, std::unique_ptr<Expression> expr);
  /// Constructor
  /// @param source the as expression source
  /// @param type the type
  /// @param expr the expr
  AsExpression(const Source& source,
               type::Type* type,
               std::unique_ptr<Expression> expr);
  /// Move constructor
  AsExpression(AsExpression&&);
  ~AsExpression() override;

  /// Sets the type
  /// @param type the type
  void set_type(type::Type* type) { type_ = std::move(type); }
  /// @returns the left side expression
  type::Type* type() const { return type_; }

  /// Sets the expr
  /// @param expr the expression
  void set_expr(std::unique_ptr<Expression> expr) { expr_ = std::move(expr); }
  /// @returns the expression
  Expression* expr() const { return expr_.get(); }

  /// @returns true if this is an as expression
  bool IsAs() const override;

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  AsExpression(const AsExpression&) = delete;

  type::Type* type_ = nullptr;
  std::unique_ptr<Expression> expr_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_AS_EXPRESSION_H_
