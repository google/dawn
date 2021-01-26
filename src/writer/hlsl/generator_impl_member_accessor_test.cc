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
#include "src/ast/identifier_expression.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/stride_decoration.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/variable.h"
#include "src/program.h"
#include "src/type/struct_type.h"
#include "src/type/vector_type.h"
#include "src/type_determiner.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_MemberAccessor = TestHelper;

TEST_F(HlslGeneratorImplTest_MemberAccessor, EmitExpression_MemberAccessor) {
  auto* strct = create<ast::Struct>(
      ast::StructMemberList{Member("mem", ty.f32, {MemberOffset(0)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Str", strct);
  auto* str_var = Var("str", ast::StorageClass::kPrivate, s);
  mod->AST().AddGlobalVariable(str_var);

  auto* expr = MemberAccessor("str", "mem");

  td.RegisterVariableForTesting(str_var);
  ASSERT_TRUE(td.DetermineResultType(expr)) << td.error();

  GeneratorImpl& gen = Build();

  gen.register_global(str_var);

  ASSERT_TRUE(gen.EmitExpression(pre, out, expr)) << gen.error();
  EXPECT_EQ(result(), "str.mem");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load) {
  // struct Data {
  //   [[offset(0)]] a : i32;
  //   [[offset(4)]] b : f32;
  // };
  // var<storage> data : Data;
  // data.b;
  //
  // -> asfloat(data.Load(4));

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32, {MemberOffset(0)}),
                            Member("b", ty.f32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, s);
  mod->AST().AddGlobalVariable(coord_var);
  td.RegisterVariableForTesting(coord_var);

  auto* expr = MemberAccessor("data", "b");

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(expr));

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitExpression(pre, out, expr)) << gen.error();
  EXPECT_EQ(result(), "asfloat(data.Load(4))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load_Int) {
  // struct Data {
  //   [[offset(0)]] a : i32;
  //   [[offset(4)]] b : f32;
  // };
  // var<storage> data : Data;
  // data.a;
  //
  // -> asint(data.Load(0));

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32, {MemberOffset(0)}),
                            Member("b", ty.f32, {MemberOffset(4)})},
      ast::StructDecorationList{});
  auto* s = ty.struct_("Data", str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, s);
  mod->AST().AddGlobalVariable(coord_var);
  td.RegisterVariableForTesting(coord_var);

  auto* expr = MemberAccessor("data", "a");

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(expr));

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitExpression(pre, out, expr)) << gen.error();
  EXPECT_EQ(result(), "asint(data.Load(0))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store_Matrix) {
  // struct Data {
  //   [[offset(0)]] z : f32;
  //   [[offset(4)]] a : mat2x3<f32>;
  // };
  // var<storage> data : Data;
  // mat2x3<f32> b;
  // data.a = b;
  //
  // -> float3x2 _tint_tmp = b;
  //    data.Store3(4 + 0, asuint(_tint_tmp[0]));
  //    data.Store3(4 + 16, asuint(_tint_tmp[1]));

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("z", ty.i32, {MemberOffset(0)}),
                            Member("a", ty.mat2x3<f32>(), {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  auto* b_var = Var("b", ast::StorageClass::kPrivate, ty.mat2x3<f32>());
  mod->AST().AddGlobalVariable(b_var);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, s);
  mod->AST().AddGlobalVariable(coord_var);

  auto* lhs = MemberAccessor("data", "a");
  auto* rhs = Expr("b");

  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);

  td.RegisterVariableForTesting(coord_var);
  td.RegisterVariableForTesting(b_var);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(assign));

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);
  gen.register_global(b_var);

  ASSERT_TRUE(gen.EmitStatement(out, assign)) << gen.error();
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
  // var<storage> data : Data;
  // data.a = mat2x3<f32>();
  //
  // -> float3x2 _tint_tmp = float3x2(0.0f, 0.0f, 0.0f,
  // 0.0f, 0.0f, 0.0f);
  //    data.Store3(4 + 0, asuint(_tint_tmp[0]);
  //    data.Store3(4 + 16, asuint(_tint_tmp[1]));

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("z", ty.i32, {MemberOffset(0)}),
                            Member("a", ty.mat2x3<f32>(), {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, s);
  mod->AST().AddGlobalVariable(coord_var);
  td.RegisterVariableForTesting(coord_var);

  auto* lhs = MemberAccessor("data", "a");
  auto* rhs = Construct(ty.mat2x3<f32>(), ast::ExpressionList{});

  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(assign));

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitStatement(out, assign)) << gen.error();
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
  // var<storage> data : Data;
  // data.a;
  //
  // -> asfloat(uint2x3(data.Load2(4 + 0), data.Load2(4 + 8),
  // data.Load2(4 + 16)));

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("z", ty.i32, {MemberOffset(0)}),
                            Member("a", ty.mat3x2<f32>(), {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, s);
  mod->AST().AddGlobalVariable(coord_var);
  td.RegisterVariableForTesting(coord_var);

  auto* expr = MemberAccessor("data", "a");

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(expr));

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitExpression(pre, out, expr)) << gen.error();
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
  // var<storage> data : Outer;
  // data.b.a;
  //
  // -> asfloat(uint3x2(data.Load3(4 + 0), data.Load3(4 + 16)));

  auto* str = create<ast::Struct>(
      ast::StructMemberList{
          Member("z", ty.i32, {MemberOffset(0)}),
          Member("a", ty.mat2x3<f32>(), {MemberOffset(4)}),
      },
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, s);
  mod->AST().AddGlobalVariable(coord_var);
  td.RegisterVariableForTesting(coord_var);

  auto* expr = MemberAccessor("data", "a");

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(expr));

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitExpression(pre, out, expr)) << gen.error();
  EXPECT_EQ(result(),
            "asfloat(uint3x2(data.Load3(4 + 0), data.Load3(4 + 16)))");
}

