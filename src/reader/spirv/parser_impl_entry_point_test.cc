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

#include <string>

#include "gmock/gmock.h"
#include "src/reader/spirv/parser_impl.h"
#include "src/reader/spirv/parser_impl_test_helper.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::HasSubstr;

std::string MakeEntryPoint(const std::string& stage,
                           const std::string& name,
                           const std::string& id = "42") {
  return std::string("OpEntryPoint ") + stage + " %" + id + " \"" + name +
         "\"\n" +  // Give the target ID a definition.
         "%" + id + " = OpTypeVoid\n";
}

TEST_F(SpvParserTest, EntryPoint_NoEntryPoint) {
  auto* p = parser(test::Assemble(""));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_ast = p->module().to_str();
  EXPECT_THAT(module_ast, Not(HasSubstr("EntryPoint")));
}

TEST_F(SpvParserTest, EntryPoint_Vertex) {
  auto* p = parser(test::Assemble(MakeEntryPoint("Vertex", "foobar")));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(EntryPoint{vertex = foobar})"));
}

TEST_F(SpvParserTest, EntryPoint_Fragment) {
  auto* p = parser(test::Assemble(MakeEntryPoint("Fragment", "blitz")));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(EntryPoint{fragment = blitz})"));
}

TEST_F(SpvParserTest, EntryPoint_Compute) {
  auto* p = parser(test::Assemble(MakeEntryPoint("GLCompute", "sort")));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(EntryPoint{compute = sort})"));
}

TEST_F(SpvParserTest, EntryPoint_MultiNameConflict) {
  auto* p = parser(test::Assemble(MakeEntryPoint("GLCompute", "work", "40") +
                                  MakeEntryPoint("Vertex", "work", "50") +
                                  MakeEntryPoint("Fragment", "work", "60")));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(EntryPoint{compute = work})"));
  EXPECT_THAT(module_str, HasSubstr(R"(EntryPoint{vertex = work_1})"));
  EXPECT_THAT(module_str, HasSubstr(R"(EntryPoint{fragment = work_2})"));
}

TEST_F(SpvParserTest, EntryPoint_NameIsSanitized) {
  auto* p = parser(test::Assemble(MakeEntryPoint("GLCompute", ".1234")));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(EntryPoint{compute = x_1234})"));
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
