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

#include "src/ast/array_accessor_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_MemberAccessor = TestHelper;

TEST_F(HlslGeneratorImplTest_MemberAccessor, EmitExpression_MemberAccessor) {
  ast::type::F32Type f32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList deco;
  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("mem", &f32, std::move(deco)));

  auto strct = std::make_unique<ast::Struct>();
  strct->set_members(std::move(members));

  ast::type::StructType s(std::move(strct));
  s.set_name("Str");

  auto str_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("str", ast::StorageClass::kPrivate, &s));

  auto str = std::make_unique<ast::IdentifierExpression>("str");
  auto mem = std::make_unique<ast::IdentifierExpression>("mem");

  ast::MemberAccessorExpression expr(std::move(str), std::move(mem));

  td().RegisterVariableForTesting(str_var.get());
  gen().register_global(str_var.get());
  mod()->AddGlobalVariable(std::move(str_var));

  ASSERT_TRUE(td().DetermineResultType(&expr)) << td().error();
  ASSERT_TRUE(gen().EmitExpression(out(), &expr)) << gen().error();
  EXPECT_EQ(result(), "str.mem");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load) {
  // struct Data {
  //   [[offset 0]] a : i32;
  //   [[offset 4]] b : f32;
  // };
  // var<storage_buffer> data : Data;
  // data.b;
  //
  // -> asfloat(data.Load(4));
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &i32, std::move(a_deco)));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));
  s.set_name("Data");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &s));

  ast::MemberAccessorExpression expr(
      std::make_unique<ast::IdentifierExpression>("data"),
      std::make_unique<ast::IdentifierExpression>("b"));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(td().DetermineResultType(&expr));

  ASSERT_TRUE(gen().EmitExpression(out(), &expr)) << gen().error();
  EXPECT_EQ(result(), "asfloat(data.Load(4))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load_Int) {
  // struct Data {
  //   [[offset 0]] a : i32;
  //   [[offset 4]] b : f32;
  // };
  // var<storage_buffer> data : Data;
  // data.a;
  //
  // -> asint(data.Load(0));
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &i32, std::move(a_deco)));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));
  s.set_name("Data");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &s));

  ast::MemberAccessorExpression expr(
      std::make_unique<ast::IdentifierExpression>("data"),
      std::make_unique<ast::IdentifierExpression>("a"));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(td().DetermineResultType(&expr));

  ASSERT_TRUE(gen().EmitExpression(out(), &expr)) << gen().error();
  EXPECT_EQ(result(), "asint(data.Load(0))");
}
TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store_Matrix) {
  // struct Data {
  //   [[offset 0]] z : f32;
  //   [[offset 4]] a : mat2x3<f32>;
  // };
  // var<storage_buffer> data : Data;
  // mat2x3<f32> b;
  // data.a = b;
  //
  // -> matrix<float, 3, 2> _tint_tmp = b;
  //    data.Store3(4 + 0, asuint(_tint_tmp[0]));
  //    data.Store3(4 + 16, asuint(_tint_tmp[1]));
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::MatrixType mat(&f32, 3, 2);

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("z", &i32, std::move(a_deco)));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &mat, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));
  s.set_name("Data");

  auto b_var =
      std::make_unique<ast::Variable>("b", ast::StorageClass::kPrivate, &mat);

  auto coord_var = std::make_unique<ast::Variable>(
      "data", ast::StorageClass::kStorageBuffer, &s);

  auto lhs = std::make_unique<ast::MemberAccessorExpression>(
      std::make_unique<ast::IdentifierExpression>("data"),
      std::make_unique<ast::IdentifierExpression>("a"));
  auto rhs = std::make_unique<ast::IdentifierExpression>("b");

  ast::AssignmentStatement assign(std::move(lhs), std::move(rhs));

  td().RegisterVariableForTesting(coord_var.get());
  td().RegisterVariableForTesting(b_var.get());
  gen().register_global(coord_var.get());
  gen().register_global(b_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));
  mod()->AddGlobalVariable(std::move(b_var));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(td().DetermineResultType(&assign));

  ASSERT_TRUE(gen().EmitStatement(out(), &assign)) << gen().error();
  EXPECT_EQ(result(), R"(matrix<float, 3, 2> _tint_tmp = b;
data.Store3(4 + 0, asuint(_tint_tmp[0]));
data.Store3(4 + 16, asuint(_tint_tmp[1]));
)");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store_Matrix_Empty) {
  // struct Data {
  //   [[offset 0]] z : f32;
  //   [[offset 4]] a : mat2x3<f32>;
  // };
  // var<storage_buffer> data : Data;
  // data.a = mat2x3<f32>();
  //
  // -> matrix<float, 3, 2> _tint_tmp = matrix<float, 3, 2>(0.0f, 0.0f, 0.0f,
  // 0.0f, 0.0f, 0.0f);
  //    data.Store3(4 + 0, asuint(_tint_tmp[0]);
  //    data.Store3(4 + 16, asuint(_tint_tmp[1]));
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::MatrixType mat(&f32, 3, 2);

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("z", &i32, std::move(a_deco)));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &mat, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));
  s.set_name("Data");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &s));

  auto lhs = std::make_unique<ast::MemberAccessorExpression>(
      std::make_unique<ast::IdentifierExpression>("data"),
      std::make_unique<ast::IdentifierExpression>("a"));
  auto rhs = std::make_unique<ast::TypeConstructorExpression>(
      &mat, ast::ExpressionList{});

  ast::AssignmentStatement assign(std::move(lhs), std::move(rhs));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(td().DetermineResultType(&assign));

  ASSERT_TRUE(gen().EmitStatement(out(), &assign)) << gen().error();
  EXPECT_EQ(
      result(),
      R"(matrix<float, 3, 2> _tint_tmp = matrix<float, 3, 2>(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
data.Store3(4 + 0, asuint(_tint_tmp[0]));
data.Store3(4 + 16, asuint(_tint_tmp[1]));
)");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load_Matrix) {
  // struct Data {
  //   [[offset 0]] z : f32;
  //   [[offset 4]] a : mat3x2<f32>;
  // };
  // var<storage_buffer> data : Data;
  // data.a;
  //
  // -> asfloat(matrix<uint, 2, 3>(data.Load2(4 + 0), data.Load2(4 + 8),
  // data.Load2(4 + 16)));
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::MatrixType mat(&f32, 2, 3);

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("z", &i32, std::move(a_deco)));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &mat, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));
  s.set_name("Data");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &s));

  ast::MemberAccessorExpression expr(
      std::make_unique<ast::IdentifierExpression>("data"),
      std::make_unique<ast::IdentifierExpression>("a"));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(td().DetermineResultType(&expr));

  ASSERT_TRUE(gen().EmitExpression(out(), &expr)) << gen().error();
  EXPECT_EQ(result(),
            "asfloat(matrix<uint, 2, 3>(data.Load2(4 + 0), data.Load2(4 + 8), "
            "data.Load2(4 + 16)))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load_Matrix_Nested) {
  // struct Data {
  //   [[offset 0]] z : f32;
  //   [[offset 4]] a : mat2x3<f32;
  // };
  // struct Outer {
  //   [[offset 0]] c : f32;
  //   [[offset 4]] b : Data;
  // };
  // var<storage_buffer> data : Outer;
  // data.b.a;
  //
  // -> asfloat(matrix<uint, 3, 2>(data.Load3(4 + 0), data.Load3(4 + 16)));
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::MatrixType mat(&f32, 3, 2);

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("z", &i32, std::move(a_deco)));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &mat, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));
  s.set_name("Data");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &s));

  ast::MemberAccessorExpression expr(
      std::make_unique<ast::IdentifierExpression>("data"),
      std::make_unique<ast::IdentifierExpression>("a"));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(td().DetermineResultType(&expr));

  ASSERT_TRUE(gen().EmitExpression(out(), &expr)) << gen().error();
  EXPECT_EQ(
      result(),
      "asfloat(matrix<uint, 3, 2>(data.Load3(4 + 0), data.Load3(4 + 16)))");
}

