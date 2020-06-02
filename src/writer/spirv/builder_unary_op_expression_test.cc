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
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/unary_op_expression.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, UnaryOp_Negation_Integer) {
  ast::type::I32Type i32;

  ast::UnaryOpExpression expr(
      ast::UnaryOp::kNegation,
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::SintLiteral>(&i32, 1)));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateUnaryOpExpression(&expr), 1u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpConstant %2 1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpSNegate %2 %3
)");
}

TEST_F(BuilderTest, UnaryOp_Negation_Float) {
  ast::type::F32Type f32;

  ast::UnaryOpExpression expr(
      ast::UnaryOp::kNegation,
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::FloatLiteral>(&f32, 1)));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateUnaryOpExpression(&expr), 1u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%3 = OpConstant %2 1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpFNegate %2 %3
)");
}

TEST_F(BuilderTest, UnaryOp_Not) {
  ast::type::I32Type i32;

  ast::UnaryOpExpression expr(
      ast::UnaryOp::kNot, std::make_unique<ast::ScalarConstructorExpression>(
                              std::make_unique<ast::SintLiteral>(&i32, 1)));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateUnaryOpExpression(&expr), 1u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpConstant %2 1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpNot %2 %3
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
