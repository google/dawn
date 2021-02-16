// Copyright 2021 The Tint Authors.
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

#ifndef SRC_SEMANTIC_STATEMENT_H_
#define SRC_SEMANTIC_STATEMENT_H_

#include "src/semantic/node.h"

namespace tint {

// Forward declarations
namespace ast {
class Statement;
}  // namespace ast

namespace semantic {

/// Statement holds the semantic information for a statement.
class Statement : public Castable<Statement, Node> {
 public:
  /// Constructor
  /// @param declaration the AST node for this statement
  explicit Statement(ast::Statement* declaration);

  /// @return the AST node for this statement
  ast::Statement* Declaration() const { return declaration_; }

 private:
  ast::Statement* const declaration_;
};

}  // namespace semantic
}  // namespace tint

#endif  // SRC_SEMANTIC_STATEMENT_H_
