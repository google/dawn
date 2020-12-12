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
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/stride_decoration.h"
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
#include "src/ast/variable.h"
#include "src/type_determiner.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_MemberAccessor = TestHelper;

TEST_F(HlslGeneratorImplTest_MemberAccessor, EmitExpression_MemberAccessor) {
  ast::type::F32 f32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList deco;
  deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "mem", &f32, deco));

  auto* strct = create<ast::Struct>(members);

  ast::type::Struct s("Str", strct);

  auto* str_var =
      create<ast::Variable>(Source{},                        // source
                            "str",                           // name
                            ast::StorageClass::kPrivate,     // storage_class
                            &s,                              // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  auto* str = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("str"), "str");
  auto* mem = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("mem"), "mem");

  ast::MemberAccessorExpression expr(Source{}, str, mem);

  td.RegisterVariableForTesting(str_var);
  gen.register_global(str_var);
  mod.AddGlobalVariable(str_var);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), "str.mem");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load) {
  // struct Data {
  //   [[offset(0)]] a : i32;
  //   [[offset(4)]] b : f32;
  // };
  // var<storage_buffer> data : Data;
  // data.b;
  //
  // -> asfloat(data.Load(4));
  ast::type::F32 f32;
  ast::type::I32 i32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "a", &i32, a_deco));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "b", &f32, b_deco));

  auto* str = create<ast::Struct>(members);

  ast::type::Struct s("Data", str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &s,                                 // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  ast::MemberAccessorExpression expr(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("data"),
                                        "data"),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"),
                                        "b"));

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&expr));

  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), "asfloat(data.Load(4))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load_Int) {
  // struct Data {
  //   [[offset(0)]] a : i32;
  //   [[offset(4)]] b : f32;
  // };
  // var<storage_buffer> data : Data;
  // data.a;
  //
  // -> asint(data.Load(0));
  ast::type::F32 f32;
  ast::type::I32 i32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "a", &i32, a_deco));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "b", &f32, b_deco));

  auto* str = create<ast::Struct>(members);

  ast::type::Struct s("Data", str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &s,                                 // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  ast::MemberAccessorExpression expr(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("data"),
                                        "data"),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("a"),
                                        "a"));

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&expr));

  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), "asint(data.Load(0))");
}
TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store_Matrix) {
  // struct Data {
  //   [[offset(0)]] z : f32;
  //   [[offset(4)]] a : mat2x3<f32>;
  // };
  // var<storage_buffer> data : Data;
  // mat2x3<f32> b;
  // data.a = b;
  //
  // -> float3x2 _tint_tmp = b;
  //    data.Store3(4 + 0, asuint(_tint_tmp[0]));
  //    data.Store3(4 + 16, asuint(_tint_tmp[1]));
  ast::type::F32 f32;
  ast::type::I32 i32;
  ast::type::Matrix mat(&f32, 3, 2);

  auto* str = create<ast::Struct>(ast::StructMemberList{
      create<ast::StructMember>(
          Source{}, "z", &i32,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(0, Source{})}),
      create<ast::StructMember>(
          Source{}, "a", &mat,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(4, Source{})}),
  });

  ast::type::Struct s("Data", str);

  auto* b_var =
      create<ast::Variable>(Source{},                        // source
                            "b",                             // name
                            ast::StorageClass::kPrivate,     // storage_class
                            &mat,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &s,                                 // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  auto* lhs = create<ast::MemberAccessorExpression>(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("data"),
                                        "data"),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("a"),
                                        "a"));
  auto* rhs =
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"), "b");

  ast::AssignmentStatement assign(Source{}, lhs, rhs);

  td.RegisterVariableForTesting(coord_var);
  td.RegisterVariableForTesting(b_var);
  gen.register_global(coord_var);
  gen.register_global(b_var);
  mod.AddGlobalVariable(coord_var);
  mod.AddGlobalVariable(b_var);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&assign));

  ASSERT_TRUE(gen.EmitStatement(out, &assign)) << gen.error();
  EXPECT_EQ(result(), R"(float3x2 _tint_tmp = b;
data.Store3(4 + 0, asuint(_tint_tmp[0]));
data.Store3(4 + 16, asuint(_tint_tmp[1]));
)");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store_Matrix_Empty) {
  // struct Data {
  //   [[offset(0)]] z : f32;
  //   [[offset(4)]] a : mat2x3<f32>;
  // };
  // var<storage_buffer> data : Data;
  // data.a = mat2x3<f32>();
  //
  // -> float3x2 _tint_tmp = float3x2(0.0f, 0.0f, 0.0f,
  // 0.0f, 0.0f, 0.0f);
  //    data.Store3(4 + 0, asuint(_tint_tmp[0]);
  //    data.Store3(4 + 16, asuint(_tint_tmp[1]));
  ast::type::F32 f32;
  ast::type::I32 i32;
  ast::type::Matrix mat(&f32, 3, 2);

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "z", &i32, a_deco));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "a", &mat, b_deco));

  auto* str = create<ast::Struct>(members);

  ast::type::Struct s("Data", str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &s,                                 // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  auto* lhs = create<ast::MemberAccessorExpression>(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("data"),
                                        "data"),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("a"),
                                        "a"));
  auto* rhs = create<ast::TypeConstructorExpression>(Source{}, &mat,
                                                     ast::ExpressionList{});

  ast::AssignmentStatement assign(Source{}, lhs, rhs);

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&assign));

  ASSERT_TRUE(gen.EmitStatement(out, &assign)) << gen.error();
  EXPECT_EQ(
      result(),
      R"(float3x2 _tint_tmp = float3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
data.Store3(4 + 0, asuint(_tint_tmp[0]));
data.Store3(4 + 16, asuint(_tint_tmp[1]));
)");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load_Matrix) {
  // struct Data {
  //   [[offset(0)]] z : f32;
  //   [[offset(4)]] a : mat3x2<f32>;
  // };
  // var<storage_buffer> data : Data;
  // data.a;
  //
  // -> asfloat(uint2x3(data.Load2(4 + 0), data.Load2(4 + 8),
  // data.Load2(4 + 16)));
  ast::type::F32 f32;
  ast::type::I32 i32;
  ast::type::Matrix mat(&f32, 2, 3);

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "z", &i32, a_deco));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "a", &mat, b_deco));

  auto* str = create<ast::Struct>(members);

  ast::type::Struct s("Data", str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &s,                                 // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  ast::MemberAccessorExpression expr(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("data"),
                                        "data"),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("a"),
                                        "a"));

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&expr));

  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(),
            "asfloat(uint2x3(data.Load2(4 + 0), data.Load2(4 + 8), "
            "data.Load2(4 + 16)))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load_Matrix_Nested) {
  // struct Data {
  //   [[offset(0)]] z : f32;
  //   [[offset(4)]] a : mat2x3<f32;
  // };
  // struct Outer {
  //   [[offset(0)]] c : f32;
  //   [[offset(4)]] b : Data;
  // };
  // var<storage_buffer> data : Outer;
  // data.b.a;
  //
  // -> asfloat(uint3x2(data.Load3(4 + 0), data.Load3(4 + 16)));
  ast::type::F32 f32;
  ast::type::I32 i32;
  ast::type::Matrix mat(&f32, 3, 2);

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "z", &i32, a_deco));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "a", &mat, b_deco));

  auto* str = create<ast::Struct>(members);

  ast::type::Struct s("Data", str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &s,                                 // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  ast::MemberAccessorExpression expr(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("data"),
                                        "data"),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("a"),
                                        "a"));

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&expr));

  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(),
            "asfloat(uint3x2(data.Load3(4 + 0), data.Load3(4 + 16)))");
}

