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

#ifndef SRC_TINT_TRANSFORM_SIMPLIFY_POINTERS_H_
#define SRC_TINT_TRANSFORM_SIMPLIFY_POINTERS_H_

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// SimplifyPointers is a Transform that moves all usage of function-scope
/// `let` statements of a pointer type into their places of usage, while also
/// simplifying any chains of address-of or indirections operators.
///
/// Parameters of a pointer type are not adjusted.
///
/// Note: SimplifyPointers does not operate on module-scope `let`s, as these
/// cannot be pointers: https://gpuweb.github.io/gpuweb/wgsl/#module-constants
/// `A module-scope let-declared constant must be of constructible type.`
///
/// @note Depends on the following transforms to have been run first:
/// * Unshadow
class SimplifyPointers : public Castable<SimplifyPointers, Transform> {
  public:
    /// Constructor
    SimplifyPointers();

    /// Destructor
    ~SimplifyPointers() override;

  protected:
    struct State;

    /// Runs the transform using the CloneContext built for transforming a
    /// program. Run() is responsible for calling Clone() on the CloneContext.
    /// @param ctx the CloneContext primed with the input program and
    /// ProgramBuilder
    /// @param inputs optional extra transform-specific input data
    /// @param outputs optional extra transform-specific output data
    void Run(CloneContext& ctx, const DataMap& inputs, DataMap& outputs) const override;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_SIMPLIFY_POINTERS_H_