TEST_F(
    HlslGeneratorImplTest_MemberAccessor,
    EmitExpression_MemberAccessor_StorageBuffer_Load_Matrix_By3_Is_16_Bytes) {
  // struct Data {
  //   [[offset 4]] a : mat3x3<f32;
  // };
  // var<storage_buffer> data : Data;
  // data.a;
  //
  // -> asfloat(matrix<uint, 3, 3>(data.Load3(0), data.Load3(16),
  // data.Load3(32)));
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::MatrixType mat(&f32, 3, 3);

  ast::StructMemberList members;
  ast::StructMemberDecorationList deco;
  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &mat, std::move(deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));
  s.set_name("Data");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &s));

  ast::MemberAccessorExpression expr(
      std::make_unique<ast::IdentifierExpression>("data"),
      std::make_unique<ast::IdentifierExpression>("a"));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(td().DetermineResultType(&expr));

  ASSERT_TRUE(gen().EmitExpression(out(), &expr)) << gen().error();
  EXPECT_EQ(result(),
            "asfloat(matrix<uint, 3, 3>(data.Load3(0 + 0), data.Load3(0 + 16), "
            "data.Load3(0 + 32)))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load_Matrix_Single_Element) {
  // struct Data {
  //   [[offset 0]] z : f32;
  //   [[offset 16]] a : mat4x3<f32>;
  // };
  // var<storage_buffer> data : Data;
  // data.a[2][1];
  //
  // -> asfloat(data.Load((2 * 16) + (1 * 4) + 16)))
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::MatrixType mat(&f32, 3, 4);

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("z", &i32, std::move(a_deco)));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(16));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &mat, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));
  s.set_name("Data");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &s));

  ast::ArrayAccessorExpression expr(
      std::make_unique<ast::ArrayAccessorExpression>(
          std::make_unique<ast::MemberAccessorExpression>(
              std::make_unique<ast::IdentifierExpression>("data"),
              std::make_unique<ast::IdentifierExpression>("a")),
          std::make_unique<ast::ScalarConstructorExpression>(
              std::make_unique<ast::SintLiteral>(&i32, 2))),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::SintLiteral>(&i32, 1)));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(td().DetermineResultType(&expr));

  ASSERT_TRUE(gen().EmitExpression(out(), &expr)) << gen().error();
  EXPECT_EQ(result(), "asfloat(data.Load((4 * 1) + (16 * 2) + 16))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_ArrayAccessor_StorageBuffer_Load_Int_FromArray) {
  // struct Data {
  //   [[offset 0]] a : [[stride 4]] array<i32, 5>;
  // };
  // var<storage_buffer> data : Data;
  // data.a[2];
  //
  // -> asint(data.Load((2 * 4));
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::ArrayType ary(&i32, 5);
  ary.set_array_stride(4);

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &ary, std::move(a_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));
  s.set_name("Data");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &s));

  ast::ArrayAccessorExpression expr(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("data"),
          std::make_unique<ast::IdentifierExpression>("a")),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::SintLiteral>(&i32, 2)));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(td().DetermineResultType(&expr)) << td().error();

  ASSERT_TRUE(gen().EmitExpression(out(), &expr)) << gen().error();
  EXPECT_EQ(result(), "asint(data.Load((4 * 2) + 0))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_ArrayAccessor_StorageBuffer_Load_Int_FromArray_ExprIdx) {
  // struct Data {
  //   [[offset 0]] a : [[stride 4]] array<i32, 5>;
  // };
  // var<storage_buffer> data : Data;
  // data.a[(2 + 4) - 3];
  //
  // -> asint(data.Load((4 * ((2 + 4) - 3)));
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::ArrayType ary(&i32, 5);
  ary.set_array_stride(4);

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &ary, std::move(a_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));
  s.set_name("Data");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &s));

  ast::ArrayAccessorExpression expr(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("data"),
          std::make_unique<ast::IdentifierExpression>("a")),
      std::make_unique<ast::BinaryExpression>(
          ast::BinaryOp::kSubtract,
          std::make_unique<ast::BinaryExpression>(
              ast::BinaryOp::kAdd,
              std::make_unique<ast::ScalarConstructorExpression>(
                  std::make_unique<ast::SintLiteral>(&i32, 2)),
              std::make_unique<ast::ScalarConstructorExpression>(
                  std::make_unique<ast::SintLiteral>(&i32, 4))),
          std::make_unique<ast::ScalarConstructorExpression>(
              std::make_unique<ast::SintLiteral>(&i32, 3))));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(td().DetermineResultType(&expr)) << td().error();

  ASSERT_TRUE(gen().EmitExpression(out(), &expr)) << gen().error();
  EXPECT_EQ(result(), "asint(data.Load((4 * ((2 + 4) - 3)) + 0))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store) {
  // struct Data {
  //   [[offset 0]] a : i32;
  //   [[offset 4]] b : f32;
  // };
  // var<storage_buffer> data : Data;
  // data.b = 2.3f;
  //
  // -> data.Store(0, asuint(2.0f));

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &i32, std::move(a_deco)));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));
  s.set_name("Data");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &s));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();

  auto lhs = std::make_unique<ast::MemberAccessorExpression>(
      std::make_unique<ast::IdentifierExpression>("data"),
      std::make_unique<ast::IdentifierExpression>("b"));
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0f));
  ast::AssignmentStatement assign(std::move(lhs), std::move(rhs));

  ASSERT_TRUE(td().DetermineResultType(&assign));
  ASSERT_TRUE(gen().EmitStatement(out(), &assign)) << gen().error();
  EXPECT_EQ(result(), R"(data.Store(4, asuint(2.00000000f));
)");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store_ToArray) {
  // struct Data {
  //   [[offset 0]] a : [[stride 4]] array<i32, 5>;
  // };
  // var<storage_buffer> data : Data;
  // data.a[2] = 2;
  //
  // -> data.Store((2 * 4), asuint(2.3f));

  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::ArrayType ary(&i32, 5);
  ary.set_array_stride(4);

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &ary, std::move(a_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));
  s.set_name("Data");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &s));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();

  auto lhs = std::make_unique<ast::ArrayAccessorExpression>(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("data"),
          std::make_unique<ast::IdentifierExpression>("a")),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::SintLiteral>(&i32, 2)));
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  ast::AssignmentStatement assign(std::move(lhs), std::move(rhs));

  ASSERT_TRUE(td().DetermineResultType(&assign)) << td().error();
  ASSERT_TRUE(gen().EmitStatement(out(), &assign)) << gen().error();
  EXPECT_EQ(result(), R"(data.Store((4 * 2) + 0, asuint(2));
)");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store_Int) {
  // struct Data {
  //   [[offset 0]] a : i32;
  //   [[offset 4]] b : f32;
  // };
  // var<storage_buffer> data : Data;
  // data.a = 2;
  //
  // -> data.Store(0, asuint(2));

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &i32, std::move(a_deco)));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));
  s.set_name("Data");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &s));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();

  auto lhs = std::make_unique<ast::MemberAccessorExpression>(
      std::make_unique<ast::IdentifierExpression>("data"),
      std::make_unique<ast::IdentifierExpression>("a"));
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  ast::AssignmentStatement assign(std::move(lhs), std::move(rhs));

  ASSERT_TRUE(td().DetermineResultType(&assign));
  ASSERT_TRUE(gen().EmitStatement(out(), &assign)) << gen().error();
  EXPECT_EQ(result(), R"(data.Store(0, asuint(2));
)");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load_Vec3) {
  // struct Data {
  //   [[offset 0]] a : vec3<i32>;
  //   [[offset 16]] b : vec3<f32>;
  // };
  // var<storage_buffer> data : Data;
  // data.b;
  //
  // -> asfloat(data.Load(16));

  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::VectorType ivec3(&i32, 3);
  ast::type::VectorType fvec3(&f32, 3);

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &ivec3, std::move(a_deco)));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(16));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &fvec3, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));
  s.set_name("Data");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &s));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();

  ast::MemberAccessorExpression expr(
      std::make_unique<ast::IdentifierExpression>("data"),
      std::make_unique<ast::IdentifierExpression>("b"));

  ASSERT_TRUE(td().DetermineResultType(&expr));
  ASSERT_TRUE(gen().EmitExpression(out(), &expr)) << gen().error();
  EXPECT_EQ(result(), "asfloat(data.Load3(16))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store_Vec3) {
  // struct Data {
  //   [[offset 0]] a : vec3<i32>;
  //   [[offset 16]] b : vec3<f32>;
  // };
  // var<storage_buffer> data : Data;
  // data.b = vec3<f32>(2.3f, 1.2f, 0.2f);
  //
  // -> data.Store(16, asuint(vector<float, 3>(2.3f, 1.2f, 0.2f)));

  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::VectorType ivec3(&i32, 3);
  ast::type::VectorType fvec3(&f32, 3);

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &ivec3, std::move(a_deco)));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(16));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &fvec3, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));
  s.set_name("Data");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &s));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();

  auto lit1 = std::make_unique<ast::FloatLiteral>(&f32, 1.f);
  auto lit2 = std::make_unique<ast::FloatLiteral>(&f32, 2.f);
  auto lit3 = std::make_unique<ast::FloatLiteral>(&f32, 3.f);
  ast::ExpressionList values;
  values.push_back(
      std::make_unique<ast::ScalarConstructorExpression>(std::move(lit1)));
  values.push_back(
      std::make_unique<ast::ScalarConstructorExpression>(std::move(lit2)));
  values.push_back(
      std::make_unique<ast::ScalarConstructorExpression>(std::move(lit3)));

  auto lhs = std::make_unique<ast::MemberAccessorExpression>(
      std::make_unique<ast::IdentifierExpression>("data"),
      std::make_unique<ast::IdentifierExpression>("b"));
  auto rhs = std::make_unique<ast::TypeConstructorExpression>(
      &fvec3, std::move(values));

  ast::AssignmentStatement assign(std::move(lhs), std::move(rhs));

  ASSERT_TRUE(td().DetermineResultType(&assign));
  ASSERT_TRUE(gen().EmitStatement(out(), &assign)) << gen().error();
  EXPECT_EQ(
      result(),
      R"(data.Store3(16, asuint(vector<float, 3>(1.00000000f, 2.00000000f, 3.00000000f)));
)");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load_MultiLevel) {
  // struct Data {
  //   [[offset 0]] a : vec3<i32>;
  //   [[offset 16]] b : vec3<f32>;
  // };
  // struct Pre {
  //   var c : [[stride 32]] array<Data, 4>;
  // };
  //
  // var<storage_buffer> data : Pre;
  // data.c[2].b
  //
  // -> asfloat(data.Load3(16 + (2 * 32)))

  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::VectorType ivec3(&i32, 3);
  ast::type::VectorType fvec3(&f32, 3);

  ast::StructMemberList members;
  ast::StructMemberDecorationList deco;
  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &ivec3, std::move(deco)));

  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(16));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &fvec3, std::move(deco)));

  auto data_str = std::make_unique<ast::Struct>();
  data_str->set_members(std::move(members));

  ast::type::StructType data(std::move(data_str));
  data.set_name("Data");

  ast::type::ArrayType ary(&data, 4);
  ary.set_array_stride(32);

  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("c", &ary, std::move(deco)));

  auto pre_str = std::make_unique<ast::Struct>();
  pre_str->set_members(std::move(members));

  ast::type::StructType pre(std::move(pre_str));
  pre.set_name("Pre");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &pre));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();

  ast::MemberAccessorExpression expr(
      std::make_unique<ast::ArrayAccessorExpression>(
          std::make_unique<ast::MemberAccessorExpression>(
              std::make_unique<ast::IdentifierExpression>("data"),
              std::make_unique<ast::IdentifierExpression>("c")),
          std::make_unique<ast::ScalarConstructorExpression>(
              std::make_unique<ast::SintLiteral>(&i32, 2))),
      std::make_unique<ast::IdentifierExpression>("b"));

  ASSERT_TRUE(td().DetermineResultType(&expr));
  ASSERT_TRUE(gen().EmitExpression(out(), &expr)) << gen().error();
  EXPECT_EQ(result(), "asfloat(data.Load3(16 + (32 * 2) + 0))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load_MultiLevel_Swizzle) {
  // struct Data {
  //   [[offset 0]] a : vec3<i32>;
  //   [[offset 16]] b : vec3<f32>;
  // };
  // struct Pre {
  //   var c : [[stride 32]] array<Data, 4>;
  // };
  //
  // var<storage_buffer> data : Pre;
  // data.c[2].b.xy
  //
  // -> asfloat(data.Load3(16 + (2 * 32))).xy

  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::VectorType ivec3(&i32, 3);
  ast::type::VectorType fvec3(&f32, 3);

  ast::StructMemberList members;
  ast::StructMemberDecorationList deco;
  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &ivec3, std::move(deco)));

  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(16));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &fvec3, std::move(deco)));

  auto data_str = std::make_unique<ast::Struct>();
  data_str->set_members(std::move(members));

  ast::type::StructType data(std::move(data_str));
  data.set_name("Data");

  ast::type::ArrayType ary(&data, 4);
  ary.set_array_stride(32);

  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("c", &ary, std::move(deco)));

  auto pre_str = std::make_unique<ast::Struct>();
  pre_str->set_members(std::move(members));

  ast::type::StructType pre(std::move(pre_str));
  pre.set_name("Pre");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &pre));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();

  ast::MemberAccessorExpression expr(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::ArrayAccessorExpression>(
              std::make_unique<ast::MemberAccessorExpression>(
                  std::make_unique<ast::IdentifierExpression>("data"),
                  std::make_unique<ast::IdentifierExpression>("c")),
              std::make_unique<ast::ScalarConstructorExpression>(
                  std::make_unique<ast::SintLiteral>(&i32, 2))),
          std::make_unique<ast::IdentifierExpression>("b")),
      std::make_unique<ast::IdentifierExpression>("xy"));

  ASSERT_TRUE(td().DetermineResultType(&expr));
  ASSERT_TRUE(gen().EmitExpression(out(), &expr)) << gen().error();
  EXPECT_EQ(result(), "asfloat(data.Load3(16 + (32 * 2) + 0)).xy");
}

