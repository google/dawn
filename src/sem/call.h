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
  /// @param statement the statement that owns this expression
  Call(ast::Expression* declaration,
       const CallTarget* target,
       Statement* statement);

  /// Destructor
  ~Call() override;

  /// @return the target of the call
  const CallTarget* Target() const { return target_; }

 private:
  CallTarget const* const target_;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_CALL_H_
