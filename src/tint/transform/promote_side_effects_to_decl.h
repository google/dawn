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

#ifndef SRC_TINT_TRANSFORM_PROMOTE_SIDE_EFFECTS_TO_DECL_H_
#define SRC_TINT_TRANSFORM_PROMOTE_SIDE_EFFECTS_TO_DECL_H_

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// A transform that hoists expressions with side-effects to variable
/// declarations before the statement of usage with the goal of ensuring
/// left-to-right order of evaluation, while respecting short-circuit
/// evaluation.
class PromoteSideEffectsToDecl final : public utils::Castable<PromoteSideEffectsToDecl, Transform> {
  public:
    /// Constructor
    PromoteSideEffectsToDecl();

    /// Destructor
    ~PromoteSideEffectsToDecl() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_PROMOTE_SIDE_EFFECTS_TO_DECL_H_
