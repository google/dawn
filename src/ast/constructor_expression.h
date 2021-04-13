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

#ifndef SRC_AST_CONSTRUCTOR_EXPRESSION_H_
#define SRC_AST_CONSTRUCTOR_EXPRESSION_H_

#include "src/ast/expression.h"

namespace tint {
namespace ast {

/// Base class for constructor style expressions
class ConstructorExpression
    : public Castable<ConstructorExpression, Expression> {
 public:
  ~ConstructorExpression() override;

 protected:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the constructor source
  ConstructorExpression(ProgramID program_id, const Source& source);
  /// Move constructor
  ConstructorExpression(ConstructorExpression&&);

 private:
  ConstructorExpression(const ConstructorExpression&) = delete;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_CONSTRUCTOR_EXPRESSION_H_
