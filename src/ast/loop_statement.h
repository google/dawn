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

#ifndef SRC_AST_LOOP_STATEMENT_H_
#define SRC_AST_LOOP_STATEMENT_H_

#include "src/ast/block_statement.h"

namespace tint {
namespace ast {

/// A loop statement
class LoopStatement : public Castable<LoopStatement, Statement> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the loop statement source
  /// @param body the body statements
  /// @param continuing the continuing statements
  LoopStatement(ProgramID program_id,
                const Source& source,
                BlockStatement* body,
                BlockStatement* continuing);
  /// Move constructor
  LoopStatement(LoopStatement&&);
  ~LoopStatement() override;

  /// @returns the body statements
  const BlockStatement* body() const { return body_; }
  /// @returns the body statements
  BlockStatement* body() { return body_; }

  /// @returns the continuing statements
  const BlockStatement* continuing() const { return continuing_; }
  /// @returns the continuing statements
  BlockStatement* continuing() { return continuing_; }
  /// @returns true if there are continuing statements in the loop
  bool has_continuing() const {
    return continuing_ != nullptr && !continuing_->empty();
  }

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  LoopStatement* Clone(CloneContext* ctx) const override;

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;

 private:
  LoopStatement(const LoopStatement&) = delete;

  BlockStatement* const body_;
  BlockStatement* const continuing_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_LOOP_STATEMENT_H_
