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
#include "src/ast/as_expression.h"
#include "src/ast/float_literal.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/u32_type.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, As) {
  ast::type::U32Type u32;
  ast::type::F32Type f32;

  ast::AsExpression as(&u32,
                       std::make_unique<ast::ScalarConstructorExpression>(
                           std::make_unique<ast::FloatLiteral>(&f32, 2.4)));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&as)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateAsExpression(&as), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 0
%3 = OpTypeFloat 32
%4 = OpConstant %3 2.4000001
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpBitcast %2 %4
)");
}

TEST_F(BuilderTest, As_DuplicateType) {
  ast::type::F32Type f32;

  ast::AsExpression as(&f32,
                       std::make_unique<ast::ScalarConstructorExpression>(
                           std::make_unique<ast::FloatLiteral>(&f32, 2.4)));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&as)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateAsExpression(&as), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%3 = OpConstant %2 2.4000001
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpCopyObject %2 %3
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
