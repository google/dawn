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

#include "src/reader/spv/parser_impl.h"

namespace tint {
namespace reader {
namespace spv {

ParserImpl::ParserImpl(const std::vector<uint32_t>& spv_binary)
    : Reader(), spv_binary_(spv_binary), fail_stream_(&success_, &errors_) {}

ParserImpl::~ParserImpl() = default;

bool ParserImpl::Parse() {
  // Exit early if we've already failed.
  if (success_) {
    Fail() << "SPIR-V parsing is not supported yet";
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
