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

#ifndef SRC_AST_STATEMENT_H_
#define SRC_AST_STATEMENT_H_

#include <vector>

#include "src/ast/node.h"

namespace tint {
namespace ast {

/// Base statement class
class Statement : public Castable<Statement, Node> {
 public:
  ~Statement() override;

  /// @returns the human readable name for the statement type.
  const char* Name() const;

 protected:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source of the expression
  Statement(ProgramID program_id, const Source& source);
  /// Move constructor
  Statement(Statement&&);

 private:
  Statement(const Statement&) = delete;
};

/// A list of statements
using StatementList = std::vector<Statement*>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_STATEMENT_H_
