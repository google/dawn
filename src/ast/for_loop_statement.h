// Copyright 2021 The Tint Authors.
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

#ifndef SRC_AST_FOR_LOOP_STATEMENT_H_
#define SRC_AST_FOR_LOOP_STATEMENT_H_

#include "src/ast/block_statement.h"

namespace tint {
namespace ast {

class Expression;

/// A for loop statement
class ForLoopStatement : public Castable<ForLoopStatement, Statement> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the for loop statement source
  /// @param initializer the optional loop initializer statement
  /// @param condition the optional loop condition expression
  /// @param continuing the optional continuing statement
  /// @param body the loop body
  ForLoopStatement(ProgramID program_id,
                   const Source& source,
                   Statement* initializer,
                   Expression* condition,
                   Statement* continuing,
                   BlockStatement* body);
  /// Move constructor
  ForLoopStatement(ForLoopStatement&&);
  ~ForLoopStatement() override;

  /// @returns the initializer statement
  const Statement* initializer() const { return initializer_; }
  /// @returns the initializer statement
  Statement* initializer() { return initializer_; }

  /// @returns the condition expression
  const Expression* condition() const { return condition_; }
  /// @returns the condition expression
  Expression* condition() { return condition_; }

  /// @returns the continuing statement
  const Statement* continuing() const { return continuing_; }
  /// @returns the continuing statement
  Statement* continuing() { return continuing_; }

  /// @returns the loop body block
  const BlockStatement* body() const { return body_; }
  /// @returns the loop body block
  BlockStatement* body() { return body_; }

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  ForLoopStatement* Clone(CloneContext* ctx) const override;

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;

 private:
  ForLoopStatement(const ForLoopStatement&) = delete;

  Statement* const initializer_;
  Expression* const condition_;
  Statement* const continuing_;
  BlockStatement* const body_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_FOR_LOOP_STATEMENT_H_
