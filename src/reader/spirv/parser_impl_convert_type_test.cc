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
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::Eq;

using SpvParserTest_ConvertType = ::testing::Test;

TEST_F(SpvParserTest_ConvertType, PreservesExistingFailure) {
  ParserImpl p(std::vector<uint32_t>{});
  p.Fail() << "boing";
  const auto* type = p.ConvertType(10);
  EXPECT_EQ(type, nullptr);
  EXPECT_THAT(p.error(), Eq("boing"));
}

TEST_F(SpvParserTest_ConvertType, NotAnId) {
  ParserImpl p(test::Assemble("%1 = OpExtInstImport \"GLSL.std.450\""));
  EXPECT_TRUE(p.BuildAndParseInternalModule()) << p.error();

  const auto* type = p.ConvertType(10);
  EXPECT_EQ(type, nullptr);
  EXPECT_EQ(nullptr, type);
  EXPECT_THAT(p.error(), Eq("ID is not a SPIR-V type: 10"));
}

TEST_F(SpvParserTest_ConvertType, IdExistsButIsNotAType) {
  ParserImpl p(test::Assemble("%1 = OpExtInstImport \"GLSL.std.450\""));
  EXPECT_TRUE(p.BuildAndParseInternalModule());

  const auto* type = p.ConvertType(1);
  EXPECT_EQ(nullptr, type);
  EXPECT_THAT(p.error(), Eq("ID is not a SPIR-V type: 1"));
}

TEST_F(SpvParserTest_ConvertType, UnhandledType) {
  // Pipes are an OpenCL type. Tint doesn't support them.
  ParserImpl p(test::Assemble("%70 = OpTypePipe WriteOnly"));
  EXPECT_TRUE(p.BuildAndParseInternalModule());

  const auto* type = p.ConvertType(70);
  EXPECT_EQ(nullptr, type);
  EXPECT_THAT(p.error(), Eq("unknown SPIR-V type: 70"));
}

TEST_F(SpvParserTest_ConvertType, Void) {
  ParserImpl p(test::Assemble("%1 = OpTypeVoid"));
  EXPECT_TRUE(p.BuildAndParseInternalModule());

  const auto* type = p.ConvertType(1);
  EXPECT_TRUE(type->IsVoid());
  EXPECT_TRUE(p.error().empty());
}

TEST_F(SpvParserTest_ConvertType, Bool) {
  ParserImpl p(test::Assemble("%100 = OpTypeBool"));
  EXPECT_TRUE(p.BuildAndParseInternalModule());

  const auto* type = p.ConvertType(100);
  EXPECT_TRUE(type->IsBool());
  EXPECT_TRUE(p.error().empty());
}

TEST_F(SpvParserTest_ConvertType, I32) {
  ParserImpl p(test::Assemble("%2 = OpTypeInt 32 1"));
  EXPECT_TRUE(p.BuildAndParseInternalModule());

  const auto* type = p.ConvertType(2);
  EXPECT_TRUE(type->IsI32());
  EXPECT_TRUE(p.error().empty());
}

TEST_F(SpvParserTest_ConvertType, U32) {
  ParserImpl p(test::Assemble("%3 = OpTypeInt 32 0"));
  EXPECT_TRUE(p.BuildAndParseInternalModule());

  const auto* type = p.ConvertType(3);
  EXPECT_TRUE(type->IsU32());
  EXPECT_TRUE(p.error().empty());
}

TEST_F(SpvParserTest_ConvertType, F32) {
  ParserImpl p(test::Assemble("%4 = OpTypeFloat 32"));
  EXPECT_TRUE(p.BuildAndParseInternalModule());

  const auto* type = p.ConvertType(4);
  EXPECT_TRUE(type->IsF32());
  EXPECT_TRUE(p.error().empty());
}

TEST_F(SpvParserTest_ConvertType, BadIntWidth) {
  ParserImpl p(test::Assemble("%5 = OpTypeInt 17 1"));
  EXPECT_TRUE(p.BuildAndParseInternalModule());

  const auto* type = p.ConvertType(5);
  EXPECT_EQ(type, nullptr);
  EXPECT_THAT(p.error(), Eq("unhandled integer width: 17"));
}

TEST_F(SpvParserTest_ConvertType, BadFloatWidth) {
  ParserImpl p(test::Assemble("%6 = OpTypeFloat 19"));
  EXPECT_TRUE(p.BuildAndParseInternalModule());

  const auto* type = p.ConvertType(6);
  EXPECT_EQ(type, nullptr);
  EXPECT_THAT(p.error(), Eq("unhandled float width: 19"));
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