TEST_F(
    HlslGeneratorImplTest_MemberAccessor,
    EmitExpression_MemberAccessor_StorageBuffer_Load_Matrix_By3_Is_16_Bytes) {
  // struct Data {
  //   [[offset(4)]] a : mat3x3<f32;
  // };
  // var<storage> data : Data;
  // data.a;
  //
  // -> asfloat(uint3x3(data.Load3(0), data.Load3(16),
  // data.Load3(32)));

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.mat3x3<f32>(), {MemberOffset(0)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, s);
  mod->AST().AddGlobalVariable(coord_var);
  td.RegisterVariableForTesting(coord_var);

  auto* expr = MemberAccessor("data", "a");

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(expr));

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitExpression(pre, out, expr)) << gen.error();
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
  // var<storage> data : Data;
  // data.a[2][1];
  //
  // -> asfloat(data.Load((2 * 16) + (1 * 4) + 16)))

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("z", ty.i32, {MemberOffset(0)}),
                            Member("a", ty.mat4x3<f32>(), {MemberOffset(16)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, s);
  mod->AST().AddGlobalVariable(coord_var);
  td.RegisterVariableForTesting(coord_var);

  auto* expr = IndexAccessor(
      IndexAccessor(MemberAccessor("data", "a"), Expr(2)), Expr(1));

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(expr));

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitExpression(pre, out, expr)) << gen.error();
  EXPECT_EQ(result(), "asfloat(data.Load((4 * 1) + (16 * 2) + 16))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_ArrayAccessor_StorageBuffer_Load_Int_FromArray) {
  // struct Data {
  //   [[offset(0)]] a : [[stride(4)]] array<i32, 5>;
  // };
  // var<storage> data : Data;
  // data.a[2];
  //
  // -> asint(data.Load((2 * 4));
  type::Array ary(ty.i32, 5,
                  ast::ArrayDecorationList{
                      create<ast::StrideDecoration>(4),
                  });

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", &ary, {MemberOffset(0)})},
      ast::StructDecorationList{});
  auto* s = ty.struct_("Data", str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, s);
  mod->AST().AddGlobalVariable(coord_var);
  td.RegisterVariableForTesting(coord_var);

  auto* expr = IndexAccessor(MemberAccessor("data", "a"), Expr(2));

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(expr)) << td.error();

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitExpression(pre, out, expr)) << gen.error();
  EXPECT_EQ(result(), "asint(data.Load((4 * 2) + 0))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_ArrayAccessor_StorageBuffer_Load_Int_FromArray_ExprIdx) {
  // struct Data {
  //   [[offset(0)]] a : [[stride(4)]] array<i32, 5>;
  // };
  // var<storage> data : Data;
  // data.a[(2 + 4) - 3];
  //
  // -> asint(data.Load((4 * ((2 + 4) - 3)));
  type::Array ary(ty.i32, 5,
                  ast::ArrayDecorationList{
                      create<ast::StrideDecoration>(4),
                  });

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", &ary, {MemberOffset(0)})},
      ast::StructDecorationList{});
  auto* s = ty.struct_("Data", str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, s);
  mod->AST().AddGlobalVariable(coord_var);
  td.RegisterVariableForTesting(coord_var);

  auto* expr = IndexAccessor(MemberAccessor("data", "a"),
                             Sub(Add(Expr(2), Expr(4)), Expr(3)));

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(expr)) << td.error();

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitExpression(pre, out, expr)) << gen.error();
  EXPECT_EQ(result(), "asint(data.Load((4 * ((2 + 4) - 3)) + 0))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store) {
  // struct Data {
  //   [[offset(0)]] a : i32;
  //   [[offset(4)]] b : f32;
  // };
  // var<storage> data : Data;
  // data.b = 2.3f;
  //
  // -> data.Store(0, asuint(2.0f));

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32, {MemberOffset(0)}),
                            Member("b", ty.f32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, s);
  mod->AST().AddGlobalVariable(coord_var);
  td.RegisterVariableForTesting(coord_var);

  auto* lhs = MemberAccessor("data", "b");
  auto* rhs = Expr(2.0f);
  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(assign));

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitStatement(out, assign)) << gen.error();
  EXPECT_EQ(result(), R"(data.Store(4, asuint(2.0f));
)");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store_ToArray) {
  // struct Data {
  //   [[offset(0)]] a : [[stride(4)]] array<i32, 5>;
  // };
  // var<storage> data : Data;
  // data.a[2] = 2;
  //
  // -> data.Store((2 * 4), asuint(2.3f));

  type::Array ary(ty.i32, 5,
                  ast::ArrayDecorationList{
                      create<ast::StrideDecoration>(4),
                  });

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", &ary, {MemberOffset(0)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, s);
  mod->AST().AddGlobalVariable(coord_var);
  td.RegisterVariableForTesting(coord_var);

  auto* lhs = IndexAccessor(MemberAccessor("data", "a"), Expr(2));
  auto* rhs = Expr(2);
  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(assign)) << td.error();

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitStatement(out, assign)) << gen.error();
  EXPECT_EQ(result(), R"(data.Store((4 * 2) + 0, asuint(2));
)");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store_Int) {
  // struct Data {
  //   [[offset(0)]] a : i32;
  //   [[offset(4)]] b : f32;
  // };
  // var<storage> data : Data;
  // data.a = 2;
  //
  // -> data.Store(0, asuint(2));

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32, {MemberOffset(0)}),
                            Member("b", ty.f32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, s);
  mod->AST().AddGlobalVariable(coord_var);
  td.RegisterVariableForTesting(coord_var);

  auto* lhs = MemberAccessor("data", "a");
  auto* rhs = Expr(2);
  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(assign));

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitStatement(out, assign)) << gen.error();
  EXPECT_EQ(result(), R"(data.Store(0, asuint(2));
)");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Load_Vec3) {
  // struct Data {
  //   [[offset(0)]] a : vec3<i32>;
  //   [[offset(16)]] b : vec3<f32>;
  // };
  // var<storage> data : Data;
  // data.b;
  //
  // -> asfloat(data.Load(16));

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.vec3<i32>(), {MemberOffset(0)}),
                            Member("b", ty.vec3<f32>(), {MemberOffset(16)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, s);
  mod->AST().AddGlobalVariable(coord_var);
  td.RegisterVariableForTesting(coord_var);

  auto* expr = MemberAccessor("data", "b");

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(expr));

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitExpression(pre, out, expr)) << gen.error();
  EXPECT_EQ(result(), "asfloat(data.Load3(16))");
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_MemberAccessor_StorageBuffer_Store_Vec3) {
  // struct Data {
  //   [[offset(0)]] a : vec3<i32>;
  //   [[offset(16)]] b : vec3<f32>;
  // };
  // var<storage> data : Data;
  // data.b = vec3<f32>(2.3f, 1.2f, 0.2f);
  //
  // -> data.Store(16, asuint(float3(2.3f, 1.2f, 0.2f)));

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.vec3<i32>(), {MemberOffset(0)}),
                            Member("b", ty.vec3<f32>(), {MemberOffset(16)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, s);
  mod->AST().AddGlobalVariable(coord_var);
  td.RegisterVariableForTesting(coord_var);

  auto* lhs = MemberAccessor("data", "b");
  auto* rhs = vec3<f32>(1.f, 2.f, 3.f);

  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(assign));

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitStatement(out, assign)) << gen.error();
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
  // var<storage> data : Pre;
  // data.c[2].b
  //
  // -> asfloat(data.Load3(16 + (2 * 32)))

  auto* data_str = create<ast::Struct>(
      ast::StructMemberList{
          Member("a", ty.vec3<i32>(), {MemberOffset(0)}),
          Member("b", ty.vec3<f32>(), {MemberOffset(16)}),
      },
      ast::StructDecorationList{});

  auto* data = ty.struct_("Data", data_str);
  type::Array ary(data, 4,
                  ast::ArrayDecorationList{
                      create<ast::StrideDecoration>(32),
                  });

  auto* pre_str = create<ast::Struct>(
      ast::StructMemberList{Member("c", &ary, {MemberOffset(0)})},
      ast::StructDecorationList{});

  auto* pre_struct = ty.struct_("Pre", pre_str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, pre_struct);
  mod->AST().AddGlobalVariable(coord_var);
  td.RegisterVariableForTesting(coord_var);

  auto* expr =
      MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), Expr(2)), "b");

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(expr));

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitExpression(pre, out, expr)) << gen.error();
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
  // var<storage> data : Pre;
  // data.c[2].b.xy
  //
  // -> asfloat(data.Load3(16 + (2 * 32))).xy

  auto* data_str = create<ast::Struct>(
      ast::StructMemberList{
          Member("a", ty.vec3<i32>(), {MemberOffset(0)}),
          Member("b", ty.vec3<f32>(), {MemberOffset(16)}),
      },
      ast::StructDecorationList{});

  auto* data = ty.struct_("Data", data_str);
  type::Array ary(data, 4,
                  ast::ArrayDecorationList{create<ast::StrideDecoration>(32)});

  auto* pre_str = create<ast::Struct>(
      ast::StructMemberList{Member("c", &ary, {MemberOffset(0)})},
      ast::StructDecorationList{});

  auto* pre_struct = ty.struct_("Pre", pre_str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, pre_struct);
  mod->AST().AddGlobalVariable(coord_var);

  td.RegisterVariableForTesting(coord_var);

  auto* expr = MemberAccessor(
      MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), Expr(2)), "b"),
      "xy");

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(expr));

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitExpression(pre, out, expr)) << gen.error();
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
  // var<storage> data : Pre;
  // data.c[2].b.g
  //
  // -> asfloat(data.Load((4 * 1) + 16 + (2 * 32) + 0))

  auto* data_str = create<ast::Struct>(
      ast::StructMemberList{
          Member("a", ty.vec3<i32>(), {MemberOffset(0)}),
          Member("b", ty.vec3<f32>(), {MemberOffset(16)}),
      },
      ast::StructDecorationList{});

  auto* data = ty.struct_("Data", data_str);
  type::Array ary(data, 4,
                  ast::ArrayDecorationList{
                      create<ast::StrideDecoration>(32),
                  });

  auto* pre_str = create<ast::Struct>(
      ast::StructMemberList{Member("c", &ary, {MemberOffset(0)})},
      ast::StructDecorationList{});

  auto* pre_struct = ty.struct_("Pre", pre_str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, pre_struct);
  mod->AST().AddGlobalVariable(coord_var);
  td.RegisterVariableForTesting(coord_var);

  auto* expr = MemberAccessor(
      MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), Expr(2)), "b"),
      "g");

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(expr));

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitExpression(pre, out, expr)) << gen.error();
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
  // var<storage> data : Pre;
  // data.c[2].b[1]
  //
  // -> asfloat(data.Load(4 + 16 + (2 * 32)))

  auto* data_str = create<ast::Struct>(
      ast::StructMemberList{
          Member("a", ty.vec3<i32>(), {MemberOffset(0)}),
          Member("b", ty.vec3<f32>(), {MemberOffset(16)}),
      },
      ast::StructDecorationList{});

  auto* data = ty.struct_("Data", data_str);
  type::Array ary(data, 4,
                  ast::ArrayDecorationList{
                      create<ast::StrideDecoration>(32),
                  });

  auto* pre_str = create<ast::Struct>(
      ast::StructMemberList{Member("c", &ary, {MemberOffset(0)})},
      ast::StructDecorationList{});

  auto* pre_struct = ty.struct_("Pre", pre_str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, pre_struct);
  mod->AST().AddGlobalVariable(coord_var);
  td.RegisterVariableForTesting(coord_var);

  auto* expr = IndexAccessor(
      MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), Expr(2)), "b"),
      Expr(1));

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(expr));

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitExpression(pre, out, expr)) << gen.error();
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
  // var<storage> data : Pre;
  // data.c[2].b = vec3<f32>(1.f, 2.f, 3.f);
  //
  // -> data.Store3(16 + (2 * 32), asuint(float3(1.0f, 2.0f, 3.0f)));

  auto* data_str = create<ast::Struct>(
      ast::StructMemberList{
          Member("a", ty.vec3<i32>(), {MemberOffset(0)}),
          Member("b", ty.vec3<f32>(), {MemberOffset(16)}),
      },
      ast::StructDecorationList{});

  auto* data = ty.struct_("Data", data_str);
  type::Array ary(data, 4,
                  ast::ArrayDecorationList{
                      create<ast::StrideDecoration>(32),
                  });

  auto* pre_str = create<ast::Struct>(
      ast::StructMemberList{Member("c", &ary, {MemberOffset(0)})},
      ast::StructDecorationList{});

  auto* pre_struct = ty.struct_("Pre", pre_str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, pre_struct);
  mod->AST().AddGlobalVariable(coord_var);
  td.RegisterVariableForTesting(coord_var);

  auto* lhs =
      MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), Expr(2)), "b");

  auto* assign =
      create<ast::AssignmentStatement>(lhs, vec3<f32>(1.f, 2.f, 3.f));

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(assign));

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitStatement(out, assign)) << gen.error();
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
  // var<storage> data : Pre;
  // data.c[2].b.y = 1.f;
  //
  // -> data.Store((4 * 1) + 16 + (2 * 32) + 0, asuint(1.0f));

  auto* data_str = create<ast::Struct>(
      ast::StructMemberList{
          Member("a", ty.vec3<i32>(), {MemberOffset(0)}),
          Member("b", ty.vec3<f32>(), {MemberOffset(16)}),
      },
      ast::StructDecorationList{});

  auto* data = ty.struct_("Data", data_str);
  type::Array ary(data, 4,
                  ast::ArrayDecorationList{
                      create<ast::StrideDecoration>(32),
                  });

  auto* pre_str = create<ast::Struct>(
      ast::StructMemberList{Member("c", &ary, {MemberOffset(0)})},
      ast::StructDecorationList{});

  auto* pre_struct = ty.struct_("Pre", pre_str);
  auto* coord_var = Var("data", ast::StorageClass::kStorage, pre_struct);
  mod->AST().AddGlobalVariable(coord_var);
  td.RegisterVariableForTesting(coord_var);

  auto* lhs = MemberAccessor(
      MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), Expr(2)), "b"),
      "y");
  auto* rhs = Expr(1.f);

  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(assign));

  GeneratorImpl& gen = Build();

  gen.register_global(coord_var);

  ASSERT_TRUE(gen.EmitStatement(out, assign)) << gen.error();
  EXPECT_EQ(result(),
            R"(data.Store((4 * 1) + 16 + (32 * 2) + 0, asuint(1.0f));
)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
