// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_LANG_WGSL_INSPECTOR_ENTRY_POINT_H_
#define SRC_TINT_LANG_WGSL_INSPECTOR_ENTRY_POINT_H_

#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include "src/tint/api/common/override_id.h"

#include "src/tint/lang/wgsl/ast/interpolate_attribute.h"
#include "src/tint/lang/wgsl/ast/pipeline_stage.h"

namespace tint::inspector {

/// Base component type of a stage variable.
enum class ComponentType : uint8_t {
    kF32,
    kU32,
    kI32,
    kF16,
    kUnknown,
};

/// Composition of components of a stage variable.
enum class CompositionType : uint8_t {
    kScalar,
    kVec2,
    kVec3,
    kVec4,
    kUnknown,
};

/// Types of `pixel_local` variable members.
enum class PixelLocalMemberType : uint8_t {
    kF32,
    kU32,
    kI32,
    kUnknown,
};

/// Type of interpolation of a stage variable.
enum class InterpolationType : uint8_t { kPerspective, kLinear, kFlat, kUnknown };

/// Type of interpolation sampling of a stage variable.
enum class InterpolationSampling : uint8_t { kNone, kCenter, kCentroid, kSample, kUnknown };

/// Reflection data about an entry point input or output.
struct StageVariable {
    /// Constructor
    StageVariable();
    /// Copy constructor
    /// @param other the StageVariable to copy
    StageVariable(const StageVariable& other);
    /// Destructor
    ~StageVariable();

    /// Name of the variable in the shader.
    std::string name;
    /// Is location attribute present
    bool has_location_attribute = false;
    /// Value of the location attribute, only valid if #has_location_attribute is
    /// true.
    uint32_t location_attribute;
    /// Scalar type that the variable is composed of.
    ComponentType component_type = ComponentType::kUnknown;
    /// How the scalars are composed for the variable.
    CompositionType composition_type = CompositionType::kUnknown;
    /// Interpolation type of the variable.
    InterpolationType interpolation_type = InterpolationType::kUnknown;
    /// Interpolation sampling of the variable.
    InterpolationSampling interpolation_sampling = InterpolationSampling::kUnknown;
};

/// Reflection data about an override variable referenced by an entry point
struct Override {
    /// Name of the override
    std::string name;

    /// ID of the override
    OverrideId id;

    /// Type of the scalar
    enum class Type {
        kBool,
        kFloat32,
        kUint32,
        kInt32,
        kFloat16,
    };

    /// Type of the scalar
    Type type;

    /// Does this override have an initializer?
    bool is_initialized = false;

    /// Does this override have a numeric ID specified explicitly?
    bool is_id_specified = false;
};

/// The pipeline stage
enum class PipelineStage { kVertex, kFragment, kCompute };

/// WorkgroupSize describes the dimensions of the workgroup grid for a compute shader.
struct WorkgroupSize {
    /// The 'x' dimension of the workgroup grid
    uint32_t x = 1;
    /// The 'y' dimension of the workgroup grid
    uint32_t y = 1;
    /// The 'z' dimension of the workgroup grid
    uint32_t z = 1;
};

/// Reflection data for an entry point in the shader.
struct EntryPoint {
    /// Constructors
    EntryPoint();
    /// Copy Constructor
    EntryPoint(EntryPoint&);
    /// Move Constructor
    EntryPoint(EntryPoint&&);
    ~EntryPoint();

    /// The entry point name
    std::string name;
    /// Remapped entry point name in the backend
    std::string remapped_name;
    /// The entry point stage
    PipelineStage stage;
    /// The workgroup size. If PipelineStage is kCompute and this holds no value, then the workgroup
    /// size is derived from an override-expression. In this situation you first need to run the
    /// tint::ast::transform::SubstituteOverride transform before using the inspector.
    std::optional<WorkgroupSize> workgroup_size;
    /// The total size in bytes of all Workgroup storage-class storage accessed via the entry point.
    uint32_t workgroup_storage_size = 0;
    /// List of the input variable accessed via this entry point.
    std::vector<StageVariable> input_variables;
    /// List of the output variable accessed via this entry point.
    std::vector<StageVariable> output_variables;
    /// List of the pipeline overridable constants accessed via this entry point.
    std::vector<Override> overrides;
    /// List of the variable types used in the `pixel_local` block accessed by this entry point (if
    /// any).
    std::vector<PixelLocalMemberType> pixel_local_members;
    /// Does the entry point use the sample_mask builtin as an input builtin
    /// variable.
    bool input_sample_mask_used = false;
    /// Does the entry point use the sample_mask builtin as an output builtin
    /// variable.
    bool output_sample_mask_used = false;
    /// Does the entry point use the position builtin as an input builtin
    /// variable.
    bool input_position_used = false;
    /// Does the entry point use the front_facing builtin
    bool front_facing_used = false;
    /// Does the entry point use the sample_index builtin
    bool sample_index_used = false;
    /// Does the entry point use the num_workgroups builtin
    bool num_workgroups_used = false;
    /// Does the entry point use the frag_depth builtin
    bool frag_depth_used = false;
    /// Does the entry point use the vertex_index builtin
    bool vertex_index_used = false;
    /// Does the entry point use the instance_index builtin
    bool instance_index_used = false;
};

}  // namespace tint::inspector

#endif  // SRC_TINT_LANG_WGSL_INSPECTOR_ENTRY_POINT_H_
