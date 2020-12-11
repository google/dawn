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
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, Expression_Call) {
  ast::type::F32 f32;
  ast::type::Void void_type;

  ast::VariableList func_params;
  func_params.push_back(
      create<ast::Variable>(Source{},                         // source
                            "a",                              // name
                            ast::StorageClass::kFunction,     // storage_class
                            &f32,                             // type
                            false,                            // is_const
                            nullptr,                          // constructor
                            ast::VariableDecorationList{}));  // decorations
  func_params.push_back(
      create<ast::Variable>(Source{},                         // source
                            "b",                              // name
                            ast::StorageClass::kFunction,     // storage_class
                            &f32,                             // type
                            false,                            // is_const
                            nullptr,                          // constructor
                            ast::VariableDecorationList{}));  // decorations

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::ReturnStatement>(
      Source{}, create<ast::BinaryExpression>(
                    ast::BinaryOp::kAdd, create<ast::IdentifierExpression>("a"),
                    create<ast::IdentifierExpression>("b"))));
  ast::Function a_func(Source{}, mod->RegisterSymbol("a_func"), "a_func",
                       func_params, &f32, body, ast::FunctionDecorationList{});

  ast::Function func(Source{}, mod->RegisterSymbol("main"), "main", {},
                     &void_type, create<ast::BlockStatement>(),
                     ast::FunctionDecorationList{});

  ast::ExpressionList call_params;
  call_params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  call_params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  ast::CallExpression expr(create<ast::IdentifierExpression>("a_func"),
                           call_params);

  ASSERT_TRUE(td.DetermineFunction(&func)) << td.error();
  ASSERT_TRUE(td.DetermineFunction(&a_func)) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

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
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Statement_Call) {
  ast::type::F32 f32;
  ast::type::Void void_type;

  ast::VariableList func_params;
  func_params.push_back(
      create<ast::Variable>(Source{},                         // source
                            "a",                              // name
                            ast::StorageClass::kFunction,     // storage_class
                            &f32,                             // type
                            false,                            // is_const
                            nullptr,                          // constructor
                            ast::VariableDecorationList{}));  // decorations
  func_params.push_back(
      create<ast::Variable>(Source{},                         // source
                            "b",                              // name
                            ast::StorageClass::kFunction,     // storage_class
                            &f32,                             // type
                            false,                            // is_const
                            nullptr,                          // constructor
                            ast::VariableDecorationList{}));  // decorations

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::ReturnStatement>(
      Source{}, create<ast::BinaryExpression>(
                    ast::BinaryOp::kAdd, create<ast::IdentifierExpression>("a"),
                    create<ast::IdentifierExpression>("b"))));

  ast::Function a_func(Source{}, mod->RegisterSymbol("a_func"), "a_func",
                       func_params, &void_type, body,
                       ast::FunctionDecorationList{});

  ast::Function func(Source{}, mod->RegisterSymbol("main"), "main", {},
                     &void_type, create<ast::BlockStatement>(),
                     ast::FunctionDecorationList{});

  ast::ExpressionList call_params;
  call_params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  call_params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  ast::CallStatement expr(create<ast::CallExpression>(
      create<ast::IdentifierExpression>("a_func"), call_params));

  ASSERT_TRUE(td.DetermineFunction(&func)) << td.error();
  ASSERT_TRUE(td.DetermineFunction(&a_func)) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

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
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
