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

#ifndef SRC_TRANSFORM_BOUND_ARRAY_ACCESSORS_TRANSFORM_H_
#define SRC_TRANSFORM_BOUND_ARRAY_ACCESSORS_TRANSFORM_H_

#include <string>

#include "src/ast/array_accessor_expression.h"
#include "src/ast/expression.h"
#include "src/ast/module.h"
#include "src/ast/statement.h"
#include "src/context.h"
#include "src/scope_stack.h"
#include "src/transform/transformer.h"

namespace tint {
namespace transform {

/// This transformer is responsible for clamping all array accesses to be within
/// the bounds of the array. Any access before the start of the array will clamp
/// to zero and any access past the end of the array will clamp to
/// (array length - 1).
class BoundArrayAccessorsTransform : public Transformer {
 public:
  /// Constructor
  /// @param ctx the Tint context object
  /// @param mod the module transform
  explicit BoundArrayAccessorsTransform(Context* ctx, ast::Module* mod);
  ~BoundArrayAccessorsTransform() override;

  /// @returns true if the transformation was successful
  bool Run() override;

 private:
  bool ProcessStatement(ast::Statement* stmt);
  bool ProcessExpression(ast::Expression* expr);
  bool ProcessArrayAccessor(ast::ArrayAccessorExpression* expr);
  bool ProcessAccessExpression(ast::ArrayAccessorExpression* expr,
                               uint32_t size);

  ScopeStack<ast::Variable*> scope_stack_;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_BOUND_ARRAY_ACCESSORS_TRANSFORM_H_
