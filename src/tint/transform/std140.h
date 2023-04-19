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

#ifndef SRC_TINT_TRANSFORM_STD140_H_
#define SRC_TINT_TRANSFORM_STD140_H_

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// Std140 is a transform that forks types used in the uniform address space that contain
/// `matNx2<f32>` matrices into `N`x`vec2<f32>` column vectors, and `matNxM<f16>` matrices into
/// `N`x`vecM<f16>` column vectors. Types that transitively use these forked types are also forked.
/// `var<uniform>` variables will use these forked types, and expressions loading from these
/// variables will do appropriate conversions to the regular WGSL types. As `matNx2<f32>` and
/// `matNxM<f16>` matrices are the only type that violate std140-layout, this transformation is
/// sufficient to have any WGSL structure be std140-layout conformant.
///
/// @note This transform requires the PromoteSideEffectsToDecl transform to have been run first.
class Std140 final : public utils::Castable<Std140, Transform> {
  public:
    /// Constructor
    Std140();
    /// Destructor
    ~Std140() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_STD140_H_
