// Copyright 2021 The Tint Authors.
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
#include "src/ast/struct.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/type/alias_type.h"
#include "src/type/array_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/struct_type.h"
#include "src/validator/validator_impl.h"
#include "src/validator/validator_test_helper.h"

namespace tint {

class ValidatorBuiltinsTest : public ValidatorTestHelper,
                              public testing::Test {};

TEST_F(ValidatorBuiltinsTest, Length_Float_Scalar) {
  auto* builtin = Call("length", 1.0f);

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Length_Float_Vec2) {
  auto* builtin = Call("length", vec2<float>(1.0f, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Length_Float_Vec3) {
  auto* builtin = Call("length", vec3<float>(1.0f, 1.0f, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Length_Float_Vec4) {
  auto* builtin = Call("length", vec4<float>(1.0f, 1.0f, 1.0f, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Length_Integer_Scalar) {
  auto* builtin = Call("length", 1);

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect type for length. Requires float scalar or vector value");
}

TEST_F(ValidatorBuiltinsTest, Length_Integer_Vec2) {
  auto* builtin = Call("length", vec2<uint32_t>(1, 1));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect type for length. Requires float scalar or vector value");
}

TEST_F(ValidatorBuiltinsTest, Length_TooManyParams) {
  auto* builtin = Call("length", 1.0f, 1.0f);

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect number of parameters for length expected 1 got 2");
}

TEST_F(ValidatorBuiltinsTest, Length_TooFewParams) {
  auto* builtin = Call("length");

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect number of parameters for length expected 1 got 0");
}

TEST_F(ValidatorBuiltinsTest, Distance_Float_Scalar) {
  auto* builtin = Call("distance", 1.0f, 1.0f);

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Distance_Float_Vec2) {
  auto* builtin =
      Call("distance", vec2<float>(1.0f, 1.0f), vec2<float>(1.0f, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Distance_Float_Vec3) {
  auto* builtin = Call("distance", vec3<float>(1.0f, 1.0f, 1.0f),
                       vec3<float>(1.0f, 1.0f, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Distance_Float_Vec4) {
  auto* builtin = Call("distance", vec4<float>(1.0f, 1.0f, 1.0f, 1.0f),
                       vec4<float>(1.0f, 1.0f, 1.0f, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Distance_Integer_Scalar) {
  auto* builtin = Call("distance", 1, 1);

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(
      v.error(),
      "incorrect type for distance. Requires float scalar or vector value");
}

TEST_F(ValidatorBuiltinsTest, Distance_Integer_Vec2) {
  auto* builtin = Call("distance", vec2<uint32_t>(1, 1), vec2<uint32_t>(1, 1));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(
      v.error(),
      "incorrect type for distance. Requires float scalar or vector value");
}

TEST_F(ValidatorBuiltinsTest, Distance_TooManyParams) {
  auto* builtin = Call("distance", 1.0f, 1.0f, 1.0f);

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect number of parameters for distance expected 2 got 3");
}

TEST_F(ValidatorBuiltinsTest, Distance_TooFewParams) {
  auto* builtin = Call("distance", 1.0f);

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect number of parameters for distance expected 2 got 1");
}

TEST_F(ValidatorBuiltinsTest, Determinant_Mat2x2) {
  auto* builtin = Call("determinant", mat2x2<float>(vec2<float>(1.0f, 1.0f),
                                                    vec2<float>(1.0f, 1.0f)));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Determinant_Mat3x3) {
  auto* builtin =
      Call("determinant", mat3x3<float>(vec3<float>(1.0f, 1.0f, 1.0f),
                                        vec3<float>(1.0f, 1.0f, 1.0f),
                                        vec3<float>(1.0f, 1.0f, 1.0f)));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Determinant_Mat4x4) {
  auto* builtin =
      Call("determinant", mat4x4<float>(vec4<float>(1.0f, 1.0f, 1.0f, 1.0f),
                                        vec4<float>(1.0f, 1.0f, 1.0f, 1.0f),
                                        vec4<float>(1.0f, 1.0f, 1.0f, 1.0f),
                                        vec4<float>(1.0f, 1.0f, 1.0f, 1.0f)));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Determinant_Mat3x2) {
  auto* builtin = Call("determinant", mat3x2<float>(vec2<float>(1.0f, 1.0f),
                                                    vec2<float>(1.0f, 1.0f),
                                                    vec2<float>(1.0f, 1.0f)));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect type for determinant. Requires a square matrix");
}

TEST_F(ValidatorBuiltinsTest, Determinant_Float_Vec2) {
  auto* builtin = Call("determinant", vec2<float>(1.0f, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(), "incorrect type for determinant. Requires matrix value");
}

TEST_F(ValidatorBuiltinsTest, Determinant_Float_Scalar) {
  auto* builtin = Call("determinant", 1.0f);

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(), "incorrect type for determinant. Requires matrix value");
}

TEST_F(ValidatorBuiltinsTest, Determinant_Integer_Scalar) {
  auto* builtin = Call("determinant", 1);

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(), "incorrect type for determinant. Requires matrix value");
}

TEST_F(ValidatorBuiltinsTest, Determinant_TooManyParams) {
  auto* builtin =
      Call("determinant",
           mat2x2<float>(vec2<float>(1.0f, 1.0f), vec2<float>(1.0f, 1.0f)),
           mat2x2<float>(vec2<float>(1.0f, 1.0f), vec2<float>(1.0f, 1.0f)));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect number of parameters for determinant expected 1 got 2");
}

TEST_F(ValidatorBuiltinsTest, Determinant_TooFewParams) {
  auto* builtin = Call("determinant");

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect number of parameters for determinant expected 1 got 0");
}

TEST_F(ValidatorBuiltinsTest, Frexp_Scalar) {
  auto* a = Var("a", ast::StorageClass::kWorkgroup, ty.i32());
  auto* b =
      Const("b", ast::StorageClass::kWorkgroup,
            ty.pointer<int>(ast::StorageClass::kWorkgroup), Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("frexp", 1.0f, Expr("b"));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->Is<type::F32>());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Frexp_Vec2) {
  auto* a = Var("a", ast::StorageClass::kWorkgroup, ty.vec2<int>());
  auto* b = Const("b", ast::StorageClass::kWorkgroup,
                  create<type::Pointer>(create<type::Vector>(ty.i32(), 2),
                                        ast::StorageClass::kWorkgroup),
                  Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("frexp", vec2<float>(1.0f, 1.0f), Expr("b"));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Frexp_Vec3) {
  auto* a = Var("a", ast::StorageClass::kWorkgroup, ty.vec3<int>());
  auto* b = Const("b", ast::StorageClass::kWorkgroup,
                  create<type::Pointer>(create<type::Vector>(ty.i32(), 3),
                                        ast::StorageClass::kWorkgroup),
                  Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("frexp", vec3<float>(1.0f, 1.0f, 1.0f), Expr("b"));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Frexp_Vec4) {
  auto* a = Var("a", ast::StorageClass::kWorkgroup, ty.vec4<int>());
  auto* b = Const("b", ast::StorageClass::kWorkgroup,
                  create<type::Pointer>(create<type::Vector>(ty.i32(), 4),
                                        ast::StorageClass::kWorkgroup),
                  Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("frexp", vec4<float>(1.0f, 1.0f, 1.0f, 1.0f), Expr("b"));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Frexp_Integer_FirstParam) {
  auto* a = Var("a", ast::StorageClass::kWorkgroup, ty.i32());
  auto* b =
      Const("b", ast::StorageClass::kWorkgroup,
            ty.pointer<int>(ast::StorageClass::kWorkgroup), Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("frexp", 1, Expr("b"));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_FALSE(TypeOf(builtin)->Is<type::F32>());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect type for frexp. Requires float scalar or vector value");
}

TEST_F(ValidatorBuiltinsTest, Frexp_Float_SecondParam) {
  auto* a = Var("a", ast::StorageClass::kWorkgroup, ty.f32());
  auto* b =
      Const("b", ast::StorageClass::kWorkgroup,
            ty.pointer<float>(ast::StorageClass::kWorkgroup), Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("frexp", 1.0f, Expr("b"));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->Is<type::F32>());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect type for frexp. Requires int scalar or vector value");
}

TEST_F(ValidatorBuiltinsTest, Frexp_NotAPointer) {
  auto* builtin = Call("frexp", 1.0f, 1);

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->Is<type::F32>());
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(), "incorrect type for frexp. Requires pointer value");
}

TEST_F(ValidatorBuiltinsTest, Frexp_Scalar_Vector) {
  auto* a = Var("a", ast::StorageClass::kWorkgroup, ty.vec2<int>());
  auto* b = Const("b", ast::StorageClass::kWorkgroup,
                  create<type::Pointer>(create<type::Vector>(ty.i32(), 2),
                                        ast::StorageClass::kWorkgroup),
                  Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("frexp", 1.0f, Expr("b"));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect types for frexp. Parameters must be matched scalars or "
            "vectors");
}

TEST_F(ValidatorBuiltinsTest, Modf_Scalar) {
  auto* a = Var("a", ast::StorageClass::kWorkgroup, ty.f32());
  auto* b =
      Const("b", ast::StorageClass::kWorkgroup,
            ty.pointer<float>(ast::StorageClass::kWorkgroup), Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("modf", 1.0f, Expr("b"));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->Is<type::F32>());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Modf_Vec2) {
  auto* a = Var("a", ast::StorageClass::kWorkgroup, ty.vec2<float>());
  auto* b = Const("b", ast::StorageClass::kWorkgroup,
                  create<type::Pointer>(create<type::Vector>(ty.f32(), 2),
                                        ast::StorageClass::kWorkgroup),
                  Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("modf", vec2<float>(1.0f, 1.0f), Expr("b"));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Modf_Vec3) {
  auto* a = Var("a", ast::StorageClass::kWorkgroup, ty.vec3<float>());
  auto* b = Const("b", ast::StorageClass::kWorkgroup,
                  create<type::Pointer>(create<type::Vector>(ty.f32(), 3),
                                        ast::StorageClass::kWorkgroup),
                  Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("modf", vec3<float>(1.0f, 1.0f, 1.0f), Expr("b"));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Modf_Vec4) {
  auto* a = Var("a", ast::StorageClass::kWorkgroup, ty.vec4<float>());
  auto* b = Const("b", ast::StorageClass::kWorkgroup,
                  create<type::Pointer>(create<type::Vector>(ty.f32(), 4),
                                        ast::StorageClass::kWorkgroup),
                  Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("modf", vec4<float>(1.0f, 1.0f, 1.0f, 1.0f), Expr("b"));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Modf_Integer_FirstParam) {
  auto* a = Var("a", ast::StorageClass::kWorkgroup, ty.f32());
  auto* b =
      Const("b", ast::StorageClass::kWorkgroup,
            ty.pointer<float>(ast::StorageClass::kWorkgroup), Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("modf", 1, Expr("b"));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_FALSE(TypeOf(builtin)->Is<type::F32>());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect type for modf. Requires float scalar or vector value");
}

TEST_F(ValidatorBuiltinsTest, Modf_Integer_SecondParam) {
  auto* a = Var("a", ast::StorageClass::kWorkgroup, ty.i32());
  auto* b =
      Const("b", ast::StorageClass::kWorkgroup,
            ty.pointer<int>(ast::StorageClass::kWorkgroup), Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("modf", 1.0f, Expr("b"));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->Is<type::F32>());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(
      v.error(),
      "expected parameter 1's unwrapped type to match result type for modf");
}

TEST_F(ValidatorBuiltinsTest, Modf_NotAPointer) {
  auto* builtin = Call("modf", 1.0f, 1.0f);

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->Is<type::F32>());
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(), "incorrect type for modf. Requires pointer value");
}

TEST_F(ValidatorBuiltinsTest, Modf_Scalar_Vector) {
  auto* a = Var("a", ast::StorageClass::kWorkgroup, ty.vec2<float>());
  auto* b = Const("b", ast::StorageClass::kWorkgroup,
                  create<type::Pointer>(create<type::Vector>(ty.f32(), 2),
                                        ast::StorageClass::kWorkgroup),
                  Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("modf", 1.0f, Expr("b"));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(
      v.error(),
      "expected parameter 1's unwrapped type to match result type for modf");
}

TEST_F(ValidatorBuiltinsTest, Modf_Vector_Scalar) {
  auto* a = Var("a", ast::StorageClass::kWorkgroup, ty.f32());
  auto* b =
      Const("b", ast::StorageClass::kWorkgroup,
            ty.pointer<int>(ast::StorageClass::kWorkgroup), Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("modf", vec2<float>(1.0f, 1.0f), Expr("b"));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(
      v.error(),
      "expected parameter 1's unwrapped type to match result type for modf");
}

TEST_F(ValidatorBuiltinsTest, Modf_Vector_Vector_MismatchedSize) {
  auto* a = Var("a", ast::StorageClass::kWorkgroup, ty.vec2<float>());
  auto* b = Const("b", ast::StorageClass::kWorkgroup,
                  create<type::Pointer>(create<type::Vector>(ty.f32(), 2),
                                        ast::StorageClass::kWorkgroup),
                  Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("modf", vec3<float>(1.0f, 1.0f, 1.0f), Expr("b"));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(
      v.error(),
      "expected parameter 1's unwrapped type to match result type for modf");
}

TEST_F(ValidatorBuiltinsTest, Cross_Float_Vec3) {
  auto* builtin = Call("cross", vec3<float>(1.0f, 1.0f, 1.0f),
                       vec3<float>(1.0f, 1.0f, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Cross_Integer_Vec3) {
  auto* builtin = Call("cross", vec3<int>(1, 1, 1), vec3<int>(1, 1, 1));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(
      v.error(),
      "expected parameter 0's unwrapped type to match result type for cross");
}

TEST_F(ValidatorBuiltinsTest, Cross_Float_Vec4) {
  auto* builtin = Call("cross", vec4<float>(1.0f, 1.0f, 1.0f, 1.0f),
                       vec4<float>(1.0f, 1.0f, 1.0f, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(
      v.error(),
      "expected parameter 0's unwrapped type to match result type for cross");
}

TEST_F(ValidatorBuiltinsTest, Cross_Float_Vec2) {
  auto* builtin =
      Call("cross", vec2<float>(1.0f, 1.0f), vec2<float>(1.0f, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(
      v.error(),
      "expected parameter 0's unwrapped type to match result type for cross");
}

TEST_F(ValidatorBuiltinsTest, Cross_Float_Scalar) {
  auto* builtin = Call("cross", 1.0f, 1.0f);

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(
      v.error(),
      "expected parameter 0's unwrapped type to match result type for cross");
}

TEST_F(ValidatorBuiltinsTest, Cross_TooManyParams) {
  auto* builtin =
      Call("cross", vec3<float>(1.0f, 1.0f, 1.0f),
           vec3<float>(1.0f, 1.0f, 1.0f), vec3<float>(1.0f, 1.0f, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect number of parameters for cross expected 2 got 3");
}

TEST_F(ValidatorBuiltinsTest, Cross_TooFewParams) {
  auto* builtin = Call("cross", vec3<float>(1.0f, 1.0f, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect number of parameters for cross expected 2 got 1");
}

TEST_F(ValidatorBuiltinsTest, Dot_Float_Scalar) {
  auto* builtin = Call("dot", Expr(1.0f), Expr(1.0f));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(), "incorrect type for dot. Requires float vector value");
}

TEST_F(ValidatorBuiltinsTest, Dot_Integer_Scalar) {
  auto* builtin = Call("dot", Expr(1), Expr(1));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(), "incorrect type for dot. Requires float vector value");
}

TEST_F(ValidatorBuiltinsTest, Dot_Float_Vec2) {
  auto* builtin = Call("dot", vec2<float>(1.0f, 1.0f), vec2<float>(1.0f, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Dot_Float_Vec3) {
  auto* builtin =
      Call("dot", vec3<float>(1.0f, 1.0f, 1.0f), vec3<float>(1.0f, 1.0f, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Dot_Float_Vec4) {
  auto* builtin = Call("dot", vec4<float>(1.0f, 1.0f, 1.0f, 1.0f),
                       vec4<float>(1.0f, 1.0f, 1.0f, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Dot_Integer_Vector) {
  auto* builtin = Call("dot", vec2<int>(1, 1), vec2<int>(1, 1));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(), "incorrect type for dot. Requires float vector value");
}

TEST_F(ValidatorBuiltinsTest, Dot_TooFewParams) {
  auto* builtin = Call("dot", vec2<float>(1.0f, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect number of parameters for dot expected 2 got 1");
}

TEST_F(ValidatorBuiltinsTest, Dot_TooManyParams) {
  auto* builtin = Call("dot", vec2<float>(1.0f, 1.0f), vec2<float>(1.0f, 1.0f),
                       vec2<float>(1.0f, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect number of parameters for dot expected 2 got 3");
}

TEST_F(ValidatorBuiltinsTest, Select_Float_Scalar) {
  auto* builtin = Call("select", Expr(1.0f), Expr(1.0f), Expr(true));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Select_Integer_Scalar) {
  auto* builtin = Call("select", Expr(1), Expr(1), Expr(true));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Select_Boolean_Scalar) {
  auto* builtin = Call("select", Expr(true), Expr(true), Expr(true));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Select_Float_Vec2) {
  auto* builtin = Call("select", vec2<float>(1.0f, 1.0f),
                       vec2<float>(1.0f, 1.0f), vec2<bool>(true, true));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Select_Integer_Vec2) {
  auto* builtin =
      Call("select", vec2<int>(1, 1), vec2<int>(1, 1), vec2<bool>(true, true));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Select_Boolean_Vec2) {
  auto* builtin = Call("select", vec2<bool>(true, true), vec2<bool>(true, true),
                       vec2<bool>(true, true));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Select_BadSelector) {
  auto* builtin = Call("select", Expr(1), Expr(1), Expr(1));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect type for select. Selector must be a bool scalar or "
            "vector value");
}

TEST_F(ValidatorBuiltinsTest, Select_Matrix) {
  auto* mat = mat2x2<float>(vec2<float>(1.0f, 1.0f), vec2<float>(1.0f, 1.0f));
  auto* builtin = Call("select", mat, mat, Expr(true));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect type for select. Requires bool, int or float scalar or "
            "vector");
}

TEST_F(ValidatorBuiltinsTest, Select_Mismatch_Scalar) {
  auto* builtin = Call("select", Expr(1.0f), Expr(1), Expr(true));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect type for select. Value parameter types must match "
            "result type");
}

TEST_F(ValidatorBuiltinsTest, Select_Mismatched_Vector) {
  auto* builtin =
      Call("select", Expr(1.0f), vec2<float>(1.0f, 1.0f), Expr(true));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect type for select. Value parameter types must match "
            "result type");
}

TEST_F(ValidatorBuiltinsTest, Select_Mismatched_VectorSize) {
  auto* builtin = Call("select", vec3<float>(1.0f, 1.0f, 1.0f),
                       vec2<float>(1.0f, 1.0f), vec3<bool>(true, true, true));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect type for select. Value parameter types must match "
            "result type");
}

TEST_F(ValidatorBuiltinsTest, Select_Mismatch_Selector_Vector) {
  auto* builtin =
      Call("select", Expr(1.0f), Expr(1.0f), vec2<bool>(true, true));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect type for select. Selector must be a bool scalar to "
            "match scalar result type");
}

TEST_F(ValidatorBuiltinsTest, Select_Mismatch_Selector_Scalar) {
  auto* builtin = Call("select", vec2<float>(1.0f, 1.0f),
                       vec2<float>(1.0f, 1.0f), Expr(true));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect type for select. Selector must be a vector with the "
            "same number of elements as the result type");
}

TEST_F(ValidatorBuiltinsTest, Select_Mismatch_Selector_VectorSize) {
  auto* builtin = Call("select", vec2<float>(1.0f, 1.0f),
                       vec2<float>(1.0f, 1.0f), vec3<bool>(true, true, true));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect type for select. Selector must be a vector with the "
            "same number of elements as the result type");
}

TEST_F(ValidatorBuiltinsTest, ArrayLength_Sized) {
  auto* var = Var("a", ast::StorageClass::kWorkgroup, ty.array<int, 4>());
  RegisterVariable(var);
  auto* builtin = Call("arrayLength", Expr("a"));

  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect type for arrayLength. Input must be a runtime array");
}

template <typename T>
class ValidatorBuiltinsTestWithParams : public ValidatorTestHelper,
                                        public testing::TestWithParam<T> {};

using FloatAllMatching =
    ValidatorBuiltinsTestWithParams<std::tuple<std::string, uint32_t>>;

TEST_P(FloatAllMatching, Scalar) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(Expr(1.0f));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->Is<type::F32>());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_P(FloatAllMatching, Vec2) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec2<float>(1.0f, 1.0f));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_P(FloatAllMatching, Vec3) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec3<float>(1.0f, 1.0f, 1.0f));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_P(FloatAllMatching, Vec4) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec4<float>(1.0f, 1.0f, 1.0f, 1.0f));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_P(FloatAllMatching, Param_TooManyParams) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params + 1; ++i) {
    params.push_back(Expr(1.0f));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->Is<type::F32>());
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(), "incorrect number of parameters for " + name +
                           " expected " + std::to_string(num_params) + " got " +
                           std::to_string(num_params + 1));
}

TEST_P(FloatAllMatching, Param_TooFewParams) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params - 1; ++i) {
    params.push_back(Expr(1.0f));
  }
  auto* builtin = Call(name, params);
  // Most intrinsics require a parameter to determine the type so expect type
  // determination to fail at zero parameters.
  if (num_params > 1) {
    EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
    EXPECT_TRUE(TypeOf(builtin)->Is<type::F32>());
    ValidatorImpl& v = Build();
    EXPECT_FALSE(v.ValidateCallExpr(builtin));
    EXPECT_EQ(v.error(), "incorrect number of parameters for " + name +
                             " expected " + std::to_string(num_params) +
                             " got " + std::to_string(num_params - 1));
  } else {
    EXPECT_FALSE(td()->DetermineResultType(builtin));
  }
}

TEST_P(FloatAllMatching, Param_Mismatch_Scalar) {
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params - 1; ++i) {
    params.push_back(Expr(1.0f));
  }
  // Can't mismatch single parameter types.
  if (num_params > 1) {
    std::string name = std::get<0>(GetParam());
    params.push_back(Expr(1));
    auto* builtin = Call(name, params);
    EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
    EXPECT_TRUE(TypeOf(builtin)->Is<type::F32>());
    ValidatorImpl& v = Build();
    EXPECT_FALSE(v.ValidateCallExpr(builtin));
    EXPECT_EQ(v.error(),
              "expected parameter " + std::to_string(num_params - 1) +
                  "'s unwrapped type to match result type for " + name);
  }
}

TEST_P(FloatAllMatching, Param_Mismatch_Vector) {
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params - 1; ++i) {
    params.push_back(Expr(1.0f));
  }
  // Can't mismatch single parameter types.
  if (num_params > 1) {
    std::string name = std::get<0>(GetParam());
    params.push_back(vec2<float>(1.0f, 1.0f));
    auto* builtin = Call(name, params);
    EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
    EXPECT_TRUE(TypeOf(builtin)->Is<type::F32>());
    ValidatorImpl& v = Build();
    EXPECT_FALSE(v.ValidateCallExpr(builtin));
    EXPECT_EQ(v.error(),
              "expected parameter " + std::to_string(num_params - 1) +
                  "'s unwrapped type to match result type for " + name);
  }
}

TEST_P(FloatAllMatching, Param_Integer) {
  std::string name = std::get<0>(GetParam());
  // abs, clamp, max and min also support integers.
  if (name != "abs" && name != "clamp" && name != "max" && name != "min") {
    uint32_t num_params = std::get<1>(GetParam());

    ast::ExpressionList params;
    for (uint32_t i = 0; i < num_params; ++i) {
      params.push_back(Expr(1));
    }
    auto* builtin = Call(name, params);
    EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
    EXPECT_FALSE(TypeOf(builtin)->Is<type::F32>());
    ValidatorImpl& v = Build();
    EXPECT_FALSE(v.ValidateCallExpr(builtin));
    EXPECT_EQ(v.error(), "incorrect type for " + name +
                             ". Requires float scalar or vector value");
  }
}

INSTANTIATE_TEST_SUITE_P(ValidatorBuiltinsTest,
                         FloatAllMatching,
                         ::testing::Values(std::make_tuple("abs", 1),
                                           std::make_tuple("acos", 1),
                                           std::make_tuple("asin", 1),
                                           std::make_tuple("atan", 1),
                                           std::make_tuple("atan2", 2),
                                           std::make_tuple("ceil", 1),
                                           std::make_tuple("clamp", 3),
                                           std::make_tuple("cos", 1),
                                           std::make_tuple("cosh", 1),
                                           std::make_tuple("dpdx", 1),
                                           std::make_tuple("dpdxCoarse", 1),
                                           std::make_tuple("dpdxFine", 1),
                                           std::make_tuple("dpdy", 1),
                                           std::make_tuple("dpdyCoarse", 1),
                                           std::make_tuple("dpdyFine", 1),
                                           std::make_tuple("exp", 1),
                                           std::make_tuple("exp2", 1),
                                           std::make_tuple("faceForward", 3),
                                           std::make_tuple("floor", 1),
                                           std::make_tuple("fma", 3),
                                           std::make_tuple("fract", 1),
                                           std::make_tuple("fwidth", 1),
                                           std::make_tuple("fwidthCoarse", 1),
                                           std::make_tuple("fwidthFine", 1),
                                           std::make_tuple("inverseSqrt", 1),
                                           std::make_tuple("ldexp", 2),
                                           std::make_tuple("log", 1),
                                           std::make_tuple("log2", 1),
                                           std::make_tuple("max", 2),
                                           std::make_tuple("min", 2),
                                           std::make_tuple("mix", 3),
                                           std::make_tuple("pow", 2),
                                           std::make_tuple("reflect", 2),
                                           std::make_tuple("round", 1),
                                           std::make_tuple("sign", 1),
                                           std::make_tuple("sin", 1),
                                           std::make_tuple("sinh", 1),
                                           std::make_tuple("smoothStep", 3),
                                           std::make_tuple("sqrt", 1),
                                           std::make_tuple("step", 2),
                                           std::make_tuple("tan", 1),
                                           std::make_tuple("tanh", 1),
                                           std::make_tuple("trunc", 1)));

using IntegerAllMatching =
    ValidatorBuiltinsTestWithParams<std::tuple<std::string, uint32_t>>;

TEST_P(IntegerAllMatching, ScalarUnsigned) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(Construct<uint32_t>(1));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->Is<type::U32>());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_P(IntegerAllMatching, Vec2Unsigned) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec2<uint32_t>(1, 1));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_unsigned_integer_vector());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_P(IntegerAllMatching, Vec3Unsigned) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec3<uint32_t>(1, 1, 1));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_unsigned_integer_vector());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_P(IntegerAllMatching, Vec4Unsigned) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec4<uint32_t>(1, 1, 1, 1));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_unsigned_integer_vector());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_P(IntegerAllMatching, ScalarSigned) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(Construct<int32_t>(1));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->Is<type::I32>());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_P(IntegerAllMatching, Vec2Signed) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec2<int32_t>(1, 1));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_signed_integer_vector());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_P(IntegerAllMatching, Vec3Signed) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec3<int32_t>(1, 1, 1));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_signed_integer_vector());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_P(IntegerAllMatching, Vec4Signed) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec4<int32_t>(1, 1, 1, 1));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_signed_integer_vector());
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_P(IntegerAllMatching, Param_TooManyParams) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params + 1; ++i) {
    params.push_back(Construct<uint32_t>(1));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  EXPECT_TRUE(TypeOf(builtin)->Is<type::U32>());
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(), "incorrect number of parameters for " + name +
                           " expected " + std::to_string(num_params) + " got " +
                           std::to_string(num_params + 1));
}

TEST_P(IntegerAllMatching, Param_TooFewParams) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params - 1; ++i) {
    params.push_back(Construct<uint32_t>(1));
  }
  auto* builtin = Call(name, params);
  // Most intrinsics require a parameter to determine the type so expect type
  // determination to fail at zero parameters.
  if (num_params > 1) {
    EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
    EXPECT_TRUE(TypeOf(builtin)->Is<type::U32>());
    ValidatorImpl& v = Build();
    EXPECT_FALSE(v.ValidateCallExpr(builtin));
    EXPECT_EQ(v.error(), "incorrect number of parameters for " + name +
                             " expected " + std::to_string(num_params) +
                             " got " + std::to_string(num_params - 1));
  } else {
    EXPECT_FALSE(td()->DetermineResultType(builtin));
  }
}

