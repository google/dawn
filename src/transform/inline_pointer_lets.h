// Copyright 2021 The Tint Authors.
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

#ifndef SRC_TRANSFORM_INLINE_POINTER_LETS_H_
#define SRC_TRANSFORM_INLINE_POINTER_LETS_H_

#include <string>
#include <unordered_map>

#include "src/transform/transform.h"

namespace tint {
namespace transform {

/// InlinePointerLets is a Transform that moves all usage of function-scope
/// `let` statements of a pointer type into their places of usage.
///
/// Parameters of a pointer type are not adjusted.
///
/// Note: InlinePointerLets does not operate on module-scope `let`s, as these
/// cannot be pointers: https://gpuweb.github.io/gpuweb/wgsl/#module-constants
/// `A module-scope let-declared constant must be of atomic-free plain type.`
class InlinePointerLets : public Transform {
 public:
  /// Constructor
  InlinePointerLets();

  /// Destructor
  ~InlinePointerLets() override;

  /// Runs the transform on `program`, returning the transformation result.
  /// @param program the source program to transform
  /// @param data optional extra transform-specific input data
  /// @returns the transformation result
  Output Run(const Program* program, const DataMap& data = {}) override;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_INLINE_POINTER_LETS_H_
