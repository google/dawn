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

#ifndef SRC_AST_ELSE_STATEMENT_H_
#define SRC_AST_ELSE_STATEMENT_H_

#include <vector>

#include "src/ast/block_statement.h"
#include "src/ast/expression.h"

namespace tint {
namespace ast {

/// An else statement
class ElseStatement : public Castable<ElseStatement, Statement> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source information
  /// @param condition the else condition
  /// @param body the else body
  ElseStatement(ProgramID program_id,
                const Source& source,
                Expression* condition,
                BlockStatement* body);
  /// Move constructor
  ElseStatement(ElseStatement&&);
  ~ElseStatement() override;

  /// @returns the else condition or nullptr if none set
  Expression* condition() const { return condition_; }
  /// @returns true if the else has a condition
  bool HasCondition() const { return condition_ != nullptr; }

  /// @returns the else body
  const BlockStatement* body() const { return body_; }
  /// @returns the else body
  BlockStatement* body() { return body_; }

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  ElseStatement* Clone(CloneContext* ctx) const override;

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;

 private:
  ElseStatement(const ElseStatement&) = delete;

  Expression* const condition_;
  BlockStatement* const body_;
};

/// A list of else statements
using ElseStatementList = std::vector<ElseStatement*>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_ELSE_STATEMENT_H_
