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

#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Identifier = TestHelper;

TEST_F(HlslGeneratorImplTest_Identifier, DISABLED_EmitExpression_Identifier) {
  ast::IdentifierExpression i(std::vector<std::string>{"std", "glsl"});
  ASSERT_TRUE(gen().EmitExpression(out(), &i)) << gen().error();
  EXPECT_EQ(result(), "std::glsl");
}

TEST_F(HlslGeneratorImplTest_Identifier, EmitIdentifierExpression_Single) {
  ast::IdentifierExpression i("foo");
  ASSERT_TRUE(gen().EmitExpression(out(), &i)) << gen().error();
  EXPECT_EQ(result(), "foo");
}

TEST_F(HlslGeneratorImplTest_Identifier,
       EmitIdentifierExpression_Single_WithCollision) {
  ast::IdentifierExpression i("virtual");
  ASSERT_TRUE(gen().EmitExpression(out(), &i)) << gen().error();
  EXPECT_EQ(result(), "virtual_tint_0");
}

// TODO(dsinclair): Handle import names
TEST_F(HlslGeneratorImplTest_Identifier,
       DISABLED_EmitIdentifierExpression_MultipleNames) {
  ast::IdentifierExpression i({"std", "glsl", "init"});
  ASSERT_TRUE(gen().EmitExpression(out(), &i)) << gen().error();
  EXPECT_EQ(result(), "std::glsl::init");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
