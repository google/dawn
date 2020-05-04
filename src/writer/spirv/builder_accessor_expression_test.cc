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
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/int_literal.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/struct_type.h"
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

  // vec3<f32> ary;
  // ary[1]  -> ptr<f32>

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

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 8u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpTypeInt 32 1
%6 = OpConstant %5 1
%7 = OpTypePointer Function %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%8 = OpAccessChain %7 %1 %6
)");
}

TEST_F(BuilderTest, Accessor_Array_LoadIndex) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  // ary : vec3<f32>;
  // idx : i32;
  // ary[idx]  -> ptr<f32>

  ast::Variable var("ary", ast::StorageClass::kFunction, &vec3);
  ast::Variable idx("idx", ast::StorageClass::kFunction, &i32);

  auto ary = std::make_unique<ast::IdentifierExpression>("ary");
  auto idx_expr = std::make_unique<ast::IdentifierExpression>("idx");

  ast::ArrayAccessorExpression expr(std::move(ary), std::move(idx_expr));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&var);
  td.RegisterVariableForTesting(&idx);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();
  ASSERT_TRUE(b.GenerateFunctionVariable(&idx)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 10u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%7 = OpTypeInt 32 1
%6 = OpTypePointer Function %7
%9 = OpTypePointer Function %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
%5 = OpVariable %6 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%8 = OpLoad %7 %5
%10 = OpAccessChain %9 %1 %8
)");
}

TEST_F(BuilderTest, ArrayAccessor_Dynamic) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  // vec3<f32> ary;
  // ary[1 + 2]  -> ptr<f32>

  ast::Variable var("ary", ast::StorageClass::kFunction, &vec3);

  auto ary = std::make_unique<ast::IdentifierExpression>("ary");

  ast::ArrayAccessorExpression expr(
      std::move(ary), std::make_unique<ast::BinaryExpression>(
                          ast::BinaryOp::kAdd,
                          std::make_unique<ast::ScalarConstructorExpression>(
                              std::make_unique<ast::IntLiteral>(&i32, 1)),
                          std::make_unique<ast::ScalarConstructorExpression>(
                              std::make_unique<ast::IntLiteral>(&i32, 2))));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 10u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpTypeInt 32 1
%6 = OpConstant %5 1
%7 = OpConstant %5 2
%9 = OpTypePointer Function %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%8 = OpIAdd %5 %6 %7
%10 = OpAccessChain %9 %1 %8
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

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 12u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%6 = OpTypeInt 32 0
%7 = OpConstant %6 4
%3 = OpTypeArray %4 %7
%2 = OpTypePointer Function %3
%8 = OpTypeInt 32 1
%9 = OpConstant %8 3
%10 = OpConstant %8 2
%11 = OpTypePointer Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%12 = OpAccessChain %11 %1 %9 %10
)");
}

TEST_F(BuilderTest, Accessor_ArrayWithSwizzle) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);
  ast::type::ArrayType ary4(&vec3, 4);

  // var a : array<vec3<f32>, 4>;
  // a[2].xy;

  ast::Variable var("ary", ast::StorageClass::kFunction, &ary4);

  ast::MemberAccessorExpression expr(
      std::make_unique<ast::ArrayAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("ary"),
          std::make_unique<ast::ScalarConstructorExpression>(
              std::make_unique<ast::IntLiteral>(&i32, 2))),
      std::make_unique<ast::IdentifierExpression>("xy"));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();
  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 14u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%6 = OpTypeInt 32 0
%7 = OpConstant %6 4
%3 = OpTypeArray %4 %7
%2 = OpTypePointer Function %3
%8 = OpTypeInt 32 1
%9 = OpConstant %8 2
%10 = OpTypePointer Function %4
%12 = OpTypeVector %5 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%11 = OpAccessChain %10 %1 %9
%13 = OpLoad %4 %11
%14 = OpVectorShuffle %12 %13 %13 0 1
)");
}

TEST_F(BuilderTest, MemberAccessor) {
  ast::type::F32Type f32;

  // my_struct {
  //   a : f32
  //   b : f32
  // }
  // var ident : my_struct
  // ident.b

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(
      std::make_unique<ast::StructMember>("a", &f32, std::move(decos)));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(decos)));

  auto s = std::make_unique<ast::Struct>(ast::StructDecoration::kNone,
                                         std::move(members));
  ast::type::StructType s_type(std::move(s));
  s_type.set_name("my_struct");

  ast::Variable var("ident", ast::StorageClass::kFunction, &s_type);

  ast::MemberAccessorExpression expr(
      std::make_unique<ast::IdentifierExpression>("ident"),
      std::make_unique<ast::IdentifierExpression>("b"));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 8u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeStruct %4 %4
