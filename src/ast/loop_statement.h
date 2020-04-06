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

#include <utility>

#include "src/ast/statement.h"

namespace tint {
namespace ast {

/// A loop statement
class LoopStatement : public Statement {
 public:
  /// Constructor
  LoopStatement();
  /// Constructor
  /// @param body the body statements
  /// @param continuing the continuing statements
  LoopStatement(StatementList body, StatementList continuing);
  /// Constructor
  /// @param source the loop statement source
  /// @param body the body statements
  /// @param continuing the continuing statements
  LoopStatement(const Source& source,
                StatementList body,
                StatementList continuing);
  /// Move constructor
  LoopStatement(LoopStatement&&) = default;
  ~LoopStatement() override;

  /// Sets the body statements
  /// @param body the body statements
  void set_body(StatementList body) { body_ = std::move(body); }
  /// @returns the body statements
  const StatementList& body() const { return body_; }

  /// Sets the continuing statements
  /// @param continuing the continuing statements
  void set_continuing(StatementList continuing) {
    continuing_ = std::move(continuing);
  }
  /// @returns the continuing statements
  const StatementList& continuing() const { return continuing_; }
  /// @returns true if there are continuing statements in the loop
  bool has_continuing() const { return !continuing_.empty(); }

  /// @returns true if this is a loop statement
  bool IsLoop() const override { return true; }

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  LoopStatement(const LoopStatement&) = delete;

  StatementList body_;
  StatementList continuing_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_LOOP_STATEMENT_H_
