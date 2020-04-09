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

#ifndef SRC_AST_UNARY_METHOD_EXPRESSION_H_
#define SRC_AST_UNARY_METHOD_EXPRESSION_H_

#include <utility>

#include "src/ast/expression.h"
#include "src/ast/literal.h"
#include "src/ast/unary_method.h"

namespace tint {
namespace ast {

/// A unary method expression
class UnaryMethodExpression : public Expression {
 public:
  /// Constructor
  UnaryMethodExpression();
  /// Constructor
  /// @param op the op
  /// @param params the params
  UnaryMethodExpression(UnaryMethod op, ExpressionList params);
  /// Constructor
  /// @param source the unary method source
  /// @param op the op
  /// @param params the params
  UnaryMethodExpression(const Source& source,
                        UnaryMethod op,
                        ExpressionList params);
  /// Move constructor
  UnaryMethodExpression(UnaryMethodExpression&&);
  ~UnaryMethodExpression() override;

  /// Sets the op
  /// @param op the op
  void set_op(UnaryMethod op) { op_ = op; }
  /// @returns the op
  UnaryMethod op() const { return op_; }

  /// Sets the params
  /// @param params the parameters
  void set_params(ExpressionList params) { params_ = std::move(params); }
  /// @returns the params
  const ExpressionList& params() const { return params_; }

  /// @returns true if this is an as expression
  bool IsUnaryMethod() const override;

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  UnaryMethodExpression(const UnaryMethodExpression&) = delete;

  UnaryMethod op_ = UnaryMethod::kAny;
  ExpressionList params_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_UNARY_METHOD_EXPRESSION_H_
