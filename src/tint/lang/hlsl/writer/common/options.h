// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_LANG_HLSL_WRITER_COMMON_OPTIONS_H_
#define SRC_TINT_LANG_HLSL_WRITER_COMMON_OPTIONS_H_

#include <bitset>
#include <optional>
#include <unordered_map>
#include <vector>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/api/options/array_length_from_uniform.h"
#include "src/tint/api/options/binding_remapper.h"
#include "src/tint/api/options/external_texture.h"
#include "src/tint/utils/reflection/reflection.h"

namespace tint::hlsl::writer {

/// Configuration options used for generating HLSL.
struct Options {
    /// Constructor
    Options();
    /// Destructor
    ~Options();
    /// Copy constructor
    Options(const Options&);
    /// Copy assignment
    /// @returns this Options
    Options& operator=(const Options&);

    /// Set to `true` to disable software robustness that prevents out-of-bounds accesses.
    bool disable_robustness = false;

    /// Set to `true` to disable workgroup memory zero initialization
    bool disable_workgroup_init = false;

    /// Set to `true` to run the TruncateInterstageVariables transform.
    bool truncate_interstage_variables = false;

    /// Set to `true` to generate polyfill for `reflect` builtin for vec2<f32>
    bool polyfill_reflect_vec2_f32 = false;

    /// Options used to specify a mapping of binding points to indices into a UBO
    /// from which to load buffer sizes.
    ArrayLengthFromUniformOptions array_length_from_uniform = {};

    /// Interstage locations actually used as inputs in the next stage of the pipeline.
    /// This is potentially used for truncating unused interstage outputs at current shader stage.
    std::bitset<16> interstage_locations;

    /// The binding point to use for information passed via root constants.
    std::optional<BindingPoint> root_constant_binding_point;

    /// Options used in the binding mappings for external textures
    ExternalTextureOptions external_texture_options = {};

    /// Options used in the bindings remapper
    BindingRemapperOptions binding_remapper_options = {};

    /// The binding points that will be ignored in the rebustness transform.
    std::vector<BindingPoint> binding_points_ignored_in_robustness_transform;

    /// AccessControls is a map of old binding point to new access control
    std::unordered_map<BindingPoint, core::Access> access_controls;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(disable_robustness,
                 disable_workgroup_init,
                 truncate_interstage_variables,
                 polyfill_reflect_vec2_f32,
                 array_length_from_uniform,
                 interstage_locations,
                 root_constant_binding_point,
                 external_texture_options,
                 binding_remapper_options,
                 binding_points_ignored_in_robustness_transform,
                 access_controls);
};

}  // namespace tint::hlsl::writer

#endif  // SRC_TINT_LANG_HLSL_WRITER_COMMON_OPTIONS_H_