TEST_P(IntegerAllMatching, Param_Mismatch_Scalar) {
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params - 1; ++i) {
    params.push_back(Construct<uint32_t>(1));
  }
  // Can't mismatch single parameter types.
  if (num_params > 1) {
    std::string name = std::get<0>(GetParam());
    params.push_back(Expr(1));
    auto* builtin = Call(name, params);
    EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
    EXPECT_TRUE(TypeOf(builtin)->Is<type::U32>());
    ValidatorImpl& v = Build();
    EXPECT_FALSE(v.ValidateCallExpr(builtin));
    EXPECT_EQ(v.error(),
              "expected parameter " + std::to_string(num_params - 1) +
                  "'s unwrapped type to match result type for " + name);
  }
}

TEST_P(IntegerAllMatching, Param_Mismatch_Vector) {
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params - 1; ++i) {
    params.push_back(Construct<uint32_t>(1));
  }
  // Can't mismatch single parameter types.
  if (num_params > 1) {
    std::string name = std::get<0>(GetParam());
    params.push_back(vec2<uint32_t>(1, 1));
    auto* builtin = Call(name, params);
    EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
    EXPECT_TRUE(TypeOf(builtin)->Is<type::U32>());
    ValidatorImpl& v = Build();
    EXPECT_FALSE(v.ValidateCallExpr(builtin));
    EXPECT_EQ(v.error(),
              "expected parameter " + std::to_string(num_params - 1) +
                  "'s unwrapped type to match result type for " + name);
  }
}

