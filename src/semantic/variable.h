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

#include <utility>
#include <vector>

#include "src/ast/storage_class.h"
#include "src/semantic/node.h"
#include "src/type/sampler_type.h"

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
  explicit Variable(ast::Variable* declaration,
                    ast::StorageClass storage_class);

  /// Destructor
  ~Variable() override;

  /// @returns the AST declaration node
  ast::Variable* Declaration() const { return declaration_; }

  /// @returns the storage class for the variable
  ast::StorageClass StorageClass() const { return storage_class_; }

 private:
  ast::Variable* const declaration_;
  ast::StorageClass const storage_class_;
};

}  // namespace semantic
}  // namespace tint

#endif  // SRC_SEMANTIC_VARIABLE_H_
