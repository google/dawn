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
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Length_Float_Vec2) {
  auto* builtin = Call("length", vec2<float>(1.0f, 1.0f));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Length_Float_Vec3) {
  auto* builtin = Call("length", vec3<float>(1.0f, 1.0f, 1.0f));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Length_Float_Vec4) {
  auto* builtin = Call("length", vec4<float>(1.0f, 1.0f, 1.0f, 1.0f));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Distance_Float_Scalar) {
  auto* builtin = Call("distance", 1.0f, 1.0f);
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Distance_Float_Vec2) {
  auto* builtin =
      Call("distance", vec2<float>(1.0f, 1.0f), vec2<float>(1.0f, 1.0f));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Distance_Float_Vec3) {
  auto* builtin = Call("distance", vec3<float>(1.0f, 1.0f, 1.0f),
                       vec3<float>(1.0f, 1.0f, 1.0f));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Distance_Float_Vec4) {
  auto* builtin = Call("distance", vec4<float>(1.0f, 1.0f, 1.0f, 1.0f),
                       vec4<float>(1.0f, 1.0f, 1.0f, 1.0f));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Determinant_Mat2x2) {
  auto* builtin = Call("determinant", mat2x2<float>(vec2<float>(1.0f, 1.0f),
                                                    vec2<float>(1.0f, 1.0f)));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Determinant_Mat3x3) {
  auto* builtin =
      Call("determinant", mat3x3<float>(vec3<float>(1.0f, 1.0f, 1.0f),
                                        vec3<float>(1.0f, 1.0f, 1.0f),
                                        vec3<float>(1.0f, 1.0f, 1.0f)));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Determinant_Mat4x4) {
  auto* builtin =
      Call("determinant", mat4x4<float>(vec4<float>(1.0f, 1.0f, 1.0f, 1.0f),
                                        vec4<float>(1.0f, 1.0f, 1.0f, 1.0f),
                                        vec4<float>(1.0f, 1.0f, 1.0f, 1.0f),
                                        vec4<float>(1.0f, 1.0f, 1.0f, 1.0f)));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Frexp_Scalar) {
  auto* a = Var("a", ty.i32(), ast::StorageClass::kWorkgroup);
  RegisterVariable(a);
  auto* builtin = Call("frexp", 1.0f, Expr("a"));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->Is<type::F32>());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Frexp_Vec2) {
  auto* a = Var("a", ty.vec2<int>(), ast::StorageClass::kWorkgroup);
  auto* b = Const("b",
                  create<type::Pointer>(create<type::Vector>(ty.i32(), 2),
                                        ast::StorageClass::kWorkgroup),
                  ast::StorageClass::kWorkgroup, Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("frexp", vec2<float>(1.0f, 1.0f), Expr("b"));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Frexp_Vec3) {
  auto* a = Var("a", ty.vec3<int>(), ast::StorageClass::kWorkgroup);
  auto* b = Const("b",
                  create<type::Pointer>(create<type::Vector>(ty.i32(), 3),
                                        ast::StorageClass::kWorkgroup),
                  ast::StorageClass::kWorkgroup, Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("frexp", vec3<float>(1.0f, 1.0f, 1.0f), Expr("b"));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Frexp_Vec4) {
  auto* a = Var("a", ty.vec4<int>(), ast::StorageClass::kWorkgroup);
  auto* b = Const("b",
                  create<type::Pointer>(create<type::Vector>(ty.i32(), 4),
                                        ast::StorageClass::kWorkgroup),
                  ast::StorageClass::kWorkgroup, Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("frexp", vec4<float>(1.0f, 1.0f, 1.0f, 1.0f), Expr("b"));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Modf_Scalar) {
  auto* a = Var("a", ty.f32(), ast::StorageClass::kWorkgroup);
  auto* b = Const("b", ty.pointer<float>(ast::StorageClass::kWorkgroup),
                  ast::StorageClass::kWorkgroup, Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("modf", 1.0f, Expr("b"));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->Is<type::F32>());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Modf_Vec2) {
  auto* a = Var("a", ty.vec2<float>(), ast::StorageClass::kWorkgroup);
  auto* b = Const("b",
                  create<type::Pointer>(create<type::Vector>(ty.f32(), 2),
                                        ast::StorageClass::kWorkgroup),
                  ast::StorageClass::kWorkgroup, Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("modf", vec2<float>(1.0f, 1.0f), Expr("b"));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Modf_Vec3) {
  auto* a = Var("a", ty.vec3<float>(), ast::StorageClass::kWorkgroup);
  auto* b = Const("b",
                  create<type::Pointer>(create<type::Vector>(ty.f32(), 3),
                                        ast::StorageClass::kWorkgroup),
                  ast::StorageClass::kWorkgroup, Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("modf", vec3<float>(1.0f, 1.0f, 1.0f), Expr("b"));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Modf_Vec4) {
  auto* a = Var("a", ty.vec4<float>(), ast::StorageClass::kWorkgroup);
  auto* b = Const("b",
                  create<type::Pointer>(create<type::Vector>(ty.f32(), 4),
                                        ast::StorageClass::kWorkgroup),
                  ast::StorageClass::kWorkgroup, Expr("a"), {});
  RegisterVariable(a);
  RegisterVariable(b);
  auto* builtin = Call("modf", vec4<float>(1.0f, 1.0f, 1.0f, 1.0f), Expr("b"));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<type::Pointer>());
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Cross_Float_Vec3) {
  auto* builtin = Call("cross", vec3<float>(1.0f, 1.0f, 1.0f),
                       vec3<float>(1.0f, 1.0f, 1.0f));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Dot_Float_Vec2) {
  auto* builtin = Call("dot", vec2<float>(1.0f, 1.0f), vec2<float>(1.0f, 1.0f));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Dot_Float_Vec3) {
  auto* builtin =
      Call("dot", vec3<float>(1.0f, 1.0f, 1.0f), vec3<float>(1.0f, 1.0f, 1.0f));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Dot_Float_Vec4) {
  auto* builtin = Call("dot", vec4<float>(1.0f, 1.0f, 1.0f, 1.0f),
                       vec4<float>(1.0f, 1.0f, 1.0f, 1.0f));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Select_Float_Scalar) {
  auto* builtin = Call("select", Expr(1.0f), Expr(1.0f), Expr(true));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Select_Integer_Scalar) {
  auto* builtin = Call("select", Expr(1), Expr(1), Expr(true));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Select_Boolean_Scalar) {
  auto* builtin = Call("select", Expr(true), Expr(true), Expr(true));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Select_Float_Vec2) {
  auto* builtin = Call("select", vec2<float>(1.0f, 1.0f),
                       vec2<float>(1.0f, 1.0f), vec2<bool>(true, true));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Select_Integer_Vec2) {
  auto* builtin =
      Call("select", vec2<int>(1, 1), vec2<int>(1, 1), vec2<bool>(true, true));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

TEST_F(ValidatorBuiltinsTest, Select_Boolean_Vec2) {
  auto* builtin = Call("select", vec2<bool>(true, true), vec2<bool>(true, true),
                       vec2<bool>(true, true));
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
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
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->Is<type::F32>());
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
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
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
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
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
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
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
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->Is<type::U32>());
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
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->is_unsigned_integer_vector());
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
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->is_unsigned_integer_vector());
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
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->is_unsigned_integer_vector());
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
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->Is<type::I32>());
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
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->is_signed_integer_vector());
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
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->is_signed_integer_vector());
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
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(TypeOf(builtin)->is_signed_integer_vector());
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
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

