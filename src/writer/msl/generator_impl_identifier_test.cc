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
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/writer/msl/generator_impl.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = testing::Test;

TEST_F(MslGeneratorImplTest, DISABLED_EmitExpression_Identifier) {
  ast::IdentifierExpression i(std::vector<std::string>{"std", "glsl"});

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitExpression(&i)) << g.error();
  EXPECT_EQ(g.result(), "std::glsl");
}

TEST_F(MslGeneratorImplTest, EmitIdentifierExpression_Single) {
  ast::IdentifierExpression i("foo");

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitExpression(&i)) << g.error();
  EXPECT_EQ(g.result(), "foo");
}

TEST_F(MslGeneratorImplTest, EmitIdentifierExpression_Single_WithCollision) {
  ast::IdentifierExpression i("virtual");

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitExpression(&i)) << g.error();
  EXPECT_EQ(g.result(), "virtual_tint_0");
}

// TODO(dsinclair): Handle import names
TEST_F(MslGeneratorImplTest, DISABLED_EmitIdentifierExpression_MultipleNames) {
  ast::IdentifierExpression i({"std", "glsl", "init"});

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitExpression(&i)) << g.error();
  EXPECT_EQ(g.result(), "std::glsl::init");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
