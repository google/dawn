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

#ifndef SRC_TINT_TRANSFORM_EXPAND_COMPOUND_ASSIGNMENT_H_
#define SRC_TINT_TRANSFORM_EXPAND_COMPOUND_ASSIGNMENT_H_

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// Converts compound assignment statements to regular assignment statements,
/// hoisting the LHS expression if necessary.
///
/// Before:
/// ```
///   a += 1;
///   vector_array[foo()][bar()] *= 2.0;
/// ```
///
/// After:
/// ```
///   a = a + 1;
///   let _vec = &vector_array[foo()];
///   let _idx = bar();
///   (*_vec)[_idx] = (*_vec)[_idx] * 2.0;
/// ```
///
/// This transform also handles increment and decrement statements in the same
/// manner, by replacing `i++` with `i = i + 1`.
class ExpandCompoundAssignment final : public utils::Castable<ExpandCompoundAssignment, Transform> {
  public:
    /// Constructor
    ExpandCompoundAssignment();
    /// Destructor
    ~ExpandCompoundAssignment() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_EXPAND_COMPOUND_ASSIGNMENT_H_
