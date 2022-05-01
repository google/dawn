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

#ifndef SRC_TINT_TRANSFORM_NUM_WORKGROUPS_FROM_UNIFORM_H_
#define SRC_TINT_TRANSFORM_NUM_WORKGROUPS_FROM_UNIFORM_H_

#include "src/tint/sem/binding_point.h"
#include "src/tint/transform/transform.h"

// Forward declarations
namespace tint {
class CloneContext;
}  // namespace tint

namespace tint::transform {

/// NumWorkgroupsFromUniform is a transform that implements the `num_workgroups`
/// builtin by loading it from a uniform buffer.
///
/// The generated uniform buffer will have the form:
/// ```
/// struct num_workgroups_struct {
///  num_workgroups : vec3<u32>;
/// };
///
/// @group(0) @binding(0)
/// var<uniform> num_workgroups_ubo : num_workgroups_struct;
/// ```
/// The binding group and number used for this uniform buffer is provided via
/// the `Config` transform input.
///
/// @note Depends on the following transforms to have been run first:
/// * CanonicalizeEntryPointIO
class NumWorkgroupsFromUniform : public Castable<NumWorkgroupsFromUniform, Transform> {
  public:
    /// Constructor
    NumWorkgroupsFromUniform();
    /// Destructor
    ~NumWorkgroupsFromUniform() override;

    /// Configuration options for the NumWorkgroupsFromUniform transform.
    struct Config : public Castable<Data, transform::Data> {
        /// Constructor
        /// @param ubo_bp the binding point to use for the generated uniform buffer.
        explicit Config(sem::BindingPoint ubo_bp);

        /// Copy constructor
        Config(const Config&);

        /// Destructor
        ~Config() override;

        /// The binding point to use for the generated uniform buffer.
        sem::BindingPoint ubo_binding;
    };

    /// @param program the program to inspect
    /// @param data optional extra transform-specific input data
    /// @returns true if this transform should be run for the given program
    bool ShouldRun(const Program* program, const DataMap& data = {}) const override;

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

#endif  // SRC_TINT_TRANSFORM_NUM_WORKGROUPS_FROM_UNIFORM_H_
