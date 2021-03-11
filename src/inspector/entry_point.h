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

#ifndef SRC_INSPECTOR_ENTRY_POINT_H_
#define SRC_INSPECTOR_ENTRY_POINT_H_

#include <string>
#include <tuple>
#include <vector>

#include "src/ast/pipeline_stage.h"

namespace tint {
namespace inspector {

/// Base component type of a stage variable.
enum class ComponentType {
  kUnknown = -1,
  kFloat,
  kUInt,
  kSInt,
};

/// Reflection data about an entry point input or output.
struct StageVariable {
  /// Name of the variable in the shader.
  std::string name;
  /// Is Location Decoration present
  bool has_location_decoration;
  /// Value of Location Decoration, only valid if |has_location_decoration| is
  /// true.
  uint32_t location_decoration;
  /// Scalar type that the variable is composed of.
  ComponentType component_type;
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
  uint32_t workgroup_size_x;
  /// The workgroup y size
  uint32_t workgroup_size_y;
  /// The workgroup z size
  uint32_t workgroup_size_z;
  /// List of the input variable accessed via this entry point.
  std::vector<StageVariable> input_variables;
  /// List of the output variable accessed via this entry point.
  std::vector<StageVariable> output_variables;

  /// @returns the size of the workgroup in {x,y,z} format
  std::tuple<uint32_t, uint32_t, uint32_t> workgroup_size() {
    return std::tuple<uint32_t, uint32_t, uint32_t>(
        workgroup_size_x, workgroup_size_y, workgroup_size_z);
  }
};

}  // namespace inspector
}  // namespace tint

#endif  // SRC_INSPECTOR_ENTRY_POINT_H_
