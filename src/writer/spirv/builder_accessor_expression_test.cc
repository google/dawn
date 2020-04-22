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
#include "src/ast/array_accessor_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/int_literal.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/variable.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, ArrayAccessor) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  ast::Variable var("ary", ast::StorageClass::kFunction, &vec3);

  auto ary = std::make_unique<ast::IdentifierExpression>("ary");
  auto idx_expr = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1));

  ast::ArrayAccessorExpression expr(std::move(ary), std::move(idx_expr));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 5u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%6 = OpTypeInt 32 1
%7 = OpConstant %6 1
%8 = OpTypePointer Function %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%5 = OpAccessChain %8 %1 %7
)");
}

TEST_F(BuilderTest, ArrayAccessor_MultiLevel) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);
  ast::type::ArrayType ary4(&vec3, 4);

  // ary = array<vec3<f32>, 4>
  // ary[3][2];

  ast::Variable var("ary", ast::StorageClass::kFunction, &ary4);

  ast::ArrayAccessorExpression expr(
      std::make_unique<ast::ArrayAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("ary"),
          std::make_unique<ast::ScalarConstructorExpression>(
              std::make_unique<ast::IntLiteral>(&i32, 3))),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::IntLiteral>(&i32, 2)));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 8u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%6 = OpTypeInt 32 0
%7 = OpConstant %6 4
%3 = OpTypeArray %4 %7
%2 = OpTypePointer Function %3
%9 = OpTypeInt 32 1
%10 = OpConstant %9 2
%11 = OpConstant %9 3
%12 = OpTypePointer Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%8 = OpAccessChain %12 %1 %11 %10
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
