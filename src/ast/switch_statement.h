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

#ifndef SRC_AST_SWITCH_STATEMENT_H_
#define SRC_AST_SWITCH_STATEMENT_H_

#include "src/ast/case_statement.h"
#include "src/ast/expression.h"

namespace tint {
namespace ast {

/// A switch statement
class SwitchStatement : public Castable<SwitchStatement, Statement> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source information
  /// @param condition the switch condition
  /// @param body the switch body
  SwitchStatement(ProgramID program_id,
                  const Source& source,
                  Expression* condition,
                  CaseStatementList body);
  /// Move constructor
  SwitchStatement(SwitchStatement&&);
  ~SwitchStatement() override;

  /// @returns the switch condition or nullptr if none set
  Expression* condition() const { return condition_; }
  /// @returns true if this is a default statement
  bool IsDefault() const { return condition_ == nullptr; }

  /// @returns the Switch body
  const CaseStatementList& body() const { return body_; }

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  SwitchStatement* Clone(CloneContext* ctx) const override;

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;

 private:
  SwitchStatement(const SwitchStatement&) = delete;

  Expression* const condition_;
  CaseStatementList const body_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_SWITCH_STATEMENT_H_
