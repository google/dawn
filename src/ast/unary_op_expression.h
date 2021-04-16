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

#ifndef SRC_AST_UNARY_OP_EXPRESSION_H_
#define SRC_AST_UNARY_OP_EXPRESSION_H_

#include "src/ast/expression.h"
#include "src/ast/unary_op.h"

namespace tint {
namespace ast {

/// A unary op expression
class UnaryOpExpression : public Castable<UnaryOpExpression, Expression> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the unary op expression source
  /// @param op the op
  /// @param expr the expr
  UnaryOpExpression(ProgramID program_id,
                    const Source& source,
                    UnaryOp op,
                    Expression* expr);
  /// Move constructor
  UnaryOpExpression(UnaryOpExpression&&);
  ~UnaryOpExpression() override;

  /// @returns the op
  UnaryOp op() const { return op_; }
  /// @returns the expression
  Expression* expr() const { return expr_; }

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  UnaryOpExpression* Clone(CloneContext* ctx) const override;

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;

 private:
  UnaryOpExpression(const UnaryOpExpression&) = delete;

  UnaryOp const op_;
  Expression* const expr_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_UNARY_OP_EXPRESSION_H_
