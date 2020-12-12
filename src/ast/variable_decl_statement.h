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

#ifndef SRC_AST_VARIABLE_DECL_STATEMENT_H_
#define SRC_AST_VARIABLE_DECL_STATEMENT_H_

#include <memory>
#include <utility>

#include "src/ast/expression.h"
#include "src/ast/statement.h"
#include "src/ast/variable.h"

namespace tint {
namespace ast {

/// A variable declaration statement
class VariableDeclStatement
    : public Castable<VariableDeclStatement, Statement> {
 public:
  /// Constructor
  /// @param source the variable statement source
  /// @param variable the variable
  VariableDeclStatement(const Source& source, Variable* variable);
  /// Move constructor
  VariableDeclStatement(VariableDeclStatement&&);
  ~VariableDeclStatement() override;

  /// @returns the variable
  Variable* variable() const { return variable_; }

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @note Semantic information such as resolved expression type and intrinsic
  /// information is not cloned.
  /// @param ctx the clone context
  /// @return the newly cloned node
  VariableDeclStatement* Clone(CloneContext* ctx) const override;

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  VariableDeclStatement(const VariableDeclStatement&) = delete;

  Variable* variable_ = nullptr;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_VARIABLE_DECL_STATEMENT_H_
