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

#ifndef SRC_TINT_TRANSFORM_DECOMPOSE_STRIDED_ARRAY_H_
#define SRC_TINT_TRANSFORM_DECOMPOSE_STRIDED_ARRAY_H_

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// DecomposeStridedArray transforms replaces arrays with a non-default
/// `@stride` attribute with an array of structure elements, where the
/// structure contains a single field with an equivalent `@size` attribute.
/// `@stride` attributes on arrays that match the default stride are also
/// removed.
///
/// @note Depends on the following transforms to have been run first:
/// * SimplifyPointers
class DecomposeStridedArray final : public utils::Castable<DecomposeStridedArray, Transform> {
  public:
    /// Constructor
    DecomposeStridedArray();

    /// Destructor
    ~DecomposeStridedArray() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_DECOMPOSE_STRIDED_ARRAY_H_
