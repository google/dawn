// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_TRANSFORM_PRESERVE_PADDING_H_
#define SRC_TINT_TRANSFORM_PRESERVE_PADDING_H_

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// Decompose assignments of whole structure and array types to preserve padding bytes.
///
/// WGSL states that memory operations on structures and arrays will not access padding bytes. To
/// avoid overwriting padding bytes when writing to buffers, this transform decomposes those
/// assignments into element-wise assignments via helper functions.
///
/// @note Assumes that the DirectVariableTransform will be run afterwards for backends that need it.
class PreservePadding final : public utils::Castable<PreservePadding, Transform> {
  public:
    /// Constructor
    PreservePadding();
    /// Destructor
    ~PreservePadding() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_PRESERVE_PADDING_H_
