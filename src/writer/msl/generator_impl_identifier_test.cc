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
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, EmitIdentifierExpression) {
  ast::IdentifierExpression i(mod.RegisterSymbol("foo"), "foo");

  ASSERT_TRUE(gen.EmitExpression(&i)) << gen.error();
  EXPECT_EQ(gen.result(), "foo");
}

TEST_F(MslGeneratorImplTest, EmitIdentifierExpression_Single_WithCollision) {
  ast::IdentifierExpression i(mod.RegisterSymbol("virtual"), "virtual");

  ASSERT_TRUE(gen.EmitExpression(&i)) << gen.error();
  EXPECT_EQ(gen.result(), "virtual_tint_0");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
