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

#ifndef SRC_TINT_TRANSFORM_FOLD_TRIVIAL_SINGLE_USE_LETS_H_
#define SRC_TINT_TRANSFORM_FOLD_TRIVIAL_SINGLE_USE_LETS_H_

#include <string>
#include <unordered_map>

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// FoldTrivialSingleUseLets is an optimizer for folding away trivial `let`
/// statements into their single place of use. This transform is intended to
/// clean up the SSA `let`s produced by the SPIR-V reader.
/// `let`s can only be folded if:
/// * There is a single usage of the `let` value.
/// * The `let` is constructed with a ScalarConstructorExpression, or with an
///   IdentifierExpression.
/// * There are only other foldable `let`s between the `let` declaration and its
///   single usage.
/// These rules prevent any hoisting of the let that may affect execution
/// behaviour.
class FoldTrivialSingleUseLets final : public Castable<FoldTrivialSingleUseLets, Transform> {
  public:
    /// Constructor
    FoldTrivialSingleUseLets();

    /// Destructor
    ~FoldTrivialSingleUseLets() override;

  protected:
    /// Runs the transform using the CloneContext built for transforming a
    /// program. Run() is responsible for calling Clone() on the CloneContext.
    /// @param ctx the CloneContext primed with the input program and
    /// ProgramBuilder
    /// @param inputs optional extra transform-specific input data
    /// @param outputs optional extra transform-specific output data
    void Run(CloneContext& ctx, const DataMap& inputs, DataMap& outputs) const override;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_FOLD_TRIVIAL_SINGLE_USE_LETS_H_
