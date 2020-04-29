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

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 5u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeStruct %4 %4
%2 = OpTypePointer Function %3
%6 = OpTypeInt 32 0
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

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 6u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeStruct %5 %5
%3 = OpTypeStruct %4
%2 = OpTypePointer Function %3
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%9 = OpTypePointer Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpAccessChain %9 %1 %8 %8
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

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 6u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeStruct %5 %5
%3 = OpTypeStruct %4
%2 = OpTypePointer Function %3
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%9 = OpTypePointer Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpAccessChain %9 %1 %8 %8
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
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%9 = OpTypePointer Function %5
%10 = OpConstant %5 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpAccessChain %9 %1 %8 %8
OpStore %6 %10
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
%9 = OpTypeInt 32 0
%10 = OpConstant %9 0
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function
%6 = OpVariable %7 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%8 = OpAccessChain %7 %1 %10 %10
%11 = OpLoad %5 %8
OpStore %6 %11
)");
}

TEST_F(BuilderTest, MemberAccessor_Swizzle_Single) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

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

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 5u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%6 = OpTypeInt 32 0
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

TEST_F(BuilderTest, DISABLED_MemberAccessor_Swizzle_MultipleNames) {
  // vec.yx
}

TEST_F(BuilderTest, DISABLED_Accessor_Mixed_ArrayAndMember) {
  // a[0].foo[2].bar.baz.yx
}

TEST_F(BuilderTest, DISABLED_MemberAccessor_Swizzle_of_Swizzle) {
  // vec.yxz.xz
}

TEST_F(BuilderTest, DISABLED_MemberAccessor_Member_of_Swizzle) {
  // vec.yxz.x
}

TEST_F(BuilderTest, DISABLED_MemberAccessor_Array_of_Swizzle) {
  // vec.yxz[1]
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
