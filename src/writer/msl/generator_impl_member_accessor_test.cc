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

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/module.h"
#include "src/writer/msl/generator_impl.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = testing::Test;

TEST_F(MslGeneratorImplTest, EmitExpression_MemberAccessor) {
  auto str = std::make_unique<ast::IdentifierExpression>("str");
  auto mem = std::make_unique<ast::IdentifierExpression>("mem");

  ast::MemberAccessorExpression expr(std::move(str), std::move(mem));

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitExpression(&expr)) << g.error();
  EXPECT_EQ(g.result(), "str.mem");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
