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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_COMMON_OPTIONS_H_
#define SRC_TINT_LANG_SPIRV_WRITER_COMMON_OPTIONS_H_

#include <unordered_map>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/utils/reflection/reflection.h"

namespace tint::spirv::writer {
namespace binding {

/// Generic binding point
struct BindingInfo {
    /// The group
    uint32_t group = 0;
    /// The binding
    uint32_t binding = 0;

    /// Equality operator
    /// @param rhs the BindingInfo to compare against
    /// @returns true if this BindingInfo is equal to `rhs`
    inline bool operator==(const BindingInfo& rhs) const {
        return group == rhs.group && binding == rhs.binding;
    }

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(group, binding);
};
using Uniform = BindingInfo;
using Storage = BindingInfo;
using Texture = BindingInfo;
using StorageTexture = BindingInfo;
using Sampler = BindingInfo;

/// An external texture
struct ExternalTexture {
    /// Metadata
    BindingInfo metadata{};
    /// Plane0 binding data
    BindingInfo plane0{};
    /// Plane1 binding data
    BindingInfo plane1{};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(metadata, plane0, plane1);
};

}  // namespace binding

// Maps the WGSL binding point to the SPIR-V group,binding for uniforms
using UniformBindings = std::unordered_map<BindingPoint, binding::Uniform>;
// Maps the WGSL binding point to the SPIR-V group,binding for storage
using StorageBindings = std::unordered_map<BindingPoint, binding::Storage>;
// Maps the WGSL binding point to the SPIR-V group,binding for textures
using TextureBindings = std::unordered_map<BindingPoint, binding::Texture>;
// Maps the WGSL binding point to the SPIR-V group,binding for storage textures
using StorageTextureBindings = std::unordered_map<BindingPoint, binding::StorageTexture>;
// Maps the WGSL binding point to the SPIR-V group,binding for samplers
using SamplerBindings = std::unordered_map<BindingPoint, binding::Sampler>;
// Maps the WGSL binding point to the plane0, plane1, and metadata information for external textures
using ExternalTextureBindings = std::unordered_map<BindingPoint, binding::ExternalTexture>;

/// Binding information
struct Bindings {
    /// Uniform bindings
    UniformBindings uniform{};
    /// Storage bindings
    StorageBindings storage{};
    /// Texture bindings
    TextureBindings texture{};
    /// Storage texture bindings
    StorageTextureBindings storage_texture{};
    /// Sampler bindings
    SamplerBindings sampler{};
    /// External bindings
    ExternalTextureBindings external_texture{};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(uniform, storage, texture, sampler, external_texture);
};

/// Configuration options used for generating SPIR-V.
struct Options {
    /// Set to `true` to disable software robustness that prevents out-of-bounds accesses.
    bool disable_robustness = false;

    /// Set to `true` to skip robustness transform on textures.
    bool disable_image_robustness = false;

    /// Set to `true` to disable index clamping on the runtime-sized arrays in robustness transform.
    bool disable_runtime_sized_array_index_clamping = false;

    /// Set to `true` to disable workgroup memory zero initialization
    bool disable_workgroup_init = false;

    /// Set to `true` to initialize workgroup memory with OpConstantNull when
    /// VK_KHR_zero_initialize_workgroup_memory is enabled.
    bool use_zero_initialize_workgroup_memory_extension = false;

    /// Set to `true` to generate a PointSize builtin and have it set to 1.0
    /// from all vertex shaders in the module.
    bool emit_vertex_point_size = true;

    /// Set to `true` to clamp frag depth
    bool clamp_frag_depth = false;

    /// Set to `true` to always pass matrices to user functions by pointer instead of by value.
    bool pass_matrix_by_pointer = false;

    /// Set to `true` to generate SPIR-V via the Tint IR instead of from the AST.
    bool use_tint_ir = false;

    /// Set to `true` to require `SPV_KHR_subgroup_uniform_control_flow` extension and
    /// `SubgroupUniformControlFlowKHR` execution mode for compute stage entry points in generated
    /// SPIRV module. Issue: dawn:464
    bool experimental_require_subgroup_uniform_control_flow = false;

    /// The bindings
    Bindings bindings;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(disable_robustness,
                 disable_image_robustness,
                 disable_runtime_sized_array_index_clamping,
                 disable_workgroup_init,
                 use_zero_initialize_workgroup_memory_extension,
                 emit_vertex_point_size,
                 clamp_frag_depth,
                 use_tint_ir,
                 experimental_require_subgroup_uniform_control_flow,
                 bindings);
};

}  // namespace tint::spirv::writer

#endif  // SRC_TINT_LANG_SPIRV_WRITER_COMMON_OPTIONS_H_
