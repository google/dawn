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

#ifndef SRC_TRANSFORM_BOUND_ARRAY_ACCESSORS_H_
#define SRC_TRANSFORM_BOUND_ARRAY_ACCESSORS_H_

#include <string>

#include "src/ast/array_accessor_expression.h"
#include "src/ast/expression.h"
#include "src/ast/statement.h"
#include "src/program.h"
#include "src/scope_stack.h"
#include "src/transform/transform.h"

namespace tint {
namespace transform {

/// This transform is responsible for clamping all array accesses to be within
/// the bounds of the array. Any access before the start of the array will clamp
/// to zero and any access past the end of the array will clamp to
/// (array length - 1).
class BoundArrayAccessors : public Transform {
 public:
  /// Constructor
  BoundArrayAccessors();
  /// Destructor
  ~BoundArrayAccessors() override;

  /// Runs the transform on `program`, returning the transformation result.
  /// @note Users of Tint should register the transform with transform manager
  /// and invoke its Run(), instead of directly calling the transform's Run().
  /// Calling Run() directly does not perform program state cleanup operations.
  /// @param program the source program to transform
  /// @returns the transformation result
  Output Run(const Program* program) override;

 private:
  ast::ArrayAccessorExpression* Transform(ast::ArrayAccessorExpression* expr,
                                          CloneContext* ctx,
                                          diag::List* diags);
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_BOUND_ARRAY_ACCESSORS_H_
