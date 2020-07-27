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

#include "src/ast/block_statement.h"
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
  LoopStatement(std::unique_ptr<BlockStatement> body,
                std::unique_ptr<BlockStatement> continuing);
  /// Constructor
  /// @param source the loop statement source
  /// @param body the body statements
  /// @param continuing the continuing statements
  LoopStatement(const Source& source,
                std::unique_ptr<BlockStatement> body,
                std::unique_ptr<BlockStatement> continuing);
  /// Move constructor
  LoopStatement(LoopStatement&&);
  ~LoopStatement() override;

  /// Sets the body statements
  /// @param body the body statements
  void set_body(std::unique_ptr<BlockStatement> body) {
    body_ = std::move(body);
  }
  /// @returns the body statements
  const BlockStatement* body() const { return body_.get(); }

  /// Sets the continuing statements
  /// @param continuing the continuing statements
  void set_continuing(std::unique_ptr<BlockStatement> continuing) {
    continuing_ = std::move(continuing);
  }
  /// @returns the continuing statements
  const BlockStatement* continuing() const { return continuing_.get(); }
  /// @returns true if there are continuing statements in the loop
  bool has_continuing() const {
    return continuing_ != nullptr && !continuing_->empty();
  }

  /// @returns true if this is a loop statement
  bool IsLoop() const override;

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  LoopStatement(const LoopStatement&) = delete;

  std::unique_ptr<BlockStatement> body_;
  std::unique_ptr<BlockStatement> continuing_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_LOOP_STATEMENT_H_
