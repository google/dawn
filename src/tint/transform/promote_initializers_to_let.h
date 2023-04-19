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

#ifndef SRC_TINT_TRANSFORM_PROMOTE_INITIALIZERS_TO_LET_H_
#define SRC_TINT_TRANSFORM_PROMOTE_INITIALIZERS_TO_LET_H_

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// A transform that hoists array and structure initializers, and identifiers resolving to a
/// 'const' array to a 'let' variable, declared just before the statement of usage.
/// This transform is used by backends that do not support expressions that operate on an immediate
/// array or structure. For example, the following is not immediately expressable for HLSL:
///   `array<i32, 2>(1, 2)[0]`
/// @see crbug.com/tint/406
class PromoteInitializersToLet final : public utils::Castable<PromoteInitializersToLet, Transform> {
  public:
    /// Constructor
    PromoteInitializersToLet();

    /// Destructor
    ~PromoteInitializersToLet() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_PROMOTE_INITIALIZERS_TO_LET_H_
