// Copyright 2021 The Tint Authors
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

#include "src/reader/spirv/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace spirv {
namespace test {

// Default to not dumping the SPIR-V assembly.
bool ParserImplWrapperForTest::dump_successfully_converted_spirv_ = false;

ParserImplWrapperForTest::ParserImplWrapperForTest(
    const std::vector<uint32_t>& input)
    : impl_(input) {}

ParserImplWrapperForTest::~ParserImplWrapperForTest() {
  if (dump_successfully_converted_spirv_ && !skip_dumping_spirv_ &&
      !impl_.spv_binary().empty() && impl_.success()) {
    std::string disassembly = Disassemble(impl_.spv_binary());
    std::cout << "BEGIN ConvertedOk:\n"
              << disassembly << "\nEND ConvertedOk" << std::endl;
  }
}

}  // namespace test
}  // namespace spirv
}  // namespace reader
}  // namespace tint
