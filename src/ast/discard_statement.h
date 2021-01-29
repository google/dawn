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

#ifndef SRC_AST_DISCARD_STATEMENT_H_
#define SRC_AST_DISCARD_STATEMENT_H_

#include "src/ast/statement.h"

namespace tint {
namespace ast {

/// A discard statement
class DiscardStatement : public Castable<DiscardStatement, Statement> {
 public:
  /// Constructor
  /// @param source the discard statement source
  explicit DiscardStatement(const Source& source);
  /// Move constructor
  DiscardStatement(DiscardStatement&&);
  ~DiscardStatement() override;

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @note Semantic information such as resolved expression type and intrinsic
  /// information is not cloned.
  /// @param ctx the clone context
  /// @return the newly cloned node
  DiscardStatement* Clone(CloneContext* ctx) const override;

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const semantic::Info& sem,
              std::ostream& out,
              size_t indent) const override;

 private:
  DiscardStatement(const DiscardStatement&) = delete;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_DISCARD_STATEMENT_H_