TEST_P(BooleanVectorInput, Vec2) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec2<bool>(true, true));
  }
  auto* builtin = Call(name, params);
  WrapInFunction(builtin);

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
  WrapInFunction(builtin);

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
  WrapInFunction(builtin);

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

INSTANTIATE_TEST_SUITE_P(ValidatorBuiltinsTest,
                         BooleanVectorInput,
                         ::testing::Values(std::make_tuple("all", 1),
                                           std::make_tuple("any", 1)));

using DataPacking4x8 = ValidatorBuiltinsTestWithParams<std::string>;

TEST_P(DataPacking4x8, Float_Vec4) {
  auto name = GetParam();
  auto* builtin = Call(name, vec4<float>(1.0f, 1.0f, 1.0f, 1.0f));
  WrapInFunction(builtin);
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

INSTANTIATE_TEST_SUITE_P(ValidatorBuiltinsTest,
                         DataPacking4x8,
                         ::testing::Values("pack4x8snorm", "pack4x8unorm"));

using DataPacking2x16 = ValidatorBuiltinsTestWithParams<std::string>;

TEST_P(DataPacking2x16, Float_Vec2) {
  auto name = GetParam();
  auto* builtin = Call(name, vec2<float>(1.0f, 1.0f));
  WrapInFunction(builtin);
  ValidatorImpl& v = Build();
  EXPECT_TRUE(v.ValidateCallExpr(builtin)) << v.error();
}

INSTANTIATE_TEST_SUITE_P(ValidatorBuiltinsTest,
                         DataPacking2x16,
                         ::testing::Values("pack2x16snorm",
                                           "pack2x16unorm",
                                           "pack2x16float"));

}  // namespace tint
