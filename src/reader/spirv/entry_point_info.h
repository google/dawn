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

#ifndef SRC_READER_SPIRV_ENTRY_POINT_INFO_H_
#define SRC_READER_SPIRV_ENTRY_POINT_INFO_H_

#include <string>
#include <vector>

#include "src/ast/pipeline_stage.h"

namespace tint {
namespace reader {
namespace spirv {

/// Entry point information for a function
struct EntryPointInfo {
  // Constructor.
  // @param the_name the name of the entry point
  // @param the_stage the pipeline stage
  // @param the_inputs list of IDs for Input variables used by the shader
  // @param the_outputs list of IDs for Output variables used by the shader
  EntryPointInfo(std::string the_name,
                 ast::PipelineStage the_stage,
                 std::vector<uint32_t>&& the_inputs,
                 std::vector<uint32_t>&& the_outputs);
  // Copy constructor
  // @param other the other entry point info to be built from
  EntryPointInfo(const EntryPointInfo& other);
  // Destructor
  ~EntryPointInfo();

  /// The entry point name
  std::string name;
  /// The entry point stage
  ast::PipelineStage stage = ast::PipelineStage::kNone;
  /// IDs of pipeline input variables, sorted and without duplicates.
  std::vector<uint32_t> inputs;
  /// IDs of pipeline output variables, sorted and without duplicates.
  std::vector<uint32_t> outputs;
};

}  // namespace spirv
}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_SPIRV_ENTRY_POINT_INFO_H_
