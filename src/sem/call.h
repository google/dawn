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

#ifndef SRC_SEM_CALL_H_
#define SRC_SEM_CALL_H_

#include <vector>

#include "src/sem/expression.h"
#include "src/sem/intrinsic.h"

namespace tint {
namespace sem {

/// Call is the base class for semantic nodes that hold semantic information for
/// ast::CallExpression nodes.
class Call : public Castable<Call, Expression> {
 public:
  /// Constructor
  /// @param declaration the AST node
  /// @param target the call target
  /// @param arguments the call arguments
  /// @param statement the statement that owns this expression
  /// @param constant the constant value of this expression
  Call(const ast::CallExpression* declaration,
       const CallTarget* target,
       std::vector<const sem::Expression*> arguments,
       const Statement* statement,
       Constant constant);

  /// Destructor
  ~Call() override;

  /// @return the target of the call
  const CallTarget* Target() const { return target_; }

  /// @return the call arguments
  const std::vector<const sem::Expression*>& Arguments() const {
    return arguments_;
  }

  /// @returns the AST node
  const ast::CallExpression* Declaration() const {
    return static_cast<const ast::CallExpression*>(declaration_);
  }

 private:
  CallTarget const* const target_;
  std::vector<const sem::Expression*> arguments_;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_CALL_H_
