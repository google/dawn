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

#include <memory>
#include <sstream>

#include "gmock/gmock.h"
#include "src/reader/spirv/parser_impl.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::Eq;

using SpvParseUserNameTest = ::testing::Test;

TEST_F(SpvParseUserNameTest, RespectOpName) {
  ParserImpl p(test::Assemble(R"(
     OpName %1 "the_void_type"
     %1 = OpTypeVoid
  )"));
  EXPECT_TRUE(p.BuildAndParseInternalModule());
  EXPECT_THAT(p.namer().GetName(1), Eq("the_void_type"));
}

TEST_F(SpvParseUserNameTest, DistinguishDuplicateSuggestion) {
  ParserImpl p(test::Assemble(R"(
     OpName %1 "vanilla"
     OpName %2 "vanilla"
     %1 = OpTypeVoid
     %2 = OpTypeInt 32 0
  )"));
  EXPECT_TRUE(p.BuildAndParseInternalModule());
  EXPECT_THAT(p.namer().GetName(1), Eq("vanilla"));
  EXPECT_THAT(p.namer().GetName(2), Eq("vanilla_1"));
}

TEST_F(SpvParseUserNameTest, RespectOpMemberName) {
  ParserImpl p(test::Assemble(R"(
     OpMemberName %3 0 "strawberry"
     OpMemberName %3 1 "vanilla"
     OpMemberName %3 2 "chocolate"
     %2 = OpTypeInt 32 0
     %3 = OpTypeStruct %2 %2 %2
  )"));
  EXPECT_TRUE(p.BuildAndParseInternalModule());
  EXPECT_THAT(p.namer().GetMemberName(3, 0), Eq("strawberry"));
  EXPECT_THAT(p.namer().GetMemberName(3, 1), Eq("vanilla"));
  EXPECT_THAT(p.namer().GetMemberName(3, 2), Eq("chocolate"));
}

TEST_F(SpvParseUserNameTest, SynthesizeMemberNames) {
  ParserImpl p(test::Assemble(R"(
     %2 = OpTypeInt 32 0
     %3 = OpTypeStruct %2 %2 %2
  )"));
  EXPECT_TRUE(p.BuildAndParseInternalModule());
  EXPECT_THAT(p.namer().GetMemberName(3, 0), Eq("field0"));
  EXPECT_THAT(p.namer().GetMemberName(3, 1), Eq("field1"));
  EXPECT_THAT(p.namer().GetMemberName(3, 2), Eq("field2"));
}

TEST_F(SpvParseUserNameTest, MemberNamesMixUserAndSynthesized) {
  ParserImpl p(test::Assemble(R"(
     OpMemberName %3 1 "vanilla"
     %2 = OpTypeInt 32 0
     %3 = OpTypeStruct %2 %2 %2
  )"));
  EXPECT_TRUE(p.BuildAndParseInternalModule());
  EXPECT_THAT(p.namer().GetMemberName(3, 0), Eq("field0"));
  EXPECT_THAT(p.namer().GetMemberName(3, 1), Eq("vanilla"));
  EXPECT_THAT(p.namer().GetMemberName(3, 2), Eq("field2"));
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