TEST_P(IntegerAllMatching, Param_Mismatch_Sign) {
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params - 1; ++i) {
    params.push_back(Construct<uint32_t>(1));
  }
  // Can't mismatch single parameter types.
  if (num_params > 1) {
    std::string name = std::get<0>(GetParam());
    params.push_back(Construct<int32_t>(1));
    auto* builtin = Call(name, params);
    EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
    EXPECT_TRUE(TypeOf(builtin)->Is<type::U32>());
    ValidatorImpl& v = Build();
    EXPECT_FALSE(v.ValidateCallExpr(builtin));
    EXPECT_EQ(v.error(),
              "expected parameter " + std::to_string(num_params - 1) +
                  "'s unwrapped type to match result type for " + name);
  }
}

TEST_P(IntegerAllMatching, Param_Float) {
  std::string name = std::get<0>(GetParam());
  // abs, clamp, max and min also support integers.
  if (name != "abs" && name != "clamp" && name != "max" && name != "min") {
    uint32_t num_params = std::get<1>(GetParam());

    ast::ExpressionList params;
    for (uint32_t i = 0; i < num_params; ++i) {
      params.push_back(Expr(1.0f));
    }
    auto* builtin = Call(name, params);
    EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
    EXPECT_TRUE(TypeOf(builtin)->Is<type::F32>());
    ValidatorImpl& v = Build();
    EXPECT_FALSE(v.ValidateCallExpr(builtin));
    EXPECT_EQ(v.error(), "incorrect type for " + name +
                             ". Requires int scalar or vector value");
  }
}

