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

#ifndef SRC_TINT_TRANSFORM_LOCALIZE_STRUCT_ARRAY_ASSIGNMENT_H_
#define SRC_TINT_TRANSFORM_LOCALIZE_STRUCT_ARRAY_ASSIGNMENT_H_

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// This transforms replaces assignment to dynamically-indexed fixed-size arrays
/// in structs on shader-local variables with code that copies the arrays to a
/// temporary local variable, assigns to the local variable, and copies the
/// array back. This is to work around FXC's compilation failure for these cases
/// (see crbug.com/tint/1206).
///
/// @note Depends on the following transforms to have been run first:
/// * SimplifyPointers
class LocalizeStructArrayAssignment final
    : public utils::Castable<LocalizeStructArrayAssignment, Transform> {
  public:
    /// Constructor
    LocalizeStructArrayAssignment();

    /// Destructor
    ~LocalizeStructArrayAssignment() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_LOCALIZE_STRUCT_ARRAY_ASSIGNMENT_H_
