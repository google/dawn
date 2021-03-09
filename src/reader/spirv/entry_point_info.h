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

namespace tint {
namespace reader {
namespace spirv {

/// Entry point information for a function
struct EntryPointInfo {
  /// The entry point name
  std::string name;
  /// The entry point stage
  ast::PipelineStage stage = ast::PipelineStage::kNone;
};

}  // namespace spirv
}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_SPIRV_ENTRY_POINT_INFO_H_
