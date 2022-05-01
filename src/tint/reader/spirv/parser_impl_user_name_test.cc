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

using ::testing::Eq;

using SpvParserUserNameTest = SpvParserTest;

TEST_F(SpvParserUserNameTest, UserName_RespectOpName) {
    auto p = parser(test::Assemble(R"(
     OpName %1 "the_void_type"
     %1 = OpTypeVoid
  )"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_THAT(p->namer().GetName(1), Eq("the_void_type"));

    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserUserNameTest, UserName_IgnoreEmptyName) {
    auto p = parser(test::Assemble(R"(
     OpName %1 ""
     %1 = OpTypeVoid
  )"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_FALSE(p->namer().HasName(1));

    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserUserNameTest, UserName_DistinguishDuplicateSuggestion) {
    auto p = parser(test::Assemble(R"(
     OpName %1 "vanilla"
     OpName %2 "vanilla"
     %1 = OpTypeVoid
     %2 = OpTypeInt 32 0
  )"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_THAT(p->namer().GetName(1), Eq("vanilla"));
    EXPECT_THAT(p->namer().GetName(2), Eq("vanilla_1"));

    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserUserNameTest, UserName_RespectOpMemberName) {
    auto p = parser(test::Assemble(R"(
     OpMemberName %3 0 "strawberry"
     OpMemberName %3 1 "vanilla"
     OpMemberName %3 2 "chocolate"
     %2 = OpTypeInt 32 0
     %3 = OpTypeStruct %2 %2 %2
  )"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_THAT(p->namer().GetMemberName(3, 0), Eq("strawberry"));
    EXPECT_THAT(p->namer().GetMemberName(3, 1), Eq("vanilla"));
    EXPECT_THAT(p->namer().GetMemberName(3, 2), Eq("chocolate"));

    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserUserNameTest, UserName_IgnoreEmptyMemberName) {
    auto p = parser(test::Assemble(R"(
     OpMemberName %3 0 ""
     %2 = OpTypeInt 32 0
     %3 = OpTypeStruct %2
  )"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_THAT(p->namer().GetMemberName(3, 0), Eq("field0"));

    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserUserNameTest, UserName_SynthesizeMemberNames) {
    auto p = parser(test::Assemble(R"(
     %2 = OpTypeInt 32 0
     %3 = OpTypeStruct %2 %2 %2
  )"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_THAT(p->namer().GetMemberName(3, 0), Eq("field0"));
    EXPECT_THAT(p->namer().GetMemberName(3, 1), Eq("field1"));
    EXPECT_THAT(p->namer().GetMemberName(3, 2), Eq("field2"));

    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserUserNameTest, UserName_MemberNamesMixUserAndSynthesized) {
    auto p = parser(test::Assemble(R"(
     OpMemberName %3 1 "vanilla"
     %2 = OpTypeInt 32 0
     %3 = OpTypeStruct %2 %2 %2
  )"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_THAT(p->namer().GetMemberName(3, 0), Eq("field0"));
    EXPECT_THAT(p->namer().GetMemberName(3, 1), Eq("vanilla"));
    EXPECT_THAT(p->namer().GetMemberName(3, 2), Eq("field2"));

    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserUserNameTest, EntryPointNames_AlwaysTakePrecedence) {
    const std::string assembly = R"(
   OpCapability Shader
   OpMemoryModel Logical Simple
   OpEntryPoint Vertex %100 "main"
   OpEntryPoint Fragment %100 "main_1"
   OpExecutionMode %100 OriginUpperLeft

   ; attempt to grab the "main_1" that would be the derived name
   ; for the second entry point.
   OpName %1 "main_1"

   %void = OpTypeVoid
   %voidfn = OpTypeFunction %void
   %uint = OpTypeInt 32 0
   %uint_0 = OpConstant %uint 0

   %100 = OpFunction %void None %voidfn
   %100_entry = OpLabel
   %1 = OpCopyObject %uint %uint_0
   OpReturn
   OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    // The first entry point grabs the best name, "main"
    EXPECT_THAT(p->namer().Name(100), Eq("main"));
    // The OpName on %1 is overriden because the second entry point
    // has grabbed "main_1" first.
    EXPECT_THAT(p->namer().Name(1), Eq("main_1_1"));

    const auto& ep_info = p->GetEntryPointInfo(100);
    ASSERT_EQ(2u, ep_info.size());
    EXPECT_EQ(ep_info[0].name, "main");
    EXPECT_EQ(ep_info[1].name, "main_1");

    // This test checks two entry point with the same implementation function.
    // But for the shader stages supported by WGSL, the SPIR-V rules require
    // conflicting execution modes be applied to them.
    // I still want to test the name disambiguation behaviour, but the cases
    // are rejected by SPIR-V validation. This is true at least for the current
    // WGSL feature set.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserUserNameTest, EntryPointNames_DistinctFromInnerNames) {
    const std::string assembly = R"(
   OpCapability Shader
   OpMemoryModel Logical Simple
   OpEntryPoint Vertex %100 "main"
   OpEntryPoint Fragment %100 "main_1"
   OpExecutionMode %100 OriginUpperLeft

   ; attempt to grab the "main_1" that would be the derived name
   ; for the second entry point.
   OpName %1 "main_1"

   %void = OpTypeVoid
   %voidfn = OpTypeFunction %void
   %uint = OpTypeInt 32 0
   %uint_0 = OpConstant %uint 0

   %100 = OpFunction %void None %voidfn
   %100_entry = OpLabel
   %1 = OpCopyObject %uint %uint_0
   OpReturn
   OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));

    EXPECT_TRUE(p->BuildAndParseInternalModule());
    // The first entry point grabs the best name, "main"
    EXPECT_THAT(p->namer().Name(100), Eq("main"));
    EXPECT_THAT(p->namer().Name(1), Eq("main_1_1"));

    const auto ep_info = p->GetEntryPointInfo(100);
    ASSERT_EQ(2u, ep_info.size());
    EXPECT_EQ(ep_info[0].name, "main");
    EXPECT_EQ(ep_info[0].inner_name, "main_2");
    // The second entry point retains its name...
    EXPECT_EQ(ep_info[1].name, "main_1");
    // ...but will use the same implementation function.
    EXPECT_EQ(ep_info[1].inner_name, "main_2");

    // This test checks two entry point with the same implementation function.
    // But for the shader stages supported by WGSL, the SPIR-V rules require
    // conflicting execution modes be applied to them.
    // I still want to test the name disambiguation behaviour, but the cases
    // are rejected by SPIR-V validation. This is true at least for the current
    // WGSL feature set.
    p->DeliberatelyInvalidSpirv();
}

}  // namespace
}  // namespace tint::reader::spirv
