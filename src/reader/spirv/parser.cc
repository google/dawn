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

#include "src/reader/spirv/parser.h"

#include "src/reader/spirv/parser_impl.h"

namespace tint {
namespace reader {
namespace spirv {

Parser::Parser(const std::vector<uint32_t>& spv_binary)
    : Reader(), impl_(std::make_unique<ParserImpl>(spv_binary)) {}

Parser::~Parser() = default;

bool Parser::Parse() {
  const auto result = impl_->Parse();
  auto err_msg = impl_->error();
  if (!err_msg.empty()) {
    // TODO(bclayton): Migrate spirv::ParserImpl to using diagnostics.
    diag::Diagnostic error{};
    error.severity = diag::Severity::Error;
    error.message = err_msg;
    set_diagnostics({error});
  }
  return result;
}

ast::Module Parser::module() {
  return impl_->module();
}

}  // namespace spirv
}  // namespace reader
}  // namespace tint
