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

#ifndef SRC_AST_UNARY_DERIVATIVE_EXPRESSION_H_
#define SRC_AST_UNARY_DERIVATIVE_EXPRESSION_H_

#include <memory>
#include <utility>

#include "src/ast/derivative_modifier.h"
#include "src/ast/expression.h"
#include "src/ast/literal.h"
#include "src/ast/unary_derivative.h"

namespace tint {
namespace ast {

/// A unary derivative expression
class UnaryDerivativeExpression : public Expression {
 public:
  /// Constructor
  UnaryDerivativeExpression();
  /// Constructor
  /// @param op the op
  /// @param mod the derivative modifier
  /// @param param the param
  UnaryDerivativeExpression(UnaryDerivative op,
                            DerivativeModifier mod,
                            std::unique_ptr<Expression> param);
  /// Constructor
  /// @param source the initializer source
  /// @param op the op
  /// @param mod the derivative modifier
  /// @param param the param
  UnaryDerivativeExpression(const Source& source,
                            UnaryDerivative op,
                            DerivativeModifier mod,
                            std::unique_ptr<Expression> param);
  /// Move constructor
  UnaryDerivativeExpression(UnaryDerivativeExpression&&) = default;
  ~UnaryDerivativeExpression() override;

  /// Sets the op
  /// @param op the op
  void set_op(UnaryDerivative op) { op_ = op; }
  /// @returns the op
  UnaryDerivative op() const { return op_; }

  /// Sets the derivative modifier
  /// @param mod the modifier
  void set_modifier(DerivativeModifier mod) { modifier_ = mod; }
  /// @returns the derivative modifier
  DerivativeModifier modifier() const { return modifier_; }

  /// Sets the param
  /// @param param the param
  void set_param(std::unique_ptr<Expression> param) {
    param_ = std::move(param);
  }
  /// @returns the param
  Expression* param() const { return param_.get(); }

  /// @returns true if this is an as expression
  bool IsUnaryDerivative() const override { return true; }

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  UnaryDerivativeExpression(const UnaryDerivativeExpression&) = delete;

  UnaryDerivative op_ = UnaryDerivative::kDpdx;
  DerivativeModifier modifier_ = DerivativeModifier::kNone;
  std::unique_ptr<Expression> param_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_UNARY_DERIVATIVE_EXPRESSION_H_
