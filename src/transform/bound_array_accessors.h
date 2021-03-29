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

#include "src/ast/array_accessor_expression.h"
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
  /// @param program the source program to transform
  /// @param data optional extra transform-specific input data
  /// @returns the transformation result
  Output Run(const Program* program, const DataMap& data = {}) override;

 private:
  ast::ArrayAccessorExpression* Transform(ast::ArrayAccessorExpression* expr,
                                          CloneContext* ctx);
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_BOUND_ARRAY_ACCESSORS_H_
