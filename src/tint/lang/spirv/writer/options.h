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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_OPTIONS_H_
#define SRC_TINT_LANG_SPIRV_WRITER_OPTIONS_H_

#include "src/tint/utils/reflection/reflection.h"
#include "tint/binding_remapper_options.h"
#include "tint/external_texture_options.h"

namespace tint::spirv::writer {

/// Configuration options used for generating SPIR-V.
struct Options {
    /// Set to `true` to disable software robustness that prevents out-of-bounds accesses.
    bool disable_robustness = false;

    /// Set to `true` to generate a PointSize builtin and have it set to 1.0
    /// from all vertex shaders in the module.
    bool emit_vertex_point_size = true;

    /// Set to `true` to disable workgroup memory zero initialization
    bool disable_workgroup_init = false;

    /// Set to `true` to clamp frag depth
    bool clamp_frag_depth = false;

    /// Options used in the binding mappings for external textures
    ExternalTextureOptions external_texture_options = {};

    /// Options used in the bindings remapper
    BindingRemapperOptions binding_remapper_options = {};

    /// Set to `true` to initialize workgroup memory with OpConstantNull when
    /// VK_KHR_zero_initialize_workgroup_memory is enabled.
    bool use_zero_initialize_workgroup_memory_extension = false;

    /// Set to `true` to skip robustness transform on textures.
    bool disable_image_robustness = false;

    /// Set to `true` to disable index clamping on the runtime-sized arrays in robustness transform.
    bool disable_runtime_sized_array_index_clamping = false;

#if TINT_BUILD_IR
    /// Set to `true` to generate SPIR-V via the Tint IR instead of from the AST.
    bool use_tint_ir = false;
#endif

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(disable_robustness,
                 emit_vertex_point_size,
                 disable_workgroup_init,
                 external_texture_options,
                 use_zero_initialize_workgroup_memory_extension,
                 disable_image_robustness,
                 disable_runtime_sized_array_index_clamping);
};

}  // namespace tint::spirv::writer

#endif  // SRC_TINT_LANG_SPIRV_WRITER_OPTIONS_H_