TEST_F(
    HlslGeneratorImplTest_MemberAccessor,
    EmitExpression_MemberAccessor_StorageBuffer_Load_MultiLevel_Swizzle_SingleLetter) {
  // struct Data {
  //   [[offset 0]] a : vec3<i32>;
  //   [[offset 16]] b : vec3<f32>;
  // };
  // struct Pre {
  //   var c : [[stride 32]] array<Data, 4>;
  // };
  //
  // var<storage_buffer> data : Pre;
  // data.c[2].b.g
  //
  // -> asfloat(data.Load((4 * 1) + 16 + (2 * 32) + 0))

  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::VectorType ivec3(&i32, 3);
  ast::type::VectorType fvec3(&f32, 3);

  ast::StructMemberList members;
  ast::StructMemberDecorationList deco;
  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &ivec3, std::move(deco)));

  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(16));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &fvec3, std::move(deco)));

  auto data_str = std::make_unique<ast::Struct>();
  data_str->set_members(std::move(members));

  ast::type::StructType data(std::move(data_str));
  data.set_name("Data");

  ast::type::ArrayType ary(&data, 4);
  ary.set_array_stride(32);

  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("c", &ary, std::move(deco)));

  auto pre_str = std::make_unique<ast::Struct>();
  pre_str->set_members(std::move(members));

  ast::type::StructType pre(std::move(pre_str));
  pre.set_name("Pre");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &pre));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();

  ast::MemberAccessorExpression expr(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::ArrayAccessorExpression>(
              std::make_unique<ast::MemberAccessorExpression>(
                  std::make_unique<ast::IdentifierExpression>("data"),
                  std::make_unique<ast::IdentifierExpression>("c")),
              std::make_unique<ast::ScalarConstructorExpression>(
                  std::make_unique<ast::SintLiteral>(&i32, 2))),
          std::make_unique<ast::IdentifierExpression>("b")),
      std::make_unique<ast::IdentifierExpression>("g"));

  ASSERT_TRUE(td().DetermineResultType(&expr));
  ASSERT_TRUE(gen().EmitExpression(out(), &expr)) << gen().error();
  EXPECT_EQ(result(), "asfloat(data.Load((4 * 1) + 16 + (32 * 2) + 0))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load_MultiLevel_Index) {
  // struct Data {
  //   [[offset 0]] a : vec3<i32>;
  //   [[offset 16]] b : vec3<f32>;
  // };
  // struct Pre {
  //   var c : [[stride 32]] array<Data, 4>;
  // };
  //
  // var<storage_buffer> data : Pre;
  // data.c[2].b[1]
  //
  // -> asfloat(data.Load(4 + 16 + (2 * 32)))

  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::VectorType ivec3(&i32, 3);
  ast::type::VectorType fvec3(&f32, 3);

  ast::StructMemberList members;
  ast::StructMemberDecorationList deco;
  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &ivec3, std::move(deco)));

  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(16));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &fvec3, std::move(deco)));

  auto data_str = std::make_unique<ast::Struct>();
  data_str->set_members(std::move(members));

  ast::type::StructType data(std::move(data_str));
  data.set_name("Data");

  ast::type::ArrayType ary(&data, 4);
  ary.set_array_stride(32);

  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("c", &ary, std::move(deco)));

  auto pre_str = std::make_unique<ast::Struct>();
  pre_str->set_members(std::move(members));

  ast::type::StructType pre(std::move(pre_str));
  pre.set_name("Pre");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &pre));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();

  ast::ArrayAccessorExpression expr(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::ArrayAccessorExpression>(
              std::make_unique<ast::MemberAccessorExpression>(
                  std::make_unique<ast::IdentifierExpression>("data"),
                  std::make_unique<ast::IdentifierExpression>("c")),
              std::make_unique<ast::ScalarConstructorExpression>(
                  std::make_unique<ast::SintLiteral>(&i32, 2))),
          std::make_unique<ast::IdentifierExpression>("b")),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::SintLiteral>(&i32, 1)));

  ASSERT_TRUE(td().DetermineResultType(&expr));
  ASSERT_TRUE(gen().EmitExpression(out(), &expr)) << gen().error();
  EXPECT_EQ(result(), "asfloat(data.Load((4 * 1) + 16 + (32 * 2) + 0))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store_MultiLevel) {
  // struct Data {
  //   [[offset 0]] a : vec3<i32>;
  //   [[offset 16]] b : vec3<f32>;
  // };
  // struct Pre {
  //   var c : [[stride 32]] array<Data, 4>;
  // };
  //
  // var<storage_buffer> data : Pre;
  // data.c[2].b = vec3<f32>(1.f, 2.f, 3.f);
  //
  // -> data.Store3(16 + (2 * 32), asuint(vector<float, 3>(1.0f, 2.0f, 3.0f)));

  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::VectorType ivec3(&i32, 3);
  ast::type::VectorType fvec3(&f32, 3);

  ast::StructMemberList members;
  ast::StructMemberDecorationList deco;
  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &ivec3, std::move(deco)));

  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(16));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &fvec3, std::move(deco)));

  auto data_str = std::make_unique<ast::Struct>();
  data_str->set_members(std::move(members));

  ast::type::StructType data(std::move(data_str));
  data.set_name("Data");

  ast::type::ArrayType ary(&data, 4);
  ary.set_array_stride(32);

  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("c", &ary, std::move(deco)));

  auto pre_str = std::make_unique<ast::Struct>();
  pre_str->set_members(std::move(members));

  ast::type::StructType pre(std::move(pre_str));
  pre.set_name("Pre");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &pre));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();

  auto lhs = std::make_unique<ast::MemberAccessorExpression>(
      std::make_unique<ast::ArrayAccessorExpression>(
          std::make_unique<ast::MemberAccessorExpression>(
              std::make_unique<ast::IdentifierExpression>("data"),
              std::make_unique<ast::IdentifierExpression>("c")),
          std::make_unique<ast::ScalarConstructorExpression>(
              std::make_unique<ast::SintLiteral>(&i32, 2))),
      std::make_unique<ast::IdentifierExpression>("b"));

  auto lit1 = std::make_unique<ast::FloatLiteral>(&f32, 1.f);
  auto lit2 = std::make_unique<ast::FloatLiteral>(&f32, 2.f);
  auto lit3 = std::make_unique<ast::FloatLiteral>(&f32, 3.f);
  ast::ExpressionList values;
  values.push_back(
      std::make_unique<ast::ScalarConstructorExpression>(std::move(lit1)));
  values.push_back(
      std::make_unique<ast::ScalarConstructorExpression>(std::move(lit2)));
  values.push_back(
      std::make_unique<ast::ScalarConstructorExpression>(std::move(lit3)));

  auto rhs = std::make_unique<ast::TypeConstructorExpression>(
      &fvec3, std::move(values));

  ast::AssignmentStatement assign(std::move(lhs), std::move(rhs));

  ASSERT_TRUE(td().DetermineResultType(&assign));
  ASSERT_TRUE(gen().EmitStatement(out(), &assign)) << gen().error();
  EXPECT_EQ(
      result(),
      R"(data.Store3(16 + (32 * 2) + 0, asuint(vector<float, 3>(1.00000000f, 2.00000000f, 3.00000000f)));
)");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store_Swizzle_SingleLetter) {
  // struct Data {
  //   [[offset 0]] a : vec3<i32>;
  //   [[offset 16]] b : vec3<f32>;
  // };
  // struct Pre {
  //   var c : [[stride 32]] array<Data, 4>;
  // };
  //
  // var<storage_buffer> data : Pre;
  // data.c[2].b.y = 1.f;
  //
  // -> data.Store((4 * 1) + 16 + (2 * 32) + 0, asuint(1.0f));

  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::VectorType ivec3(&i32, 3);
  ast::type::VectorType fvec3(&f32, 3);

  ast::StructMemberList members;
  ast::StructMemberDecorationList deco;
  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &ivec3, std::move(deco)));

  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(16));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &fvec3, std::move(deco)));

  auto data_str = std::make_unique<ast::Struct>();
  data_str->set_members(std::move(members));

  ast::type::StructType data(std::move(data_str));
  data.set_name("Data");

  ast::type::ArrayType ary(&data, 4);
  ary.set_array_stride(32);

  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("c", &ary, std::move(deco)));

  auto pre_str = std::make_unique<ast::Struct>();
  pre_str->set_members(std::move(members));

  ast::type::StructType pre(std::move(pre_str));
  pre.set_name("Pre");

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &pre));

  td().RegisterVariableForTesting(coord_var.get());
  gen().register_global(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ASSERT_TRUE(td().Determine()) << td().error();

  auto lhs = std::make_unique<ast::MemberAccessorExpression>(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::ArrayAccessorExpression>(
              std::make_unique<ast::MemberAccessorExpression>(
                  std::make_unique<ast::IdentifierExpression>("data"),
                  std::make_unique<ast::IdentifierExpression>("c")),
              std::make_unique<ast::ScalarConstructorExpression>(
                  std::make_unique<ast::SintLiteral>(&i32, 2))),
          std::make_unique<ast::IdentifierExpression>("b")),
      std::make_unique<ast::IdentifierExpression>("y"));

  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&i32, 1.f));

  ast::AssignmentStatement assign(std::move(lhs), std::move(rhs));

  ASSERT_TRUE(td().DetermineResultType(&assign));
  ASSERT_TRUE(gen().EmitStatement(out(), &assign)) << gen().error();
  EXPECT_EQ(result(),
            R"(data.Store((4 * 1) + 16 + (32 * 2) + 0, asuint(1.00000000f));
)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
