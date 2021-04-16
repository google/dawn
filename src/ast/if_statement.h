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

#ifndef SRC_AST_IF_STATEMENT_H_
#define SRC_AST_IF_STATEMENT_H_

#include <utility>

#include "src/ast/else_statement.h"

namespace tint {
namespace ast {

/// An if statement
class IfStatement : public Castable<IfStatement, Statement> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source information
  /// @param condition the if condition
  /// @param body the if body
  /// @param else_stmts the else statements
  IfStatement(ProgramID program_id,
              const Source& source,
              Expression* condition,
              BlockStatement* body,
              ElseStatementList else_stmts);
  /// Move constructor
  IfStatement(IfStatement&&);
  ~IfStatement() override;

  /// @returns the if condition or nullptr if none set
  Expression* condition() const { return condition_; }
  /// @returns the if body
  const BlockStatement* body() const { return body_; }
  /// @returns the if body
  BlockStatement* body() { return body_; }

  /// @returns the else statements
  const ElseStatementList& else_statements() const { return else_statements_; }

  /// @returns true if there are else statements
  bool has_else_statements() const { return !else_statements_.empty(); }

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  IfStatement* Clone(CloneContext* ctx) const override;

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;

 private:
  IfStatement(const IfStatement&) = delete;

  Expression* const condition_;
  BlockStatement* const body_;
  ElseStatementList const else_statements_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_IF_STATEMENT_H_
