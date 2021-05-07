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

#include "gmock/gmock.h"
#include "src/reader/spirv/parser_impl_test_helper.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::HasSubstr;

TEST_F(SpvParserTest, NamedTypes_AnonStruct) {
  auto p = parser(test::Assemble(R"(
    %uint = OpTypeInt 32 0
    %s = OpTypeStruct %uint %uint
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->program().to_str(), HasSubstr("Struct S"));

  p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserTest, NamedTypes_NamedStruct) {
  auto p = parser(test::Assemble(R"(
    OpName %s "mystruct"
    %uint = OpTypeInt 32 0
    %s = OpTypeStruct %uint %uint
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->program().to_str(), HasSubstr("Struct mystruct"));

  p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserTest, NamedTypes_Dup_EmitBoth) {
  auto p = parser(test::Assemble(R"(
    %uint = OpTypeInt 32 0
    %s = OpTypeStruct %uint %uint
    %s2 = OpTypeStruct %uint %uint
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule()) << p->error();
  EXPECT_THAT(p->program().to_str(), HasSubstr(R"(Struct S {
    StructMember{field0: __u32}
    StructMember{field1: __u32}
  }
  Struct S_1 {
    StructMember{field0: __u32}
    StructMember{field1: __u32}
  })"));

  p->DeliberatelyInvalidSpirv();
}

// TODO(dneto): Should we make an alias for an un-decoratrd array with
// an OpName?

TEST_F(SpvParserTest, NamedTypes_AnonRTArrayWithDecoration) {
  // Runtime arrays are always in SSBO, and those are always laid out.
  auto p = parser(test::Assemble(R"(
    OpDecorate %arr ArrayStride 8
    %uint = OpTypeInt 32 0
    %arr = OpTypeRuntimeArray %uint
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->program().to_str(),
              HasSubstr("RTArr -> __array__u32_stride_8\n"));

  p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserTest, NamedTypes_AnonRTArray_Dup_EmitBoth) {
  auto p = parser(test::Assemble(R"(
    OpDecorate %arr ArrayStride 8
    OpDecorate %arr2 ArrayStride 8
    %uint = OpTypeInt 32 0
    %arr = OpTypeRuntimeArray %uint
    %arr2 = OpTypeRuntimeArray %uint
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->program().to_str(),
              HasSubstr("RTArr -> __array__u32_stride_8\n  RTArr_1 -> "
                        "__array__u32_stride_8\n"));

  p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserTest, NamedTypes_NamedRTArray) {
  auto p = parser(test::Assemble(R"(
    OpName %arr "myrtarr"
    OpDecorate %arr ArrayStride 8
    %uint = OpTypeInt 32 0
    %arr = OpTypeRuntimeArray %uint
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->program().to_str(),
              HasSubstr("myrtarr -> __array__u32_stride_8\n"));

  p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserTest, NamedTypes_NamedArray) {
  auto p = parser(test::Assemble(R"(
    OpName %arr "myarr"
    OpDecorate %arr ArrayStride 8
    %uint = OpTypeInt 32 0
    %uint_5 = OpConstant %uint 5
    %arr = OpTypeArray %uint %uint_5
    %arr2 = OpTypeArray %uint %uint_5
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->program().to_str(),
              HasSubstr("myarr -> __array__u32_5_stride_8"));

  p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserTest, NamedTypes_AnonArray_Dup_EmitBoth) {
  auto p = parser(test::Assemble(R"(
    OpDecorate %arr ArrayStride 8
    OpDecorate %arr2 ArrayStride 8
    %uint = OpTypeInt 32 0
    %uint_5 = OpConstant %uint 5
    %arr = OpTypeArray %uint %uint_5
    %arr2 = OpTypeArray %uint %uint_5
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->program().to_str(),
              HasSubstr("Arr -> __array__u32_5_stride_8\n  Arr_1 -> "
                        "__array__u32_5_stride_8"));

  p->DeliberatelyInvalidSpirv();
}

// TODO(dneto): Handle arrays sized by a spec constant.
// Blocked by crbug.com/tint/32

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
