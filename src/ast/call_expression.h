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

#ifndef SRC_AST_CALL_EXPRESSION_H_
#define SRC_AST_CALL_EXPRESSION_H_

#include <memory>
#include <utility>

#include "src/ast/expression.h"
#include "src/ast/literal.h"

namespace tint {
namespace ast {

/// A call expression
class CallExpression : public Castable<CallExpression, Expression> {
 public:
  /// Constructor
  CallExpression();
  /// Constructor
  /// @param func the function
  /// @param params the parameters
  CallExpression(Expression* func, ExpressionList params);
  /// Constructor
  /// @param source the call expression source
  /// @param func the function
  /// @param params the parameters
  CallExpression(const Source& source, Expression* func, ExpressionList params);
  /// Move constructor
  CallExpression(CallExpression&&);
  ~CallExpression() override;

  /// Sets the func
  /// @param func the func
  void set_func(Expression* func) { func_ = func; }
  /// @returns the func
  Expression* func() const { return func_; }

  /// Sets the parameters
  /// @param params the parameters
  void set_params(ExpressionList params) { params_ = params; }
  /// @returns the parameters
  const ExpressionList& params() const { return params_; }

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @note Semantic information such as resolved expression type and intrinsic
  /// information is not cloned.
  /// @param ctx the clone context
  /// @return the newly cloned node
  CallExpression* Clone(CloneContext* ctx) const override;

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  CallExpression(const CallExpression&) = delete;

  Expression* func_ = nullptr;
  ExpressionList params_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_CALL_EXPRESSION_H_
