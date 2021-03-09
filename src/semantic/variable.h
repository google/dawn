// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#ifndef SRC_SEMANTIC_VARIABLE_H_
#define SRC_SEMANTIC_VARIABLE_H_

#include <vector>

#include "src/ast/storage_class.h"
#include "src/semantic/expression.h"

namespace tint {

// Forward declarations
namespace ast {
class Variable;
}  // namespace ast
namespace type {
class Type;
}  // namespace type

namespace semantic {

/// Variable holds the semantic information for variables.
class Variable : public Castable<Variable, Node> {
 public:
  /// Constructor
  /// @param declaration the AST declaration node
  /// @param storage_class the variable storage class
  /// @param users the expressions that use the variable
  explicit Variable(ast::Variable* declaration,
                    ast::StorageClass storage_class,
                    std::vector<const Expression*> users);

  /// Destructor
  ~Variable() override;

  /// @returns the AST declaration node
  ast::Variable* Declaration() const { return declaration_; }

  /// @returns the storage class for the variable
  ast::StorageClass StorageClass() const { return storage_class_; }

  /// @returns the expressions that use the variable
  const std::vector<const Expression*>& Users() const { return users_; }

 private:
  ast::Variable* const declaration_;
  ast::StorageClass const storage_class_;
  std::vector<const Expression*> const users_;
};

}  // namespace semantic
}  // namespace tint

#endif  // SRC_SEMANTIC_VARIABLE_H_
