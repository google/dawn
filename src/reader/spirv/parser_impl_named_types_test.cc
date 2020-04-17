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

#include <cstdint>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "src/ast/struct.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/vector_type.h"
#include "src/reader/spirv/parser_impl.h"
#include "src/reader/spirv/parser_impl_test_helper.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"
#include "src/type_manager.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::HasSubstr;

TEST_F(SpvParserTest, NamedTypes_AnonStruct) {
  auto* p = parser(test::Assemble(R"(
    %uint = OpTypeInt 32 0
    %s = OpTypeStruct %uint %uint
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->module().to_str(), HasSubstr("S -> __struct_"));
}

TEST_F(SpvParserTest, NamedTypes_NamedStruct) {
  auto* p = parser(test::Assemble(R"(
    OpName %s "mystruct"
    %uint = OpTypeInt 32 0
    %s = OpTypeStruct %uint %uint
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->module().to_str(), HasSubstr("mystruct -> __struct_"));
}

// TODO(dneto): Enable this when array types can have ArrayStride
TEST_F(SpvParserTest, DISABLED_NamedTypes_AnonArrayWithDecoration) {
  auto* p = parser(test::Assemble(R"(
    OpDecorate %arr ArrayStride 16
    %uint = OpTypeInt 32 0
    %uint_3 = OpConstant %uint 3
    %arr = OpTypeArray %uint %uint_3
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->module().to_str(), HasSubstr("Arr -> __array__u32"));
}

// TODO(dneto): Should we make an alias for an un-decoratrd array with
// an OpName?

TEST_F(SpvParserTest, NamedTypes_AnonRTArray) {
  auto* p = parser(test::Assemble(R"(
    %uint = OpTypeInt 32 0
    %arr = OpTypeRuntimeArray %uint
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->module().to_str(), HasSubstr("RTArr -> __array__u32"));
}

TEST_F(SpvParserTest, NamedTypes_NamedRTArray) {
  auto* p = parser(test::Assemble(R"(
    OpName %arr "myrtarr"
    %uint = OpTypeInt 32 0
    %arr = OpTypeRuntimeArray %uint
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->module().to_str(), HasSubstr("myrtarr -> __array__u32"));
}

// TODO(dneto): Handle arrays sized by a spec constant.
// Blocked by crbug.com/tint/32

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
