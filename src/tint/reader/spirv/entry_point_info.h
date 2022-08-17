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

#ifndef SRC_TINT_READER_SPIRV_ENTRY_POINT_INFO_H_
#define SRC_TINT_READER_SPIRV_ENTRY_POINT_INFO_H_

#include <string>

#include "src/tint/ast/pipeline_stage.h"
#include "src/tint/utils/vector.h"

namespace tint::reader::spirv {

/// The size of an integer-coordinate grid, in the x, y, and z dimensions.
struct GridSize {
    /// x value
    uint32_t x = 0;
    /// y value
    uint32_t y = 0;
    /// z value
    uint32_t z = 0;
};

/// Entry point information for a function
struct EntryPointInfo {
    /// Constructor.
    /// @param the_name the name of the entry point
    /// @param the_stage the pipeline stage
    /// @param the_owns_inner_implementation if true, this entry point is
    /// responsible for generating the inner implementation function.
    /// @param the_inner_name the name of the inner implementation function of the
    /// entry point
    /// @param the_inputs list of IDs for Input variables used by the shader
    /// @param the_outputs list of IDs for Output variables used by the shader
    /// @param the_wg_size the workgroup_size, for a compute shader
    EntryPointInfo(std::string the_name,
                   ast::PipelineStage the_stage,
                   bool the_owns_inner_implementation,
                   std::string the_inner_name,
                   utils::VectorRef<uint32_t> the_inputs,
                   utils::VectorRef<uint32_t> the_outputs,
                   GridSize the_wg_size);
    /// Copy constructor
    /// @param other the other entry point info to be built from
    EntryPointInfo(const EntryPointInfo& other);
    /// Destructor
    ~EntryPointInfo();

    /// The entry point name.
    /// In the WGSL output, this function will have pipeline inputs and outputs
    /// as parameters. This function will store them into Private variables,
    /// and then call the "inner" function, named by the next memeber.
    /// Then outputs are copied from the private variables to the return value.
    std::string name;
    /// The entry point stage
    ast::PipelineStage stage = ast::PipelineStage::kNone;

    /// True when this entry point is responsible for generating the
    /// inner implementation function.  False when this is the second entry
    /// point encountered for the same function in SPIR-V. It's unusual, but
    /// possible for the same function to be the implementation for multiple
    /// entry points.
    bool owns_inner_implementation;
    /// The name of the inner implementation function of the entry point.
    std::string inner_name;
    /// IDs of pipeline input variables, sorted and without duplicates.
    utils::Vector<uint32_t, 8> inputs;
    /// IDs of pipeline output variables, sorted and without duplicates.
    utils::Vector<uint32_t, 8> outputs;

    /// If this is a compute shader, this is the workgroup size in the x, y,
    /// and z dimensions set via LocalSize, or via the composite value
    /// decorated as the WorkgroupSize BuiltIn.  The WorkgroupSize builtin
    /// takes priority.
    GridSize workgroup_size;
};

}  // namespace tint::reader::spirv

#endif  // SRC_TINT_READER_SPIRV_ENTRY_POINT_INFO_H_