INSTANTIATE_TEST_SUITE_P(ValidatorBuiltinsTest,
                         IntegerAllMatching,
                         ::testing::Values(std::make_tuple("abs", 1),
                                           std::make_tuple("clamp", 3),
                                           std::make_tuple("countOneBits", 1),
                                           std::make_tuple("max", 2),
                                           std::make_tuple("min", 2),
                                           std::make_tuple("reverseBits", 1)));

using BooleanVectorInput =
    ValidatorBuiltinsTestWithParams<std::tuple<std::string, uint32_t>>;

TEST_P(BooleanVectorInput, Scalar) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(Expr(true));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect type for " + name + ". Requires bool vector value");
}

TEST_P(BooleanVectorInput, Vec2) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec2<bool>(true, true));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_P(BooleanVectorInput, Vec3) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec3<bool>(true, true, true));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_P(BooleanVectorInput, Vec4) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec4<bool>(true, true, true, true));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_P(BooleanVectorInput, NoParams) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  auto* builtin = Call(name);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(), "incorrect number of parameters for " + name +
                           " expected " + std::to_string(num_params) +
                           " got 0");
}

TEST_P(BooleanVectorInput, TooManyParams) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params + 1; ++i) {
    params.push_back(vec2<bool>(true, true));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(), "incorrect number of parameters for " + name +
                           " expected " + std::to_string(num_params) + " got " +
                           std::to_string(num_params + 1));
}

TEST_P(BooleanVectorInput, Integer) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec2<uint32_t>(1, 1));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect type for " + name + ". Requires bool vector value");
}

TEST_P(BooleanVectorInput, Float) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec2<float>(1.0f, 1.0f));
  }
  auto* builtin = Call(name, params);
  EXPECT_TRUE(td()->DetermineResultType(builtin)) << td()->error();
  ValidatorImpl& v = Build();
  EXPECT_FALSE(v.ValidateCallExpr(builtin));
  EXPECT_EQ(v.error(),
            "incorrect type for " + name + ". Requires bool vector value");
}

INSTANTIATE_TEST_SUITE_P(ValidatorBuiltinsTest,
                         BooleanVectorInput,
                         ::testing::Values(std::make_tuple("all", 1),
                                           std::make_tuple("any", 1)));

}  // namespace tint
