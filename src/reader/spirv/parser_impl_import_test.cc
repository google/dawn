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
#include "src/reader/spirv/parser_impl_test_helper.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::Not;
using ::testing::UnorderedElementsAre;

TEST_F(SpvParserTest, Import_NoImport) {
  auto* p = parser(test::Assemble("%1 = OpTypeVoid"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_ast = p->module().to_str();
  EXPECT_THAT(module_ast, Not(HasSubstr("Import")));
}

TEST_F(SpvParserTest, Import_ImportGlslStd450) {
  auto* p = parser(test::Assemble(R"(%1 = OpExtInstImport "GLSL.std.450")"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  EXPECT_THAT(p->glsl_std_450_imports(), ElementsAre(1));
  const auto module_ast = p->module().to_str();
  EXPECT_THAT(module_ast, HasSubstr(R"(Import{"GLSL.std.450" as std::glsl})"));
}

TEST_F(SpvParserTest, Import_ImportGlslStd450Twice) {
  auto* p = parser(test::Assemble(R"(
    %1  = OpExtInstImport "GLSL.std.450"
    %42 = OpExtInstImport "GLSL.std.450"
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  EXPECT_THAT(p->glsl_std_450_imports(), UnorderedElementsAre(1, 42));
  const auto module = p->module();
  EXPECT_EQ(module.imports().size(), 1u);
  const auto module_ast = module.to_str();
  // TODO(dneto): Use a matcher to show there is only one import.
  EXPECT_THAT(module_ast, HasSubstr(R"(Import{"GLSL.std.450" as std::glsl})"));
}

// TODO(dneto): We don't currently support other kinds of extended instruction
// imports.

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
