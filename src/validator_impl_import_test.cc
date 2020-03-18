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

#include <iostream>

#include "gtest/gtest.h"
#include "src/ast/import.h"
#include "src/ast/module.h"
#include "src/validator_impl.h"

namespace tint {
namespace {

using ValidatorImplTest = testing::Test;

TEST_F(ValidatorImplTest, Import) {
  ast::Module m;
  m.AddImport(std::make_unique<ast::Import>("GLSL.std.450", "glsl"));

  tint::ValidatorImpl v;
  EXPECT_TRUE(v.CheckImports(m));
}

TEST_F(ValidatorImplTest, Import_Fail_NotGLSL) {
  ast::Module m;
  m.AddImport(std::make_unique<ast::Import>(Source{1, 1}, "not.GLSL", "glsl"));

  tint::ValidatorImpl v;
  EXPECT_FALSE(v.CheckImports(m));
  ASSERT_TRUE(v.has_error());
  EXPECT_EQ(v.error(), "1:1: v-0001: unknown import: not.GLSL");
}

TEST_F(ValidatorImplTest, Import_Fail_Typo) {
  ast::Module m;
  m.AddImport(
      std::make_unique<ast::Import>(Source{1, 1}, "GLSL.std.4501", "glsl"));

  tint::ValidatorImpl v;
  EXPECT_FALSE(v.CheckImports(m));
  ASSERT_TRUE(v.has_error());
  EXPECT_EQ(v.error(), "1:1: v-0001: unknown import: GLSL.std.4501");
}

}  // namespace
}  // namespace tint
