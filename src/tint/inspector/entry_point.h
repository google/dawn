// Copyright 2020 The Tint Authors.
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

#ifndef SRC_TINT_INSPECTOR_ENTRY_POINT_H_
#define SRC_TINT_INSPECTOR_ENTRY_POINT_H_

#include <string>
#include <tuple>
#include <vector>

#include "src/tint/ast/interpolate_attribute.h"
#include "src/tint/ast/pipeline_stage.h"

namespace tint::inspector {

/// Base component type of a stage variable.
enum class ComponentType {
    kUnknown = -1,
    kFloat,
    kUInt,
    kSInt,
};

/// Composition of components of a stage variable.
enum class CompositionType {
    kUnknown = -1,
    kScalar,
    kVec2,
    kVec3,
    kVec4,
};

/// Type of interpolation of a stage variable.
enum class InterpolationType { kUnknown = -1, kPerspective, kLinear, kFlat };

/// Type of interpolation sampling of a stage variable.
enum class InterpolationSampling { kUnknown = -1, kNone, kCenter, kCentroid, kSample };

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
    /// Is Location attribute present
    /// [DEPRECATED]: Use #has_location_attribute
    bool& has_location_decoration = has_location_attribute;
    /// Value of Location Decoration, only valid if #has_location_decoration is
    /// true.
    /// [DEPRECATED]: Use #location_attribute
    uint32_t& location_decoration = location_attribute;
    /// Scalar type that the variable is composed of.
    ComponentType component_type = ComponentType::kUnknown;
    /// How the scalars are composed for the variable.
    CompositionType composition_type = CompositionType::kUnknown;
    /// Interpolation type of the variable.
    InterpolationType interpolation_type = InterpolationType::kUnknown;
    /// Interpolation sampling of the variable.
    InterpolationSampling interpolation_sampling = InterpolationSampling::kUnknown;
};

/// Convert from internal ast::InterpolationType to public ::InterpolationType.
/// @param ast_type internal value to convert from
/// @returns the publicly visible equivalent
InterpolationType ASTToInspectorInterpolationType(ast::InterpolationType ast_type);

/// Convert from internal ast::InterpolationSampling to public
/// ::InterpolationSampling
/// @param sampling internal value to convert from
/// @returns the publicly visible equivalent
InterpolationSampling ASTToInspectorInterpolationSampling(ast::InterpolationSampling sampling);

/// Reflection data about a pipeline overridable constant referenced by an entry
/// point
struct OverridableConstant {
    /// Name of the constant
    std::string name;

    /// ID of the constant
    uint16_t numeric_id;

    /// Type of the scalar
    enum class Type {
        kBool,
        kFloat32,
        kUint32,
        kInt32,
    };

    /// Type of the scalar
    Type type;

    /// Does this pipeline overridable constant have an initializer?
    bool is_initialized = false;

    /// Does this pipeline overridable constant have a numeric ID specified
    /// explicitly?
    bool is_numeric_id_specified = false;
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
    ast::PipelineStage stage = ast::PipelineStage::kNone;
    /// The workgroup x size
    uint32_t workgroup_size_x = 0;
    /// The workgroup y size
    uint32_t workgroup_size_y = 0;
    /// The workgroup z size
    uint32_t workgroup_size_z = 0;
    /// List of the input variable accessed via this entry point.
    std::vector<StageVariable> input_variables;
    /// List of the output variable accessed via this entry point.
    std::vector<StageVariable> output_variables;
    /// List of the pipeline overridable constants accessed via this entry point.
    std::vector<OverridableConstant> overridable_constants;
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

    /// @returns the size of the workgroup in {x,y,z} format
    std::tuple<uint32_t, uint32_t, uint32_t> workgroup_size() {
        return std::tuple<uint32_t, uint32_t, uint32_t>(workgroup_size_x, workgroup_size_y,
                                                        workgroup_size_z);
    }
};

}  // namespace tint::inspector

#endif  // SRC_TINT_INSPECTOR_ENTRY_POINT_H_
