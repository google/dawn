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
#include "src/ast/cast_expression.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/int_literal.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, DISABLED_Cast_FloatToU32) {}

TEST_F(BuilderTest, DISABLED_Cast_FloatToI32) {}

TEST_F(BuilderTest, Cast_I32ToFloat) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::CastExpression cast(&f32,
                           std::make_unique<ast::ScalarConstructorExpression>(
                               std::make_unique<ast::IntLiteral>(&i32, 2)));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateCastExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%3 = OpTypeInt 32 1
%4 = OpConstant %3 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpConvertSToF %2 %4
)");
}

TEST_F(BuilderTest, DISABLED_Cast_U32ToFloat) {}

TEST_F(BuilderTest, Cast_WithLoad) {
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  // var i : i32 = 1;
  // cast<f32>(i);
  auto var =
      std::make_unique<ast::Variable>("i", ast::StorageClass::kPrivate, &i32);

  ast::CastExpression cast(&f32,
                           std::make_unique<ast::IdentifierExpression>("i"));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
  EXPECT_EQ(b.GenerateCastExpression(&cast), 4u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%5 = OpTypeFloat 32
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpLoad %3 %1
%4 = OpConvertSToF %5 %6
)");
}

TEST_F(BuilderTest, DISABLED_Cast_WithAlias) {}

// TODO(dsinclair): Are here i32 -> u32 and u32->i32 casts?

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
