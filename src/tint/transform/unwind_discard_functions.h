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

#ifndef SRC_TINT_TRANSFORM_UNWIND_DISCARD_FUNCTIONS_H_
#define SRC_TINT_TRANSFORM_UNWIND_DISCARD_FUNCTIONS_H_

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// This transform is responsible for implementing discard semantics as per the
/// WGSL specification: https://gpuweb.github.io/gpuweb/wgsl/#discard-statement
///
/// Not all backend languages implement discard this way (e.g. HLSL), so this
/// transform does the following:
///
/// * Replaces discard statements with setting a module-level boolean
/// "tint_discard" to true and returning immediately.
/// * Wherever calls are made to discarding functions (directly or
/// transitively), it inserts a check afterwards for if "tint_discard" is true,
/// to return immediately.
/// * Finally, entry point functions that call discarding functions
/// emit a call to "tint_discard_func()" that contains the sole discard
/// statement.
///
/// @note Depends on the following transforms to have been run first:
/// * PromoteSideEffectsToDecl
class UnwindDiscardFunctions : public Castable<UnwindDiscardFunctions, Transform> {
  public:
    /// Constructor
    UnwindDiscardFunctions();

    /// Destructor
    ~UnwindDiscardFunctions() override;

  protected:
    /// Runs the transform using the CloneContext built for transforming a
    /// program. Run() is responsible for calling Clone() on the CloneContext.
    /// @param ctx the CloneContext primed with the input program and
    /// ProgramBuilder
    /// @param inputs optional extra transform-specific input data
    /// @param outputs optional extra transform-specific output data
    void Run(CloneContext& ctx, const DataMap& inputs, DataMap& outputs) const override;

    /// @param program the program to inspect
    /// @param data optional extra transform-specific input data
    /// @returns true if this transform should be run for the given program
    bool ShouldRun(const Program* program, const DataMap& data = {}) const override;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_UNWIND_DISCARD_FUNCTIONS_H_
