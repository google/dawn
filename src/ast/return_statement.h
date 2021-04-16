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

#ifndef SRC_AST_RETURN_STATEMENT_H_
#define SRC_AST_RETURN_STATEMENT_H_

#include "src/ast/expression.h"
#include "src/ast/statement.h"

namespace tint {
namespace ast {

/// A return statement
class ReturnStatement : public Castable<ReturnStatement, Statement> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source information
  ReturnStatement(ProgramID program_id, const Source& source);

  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the return statement source
  /// @param value the return value
  ReturnStatement(ProgramID program_id,
                  const Source& source,
                  Expression* value);
  /// Move constructor
  ReturnStatement(ReturnStatement&&);
  ~ReturnStatement() override;

  /// @returns the value
  Expression* value() const { return value_; }
  /// @returns true if the return has a value
  bool has_value() const { return value_ != nullptr; }

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  ReturnStatement* Clone(CloneContext* ctx) const override;

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;

 private:
  ReturnStatement(const ReturnStatement&) = delete;

  Expression* const value_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_RETURN_STATEMENT_H_
