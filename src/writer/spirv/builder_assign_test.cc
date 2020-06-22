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
#include "src/ast/array_accessor_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, Assign_Var) {
  ast::type::F32Type f32;

  ast::Variable v("var", ast::StorageClass::kOutput, &f32);

  auto ident = std::make_unique<ast::IdentifierExpression>("var");
  auto val = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f));

  ast::AssignmentStatement assign(std::move(ident), std::move(val));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&v);

  ASSERT_TRUE(td.DetermineResultType(&assign)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_TRUE(b.GenerateAssignStatement(&assign)) << b.error();
  EXPECT_FALSE(b.has_error());

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Output %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Output %4
%5 = OpConstant %3 1
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"(OpStore %1 %5
)");
}

TEST_F(BuilderTest, Assign_Var_ZeroConstructor) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::Variable v("var", ast::StorageClass::kOutput, &vec);

  auto ident = std::make_unique<ast::IdentifierExpression>("var");
  ast::ExpressionList vals;
  auto val =
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals));

  ast::AssignmentStatement assign(std::move(ident), std::move(val));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&v);

  ASSERT_TRUE(td.DetermineResultType(&assign)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_TRUE(b.GenerateAssignStatement(&assign)) << b.error();
  EXPECT_FALSE(b.has_error());

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Output %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Output %5
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"(OpStore %1 %5
)");
}

TEST_F(BuilderTest, Assign_Var_Complex_ConstructorWithExtract) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);
  ast::type::VectorType vec2(&f32, 2);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0f)));
  auto first =
      std::make_unique<ast::TypeConstructorExpression>(&vec2, std::move(vals));

  vals.push_back(std::move(first));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  auto init =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  ast::Variable v("var", ast::StorageClass::kOutput, &vec3);

  ast::AssignmentStatement assign(
      std::make_unique<ast::IdentifierExpression>("var"), std::move(init));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&v);
  ASSERT_TRUE(td.DetermineResultType(&assign)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_TRUE(b.GenerateAssignStatement(&assign)) << b.error();
  EXPECT_FALSE(b.has_error());

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Output %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Output %5
%6 = OpTypeVector %4 2
%7 = OpConstant %4 1
%8 = OpConstant %4 2
%9 = OpConstantComposite %6 %7 %8
%12 = OpConstant %4 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%10 = OpCompositeExtract %4 %9 0
%11 = OpCompositeExtract %4 %9 1
%13 = OpCompositeConstruct %3 %10 %11 %12
OpStore %1 %13
)");
}

TEST_F(BuilderTest, Assign_Var_Complex_Constructor) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);
  ast::type::VectorType vec(&vec3, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));
  auto first =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  auto second =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));
  auto third =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  vals.push_back(std::move(first));
  vals.push_back(std::move(second));
  vals.push_back(std::move(third));

  auto init =
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals));

  ast::Variable v("var", ast::StorageClass::kOutput, &vec);

  ast::AssignmentStatement assign(
      std::make_unique<ast::IdentifierExpression>("var"), std::move(init));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&v);
  ASSERT_TRUE(td.DetermineResultType(&assign)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_TRUE(b.GenerateAssignStatement(&assign)) << b.error();
  EXPECT_FALSE(b.has_error());

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%3 = OpTypeVector %4 3
%2 = OpTypePointer Output %3
%6 = OpConstantNull %3
%1 = OpVariable %2 Output %6
%7 = OpConstant %5 1
%8 = OpConstant %5 2
%9 = OpConstant %5 3
%10 = OpConstantComposite %4 %7 %8 %9
%11 = OpConstantComposite %4 %9 %8 %7
%12 = OpConstantComposite %4 %8 %7 %9
%13 = OpConstantComposite %3 %10 %11 %12
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"(OpStore %1 %13
)");
}

TEST_F(BuilderTest, Assign_StructMember) {
  ast::type::F32Type f32;

  // my_struct {
  //   a : f32
  //   b : f32
  // }
  // var ident : my_struct
  // ident.b = 4.0;

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

  ast::Variable v("ident", ast::StorageClass::kFunction, &s_type);

  auto ident = std::make_unique<ast::MemberAccessorExpression>(
      std::make_unique<ast::IdentifierExpression>("ident"),
      std::make_unique<ast::IdentifierExpression>("b"));

  auto val = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 4.0f));

  ast::AssignmentStatement assign(std::move(ident), std::move(val));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&v);

  ASSERT_TRUE(td.DetermineResultType(&assign)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_TRUE(b.GenerateAssignStatement(&assign)) << b.error();
  EXPECT_FALSE(b.has_error());

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeStruct %4 %4
%2 = OpTypePointer Function %3
%1 = OpVariable %2 Function
%5 = OpTypeInt 32 0
%6 = OpConstant %5 1
%7 = OpTypePointer Function %4
%9 = OpConstant %4 4
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%8 = OpAccessChain %7 %1 %6
OpStore %8 %9
)");
}

TEST_F(BuilderTest, Assign_Vector) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  ast::Variable v("var", ast::StorageClass::kOutput, &vec3);

  auto ident = std::make_unique<ast::IdentifierExpression>("var");

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  auto val =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  ast::AssignmentStatement assign(std::move(ident), std::move(val));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&v);

  ASSERT_TRUE(td.DetermineResultType(&assign)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_TRUE(b.GenerateAssignStatement(&assign)) << b.error();
  EXPECT_FALSE(b.has_error());

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Output %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Output %5
%6 = OpConstant %4 1
%7 = OpConstant %4 3
%8 = OpConstantComposite %3 %6 %6 %7
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"(OpStore %1 %8
)");
}

TEST_F(BuilderTest, Assign_Vector_MemberByName) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  // var.y = 1

  ast::Variable v("var", ast::StorageClass::kOutput, &vec3);

  auto ident = std::make_unique<ast::MemberAccessorExpression>(
      std::make_unique<ast::IdentifierExpression>("var"),
      std::make_unique<ast::IdentifierExpression>("y"));
  auto val = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f));

  ast::AssignmentStatement assign(std::move(ident), std::move(val));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&v);

  ASSERT_TRUE(td.DetermineResultType(&assign)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_TRUE(b.GenerateAssignStatement(&assign)) << b.error();
  EXPECT_FALSE(b.has_error());

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Output %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Output %5
%6 = OpTypeInt 32 0
%7 = OpConstant %6 1
%8 = OpTypePointer Output %4
%10 = OpConstant %4 1
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpAccessChain %8 %1 %7
OpStore %9 %10
)");
}

TEST_F(BuilderTest, Assign_Vector_MemberByIndex) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  // var[1] = 1

  ast::Variable v("var", ast::StorageClass::kOutput, &vec3);

  auto ident = std::make_unique<ast::ArrayAccessorExpression>(
      std::make_unique<ast::IdentifierExpression>("var"),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::SintLiteral>(&i32, 1)));
  auto val = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f));

  ast::AssignmentStatement assign(std::move(ident), std::move(val));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&v);

  ASSERT_TRUE(td.DetermineResultType(&assign)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_TRUE(b.GenerateAssignStatement(&assign)) << b.error();
  EXPECT_FALSE(b.has_error());

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Output %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Output %5
%6 = OpTypeInt 32 1
%7 = OpConstant %6 1
%8 = OpTypePointer Output %4
%10 = OpConstant %4 1
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpAccessChain %8 %1 %7
OpStore %9 %10
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
