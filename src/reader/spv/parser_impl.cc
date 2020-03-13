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

#include <cstring>

#include "spirv-tools/libspirv.hpp"
#include "src/reader/spv/parser_impl.h"

namespace tint {
namespace reader {
namespace spv {

ParserImpl::ParserImpl(const std::vector<uint32_t>& spv_binary)
    : Reader(), spv_binary_(spv_binary), fail_stream_(&success_, &errors_) {}

ParserImpl::~ParserImpl() = default;

bool ParserImpl::Parse() {
  if (!success_) {
    return false;
  }

  // Set up use of SPIRV-Tools utilities.
  // TODO(dneto): Add option to handle other environments.
  spvtools::SpirvTools spv_tools(SPV_ENV_WEBGPU_0);

  // Error messages from SPIRV-Tools are forwarded as failures.
  auto message_consumer =
      [this](spv_message_level_t level, const char* /*source*/,
             const spv_position_t& position, const char* message) {
        switch (level) {
          // Drop info and warning message.
          case SPV_MSG_WARNING:
          case SPV_MSG_INFO:
          default:
            // For binary validation errors, we only have the instruction
            // number.  It's not text, so there is no column number.
            this->Fail() << "line:" << position.index << ": " << message;
        }
      };
  spv_tools.SetMessageConsumer(message_consumer);

  // Only consider valid modules.
  if (success_) {
    success_ = spv_tools.Validate(spv_binary_);
  }

  return success_;
}

ast::Module ParserImpl::module() {
  // TODO(dneto): Should we clear out spv_binary_ here, to reduce
  // memory usage?
  return std::move(ast_module_);
}

}  // namespace spv
}  // namespace reader
}  // namespace tint
