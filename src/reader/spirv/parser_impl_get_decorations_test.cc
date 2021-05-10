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

using ::testing::Eq;
using ::testing::UnorderedElementsAre;

using SpvParserGetDecorationsTest = SpvParserTest;

TEST_F(SpvParserGetDecorationsTest, GetDecorationsFor_NotAnId) {
  auto p = parser(test::Assemble(""));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  auto decorations = p->GetDecorationsFor(42);
  EXPECT_TRUE(decorations.empty());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserGetDecorationsTest, GetDecorationsFor_NoDecorations) {
  auto p = parser(test::Assemble("%1 = OpTypeVoid"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  auto decorations = p->GetDecorationsFor(1);
  EXPECT_TRUE(decorations.empty());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserGetDecorationsTest, GetDecorationsFor_OneDecoration) {
  auto p = parser(test::Assemble(R"(
    OpDecorate %10 Block
    %float = OpTypeFloat 32
    %10 = OpTypeStruct %float
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  auto decorations = p->GetDecorationsFor(10);
  EXPECT_THAT(decorations,
              UnorderedElementsAre(Decoration{SpvDecorationBlock}));
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserGetDecorationsTest, GetDecorationsFor_MultiDecoration) {
  auto p = parser(test::Assemble(R"(
    OpDecorate %5 RelaxedPrecision
    OpDecorate %5 Location 7      ; Invalid case made up for test
    %float = OpTypeFloat 32
    %5 = OpConstant %float 3.14
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  auto decorations = p->GetDecorationsFor(5);
  EXPECT_THAT(decorations,
              UnorderedElementsAre(Decoration{SpvDecorationRelaxedPrecision},
                                   Decoration{SpvDecorationLocation, 7}));
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserGetDecorationsTest, GetDecorationsForMember_NotAnId) {
  auto p = parser(test::Assemble(""));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  auto decorations = p->GetDecorationsForMember(42, 9);
  EXPECT_TRUE(decorations.empty());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserGetDecorationsTest, GetDecorationsForMember_NotAStruct) {
  auto p = parser(test::Assemble("%1 = OpTypeVoid"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  auto decorations = p->GetDecorationsFor(1);
  EXPECT_TRUE(decorations.empty());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserGetDecorationsTest,
       GetDecorationsForMember_MemberWithoutDecoration) {
  auto p = parser(test::Assemble(R"(
    %uint = OpTypeInt 32 0
    %10 = OpTypeStruct %uint
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  auto decorations = p->GetDecorationsForMember(10, 0);
  EXPECT_TRUE(decorations.empty());
  EXPECT_TRUE(p->error().empty());
}

// TODO(dneto): Enable when ArrayStride is handled
TEST_F(SpvParserGetDecorationsTest,
       DISABLED_GetDecorationsForMember_OneDecoration) {
  auto p = parser(test::Assemble(R"(
    OpMemberDecorate %10 1 ArrayStride 12
    %uint = OpTypeInt 32 0
    %uint_2 = OpConstant %uint 2
    %arr = OpTypeArray %uint %uint_2
    %10 = OpTypeStruct %uint %arr
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule()) << p->error();
  auto decorations = p->GetDecorationsForMember(10, 1);
  EXPECT_THAT(decorations,
              UnorderedElementsAre(Decoration{SpvDecorationArrayStride, 12}));
  EXPECT_TRUE(p->error().empty());
}

// TODO(dneto): Enable when ArrayStride, MatrixStride, ColMajor are handled
// crbug.com/tint/30 for ArrayStride
// crbug.com/tint/31 for matrix layout
TEST_F(SpvParserGetDecorationsTest,
       DISABLED_GetDecorationsForMember_MultiDecoration) {
  auto p = parser(test::Assemble(R"(
    OpMemberDecorate %50 1 RelaxedPrecision
    OpMemberDecorate %50 2 ArrayStride 16
    OpMemberDecorate %50 2 MatrixStride 8
    OpMemberDecorate %50 2 ColMajor
    %float = OpTypeFloat 32
    %vec = OpTypeVector %float 2
    %mat = OpTypeMatrix %vec 2
    %uint = OpTypeInt 32 0
    %uint_2 = OpConstant %uint 2
    %arr = OpTypeArray %mat %uint_2
    %50 = OpTypeStruct %uint %float %arr
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule()) << p->error();

  EXPECT_TRUE(p->GetDecorationsForMember(50, 0).empty());
  EXPECT_THAT(p->GetDecorationsForMember(50, 1),
              UnorderedElementsAre(Decoration{SpvDecorationRelaxedPrecision}));
  EXPECT_THAT(p->GetDecorationsForMember(50, 2),
              UnorderedElementsAre(Decoration{SpvDecorationColMajor},
                                   Decoration{SpvDecorationMatrixStride, 8},
                                   Decoration{SpvDecorationArrayStride, 16}));
  EXPECT_TRUE(p->error().empty());
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
