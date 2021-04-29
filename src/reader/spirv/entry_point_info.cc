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

#include "src/reader/spirv/entry_point_info.h"

#include <utility>

namespace tint {
namespace reader {
namespace spirv {

EntryPointInfo::EntryPointInfo(std::string the_name,
                               ast::PipelineStage the_stage,
                               bool the_owns_inner_implementation,
                               std::string the_inner_name,
                               std::vector<uint32_t>&& the_inputs,
                               std::vector<uint32_t>&& the_outputs)
    : name(the_name),
      stage(the_stage),
      owns_inner_implementation(the_owns_inner_implementation),
      inner_name(std::move(the_inner_name)),
      inputs(std::move(the_inputs)),
      outputs(std::move(the_outputs)) {}

EntryPointInfo::EntryPointInfo(const EntryPointInfo&) = default;

EntryPointInfo::~EntryPointInfo() = default;

}  // namespace spirv
}  // namespace reader
}  // namespace tint
