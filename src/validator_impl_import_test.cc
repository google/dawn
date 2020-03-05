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

#include <assert.h>

#include <iostream>

#include "gtest/gtest.h"
#include "src/reader/wgsl/parser.h"
#include "src/validator_impl.h"

namespace tint {
namespace {

ast::Module build_module(std::string data) {
  auto reader = std::make_unique<tint::reader::wgsl::Parser>(
      std::string(data.begin(), data.end()));
  assert(reader->Parse());
  return reader->module();
}

}  // namespace

using ValidatorImplTest = testing::Test;

TEST_F(ValidatorImplTest, Import) {
  std::string input = "import \"GLSL.std.450\" as glsl;";
  auto module = build_module(input);
  tint::ValidatorImpl v;
  EXPECT_TRUE(v.CheckImports(module));
}

TEST_F(ValidatorImplTest, Import_Fail_NotGLSL) {
  std::string input = "import \"not.GLSL\" as glsl;";
  auto module = build_module(input);
  tint::ValidatorImpl v;
  EXPECT_FALSE(v.CheckImports(module));
  ASSERT_TRUE(v.has_error());
  EXPECT_EQ(v.error(), "1:1: v-0001: unknown import: not.GLSL");
}

TEST_F(ValidatorImplTest, Import_Fail_Typo) {
  std::string input = "import \"GLSL.std.4501\" as glsl;";
  auto module = build_module(input);
  tint::ValidatorImpl v;
  EXPECT_FALSE(v.CheckImports(module));
  ASSERT_TRUE(v.has_error());
  EXPECT_EQ(v.error(), "1:1: v-0001: unknown import: GLSL.std.4501");
}

}  // namespace tint