%2 = OpTypePointer Function %3
%5 = OpTypeInt 32 0
%6 = OpConstant %5 1
%7 = OpTypePointer Function %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%8 = OpAccessChain %7 %1 %6
)");
}

TEST_F(BuilderTest, MemberAccessor_Nested) {
  ast::type::F32Type f32;

  // inner_struct {
  //   a : f32
  // }
  // my_struct {
  //   inner : inner_struct
  // }
  //
  // var ident : my_struct
  // ident.inner.a
  ast::StructMemberDecorationList decos;
  ast::StructMemberList inner_members;
  inner_members.push_back(
      std::make_unique<ast::StructMember>("a", &f32, std::move(decos)));
  inner_members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(decos)));

  ast::type::StructType inner_struct(std::make_unique<ast::Struct>(
      ast::StructDecoration::kNone, std::move(inner_members)));

  ast::StructMemberList outer_members;
  outer_members.push_back(std::make_unique<ast::StructMember>(
      "inner", &inner_struct, std::move(decos)));

  ast::type::StructType s_type(std::make_unique<ast::Struct>(
      ast::StructDecoration::kNone, std::move(outer_members)));
  s_type.set_name("my_struct");

  ast::Variable var("ident", ast::StorageClass::kFunction, &s_type);

  ast::MemberAccessorExpression expr(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("ident"),
          std::make_unique<ast::IdentifierExpression>("inner")),
      std::make_unique<ast::IdentifierExpression>("a"));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 9u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeStruct %5 %5
%3 = OpTypeStruct %4
%2 = OpTypePointer Function %3
%6 = OpTypeInt 32 0
%7 = OpConstant %6 0
%8 = OpTypePointer Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpAccessChain %8 %1 %7 %7
)");
}

TEST_F(BuilderTest, MemberAccessor_Nested_WithAlias) {
  ast::type::F32Type f32;

  // type Inner = struct {
  //   a : f32
  //   b : f32
  // }
  // my_struct {
  //   inner : Inner
  // }
  //
  // var ident : my_struct
  // ident.inner.a
  ast::StructMemberDecorationList decos;
  ast::StructMemberList inner_members;
  inner_members.push_back(
      std::make_unique<ast::StructMember>("a", &f32, std::move(decos)));
  inner_members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(decos)));

  ast::type::StructType inner_struct(std::make_unique<ast::Struct>(
      ast::StructDecoration::kNone, std::move(inner_members)));

  ast::type::AliasType alias("Inner", &inner_struct);

  ast::StructMemberList outer_members;
  outer_members.push_back(
      std::make_unique<ast::StructMember>("inner", &alias, std::move(decos)));

  ast::type::StructType s_type(std::make_unique<ast::Struct>(
      ast::StructDecoration::kNone, std::move(outer_members)));
  s_type.set_name("my_struct");

  ast::Variable var("ident", ast::StorageClass::kFunction, &s_type);

  ast::MemberAccessorExpression expr(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("ident"),
          std::make_unique<ast::IdentifierExpression>("inner")),
      std::make_unique<ast::IdentifierExpression>("a"));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 9u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeStruct %5 %5
%3 = OpTypeStruct %4
%2 = OpTypePointer Function %3
%6 = OpTypeInt 32 0
%7 = OpConstant %6 0
%8 = OpTypePointer Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpAccessChain %8 %1 %7 %7
)");
}

TEST_F(BuilderTest, MemberAccessor_Nested_Assignment_LHS) {
  ast::type::F32Type f32;

  // inner_struct {
  //   a : f32
  // }
  // my_struct {
  //   inner : inner_struct
  // }
  //
  // var ident : my_struct
  // ident.inner.a = 2.0f;

  ast::StructMemberDecorationList decos;
  ast::StructMemberList inner_members;
  inner_members.push_back(
      std::make_unique<ast::StructMember>("a", &f32, std::move(decos)));
  inner_members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(decos)));

  ast::type::StructType inner_struct(std::make_unique<ast::Struct>(
      ast::StructDecoration::kNone, std::move(inner_members)));

  ast::StructMemberList outer_members;
  outer_members.push_back(std::make_unique<ast::StructMember>(
      "inner", &inner_struct, std::move(decos)));

  ast::type::StructType s_type(std::make_unique<ast::Struct>(
      ast::StructDecoration::kNone, std::move(outer_members)));
  s_type.set_name("my_struct");

  ast::Variable var("ident", ast::StorageClass::kFunction, &s_type);

  auto lhs = std::make_unique<ast::MemberAccessorExpression>(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("ident"),
          std::make_unique<ast::IdentifierExpression>("inner")),
      std::make_unique<ast::IdentifierExpression>("a"));

  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.f));

  ast::AssignmentStatement expr(std::move(lhs), std::move(rhs));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_TRUE(b.GenerateAssignStatement(&expr)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeStruct %5 %5
%3 = OpTypeStruct %4
%2 = OpTypePointer Function %3
%6 = OpTypeInt 32 0
%7 = OpConstant %6 0
%8 = OpTypePointer Function %5
%10 = OpConstant %5 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpAccessChain %8 %1 %7 %7
OpStore %9 %10
)");
}

