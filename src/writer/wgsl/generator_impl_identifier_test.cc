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

#include "gtest/gtest.h"
#include "src/writer/wgsl/generator_impl.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = testing::Test;

TEST_F(WgslGeneratorImplTest, EmitExpression_Identifier) {
  ast::IdentifierExpression i(std::vector<std::string>{"std", "glsl"});

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitExpression(&i)) << g.error();
  EXPECT_EQ(g.result(), "std::glsl");
}

TEST_F(WgslGeneratorImplTest, EmitIdentifierExpression_Single) {
  ast::IdentifierExpression i("glsl");

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitExpression(&i)) << g.error();
  EXPECT_EQ(g.result(), "glsl");
}

TEST_F(WgslGeneratorImplTest, EmitIdentifierExpression_MultipleNames) {
  ast::IdentifierExpression i({"std", "glsl", "init"});

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitExpression(&i)) << g.error();
  EXPECT_EQ(g.result(), "std::glsl::init");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
