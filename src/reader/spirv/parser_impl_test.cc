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

#include "src/reader/spirv/parser_impl.h"

#include <cstdint>
#include <vector>

#include "gmock/gmock.h"
#include "src/reader/spirv/parser_impl_test_helper.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"

namespace tint {
namespace reader {
namespace spirv {

namespace {

using ::testing::HasSubstr;

TEST_F(SpvParserTest, Impl_Uint32VecEmpty) {
  std::vector<uint32_t> data;
  auto p = parser(data);
  EXPECT_FALSE(p->Parse());
  // TODO(dneto): What message?
}

TEST_F(SpvParserTest, Impl_InvalidModuleFails) {
  auto invalid_spv = test::Assemble("%ty = OpTypeInt 3 0");
  auto p = parser(invalid_spv);
  EXPECT_FALSE(p->Parse());
  EXPECT_THAT(
      p->error(),
      HasSubstr("TypeInt cannot appear before the memory model instruction"));
  EXPECT_THAT(p->error(), HasSubstr("OpTypeInt 3 0"));
}

// TODO(dneto): uint32 vec, valid SPIR-V

}  // namespace

}  // namespace spirv
}  // namespace reader
}  // namespace tint
