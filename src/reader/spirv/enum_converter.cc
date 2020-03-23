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

#include "src/reader/spirv/enum_converter.h"

namespace tint {
namespace reader {
namespace spirv {

EnumConverter::EnumConverter(const FailStream& fs) : fail_stream_(fs) {}

EnumConverter::~EnumConverter() = default;

ast::PipelineStage EnumConverter::ToPipelineStage(SpvExecutionModel model) {
  switch (model) {
    case SpvExecutionModelVertex:
      return ast::PipelineStage::kVertex;
    case SpvExecutionModelFragment:
      return ast::PipelineStage::kFragment;
    case SpvExecutionModelGLCompute:
      return ast::PipelineStage::kCompute;
    default:
      break;
  }

  Fail() << "unknown SPIR-V execution model: " << uint32_t(model);
  return ast::PipelineStage::kNone;
}

}  // namespace spirv
}  // namespace reader
}  // namespace tint