TEST_F(BuilderTest, MemberAccessor_Nested_Assignment_RHS) {
  ast::type::F32Type f32;

  // inner_struct {
  //   a : f32
  // }
  // my_struct {
  //   inner : inner_struct
  // }
  //
  // var ident : my_struct
  // var store : f32 = ident.inner.a

  ast::StructMemberDecorationList decos;
  ast::StructMemberList inner_members;
  inner_members.push_back(
      std::make_unique<ast::StructMember>("a", &f32, std::move(decos)));
  inner_members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(decos)));

  ast::type::StructType inner_struct(std::make_unique<ast::Struct>(
      ast::StructDecoration::kNone, std::move(inner_members)));

  ast::StructMemberList outer_members;
  outer_members.push_back(std::make_unique<ast::StructMember>(
      "inner", &inner_struct, std::move(decos)));

  ast::type::StructType s_type(std::make_unique<ast::Struct>(
      ast::StructDecoration::kNone, std::move(outer_members)));
  s_type.set_name("my_struct");

  ast::Variable var("ident", ast::StorageClass::kFunction, &s_type);
  ast::Variable store("store", ast::StorageClass::kFunction, &f32);

  auto lhs = std::make_unique<ast::IdentifierExpression>("store");

  auto rhs = std::make_unique<ast::MemberAccessorExpression>(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("ident"),
          std::make_unique<ast::IdentifierExpression>("inner")),
      std::make_unique<ast::IdentifierExpression>("a"));

  ast::AssignmentStatement expr(std::move(lhs), std::move(rhs));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&var);
  td.RegisterVariableForTesting(&store);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();
  ASSERT_TRUE(b.GenerateFunctionVariable(&store)) << b.error();

  EXPECT_TRUE(b.GenerateAssignStatement(&expr)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeStruct %5 %5
%3 = OpTypeStruct %4
%2 = OpTypePointer Function %3
%7 = OpTypePointer Function %5
%8 = OpTypeInt 32 0
%9 = OpConstant %8 0
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
%6 = OpVariable %7 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%10 = OpAccessChain %7 %1 %9 %9
%11 = OpLoad %5 %10
OpStore %6 %11
)");
}

TEST_F(BuilderTest, MemberAccessor_Swizzle_Single) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  // ident.y

  ast::Variable var("ident", ast::StorageClass::kFunction, &vec3);

  ast::MemberAccessorExpression expr(
      std::make_unique<ast::IdentifierExpression>("ident"),
      std::make_unique<ast::IdentifierExpression>("y"));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 8u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpTypeInt 32 0
%6 = OpConstant %5 1
%7 = OpTypePointer Function %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%8 = OpAccessChain %7 %1 %6
)");
}

TEST_F(BuilderTest, MemberAccessor_Swizzle_MultipleNames) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  // ident.yx

  ast::Variable var("ident", ast::StorageClass::kFunction, &vec3);

  ast::MemberAccessorExpression expr(
      std::make_unique<ast::IdentifierExpression>("ident"),
      std::make_unique<ast::IdentifierExpression>("yx"));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 7u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpTypeVector %4 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpLoad %3 %1
%7 = OpVectorShuffle %5 %6 %6 1 0
)");
}

TEST_F(BuilderTest, MemberAccessor_Swizzle_of_Swizzle) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  // ident.yxz.xz

  ast::Variable var("ident", ast::StorageClass::kFunction, &vec3);

  ast::MemberAccessorExpression expr(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("ident"),
          std::make_unique<ast::IdentifierExpression>("yxz")),
      std::make_unique<ast::IdentifierExpression>("xz"));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 8u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%7 = OpTypeVector %4 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%5 = OpLoad %3 %1
%6 = OpVectorShuffle %3 %5 %5 1 0 2
%8 = OpVectorShuffle %7 %6 %6 0 2
)");
}

TEST_F(BuilderTest, MemberAccessor_Member_of_Swizzle) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  // ident.yxz.x

  ast::Variable var("ident", ast::StorageClass::kFunction, &vec3);

  ast::MemberAccessorExpression expr(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("ident"),
          std::make_unique<ast::IdentifierExpression>("yxz")),
      std::make_unique<ast::IdentifierExpression>("x"));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 7u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%5 = OpLoad %3 %1
%6 = OpVectorShuffle %3 %5 %5 1 0 2
%7 = OpCompositeExtract %4 %6 0
)");
}

