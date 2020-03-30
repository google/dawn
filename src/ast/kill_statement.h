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

#ifndef SRC_AST_KILL_STATEMENT_H_
#define SRC_AST_KILL_STATEMENT_H_

#include "src/ast/statement.h"

namespace tint {
namespace ast {

/// A kill statement
class KillStatement : public Statement {
 public:
  /// Constructor
  KillStatement();
  /// Constructor
  /// @param source the kill statement source
  explicit KillStatement(const Source& source);
  /// Move constructor
  KillStatement(KillStatement&&) = default;
  ~KillStatement() override;

  /// @returns true if this is a kill statement
  bool IsKill() const override { return true; }

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  KillStatement(const KillStatement&) = delete;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_KILL_STATEMENT_H_
