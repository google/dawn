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

#ifndef SRC_AST_CASE_STATEMENT_H_
#define SRC_AST_CASE_STATEMENT_H_

#include <vector>

#include "src/ast/block_statement.h"
#include "src/ast/int_literal.h"

namespace tint {
namespace ast {

/// A list of case literals
using CaseSelectorList = std::vector<IntLiteral*>;

/// A case statement
class CaseStatement : public Castable<CaseStatement, Statement> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source information
  /// @param selectors the case selectors
  /// @param body the case body
  CaseStatement(ProgramID program_id,
                const Source& source,
                CaseSelectorList selectors,
                BlockStatement* body);
  /// Move constructor
  CaseStatement(CaseStatement&&);
  ~CaseStatement() override;

  /// @returns the case selectors, empty if none set
  const CaseSelectorList& selectors() const { return selectors_; }
  /// @returns true if this is a default statement
  bool IsDefault() const { return selectors_.empty(); }

  /// @returns the case body
  const BlockStatement* body() const { return body_; }
  /// @returns the case body
  BlockStatement* body() { return body_; }

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  CaseStatement* Clone(CloneContext* ctx) const override;

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;

 private:
  CaseStatement(const CaseStatement&) = delete;

  CaseSelectorList const selectors_;
  BlockStatement* const body_;
};

/// A list of case statements
using CaseStatementList = std::vector<CaseStatement*>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_CASE_STATEMENT_H_