TEST_F(BuilderTest, MemberAccessor_Array_of_Swizzle) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  // index.yxz[1]

  ast::Variable var("ident", ast::StorageClass::kFunction, &vec3);

  ast::ArrayAccessorExpression expr(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("ident"),
          std::make_unique<ast::IdentifierExpression>("yxz")),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::IntLiteral>(&i32, 1)));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 9u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%7 = OpTypeInt 32 1
%8 = OpConstant %7 1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%5 = OpLoad %3 %1
%6 = OpVectorShuffle %3 %5 %5 1 0 2
%9 = OpVectorExtractDynamic %4 %6 %8
)");
}

TEST_F(BuilderTest, Accessor_Mixed_ArrayAndMember) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  // type C = struct {
  //   baz : vec3<f32>
  // }
  // type B = struct {
  //  bar : C;
  // }
  // type A = struct {
  //   foo : array<B, 3>
  // }
  // var index : array<A, 2>
  // index[0].foo[2].bar.baz.yx

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(
      std::make_unique<ast::StructMember>("baz", &vec3, std::move(decos)));
  auto s = std::make_unique<ast::Struct>(ast::StructDecoration::kNone,
                                         std::move(members));
  ast::type::StructType c_type(std::move(s));
  c_type.set_name("C");

  members.push_back(
      std::make_unique<ast::StructMember>("bar", &c_type, std::move(decos)));
  s = std::make_unique<ast::Struct>(ast::StructDecoration::kNone,
                                    std::move(members));
  ast::type::StructType b_type(std::move(s));
  b_type.set_name("B");

  ast::type::ArrayType b_ary_type(&b_type, 3);

  members.push_back(std::make_unique<ast::StructMember>("foo", &b_ary_type,
                                                        std::move(decos)));
  s = std::make_unique<ast::Struct>(ast::StructDecoration::kNone,
                                    std::move(members));
  ast::type::StructType a_type(std::move(s));
  a_type.set_name("A");

  ast::type::ArrayType a_ary_type(&a_type, 2);

  ast::Variable var("index", ast::StorageClass::kFunction, &a_ary_type);

  ast::MemberAccessorExpression expr(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::MemberAccessorExpression>(
              std::make_unique<ast::ArrayAccessorExpression>(
                  std::make_unique<ast::MemberAccessorExpression>(
                      std::make_unique<ast::ArrayAccessorExpression>(
                          std::make_unique<ast::IdentifierExpression>("index"),
                          std::make_unique<ast::ScalarConstructorExpression>(
                              std::make_unique<ast::IntLiteral>(&i32, 0))),
                      std::make_unique<ast::IdentifierExpression>("foo")),
                  std::make_unique<ast::ScalarConstructorExpression>(
                      std::make_unique<ast::IntLiteral>(&i32, 2))),
              std::make_unique<ast::IdentifierExpression>("bar")),
          std::make_unique<ast::IdentifierExpression>("baz")),
      std::make_unique<ast::IdentifierExpression>("yx"));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 19u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%9 = OpTypeFloat 32
%8 = OpTypeVector %9 3
%7 = OpTypeStruct %8
%6 = OpTypeStruct %7
%10 = OpTypeInt 32 0
%11 = OpConstant %10 3
%5 = OpTypeArray %6 %11
%4 = OpTypeStruct %5
%12 = OpConstant %10 2
%3 = OpTypeArray %4 %12
%2 = OpTypePointer Function %3
%13 = OpTypeInt 32 1
%14 = OpConstant %13 0
%15 = OpTypePointer Function %8
%17 = OpTypeVector %9 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%16 = OpAccessChain %15 %1 %14 %14 %12 %14 %14
%18 = OpLoad %8 %16
%19 = OpVectorShuffle %17 %18 %18 1 0
)");
}

TEST_F(BuilderTest, DISABLED_Accessor_Array_NonPointer) {
  // const a : array<f32, 3>;
  // a[2]
  //
  // This has to generate an OpConstantExtract and will need to read the 3 value
  // out of the ScalarConstructor as extract requires integer indices.
}

TEST_F(BuilderTest, DISABLED_Accessor_Struct_NonPointer) {
  // type A = struct {
  //   a : f32;
  //   b : f32;
  // };
  // const b : A;
  // b.b
  //
  // This needs to do an OpCompositeExtract on the struct.
}

TEST_F(BuilderTest, DISABLED_Accessor_NonPointer_Multi) {
  // type A = struct {
  //   a : f32;
  //   b : vec3<f32, 3>;
  // };
  // type B = struct {
  //   c : A;
  // }
  // const b : array<B, 3>;
  // b[2].c.b.yx.x
  //
  // This needs to do an OpCompositeExtract similar to the AccessChain case
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
