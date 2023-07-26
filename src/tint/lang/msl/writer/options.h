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

#ifndef SRC_TINT_LANG_MSL_WRITER_OPTIONS_H_
#define SRC_TINT_LANG_MSL_WRITER_OPTIONS_H_

#include "src/tint/utils/reflection/reflection.h"
#include "tint/array_length_from_uniform_options.h"
#include "tint/binding_remapper_options.h"
#include "tint/external_texture_options.h"

namespace tint::msl::writer {

/// Configuration options used for generating MSL.
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

    /// The index to use when generating a UBO to receive storage buffer sizes.
    /// Defaults to 30, which is the last valid buffer slot.
    uint32_t buffer_size_ubo_index = 30;

    /// The fixed sample mask to combine with fragment shader outputs.
    /// Defaults to 0xFFFFFFFF.
    uint32_t fixed_sample_mask = 0xFFFFFFFF;

    /// Set to `true` to generate a [[point_size]] attribute which is set to 1.0
    /// for all vertex shaders in the module.
    bool emit_vertex_point_size = false;

    /// Set to `true` to disable workgroup memory zero initialization
    bool disable_workgroup_init = false;

    /// Options used in the binding mappings for external textures
    ExternalTextureOptions external_texture_options = {};

    /// Options used to specify a mapping of binding points to indices into a UBO
    /// from which to load buffer sizes.
    ArrayLengthFromUniformOptions array_length_from_uniform = {};

    /// Options used in the bindings remapper
    BindingRemapperOptions binding_remapper_options = {};

#if TINT_BUILD_IR
    /// Set to `true` to generate MSL via the Tint IR instead of from the AST.
    bool use_tint_ir = false;
#endif

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(disable_robustness,
                 buffer_size_ubo_index,
                 fixed_sample_mask,
                 emit_vertex_point_size,
                 disable_workgroup_init,
                 external_texture_options,
                 array_length_from_uniform);
};

}  // namespace tint::msl::writer

#endif  // SRC_TINT_LANG_MSL_WRITER_OPTIONS_H_