TEST_F(
    HlslGeneratorImplTest_MemberAccessor,
    EmitExpression_MemberAccessor_StorageBuffer_Load_Matrix_By3_Is_16_Bytes) {
  // struct Data {
  //   [[offset(4)]] a : mat3x3<f32;
  // };
  // var<storage_buffer> data : Data;
  // data.a;
  //
  // -> asfloat(uint3x3(data.Load3(0), data.Load3(16),
  // data.Load3(32)));
  ast::type::F32 f32;
  ast::type::I32 i32;
  ast::type::Matrix mat(&f32, 3, 3);

  ast::StructMemberList members;
  ast::StructMemberDecorationList deco;
  deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "a", &mat, deco));

  auto* str = create<ast::Struct>(members);

  ast::type::Struct s("Data", str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &s,                                 // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  ast::MemberAccessorExpression expr(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("data"),
                                        "data"),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("a"),
                                        "a"));

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&expr));

  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(),
            "asfloat(uint3x3(data.Load3(0 + 0), data.Load3(0 + 16), "
            "data.Load3(0 + 32)))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load_Matrix_Single_Element) {
  // struct Data {
  //   [[offset(0)]] z : f32;
  //   [[offset(16)]] a : mat4x3<f32>;
  // };
  // var<storage_buffer> data : Data;
  // data.a[2][1];
  //
  // -> asfloat(data.Load((2 * 16) + (1 * 4) + 16)))
  ast::type::F32 f32;
  ast::type::I32 i32;
  ast::type::Matrix mat(&f32, 3, 4);

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "z", &i32, a_deco));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(16, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "a", &mat, b_deco));

  auto* str = create<ast::Struct>(members);

  ast::type::Struct s("Data", str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &s,                                 // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  ast::ArrayAccessorExpression expr(
      Source{},
      create<ast::ArrayAccessorExpression>(
          Source{},
          create<ast::MemberAccessorExpression>(
              Source{},
              create<ast::IdentifierExpression>(
                  Source{}, mod.RegisterSymbol("data"), "data"),
              create<ast::IdentifierExpression>(Source{},
                                                mod.RegisterSymbol("a"), "a")),
          create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::SintLiteral>(Source{}, &i32, 2))),
      create<ast::ScalarConstructorExpression>(
          Source{}, create<ast::SintLiteral>(Source{}, &i32, 1)));

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&expr));

  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), "asfloat(data.Load((4 * 1) + (16 * 2) + 16))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_ArrayAccessor_StorageBuffer_Load_Int_FromArray) {
  // struct Data {
  //   [[offset(0)]] a : [[stride(4)]] array<i32, 5>;
  // };
  // var<storage_buffer> data : Data;
  // data.a[2];
  //
  // -> asint(data.Load((2 * 4));
  ast::type::F32 f32;
  ast::type::I32 i32;
  ast::type::Array ary(&i32, 5,
                       ast::ArrayDecorationList{
                           create<ast::StrideDecoration>(4, Source{}),
                       });

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "a", &ary, a_deco));

  auto* str = create<ast::Struct>(members);

  ast::type::Struct s("Data", str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &s,                                 // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  ast::ArrayAccessorExpression expr(
      Source{},
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::IdentifierExpression>(Source{},
                                            mod.RegisterSymbol("data"), "data"),
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("a"),
                                            "a")),
      create<ast::ScalarConstructorExpression>(
          Source{}, create<ast::SintLiteral>(Source{}, &i32, 2)));

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), "asint(data.Load((4 * 2) + 0))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_ArrayAccessor_StorageBuffer_Load_Int_FromArray_ExprIdx) {
  // struct Data {
  //   [[offset(0)]] a : [[stride(4)]] array<i32, 5>;
  // };
  // var<storage_buffer> data : Data;
  // data.a[(2 + 4) - 3];
  //
  // -> asint(data.Load((4 * ((2 + 4) - 3)));
  ast::type::F32 f32;
  ast::type::I32 i32;
  ast::type::Array ary(&i32, 5,
                       ast::ArrayDecorationList{
                           create<ast::StrideDecoration>(4, Source{}),
                       });

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "a", &ary, a_deco));

  auto* str = create<ast::Struct>(members);

  ast::type::Struct s("Data", str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &s,                                 // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  ast::ArrayAccessorExpression expr(
      Source{},
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::IdentifierExpression>(Source{},
                                            mod.RegisterSymbol("data"), "data"),
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("a"),
                                            "a")),
      create<ast::BinaryExpression>(
          Source{}, ast::BinaryOp::kSubtract,
          create<ast::BinaryExpression>(
              Source{}, ast::BinaryOp::kAdd,
              create<ast::ScalarConstructorExpression>(
                  Source{}, create<ast::SintLiteral>(Source{}, &i32, 2)),
              create<ast::ScalarConstructorExpression>(
                  Source{}, create<ast::SintLiteral>(Source{}, &i32, 4))),
          create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::SintLiteral>(Source{}, &i32, 3))));

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), "asint(data.Load((4 * ((2 + 4) - 3)) + 0))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store) {
  // struct Data {
  //   [[offset(0)]] a : i32;
  //   [[offset(4)]] b : f32;
  // };
  // var<storage_buffer> data : Data;
  // data.b = 2.3f;
  //
  // -> data.Store(0, asuint(2.0f));

  ast::type::F32 f32;
  ast::type::I32 i32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "a", &i32, a_deco));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "b", &f32, b_deco));

  auto* str = create<ast::Struct>(members);

  ast::type::Struct s("Data", str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &s,                                 // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();

  auto* lhs = create<ast::MemberAccessorExpression>(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("data"),
                                        "data"),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"),
                                        "b"));
  auto* rhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 2.0f));
  ast::AssignmentStatement assign(Source{}, lhs, rhs);

  ASSERT_TRUE(td.DetermineResultType(&assign));
  ASSERT_TRUE(gen.EmitStatement(out, &assign)) << gen.error();
  EXPECT_EQ(result(), R"(data.Store(4, asuint(2.0f));
)");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store_ToArray) {
  // struct Data {
  //   [[offset(0)]] a : [[stride(4)]] array<i32, 5>;
  // };
  // var<storage_buffer> data : Data;
  // data.a[2] = 2;
  //
  // -> data.Store((2 * 4), asuint(2.3f));

  ast::type::F32 f32;
  ast::type::I32 i32;
  ast::type::Array ary(&i32, 5,
                       ast::ArrayDecorationList{
                           create<ast::StrideDecoration>(4, Source{}),
                       });

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "a", &ary, a_deco));

  auto* str = create<ast::Struct>(members);

  ast::type::Struct s("Data", str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &s,                                 // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();

  auto* lhs = create<ast::ArrayAccessorExpression>(
      Source{},
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::IdentifierExpression>(Source{},
                                            mod.RegisterSymbol("data"), "data"),
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("a"),
                                            "a")),
      create<ast::ScalarConstructorExpression>(
          Source{}, create<ast::SintLiteral>(Source{}, &i32, 2)));
  auto* rhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::SintLiteral>(Source{}, &i32, 2));
  ast::AssignmentStatement assign(Source{}, lhs, rhs);

  ASSERT_TRUE(td.DetermineResultType(&assign)) << td.error();
  ASSERT_TRUE(gen.EmitStatement(out, &assign)) << gen.error();
  EXPECT_EQ(result(), R"(data.Store((4 * 2) + 0, asuint(2));
)");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store_Int) {
  // struct Data {
  //   [[offset(0)]] a : i32;
  //   [[offset(4)]] b : f32;
  // };
  // var<storage_buffer> data : Data;
  // data.a = 2;
  //
  // -> data.Store(0, asuint(2));

  ast::type::F32 f32;
  ast::type::I32 i32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "a", &i32, a_deco));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "b", &f32, b_deco));

  auto* str = create<ast::Struct>(members);

  ast::type::Struct s("Data", str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &s,                                 // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();

  auto* lhs = create<ast::MemberAccessorExpression>(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("data"),
                                        "data"),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("a"),
                                        "a"));
  auto* rhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::SintLiteral>(Source{}, &i32, 2));
  ast::AssignmentStatement assign(Source{}, lhs, rhs);

  ASSERT_TRUE(td.DetermineResultType(&assign));
  ASSERT_TRUE(gen.EmitStatement(out, &assign)) << gen.error();
  EXPECT_EQ(result(), R"(data.Store(0, asuint(2));
)");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load_Vec3) {
  // struct Data {
  //   [[offset(0)]] a : vec3<i32>;
  //   [[offset(16)]] b : vec3<f32>;
  // };
  // var<storage_buffer> data : Data;
  // data.b;
  //
  // -> asfloat(data.Load(16));

  ast::type::F32 f32;
  ast::type::I32 i32;
  ast::type::Vector ivec3(&i32, 3);
  ast::type::Vector fvec3(&f32, 3);

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "a", &ivec3, a_deco));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(16, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "b", &fvec3, b_deco));

  auto* str = create<ast::Struct>(members);

  ast::type::Struct s("Data", str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &s,                                 // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();

  ast::MemberAccessorExpression expr(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("data"),
                                        "data"),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"),
                                        "b"));

  ASSERT_TRUE(td.DetermineResultType(&expr));
  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), "asfloat(data.Load3(16))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store_Vec3) {
  // struct Data {
  //   [[offset(0)]] a : vec3<i32>;
  //   [[offset(16)]] b : vec3<f32>;
  // };
  // var<storage_buffer> data : Data;
  // data.b = vec3<f32>(2.3f, 1.2f, 0.2f);
  //
  // -> data.Store(16, asuint(float3(2.3f, 1.2f, 0.2f)));

  ast::type::F32 f32;
  ast::type::I32 i32;
  ast::type::Vector ivec3(&i32, 3);
  ast::type::Vector fvec3(&f32, 3);

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "a", &ivec3, a_deco));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(16, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "b", &fvec3, b_deco));

  auto* str = create<ast::Struct>(members);

  ast::type::Struct s("Data", str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &s,                                 // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();

  auto* lit1 = create<ast::FloatLiteral>(Source{}, &f32, 1.f);
  auto* lit2 = create<ast::FloatLiteral>(Source{}, &f32, 2.f);
  auto* lit3 = create<ast::FloatLiteral>(Source{}, &f32, 3.f);
  ast::ExpressionList values;
  values.push_back(create<ast::ScalarConstructorExpression>(Source{}, lit1));
  values.push_back(create<ast::ScalarConstructorExpression>(Source{}, lit2));
  values.push_back(create<ast::ScalarConstructorExpression>(Source{}, lit3));

  auto* lhs = create<ast::MemberAccessorExpression>(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("data"),
                                        "data"),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"),
                                        "b"));
  auto* rhs = create<ast::TypeConstructorExpression>(Source{}, &fvec3, values);

  ast::AssignmentStatement assign(Source{}, lhs, rhs);

  ASSERT_TRUE(td.DetermineResultType(&assign));
  ASSERT_TRUE(gen.EmitStatement(out, &assign)) << gen.error();
  EXPECT_EQ(result(),
            R"(data.Store3(16, asuint(float3(1.0f, 2.0f, 3.0f)));
)");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load_MultiLevel) {
  // struct Data {
  //   [[offset(0)]] a : vec3<i32>;
  //   [[offset(16)]] b : vec3<f32>;
  // };
  // struct Pre {
  //   var c : [[stride(32)]] array<Data, 4>;
  // };
  //
  // var<storage_buffer> data : Pre;
  // data.c[2].b
  //
  // -> asfloat(data.Load3(16 + (2 * 32)))

  ast::type::F32 f32;
  ast::type::I32 i32;
  ast::type::Vector ivec3(&i32, 3);
  ast::type::Vector fvec3(&f32, 3);

  auto* data_str = create<ast::Struct>(ast::StructMemberList{
      create<ast::StructMember>(
          Source{}, "a", &ivec3,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(0, Source{})}),
      create<ast::StructMember>(
          Source{}, "b", &fvec3,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(16, Source{})}),
  });

  ast::type::Struct data("Data", data_str);

  ast::type::Array ary(&data, 4,
                       ast::ArrayDecorationList{
                           create<ast::StrideDecoration>(32, Source{}),
                       });

  auto* pre_str = create<ast::Struct>(ast::StructMemberList{
      create<ast::StructMember>(
          Source{}, "c", &ary,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(0, Source{})}),
  });

  ast::type::Struct pre_struct("Pre", pre_str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &pre_struct,                        // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();

  ast::MemberAccessorExpression expr(
      Source{},
      create<ast::ArrayAccessorExpression>(
          Source{},
          create<ast::MemberAccessorExpression>(
              Source{},
              create<ast::IdentifierExpression>(
                  Source{}, mod.RegisterSymbol("data"), "data"),
              create<ast::IdentifierExpression>(Source{},
                                                mod.RegisterSymbol("c"), "c")),
          create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::SintLiteral>(Source{}, &i32, 2))),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"),
                                        "b"));

  ASSERT_TRUE(td.DetermineResultType(&expr));
  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), "asfloat(data.Load3(16 + (32 * 2) + 0))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load_MultiLevel_Swizzle) {
  // struct Data {
  //   [[offset(0)]] a : vec3<i32>;
  //   [[offset(16)]] b : vec3<f32>;
  // };
  // struct Pre {
  //   var c : [[stride(32)]] array<Data, 4>;
  // };
  //
  // var<storage_buffer> data : Pre;
  // data.c[2].b.xy
  //
  // -> asfloat(data.Load3(16 + (2 * 32))).xy

  ast::type::F32 f32;
  ast::type::I32 i32;
  ast::type::Vector ivec3(&i32, 3);
  ast::type::Vector fvec3(&f32, 3);

  ast::StructMemberList members;
  ast::StructMemberDecorationList deco;

  auto* data_str = create<ast::Struct>(ast::StructMemberList{
      create<ast::StructMember>(
          Source{}, "a", &ivec3,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(0, Source{})}),
      create<ast::StructMember>(
          Source{}, "b", &fvec3,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(16, Source{})}),
  });

  ast::type::Struct data("Data", data_str);

  ast::type::Array ary(
      &data, 4,
      ast::ArrayDecorationList{create<ast::StrideDecoration>(32, Source{})});

  auto* pre_str =
      create<ast::Struct>(ast::StructMemberList{create<ast::StructMember>(
          Source{}, "c", &ary,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(0, Source{})})});

  ast::type::Struct pre_struct("Pre", pre_str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &pre_struct,                        // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();

  ast::MemberAccessorExpression expr(
      Source{},
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::ArrayAccessorExpression>(
              Source{},
              create<ast::MemberAccessorExpression>(
                  Source{},
                  create<ast::IdentifierExpression>(
                      Source{}, mod.RegisterSymbol("data"), "data"),
                  create<ast::IdentifierExpression>(
                      Source{}, mod.RegisterSymbol("c"), "c")),
              create<ast::ScalarConstructorExpression>(
                  Source{}, create<ast::SintLiteral>(Source{}, &i32, 2))),
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"),
                                            "b")),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("xy"),
                                        "xy"));

  ASSERT_TRUE(td.DetermineResultType(&expr));
  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), "asfloat(data.Load3(16 + (32 * 2) + 0)).xy");
}

