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
#include "src/ast/binary_expression.h"
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/void_type.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, Call_GLSLMethod) {
  ast::type::F32Type f32;
  ast::type::VoidType void_type;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(
                               std::vector<std::string>{"std", "round"}),
                           std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  auto imp = std::make_unique<ast::Import>("GLSL.std.450", "std");
  auto* glsl = imp.get();
  mod.AddImport(std::move(imp));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  b.GenerateImport(glsl);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 7u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%1 = OpExtInstImport "GLSL.std.450"
OpName %4 "a_func"
%3 = OpTypeVoid
%2 = OpTypeFunction %3
%6 = OpTypeFloat 32
%8 = OpConstant %6 1
%4 = OpFunction %3 None %2
%5 = OpLabel
%7 = OpExtInst %6 %1 Round %8
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Call_GLSLMethod_WithLoad) {
  ast::type::F32Type f32;
  ast::type::VoidType void_type;

  auto var = std::make_unique<ast::Variable>("ident",
                                             ast::StorageClass::kPrivate, &f32);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("ident"));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(
                               std::vector<std::string>{"std", "round"}),
                           std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  auto imp = std::make_unique<ast::Import>("GLSL.std.450", "std");
  auto* glsl = imp.get();
  mod.AddImport(std::move(imp));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  b.GenerateImport(glsl);
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 10u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%1 = OpExtInstImport "GLSL.std.450"
OpName %2 "ident"
OpName %8 "a_func"
%4 = OpTypeFloat 32
%3 = OpTypePointer Private %4
%5 = OpConstantNull %4
%2 = OpVariable %3 Private %5
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%8 = OpFunction %7 None %6
%9 = OpLabel
%11 = OpLoad %4 %2
%10 = OpExtInst %4 %1 Round %11
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Expression_Call) {
  ast::type::F32Type f32;
  ast::type::VoidType void_type;

  ast::VariableList func_params;
  func_params.push_back(
      std::make_unique<ast::Variable>("a", ast::StorageClass::kFunction, &f32));
  func_params.push_back(
      std::make_unique<ast::Variable>("b", ast::StorageClass::kFunction, &f32));

  ast::Function a_func("a_func", std::move(func_params), &f32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>(
      std::make_unique<ast::BinaryExpression>(
          ast::BinaryOp::kAdd, std::make_unique<ast::IdentifierExpression>("a"),
          std::make_unique<ast::IdentifierExpression>("b"))));
  a_func.set_body(std::move(body));

  ast::Function func("main", {}, &void_type);

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("a_func"),
      std::move(call_params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineFunction(&func)) << td.error();
  ASSERT_TRUE(td.DetermineFunction(&a_func)) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&a_func)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 14u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(OpName %3 "a_func"
OpName %4 "a"
OpName %5 "b"
OpName %12 "main"
%2 = OpTypeFloat 32
%1 = OpTypeFunction %2 %2 %2
%11 = OpTypeVoid
%10 = OpTypeFunction %11
%15 = OpConstant %2 1
%3 = OpFunction %2 None %1
%4 = OpFunctionParameter %2
%5 = OpFunctionParameter %2
%6 = OpLabel
%7 = OpLoad %2 %4
%8 = OpLoad %2 %5
%9 = OpFAdd %2 %7 %8
OpReturnValue %9
OpFunctionEnd
%12 = OpFunction %11 None %10
%13 = OpLabel
%14 = OpFunctionCall %2 %3 %15 %15
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Statement_Call) {
  ast::type::F32Type f32;
  ast::type::VoidType void_type;

  ast::VariableList func_params;
  func_params.push_back(
      std::make_unique<ast::Variable>("a", ast::StorageClass::kFunction, &f32));
  func_params.push_back(
      std::make_unique<ast::Variable>("b", ast::StorageClass::kFunction, &f32));

  ast::Function a_func("a_func", std::move(func_params), &void_type);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>(
      std::make_unique<ast::BinaryExpression>(
          ast::BinaryOp::kAdd, std::make_unique<ast::IdentifierExpression>("a"),
          std::make_unique<ast::IdentifierExpression>("b"))));
  a_func.set_body(std::move(body));

  ast::Function func("main", {}, &void_type);

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::CallStatement expr(std::make_unique<ast::CallExpression>(
      std::make_unique<ast::IdentifierExpression>("a_func"),
      std::move(call_params)));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineFunction(&func)) << td.error();
  ASSERT_TRUE(td.DetermineFunction(&a_func)) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&a_func)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_TRUE(b.GenerateStatement(&expr)) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(OpName %4 "a_func"
OpName %5 "a"
OpName %6 "b"
OpName %12 "main"
%2 = OpTypeVoid
%3 = OpTypeFloat 32
%1 = OpTypeFunction %2 %3 %3
%11 = OpTypeFunction %2
%15 = OpConstant %3 1
%4 = OpFunction %2 None %1
%5 = OpFunctionParameter %3
%6 = OpFunctionParameter %3
%7 = OpLabel
%8 = OpLoad %3 %5
%9 = OpLoad %3 %6
%10 = OpFAdd %3 %8 %9
OpReturnValue %10
OpFunctionEnd
%12 = OpFunction %2 None %11
%13 = OpLabel
%14 = OpFunctionCall %2 %4 %15 %15
OpFunctionEnd
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
