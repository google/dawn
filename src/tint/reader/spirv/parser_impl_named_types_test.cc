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
#include "src/tint/reader/spirv/parser_impl_test_helper.h"
#include "src/tint/reader/spirv/spirv_tools_helpers_test.h"

namespace tint::reader::spirv {
namespace {

using ::testing::HasSubstr;

TEST_F(SpvParserTest, NamedTypes_AnonStruct) {
    auto p = parser(test::Assemble(R"(
    %uint = OpTypeInt 32 0
    %s = OpTypeStruct %uint %uint
  )"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_THAT(test::ToString(p->program()), HasSubstr("struct S"));

    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserTest, NamedTypes_NamedStruct) {
    auto p = parser(test::Assemble(R"(
    OpName %s "mystruct"
    %uint = OpTypeInt 32 0
    %s = OpTypeStruct %uint %uint
  )"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_THAT(test::ToString(p->program()), HasSubstr("struct mystruct"));

    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserTest, NamedTypes_Dup_EmitBoth) {
    auto p = parser(test::Assemble(R"(
    %uint = OpTypeInt 32 0
    %s = OpTypeStruct %uint %uint
    %s2 = OpTypeStruct %uint %uint
  )"));
    EXPECT_TRUE(p->BuildAndParseInternalModule()) << p->error();
    EXPECT_THAT(test::ToString(p->program()), HasSubstr(R"(struct S {
  field0 : u32,
  field1 : u32,
}

struct S_1 {
  field0 : u32,
  field1 : u32,
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
    EXPECT_THAT(test::ToString(p->program()), HasSubstr("RTArr = @stride(8) array<u32>;\n"));

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
    EXPECT_THAT(test::ToString(p->program()), HasSubstr(R"(alias RTArr = @stride(8) array<u32>;

alias RTArr_1 = @stride(8) array<u32>;
)"));

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
    EXPECT_THAT(test::ToString(p->program()), HasSubstr("myrtarr = @stride(8) array<u32>;\n"));

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
    EXPECT_THAT(test::ToString(p->program()), HasSubstr("myarr = @stride(8) array<u32, 5u>;"));

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
    EXPECT_THAT(test::ToString(p->program()), HasSubstr(R"(alias Arr = @stride(8) array<u32, 5u>;

alias Arr_1 = @stride(8) array<u32, 5u>;
)"));

    p->DeliberatelyInvalidSpirv();
}

// TODO(dneto): Handle arrays sized by a spec constant.
// Blocked by crbug.com/tint/32

}  // namespace
}  // namespace tint::reader::spirv
