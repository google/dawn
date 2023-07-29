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

#ifndef SRC_TINT_LANG_WGSL_AST_TRANSFORM_NUM_WORKGROUPS_FROM_UNIFORM_H_
#define SRC_TINT_LANG_WGSL_AST_TRANSFORM_NUM_WORKGROUPS_FROM_UNIFORM_H_

#include <optional>

#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "tint/binding_point.h"

namespace tint::ast::transform {

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
class NumWorkgroupsFromUniform final : public Castable<NumWorkgroupsFromUniform, Transform> {
  public:
    /// Constructor
    NumWorkgroupsFromUniform();
    /// Destructor
    ~NumWorkgroupsFromUniform() override;

    /// Configuration options for the NumWorkgroupsFromUniform transform.
    struct Config final : public Castable<Config, Data> {
        /// Constructor
        /// @param ubo_bp the binding point to use for the generated uniform buffer. If ubo_bp
        /// contains no value, a free binding point will be used to ensure the generated program is
        /// valid. Specifically, binding 0 of the largest used group plus 1 is used if at least one
        /// resource is bound, otherwise group 0 binding 0 is used.
        explicit Config(std::optional<BindingPoint> ubo_bp);

        /// Copy constructor
        Config(const Config&);

        /// Destructor
        ~Config() override;

        /// The binding point to use for the generated uniform buffer. If ubo_bp contains no value,
        /// a free binding point will be used. Specifically, binding 0 of the largest used group
        /// plus 1 is used if at least one resource is bound, otherwise group 0 binding 0 is used.
        std::optional<BindingPoint> ubo_binding;
    };

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;
};

}  // namespace tint::ast::transform

#endif  // SRC_TINT_LANG_WGSL_AST_TRANSFORM_NUM_WORKGROUPS_FROM_UNIFORM_H_
