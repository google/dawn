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

#ifndef SRC_AST_DECORATED_VARIABLE_H_
#define SRC_AST_DECORATED_VARIABLE_H_

#include <memory>
#include <utility>

#include "src/ast/variable.h"
#include "src/ast/variable_decoration.h"

namespace tint {
namespace ast {

/// A Decorated Variable statement.
class DecoratedVariable : public Variable {
 public:
  /// Create a new empty decorated variable statement
  DecoratedVariable();
  /// Create a decorated variable from an existing variable
  /// @param var the variable to initialize from
  explicit DecoratedVariable(std::unique_ptr<Variable> var);
  /// Move constructor
  DecoratedVariable(DecoratedVariable&&);

  ~DecoratedVariable() override;

  /// Sets a decoration to the variable
  /// @param decos the decorations to set
  void set_decorations(VariableDecorationList decos) {
    decorations_ = std::move(decos);
  }
  /// @returns the decorations attached to this variable
  const VariableDecorationList& decorations() const { return decorations_; }

  /// @returns true if the decorations include a LocationDecoration
  bool HasLocationDecoration() const;
  /// @returns true if the deocrations include a BuiltinDecoration
  bool HasBuiltinDecoration() const;

  /// @returns true if this is a decorated variable
  bool IsDecorated() const override;

  /// @returns true if the name and path are both present
  bool IsValid() const override;

  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  DecoratedVariable(const DecoratedVariable&) = delete;

  VariableDecorationList decorations_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_DECORATED_VARIABLE_H_