TEST_F(
    HlslGeneratorImplTest_MemberAccessor,
    EmitExpression_MemberAccessor_StorageBuffer_Load_MultiLevel_Swizzle_SingleLetter) {  // NOLINT
  // struct Data {
  //   [[offset(0)]] a : vec3<i32>;
  //   [[offset(16)]] b : vec3<f32>;
  // };
  // struct Pre {
  //   var c : [[stride(32)]] array<Data, 4>;
  // };
  //
  // var<storage_buffer> data : Pre;
  // data.c[2].b.g
  //
  // -> asfloat(data.Load((4 * 1) + 16 + (2 * 32) + 0))

  ast::type::F32 f32;
  ast::type::I32 i32;
  ast::type::Vector ivec3(&i32, 3);
  ast::type::Vector fvec3(&f32, 3);

  auto* data_str = create<ast::Struct>(ast::StructMemberList{
      create<ast::StructMember>(
          Source{}, "a", &ivec3,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(0, Source{})}),
      create<ast::StructMember>(
          Source{}, "b", &fvec3,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(16, Source{})}),
  });

  ast::type::Struct data("Data", data_str);

  ast::type::Array ary(&data, 4,
                       ast::ArrayDecorationList{
                           create<ast::StrideDecoration>(32, Source{}),
                       });

  auto* pre_str =
      create<ast::Struct>(ast::StructMemberList{create<ast::StructMember>(
          Source{}, "c", &ary,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(0, Source{})})});

  ast::type::Struct pre_struct("Pre", pre_str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &pre_struct,                        // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();

  ast::MemberAccessorExpression expr(
      Source{},
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::ArrayAccessorExpression>(
              Source{},
              create<ast::MemberAccessorExpression>(
                  Source{},
                  create<ast::IdentifierExpression>(
                      Source{}, mod.RegisterSymbol("data"), "data"),
                  create<ast::IdentifierExpression>(
                      Source{}, mod.RegisterSymbol("c"), "c")),
              create<ast::ScalarConstructorExpression>(
                  Source{}, create<ast::SintLiteral>(Source{}, &i32, 2))),
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"),
                                            "b")),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("g"),
                                        "g"));

  ASSERT_TRUE(td.DetermineResultType(&expr));
  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), "asfloat(data.Load((4 * 1) + 16 + (32 * 2) + 0))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load_MultiLevel_Index) {
  // struct Data {
  //   [[offset(0)]] a : vec3<i32>;
  //   [[offset(16)]] b : vec3<f32>;
  // };
  // struct Pre {
  //   var c : [[stride(32)]] array<Data, 4>;
  // };
  //
  // var<storage_buffer> data : Pre;
  // data.c[2].b[1]
  //
  // -> asfloat(data.Load(4 + 16 + (2 * 32)))

  ast::type::F32 f32;
  ast::type::I32 i32;
  ast::type::Vector ivec3(&i32, 3);
  ast::type::Vector fvec3(&f32, 3);

  auto* data_str = create<ast::Struct>(ast::StructMemberList{
      create<ast::StructMember>(
          Source{}, "a", &ivec3,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(0, Source{})}),
      create<ast::StructMember>(
          Source{}, "b", &fvec3,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(16, Source{})}),
  });

  ast::type::Struct data("Data", data_str);

  ast::type::Array ary(&data, 4,
                       ast::ArrayDecorationList{
                           create<ast::StrideDecoration>(32, Source{}),
                       });

  auto* pre_str =
      create<ast::Struct>(ast::StructMemberList{create<ast::StructMember>(
          Source{}, "c", &ary,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(0, Source{})})});

  ast::type::Struct pre_struct("Pre", pre_str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &pre_struct,                        // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();

  ast::ArrayAccessorExpression expr(
      Source{},
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::ArrayAccessorExpression>(
              Source{},
              create<ast::MemberAccessorExpression>(
                  Source{},
                  create<ast::IdentifierExpression>(
                      Source{}, mod.RegisterSymbol("data"), "data"),
                  create<ast::IdentifierExpression>(
                      Source{}, mod.RegisterSymbol("c"), "c")),
              create<ast::ScalarConstructorExpression>(
                  Source{}, create<ast::SintLiteral>(Source{}, &i32, 2))),
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"),
                                            "b")),
      create<ast::ScalarConstructorExpression>(
          Source{}, create<ast::SintLiteral>(Source{}, &i32, 1)));

  ASSERT_TRUE(td.DetermineResultType(&expr));
  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), "asfloat(data.Load((4 * 1) + 16 + (32 * 2) + 0))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store_MultiLevel) {
  // struct Data {
  //   [[offset(0)]] a : vec3<i32>;
  //   [[offset(16)]] b : vec3<f32>;
  // };
  // struct Pre {
  //   var c : [[stride(32)]] array<Data, 4>;
  // };
  //
  // var<storage_buffer> data : Pre;
  // data.c[2].b = vec3<f32>(1.f, 2.f, 3.f);
  //
  // -> data.Store3(16 + (2 * 32), asuint(float3(1.0f, 2.0f, 3.0f)));

  ast::type::F32 f32;
  ast::type::I32 i32;
  ast::type::Vector ivec3(&i32, 3);
  ast::type::Vector fvec3(&f32, 3);

  auto* data_str = create<ast::Struct>(ast::StructMemberList{
      create<ast::StructMember>(
          Source{}, "a", &ivec3,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(0, Source{})}),
      create<ast::StructMember>(
          Source{}, "b", &fvec3,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(16, Source{})}),
  });

  ast::type::Struct data("Data", data_str);

  ast::type::Array ary(&data, 4,
                       ast::ArrayDecorationList{
                           create<ast::StrideDecoration>(32, Source{}),
                       });

  auto* pre_str =
      create<ast::Struct>(ast::StructMemberList{create<ast::StructMember>(
          Source{}, "c", &ary,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(0, Source{})})});

  ast::type::Struct pre_struct("Pre", pre_str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &pre_struct,                        // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();

  auto* lhs = create<ast::MemberAccessorExpression>(
      Source{},
      create<ast::ArrayAccessorExpression>(
          Source{},
          create<ast::MemberAccessorExpression>(
              Source{},
              create<ast::IdentifierExpression>(
                  Source{}, mod.RegisterSymbol("data"), "data"),
              create<ast::IdentifierExpression>(Source{},
                                                mod.RegisterSymbol("c"), "c")),
          create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::SintLiteral>(Source{}, &i32, 2))),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"),
                                        "b"));

  auto* lit1 = create<ast::FloatLiteral>(Source{}, &f32, 1.f);
  auto* lit2 = create<ast::FloatLiteral>(Source{}, &f32, 2.f);
  auto* lit3 = create<ast::FloatLiteral>(Source{}, &f32, 3.f);
  ast::ExpressionList values;
  values.push_back(create<ast::ScalarConstructorExpression>(Source{}, lit1));
  values.push_back(create<ast::ScalarConstructorExpression>(Source{}, lit2));
  values.push_back(create<ast::ScalarConstructorExpression>(Source{}, lit3));

  auto* rhs = create<ast::TypeConstructorExpression>(Source{}, &fvec3, values);

  ast::AssignmentStatement assign(Source{}, lhs, rhs);

  ASSERT_TRUE(td.DetermineResultType(&assign));
  ASSERT_TRUE(gen.EmitStatement(out, &assign)) << gen.error();
  EXPECT_EQ(result(),
            R"(data.Store3(16 + (32 * 2) + 0, asuint(float3(1.0f, 2.0f, 3.0f)));
)");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store_Swizzle_SingleLetter) {
  // struct Data {
  //   [[offset(0)]] a : vec3<i32>;
  //   [[offset(16)]] b : vec3<f32>;
  // };
  // struct Pre {
  //   var c : [[stride(32)]] array<Data, 4>;
  // };
  //
  // var<storage_buffer> data : Pre;
  // data.c[2].b.y = 1.f;
  //
  // -> data.Store((4 * 1) + 16 + (2 * 32) + 0, asuint(1.0f));

  ast::type::F32 f32;
  ast::type::I32 i32;
  ast::type::Vector ivec3(&i32, 3);
  ast::type::Vector fvec3(&f32, 3);

  auto* data_str = create<ast::Struct>(ast::StructMemberList{
      create<ast::StructMember>(
          Source{}, "a", &ivec3,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(0, Source{})}),
      create<ast::StructMember>(
          Source{}, "b", &fvec3,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(16, Source{})}),
  });

  ast::type::Struct data("Data", data_str);

  ast::type::Array ary(&data, 4,
                       ast::ArrayDecorationList{
                           create<ast::StrideDecoration>(32, Source{}),
                       });

  auto* pre_str =
      create<ast::Struct>(ast::StructMemberList{create<ast::StructMember>(
          Source{}, "c", &ary,
          ast::StructMemberDecorationList{
              create<ast::StructMemberOffsetDecoration>(0, Source{})})});

  ast::type::Struct pre_struct("Pre", pre_str);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &pre_struct,                        // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations

  td.RegisterVariableForTesting(coord_var);
  gen.register_global(coord_var);
  mod.AddGlobalVariable(coord_var);

  ASSERT_TRUE(td.Determine()) << td.error();

  auto* lhs = create<ast::MemberAccessorExpression>(
      Source{},
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::ArrayAccessorExpression>(
              Source{},
              create<ast::MemberAccessorExpression>(
                  Source{},
                  create<ast::IdentifierExpression>(
                      Source{}, mod.RegisterSymbol("data"), "data"),
                  create<ast::IdentifierExpression>(
                      Source{}, mod.RegisterSymbol("c"), "c")),
              create<ast::ScalarConstructorExpression>(
                  Source{}, create<ast::SintLiteral>(Source{}, &i32, 2))),
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"),
                                            "b")),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("y"),
                                        "y"));

  auto* rhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &i32, 1.f));

  ast::AssignmentStatement assign(Source{}, lhs, rhs);

  ASSERT_TRUE(td.DetermineResultType(&assign));
  ASSERT_TRUE(gen.EmitStatement(out, &assign)) << gen.error();
  EXPECT_EQ(result(),
            R"(data.Store((4 * 1) + 16 + (32 * 2) + 0, asuint(1.0f));
)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
